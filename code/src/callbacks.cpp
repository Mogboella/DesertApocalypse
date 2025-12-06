#include "callbacks.h"
#include <iostream>
using namespace std;

Camera *gCamera = nullptr;
float gLastX = 400.0f;
float gLastY = 300.0f;
bool gFirstMouse = true;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = (float)xpos;
        gLastY = (float)ypos;
        gFirstMouse = false;
    }

    float xoffset = (float)xpos - gLastX;
    float yoffset = gLastY - (float)ypos;
    gLastX = (float)xpos;
    gLastY = (float)ypos;

    if (gCamera)
        gCamera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (gCamera)
        gCamera->ProcessMouseScroll((float)yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int modifiers)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void processInput(GLFWwindow *window, Camera &camera, float deltaTime)
{
    camera.UpdateTransition(deltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        camera.SetPresetPosition(0);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        camera.SetPresetPosition(1);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        camera.SetPresetPosition(2);
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        camera.SetPresetPosition(3);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FRONT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACK, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
}
