#ifndef CALLBACKS_H
#define CALLBACKS_H

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>
#include "camera.h"

extern Camera *gCamera;
extern float gLastX;
extern float gLastY;
extern bool gFirstMouse;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int modifiers);
void processInput(GLFWwindow *window, Camera &camera, float deltaTime);

#endif
