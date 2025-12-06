#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "skybox.h"
#include "shader_utils.h"

using namespace glm;

Skybox::Skybox()
    : VAO(0), VBO(0), shaderProgram(0), sunSize(500.0f)
{
    horizonColor = vec3(0.95f, 0.85f, 0.70f);
    zenithColor = vec3(0.40f, 0.60f, 0.85f);

    sun1Dir = normalize(vec3(-0.4f, 0.5f, -0.8f));
    sun2Dir = normalize(vec3(-0.6f, 0.5f, -0.6f));
    sun1Color = vec3(1.0f, 0.95f, 0.7f) * 3.0f;
    sun2Color = vec3(1.0f, 0.6f, 0.3f) * 2.0f;
}

Skybox::~Skybox()
{
    cleanup();
}

void Skybox::init(const char *vertPath, const char *fragPath)
{
    shaderProgram = CompileShaders(vertPath, fragPath);

    if (shaderProgram == 0)
    {
        std::cerr << "Failed to compile skybox shaders!" << std::endl;
        return;
    }

    setupMesh();
}

void Skybox::setupMesh()
{
    float vertices[] = {
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Skybox::render(const mat4 &view, const mat4 &proj)
{
    glDepthFunc(GL_LEQUAL);
    glUseProgram(shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, value_ptr(proj));

    glUniform3f(glGetUniformLocation(shaderProgram, "horizonColor"),
                horizonColor.x, horizonColor.y, horizonColor.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "zenithColor"),
                zenithColor.x, zenithColor.y, zenithColor.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "sun1Dir"),
                sun1Dir.x, sun1Dir.y, sun1Dir.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "sun1Color"),
                sun1Color.x, sun1Color.y, sun1Color.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "sun2Dir"),
                sun2Dir.x, sun2Dir.y, sun2Dir.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "sun2Color"),
                sun2Color.x, sun2Color.y, sun2Color.z);

    glUniform1f(glGetUniformLocation(shaderProgram, "sunSize"), sunSize);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

void Skybox::cleanup()
{
    if (VAO)
    {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO)
    {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (shaderProgram)
    {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
}
