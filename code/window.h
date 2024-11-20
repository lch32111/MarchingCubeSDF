#ifndef __WINDOW_H__
#define __WINDOW_H__

struct GLFWwindow;

struct WindowState
{
    GLFWwindow* window;
    double time;
    int window_width;
    int window_height;
    double prev_mouse_x, prev_mouse_y;
    double mouse_x, mouse_y;
    bool mouse_down_once;
    bool mouse_clicked;
    bool prev_mouse_clicked;
    bool is_mouse_drag;
    double drag_x_delta, drag_y_delta;
};
extern WindowState g_window_state;

void glfw_callback_framebuffer_size(GLFWwindow* window, int width, int height);
void glfw_init();
void glfw_terminate();

void glfw_state_update();

#endif