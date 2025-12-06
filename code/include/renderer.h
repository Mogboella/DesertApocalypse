#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <functional>
#include <cmath>

#include "mesh_loader.h"

using namespace std;
using namespace glm;

struct DayNightParams
{
    vec3 lightDir;
    vec3 lightColor;
    vec3 ambientColor;
    vec3 fogColor;
    vec3 skyHorizonColor;
    vec3 skyZenithColor;
    vec3 sunDir;
    vec3 sunColor;
    float lightIntensity;
};

void renderScene(float delta, const mat4 &view, const mat4 &proj, GLuint shaderProgramID, const vec3 &cameraPos, float timeOfDay = 12.0f);
void renderHierarchicalMeshes(float delta, const mat4 &view, const mat4 &proj, GLuint shaderProgramID, const vec3 &cameraPos, float timeOfDay = 12.0f);

void updateDynamicLights(GLuint shaderProgramID, float time, const vec3 &cameraPos, bool enablePointLights = true, bool enableSpotLights = true);
void updateOrbitalMotion(HierarchicalModel &parentModel,
                         HierarchicalModel &childModel,
                         float delta,
                         float orbitRadius = 10.0f,
                         float orbitSpeed = 1.0f,
                         float forwardSpeed = 5.0f);

DayNightParams calculateDayNightCycle(float timeOfDay);
void cleanupScene();

#endif
