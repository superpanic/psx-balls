#pragma once

#include "psyqo/trigonometry.hh"
#include "psyqo/ordering-table.hh"
#include "gbl_parser.hh"
#include "psyqo/vector.hh"

class GameObject {
	public:
		void update(psyqo::Angle angle);
		void draw(psyqo::OrderingTable<> &ot);

		Mesh *mesh = nullptr;
		Texture *texture = nullptr;

		psyqo::Vec3 position;
		psyqo::Vec3 rotation;
		psyqo::Vec3 scale = {1.0fp, 1.0fp, 1.0fp};

		bool visible = true;
		bool active = true;

	private:

};
