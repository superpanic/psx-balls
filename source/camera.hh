#pragma once
#include "psyqo/matrix.hh"
#include "psyqo/vector.hh"
#include "psyqo/fixed-point.hh"
#include "psyqo/soft-math.hh"

using namespace psyqo::fixed_point_literals;

namespace game_camera {

	static constexpr psyqo::Matrix33 identity = {{
		{1.0_fp, 0.0_fp, 0.0_fp},
		{0.0_fp, 1.0_fp, 0.0_fp},
		{0.0_fp, 0.0_fp, 1.0_fp}
	}};

	typedef struct Camera {
		uint32_t fov = 180;
		psyqo::Matrix33 orientation = identity;
		psyqo::Vec3 position = {0.0_fp, 0.0_fp, 0.0_fp};
	} Camera;

}
