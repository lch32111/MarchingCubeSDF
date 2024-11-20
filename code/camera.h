#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Camera
{
    float move_speed;
    float mouse_sensitivity;

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 forward;

    // view space matrix
    glm::mat4 view;

    // clip space matrix using Perspective Projection
    float fov_degree;
    float near_plane;
    float far_plane;
    glm::mat4 projection;
};

void camera_reset(Camera* c, glm::vec3 pos, int window_width, int window_height);
void camera_update(Camera* c, int window_width, int window_height);

#endif