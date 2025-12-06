#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/gl.h>
#include <glm/glm.hpp>

using namespace glm;

class Skybox
{
public:
    Skybox();
    ~Skybox();

    void init(const char *vertPath, const char *fragPath);
    void render(const mat4 &view, const mat4 &proj);
    void cleanup();

    void setHorizonColor(const vec3 &color) { horizonColor = color; }
    void setZenithColor(const vec3 &color) { zenithColor = color; }
    void setSun1(const vec3 &dir, const vec3 &color)
    {
        sun1Dir = dir;
        sun1Color = color;
    }
    void setSun2(const vec3 &dir, const vec3 &color)
    {
        sun2Dir = dir;
        sun2Color = color;
    }
    void setSunSize(float size) { sunSize = size; }

private:
    GLuint VAO, VBO;
    GLuint shaderProgram;

    vec3 horizonColor;
    vec3 zenithColor;
    vec3 sun1Dir;
    vec3 sun2Dir;
    vec3 sun1Color;
    vec3 sun2Color;
    float sunSize;

    void setupMesh();
};

#endif
