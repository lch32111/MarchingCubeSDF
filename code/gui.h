#ifndef __GUI_H__
#define __GUI_H__

struct GLFWwindow;

void imgui_init();
void imgui_terminate();
void imgui_prepare();
void imgui_draw();

void imgui_glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void imgui_glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void imgui_glfw_char_callback(GLFWwindow* window, unsigned int c);
const char* imgui_glfw_clipboard_get(void* user_data);
void imgui_glfw_clipboard_set(void* user_data, const char* text);

#endif