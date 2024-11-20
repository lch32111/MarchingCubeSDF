#include "camera.h"
#include "common.h"
#include <imgui/imgui.h>
#include <GLFW/glfw3.h>

void camera_reset(Camera* c, glm::vec3 pos, int window_width, int window_height)
{
    c->move_speed = 5.f;
    c->mouse_sensitivity = 0.07f;

    c->position = pos;
    c->rotation = glm::quat(1.f, 0.f, 0.f, 0.f);

    c->right = glm::vec3(1.f, 0.f, 0.f);
    c->up = glm::vec3(0.f, 1.f, 0.f);
    c->forward = glm::vec3(0.f, 0.f, 1.f);

    c->view = glm::mat4_cast(glm::conjugate(c->rotation));
    c->view[3][0] = -(c->view[0][0] * c->position[0] + c->view[1][0] * c->position[1] + c->view[2][0] * c->position[2]);
    c->view[3][1] = -(c->view[0][1] * c->position[0] + c->view[1][1] * c->position[1] + c->view[2][1] * c->position[2]);
    c->view[3][2] = -(c->view[0][2] * c->position[0] + c->view[1][2] * c->position[1] + c->view[2][2] * c->position[2]);
    c->view[3][3] = 1.f;

    c->fov_degree = 60.f;
    c->near_plane = 0.1f;
    c->far_plane = 1000.f;

    float aspect = (float)window_width;
    if (window_height != 0)
    {
        // handle with when the window minimized
        aspect /= window_height;
    }
    
    c->projection = glm::perspective(GET_RADIANF(c->fov_degree), aspect, c->near_plane, c->far_plane);
}

void camera_update(Camera* c, int window_width, int window_height)
{
    ImGuiIO& io = ImGui::GetIO();

    bool should_update_view_matrix = false;
    
    if (io.MouseDown[GLFW_MOUSE_BUTTON_LEFT] && !io.WantCaptureMouse)
    {
        should_update_view_matrix = true;

        float x_delta = GET_RADIANF(io.MouseDelta.x) * c->mouse_sensitivity;
        float y_delta = GET_RADIANF(io.MouseDelta.y) * c->mouse_sensitivity;
        glm::quat y_rot = glm::angleAxis(-x_delta, glm::vec3(0.f, 1.f, 0.f));
        glm::quat x_rot = glm::angleAxis(-y_delta, glm::vec3(1.f, 0.f, 0.f));

        c->rotation = (y_rot * c->rotation) * x_rot;
        
        glm::mat3 rot_mat = glm::mat3_cast(c->rotation);
        c->right = rot_mat[0];
        c->up = rot_mat[1];
        c->forward = rot_mat[2];
    }

    if (!io.WantCaptureKeyboard)
    {
        float delta_move = io.DeltaTime * c->move_speed;

        if (io.KeyShift)
        {
            delta_move *= 10.f;
        }

        if (io.KeysDown[GLFW_KEY_W])
        {
            should_update_view_matrix = true;
            c->position -= c->forward * delta_move;
        }

        if (io.KeysDown[GLFW_KEY_A])
        {
            should_update_view_matrix = true;
            c->position -= c->right * delta_move;
        }

        if (io.KeysDown[GLFW_KEY_S])
        {
            should_update_view_matrix = true;
            c->position += c->forward * delta_move;
        }

        if (io.KeysDown[GLFW_KEY_D])
        {
            should_update_view_matrix = true;
            c->position += c->right * delta_move;
        }
    }

    if (should_update_view_matrix)
    {
        c->view = glm::mat4_cast(glm::conjugate(c->rotation));
        c->view[3][0] = -(c->view[0][0] * c->position[0] + c->view[1][0] * c->position[1] + c->view[2][0] * c->position[2]);
        c->view[3][1] = -(c->view[0][1] * c->position[0] + c->view[1][1] * c->position[1] + c->view[2][1] * c->position[2]);
        c->view[3][2] = -(c->view[0][2] * c->position[0] + c->view[1][2] * c->position[1] + c->view[2][2] * c->position[2]);
        c->view[3][3] = 1.f;
    }

    float aspect = (float)window_width;
    if (window_height != 0)
    {
        // handle with when the window minimized
        aspect /= window_height;
    }

    c->projection = glm::perspective(GET_RADIANF(c->fov_degree), aspect, c->near_plane, c->far_plane);
}