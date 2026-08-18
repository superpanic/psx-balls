#include "psyqo/application.hh"
#include "psyqo/cdrom-device.hh"
#include "psyqo/fixed-point.hh"
#include "psyqo/fragments.hh"
#include "psyqo/gpu.hh"
#include "psyqo/gte-kernels.hh"
#include "psyqo/gte-registers.hh"
#include "psyqo/primitives/common.hh"
#include "psyqo/primitives/quads.hh"
#include "psyqo/primitives/triangles.hh"
#include "psyqo/scene.hh"
#include "psyqo/soft-math.hh"
#include "psyqo/trigonometry.hh"
#include "psyqo/vector.hh"
#include "psyqo/xprintf.h"
#include "psyqo/buffer.hh"
#include "psyqo/iso9660-parser.hh"

#include <cstddef>
#include <cstdint>

#include "gbl_parser.hh"
#include "cd.hh"

using namespace psyqo::fixed_point_literals;
using namespace psyqo::trig_literals;

static constexpr unsigned NUM_CUBE_VERTICES = 8;
static constexpr unsigned NUM_CUBE_FACES = 6;
static constexpr unsigned ORDERING_TABLE_SIZE = 1024;

typedef struct {
	uint8_t vertices[4];
	psyqo::Color color;
} Face;

static constexpr psyqo::Matrix33 identity = {{
	{1.0_fp, 0.0_fp, 0.0_fp},
	{0.0_fp, 1.0_fp, 0.0_fp},
	{0.0_fp, 0.0_fp, 1.0_fp},
}};

class Cube final : public psyqo::Application {
	void prepare() override;
	void createScene() override;

	public:
		psyqo::Trig<> m_trig;

};

class CubeScene final : public psyqo::Scene {
	public:
		void start(StartReason reason) override;
		void frame() override;

		psyqo::Angle m_rot = 0;

		// We need to create 2 OrderingTable objects since we can't reuse a single one
		// for both framebuffers, as the previous one may not finish transfering in
		// time.
		psyqo::OrderingTable<ORDERING_TABLE_SIZE> m_ots[2];

		// Since we're using an ordering table, we need to sort fill commands as well,
		// otherwise they'll draw over our beautiful cube.
		psyqo::Fragments::SimpleFragment<psyqo::Prim::FastFill> m_clear[2];
		// define an array of triangles used to draw the object
		eastl::array<psyqo::Fragments::SimpleFragment<psyqo::Prim::Triangle>, 64> m_triangles;
		// background color for the clear command
		static constexpr psyqo::Color c_bg = {.r = 63, .g = 63, .b = 63};

		static constexpr psyqo::Vec3 c_cubeVertices[NUM_CUBE_VERTICES] = {
			{.x = -0.05, .y = -0.05, .z = -0.05}, {.x = 0.05, .y = -0.05, .z = -0.05}, {.x = -0.05, .y = 0.05, .z = -0.05}, {.x = 0.05, .y = 0.05, .z = -0.05}, {.x = -0.05, .y = -0.05, .z = 0.05}, {.x = 0.05, .y = -0.05, .z = 0.05}, {.x = -0.05, .y = 0.05, .z = 0.05}, {.x = 0.05, .y = 0.05, .z = 0.05}
		};

		static constexpr Face c_cubeFaces[NUM_CUBE_FACES] = {
			{.vertices = {0, 1, 2, 3}, .color = {0, 0, 255}},
			{.vertices = {6, 7, 4, 5}, .color = {0, 255, 0}},
			{.vertices = {4, 5, 0, 1}, .color = {0, 255, 255}},
			{.vertices = {7, 6, 3, 2}, .color = {255, 0, 0}},
			{.vertices = {6, 4, 2, 0}, .color = {255, 0, 255}},
			{.vertices = {5, 7, 1, 3}, .color = {255, 255, 0}}
		};

	private:
		CD m_cdrom;
		Mesh m_cubemesh;
		psyqo::Color m_color;
};

static Cube cube;
static CubeScene cubeScene;

void Cube::prepare() {
	psyqo::GPU::Configuration config;
	config.set(psyqo::GPU::Resolution::W320)
	    .set(psyqo::GPU::VideoMode::AUTO)
	    .set(psyqo::GPU::ColorMode::C15BITS)
	    .set(psyqo::GPU::Interlace::PROGRESSIVE);

	gpu().initialize(config);
}

void Cube::createScene() { 
	pushScene(&cubeScene); 
}

void CubeScene::start(StartReason reason) {
	// Clear the translation registers
	psyqo::GTE::clear<psyqo::GTE::Register::TRX, psyqo::GTE::Unsafe>();
	psyqo::GTE::clear<psyqo::GTE::Register::TRY, psyqo::GTE::Unsafe>();
	psyqo::GTE::clear<psyqo::GTE::Register::TRZ, psyqo::GTE::Unsafe>();

	// Set the screen offset in the GTE. (this is half the X and Y resolutions as
	// standard)
	psyqo::GTE::write<psyqo::GTE::Register::OFX, psyqo::GTE::Unsafe>(psyqo::FixedPoint<16>(160.0).raw());
	psyqo::GTE::write<psyqo::GTE::Register::OFY, psyqo::GTE::Unsafe>(psyqo::FixedPoint<16>(120.0).raw());

	// Write the projection plane distance.
	psyqo::GTE::write<psyqo::GTE::Register::H, psyqo::GTE::Unsafe>(180);

	// Set the scaling for Z averaging.
	psyqo::GTE::write<psyqo::GTE::Register::ZSF3, psyqo::GTE::Unsafe>(ORDERING_TABLE_SIZE / 3);
	psyqo::GTE::write<psyqo::GTE::Register::ZSF4, psyqo::GTE::Unsafe>(ORDERING_TABLE_SIZE / 4);

	m_cdrom.read("MILK.GLB;1");
	m_color = {.r = 255, .g = 0, .b = 0};
}

