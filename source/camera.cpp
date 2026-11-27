#include "camera.hh"

namespace game_camera {

void look_at(Camera &camera, const psyqo::Vec3 &target) {
    psyqo::Vec3 forward = target - camera.position;
    psyqo::SoftMath::fastNormalizeVec3(&forward);
    
    // Choose initial up based on forward direction to avoid parallel vectors
    psyqo::Vec3 up = {0.0_fp, 1.0_fp, 0.0_fp};
    if (forward.y.abs() > 0.9_fp) {  // Nearly looking up/down
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

}
