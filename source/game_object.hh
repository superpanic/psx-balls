#pragma once

#include "psyqo/trigonometry.hh"
#include "psyqo/ordering-table.hh"
#include "gbl_parser.hh"
#include "psyqo/vector.hh"
#include "texture.hh"

#define SMALL_MODEL_MAX_VERTICES (512)
#define SMALL_MODEL_MAX_INDICES (1024)

typedef struct Mesh {
	eastl::string name;
	psyqo::Vec3 vertices[SMALL_MODEL_MAX_VERTICES];
	uint16_t indices[SMALL_MODEL_MAX_INDICES];
	psyqo::Vec2 texcoords[SMALL_MODEL_MAX_VERTICES];
	unsigned num_vertices = 0;
	unsigned num_texcoords = 0;
	unsigned num_indices = 0;
	bool isValid() const { return num_vertices > 0 && num_indices > 0; }
} Mesh;

// will be replaced by GameObject class in the future
typedef struct Object {
	Mesh mesh;
	Texture texture;
	psyqo::Vec3 position;
	psyqo::Vec3 rotation;
	psyqo::Vec3 scale = {1.0f, 1.0f, 1.0f};
} Object;

class GameObject {
	public:
		void update(psyqo::Angle angle);
		void draw(psyqo::OrderingTable<> &ot);

		Mesh *mesh = nullptr;
		Texture *texture = nullptr;

		psyqo::Vec3 position;
		psyqo::Vec3 rotation;
		psyqo::Vec3 scale = {1.0f, 1.0f, 1.0f};

		bool visible = true;
		bool active = true;

	private:

};