void CubeScene::frame() {
	m_cdrom.advance();   // Drive the state machine

	if (!m_cdrom.isReady()) {
		// still loading → just clear screen
		int parity = gpu().getParity();
		auto &clear = m_clear[parity];
		gpu().getNextClear(clear.primitive, c_bg);
		gpu().chain(clear);
		return;
	}

	if (!m_cubemesh.isValid()) {
		// load the cube mesh from the GLB file
		parse_GBL(m_cdrom.getFileBuffer(), m_cdrom.getEntry().size, &m_cubemesh);
		psyqo::Kernel::assert(m_cubemesh.isValid(), "Failed to load Cube mesh from GLB file");
	}

	// holding the projected 2D results of the 3D vertices, 
	// which will be used to draw a polygon on screen
	eastl::array<psyqo::Vertex, 3> projected;

	// get which frame we're currently drawing
	int parity = gpu().getParity();
	auto &ot = m_ots[parity];
	auto &clear = m_clear[parity];

	// chain the fill command accordingly to clear the buffer
	gpu().getNextClear(clear.primitive, c_bg);
	gpu().chain(clear);

	// distance
	psyqo::GTE::write<psyqo::GTE::Register::TRZ, psyqo::GTE::Unsafe>(24000);

	// 1. Spinning rotations (X then Y)
	auto transform = psyqo::SoftMath::generateRotationMatrix33(m_rot, psyqo::SoftMath::Axis::X, cube.m_trig);

	auto rotY = psyqo::SoftMath::generateRotationMatrix33(m_rot, psyqo::SoftMath::Axis::Y, cube.m_trig);

	psyqo::SoftMath::multiplyMatrix33(transform, rotY, &transform);

	// 2. Optional Z rotation (currently identity)
	auto rotZ = psyqo::SoftMath::generateRotationMatrix33(0, psyqo::SoftMath::Axis::Z, cube.m_trig);
	psyqo::SoftMath::multiplyMatrix33(transform, rotZ, &transform);

	// 3. Force the plane to face the camera (90° around X)
	//    Apply this *after* the spinning rotations so the plane stays facing us while it spins.
	auto faceCamera = psyqo::SoftMath::generateRotationMatrix33(0.5_pi, psyqo::SoftMath::Axis::X, cube.m_trig);

	psyqo::SoftMath::multiplyMatrix33(faceCamera, transform, &transform);

	// 4. Write the final matrix once
	psyqo::GTE::writeUnsafe<psyqo::GTE::PseudoRegister::Rotation>(transform);


	for(int i=0, t=0; i<m_cubemesh.num_indices; i+=3, t++) {
		// load 3 vertices into the GTE.
		psyqo::GTE::writeUnsafe<psyqo::GTE::PseudoRegister::V0>(m_cubemesh.vertices[m_cubemesh.indices[i+2]]); // count backwards because the GTE expects them in reverse order
		psyqo::GTE::writeUnsafe<psyqo::GTE::PseudoRegister::V1>(m_cubemesh.vertices[m_cubemesh.indices[i+1]]);
		psyqo::GTE::writeUnsafe<psyqo::GTE::PseudoRegister::V2>(m_cubemesh.vertices[m_cubemesh.indices[i+0]]);

		// perform rtpt (perspective transformation) to the three verticies.
		psyqo::GTE::Kernels::rtpt();

		// nclip determines the winding of the vertices, used to check which direction the face is pointing. Clockwise winding means the face is oriented towards us.
		psyqo::GTE::Kernels::nclip();

		// read the result of nclip and skip rendering this face if it's not facing us
		int32_t mac0 = 0;
		psyqo::GTE::read<psyqo::GTE::Register::MAC0>(reinterpret_cast<uint32_t*>(&mac0));
		if(mac0 <= 0) continue;

		psyqo::GTE::Kernels::avsz3();
		int32_t zIndex = 0;
		psyqo::GTE::read<psyqo::GTE::Register::OTZ>(reinterpret_cast<uint32_t*>(&zIndex));
// if(zIndex < 0 || zIndex >= ORDERING_TABLE_SIZE) continue;

		psyqo::GTE::read<psyqo::GTE::Register::SXY0>(&projected[0].packed);
		psyqo::GTE::read<psyqo::GTE::Register::SXY1>(&projected[1].packed);
		psyqo::GTE::read<psyqo::GTE::Register::SXY2>(&projected[2].packed);

		auto &tri = m_triangles[t];
		tri.primitive.setPointA(projected[0]);
		tri.primitive.setPointB(projected[1]);
		tri.primitive.setPointC(projected[2]);
		tri.primitive.setColor(m_color);
		tri.primitive.setOpaque();

//		printf("Triangle %d: zIndex=%d, points=(%d,%d), (%d,%d), (%d,%d)\n", t, zIndex, projected[0].x, projected[0].y, projected[1].x, projected[1].y, projected[2].x, projected[2].y);

		ot.insert(tri, zIndex);
	}

	gpu().chain(ot);
	m_rot += psyqo::Angle(0.01);
}

int main() { 
	return cube.run(); 
}
