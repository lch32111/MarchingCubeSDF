#include "window.h"

#include <stdio.h>
#include <assert.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

WindowState g_window_state;

void glfw_callback_framebuffer_size(GLFWwindow* window, int width, int height)
{
    g_window_state.window_width = width;
    g_window_state.window_height = height;
    glViewport(0, 0, width, height);
}

void glfw_init()
{
    constexpr int INITIAL_WINDOW_WIDTH = 600;
    constexpr int INITIAL_WINDOW_HEIGHT = 480;

    glfwInit();

    int window_pos_x = 0, window_pos_y = 0;

    if (GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor())
    {
        int monitor_pos_x = 0, monitor_pos_y = 0;
        int monitor_width = 0, monitor_height = 0;
        glfwGetMonitorWorkarea(primary_monitor, &monitor_pos_x, &monitor_pos_y, &monitor_width, &monitor_height);

        constexpr float app_width_factor = 0.7f;
        constexpr float app_height_factor = 0.7f;
        g_window_state.window_width = (int)(monitor_width * app_width_factor);
        g_window_state.window_height = (int)(monitor_height * app_height_factor);
        window_pos_x = (int)(monitor_pos_x + (monitor_width - g_window_state.window_width) / 2.f);
        window_pos_y = (int)(monitor_pos_y + (monitor_height - g_window_state.window_height) / 2.f);
    }
    else
    {
        g_window_state.window_width = INITIAL_WINDOW_WIDTH;
        g_window_state.window_height = INITIAL_WINDOW_HEIGHT;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_window_state.window = glfwCreateWindow(g_window_state.window_width, g_window_state.window_height, "COMP 477 Submission (40228578/Chanhaeng Lee)", NULL, NULL);
    if (g_window_state.window == NULL)
    {
        printf("Failed to create GLFW window\n");
        assert(false);
    }
    glfwSetFramebufferSizeCallback(g_window_state.window, glfw_callback_framebuffer_size);
    glfwSetWindowPos(g_window_state.window, window_pos_x, window_pos_y);

    glfwMakeContextCurrent(g_window_state.window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        assert(false);
    }

    glfwSwapInterval(0);
}

void glfw_terminate()
{
    glfwDestroyWindow(g_window_state.window);
    glfwTerminate();
}

void glfw_state_update()
{
    WindowState& ws = g_window_state;

    ws.prev_mouse_x = ws.mouse_x;
    ws.prev_mouse_y = ws.mouse_y;
    glfwGetCursorPos(ws.window, &ws.mouse_x, &ws.mouse_y);

    ws.prev_mouse_clicked = ws.mouse_clicked;
    ws.mouse_clicked = glfwGetMouseButton(ws.window, GLFW_MOUSE_BUTTON_1);
    ws.mouse_down_once = ws.prev_mouse_clicked == false && ws.mouse_clicked == true;
    ws.is_mouse_drag = ws.prev_mouse_clicked == true && ws.mouse_clicked == true;

    if (ws.is_mouse_drag)
    {
        ws.drag_x_delta += ws.mouse_x - ws.prev_mouse_x;
        ws.drag_y_delta += ws.mouse_y - ws.prev_mouse_y;
    }

    if (ws.mouse_clicked == false)
    {
        ws.drag_x_delta = 0.0;
        ws.drag_y_delta = 0.0;
    }
}