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

void look_at(Camera &camera, const psyqo::Vec3 &target);

}

/*
void look_at(Camera &camera, const psyqo::Vec3 &target) {
    psyqo::Vec3 forward = target - camera.position;
    psyqo::SoftMath::fastNormalizeVec3(&forward);
    
    // Choose initial up based on forward direction to avoid parallel vectors
    psyqo::Vec3 up = {0.0_fp, 1.0_fp, 0.0_fp};
    if (psyqo::SoftMath::abs(forward.y) > 0.9_fp) {  // Nearly looking up/down
        up = {1.0_fp, 0.0_fp, 0.0_fp};
    }
    
    psyqo::Vec3 right;
    psyqo::SoftMath::crossProductVec3(up, forward, &right);
    psyqo::SoftMath::fastNormalizeVec3(&right);
    psyqo::SoftMath::crossProductVec3(forward, right, &up);
    
    camera.orientation = {{
        {right.x, right.y, right.z},
        {up.x, up.y, up.z},
        {forward.x, forward.y, forward.z}
    }};
}
*/