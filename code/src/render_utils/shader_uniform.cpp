#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>

#include "shader_uniform.h"
#include "mesh_loader.h"
#include "glm_compat.h"

using namespace std;
using namespace glm;

void ShaderUniformManager::initialize(GLuint shaderProgramID)
{
    shaderProgram = shaderProgramID;
    locModel = glGetUniformLocation(shaderProgram, "model");
    locView = glGetUniformLocation(shaderProgram, "view");
    locProj = glGetUniformLocation(shaderProgram, "projection");
    locViewPos = glGetUniformLocation(shaderProgram, "viewPos");
    locAmbient = glGetUniformLocation(shaderProgram, "ambientColor");
    locSpecularStrength = glGetUniformLocation(shaderProgram, "specularStrength");
    locShininess = glGetUniformLocation(shaderProgram, "shininess");
    locLightDir = glGetUniformLocation(shaderProgram, "lightDir");
    locLightColor = glGetUniformLocation(shaderProgram, "lightColor");
    locHasBones = glGetUniformLocation(shaderProgram, "hasBones");
    locFogColor = glGetUniformLocation(shaderProgram, "fogColor");
    locFogDensity = glGetUniformLocation(shaderProgram, "fogDensity");
    locFogStart = glGetUniformLocation(shaderProgram, "fogStart");
    locFogEnd = glGetUniformLocation(shaderProgram, "fogEnd");
    locNumPointLights = glGetUniformLocation(shaderProgram, "numPointLights");
    locNumSpotLights = glGetUniformLocation(shaderProgram, "numSpotLights");
    locPointLights = glGetUniformLocation(shaderProgram, "pointLights");
    locSpotLights = glGetUniformLocation(shaderProgram, "spotLights");
    locTime = glGetUniformLocation(shaderProgram, "time");
    locUseHeatShimmer = glGetUniformLocation(shaderProgram, "useHeatShimmer");
    locHeatShimmerIntensity = glGetUniformLocation(shaderProgram, "heatShimmerIntensity");
    locFogHeight = glGetUniformLocation(shaderProgram, "fogHeight");
    locFogHeightRange = glGetUniformLocation(shaderProgram, "fogHeightRange");
}

void ShaderUniformManager::setViewProjection(const mat4 &view, const mat4 &proj, const vec3 &cameraPos)
{
    if (locView >= 0)
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
    if (locProj >= 0)
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(proj));
    if (locViewPos >= 0)
        glUniform3f(locViewPos, cameraPos.x, cameraPos.y, cameraPos.z);
}

void ShaderUniformManager::setLighting(const vec3 &lightDir, const vec3 &lightColor)
{
    if (locLightDir >= 0)
        glUniform3f(locLightDir, lightDir.x, lightDir.y, lightDir.z);
    if (locLightColor >= 0)
        glUniform3f(locLightColor, lightColor.x, lightColor.y, lightColor.z);
}

void ShaderUniformManager::setMaterial(float ambientR, float ambientG, float ambientB, float specularStrength, float shininess)
{
    if (locAmbient >= 0)
        glUniform3f(locAmbient, ambientR, ambientG, ambientB);
    if (locSpecularStrength >= 0)
        glUniform1f(locSpecularStrength, specularStrength);
    if (locShininess >= 0)
        glUniform1f(locShininess, shininess);
}

void ShaderUniformManager::setModelMatrix(const mat4 &model)
{
    if (locModel >= 0)
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
}

void ShaderUniformManager::setHasBones(bool hasBones)
{
    if (locHasBones >= 0)
        glUniform1i(locHasBones, hasBones);
}

void ShaderUniformManager::setFog(const vec3 &fogColor, float fogDensity, float fogStart, float fogEnd, float fogHeight, float fogHeightRange)
{
    if (locFogColor >= 0)
        glUniform3f(locFogColor, fogColor.x, fogColor.y, fogColor.z);
    if (locFogDensity >= 0)
        glUniform1f(locFogDensity, fogDensity);
    if (locFogStart >= 0)
        glUniform1f(locFogStart, fogStart);
    if (locFogEnd >= 0)
        glUniform1f(locFogEnd, fogEnd);
    if (locFogHeight >= 0)
        glUniform1f(locFogHeight, fogHeight);
    if (locFogHeightRange >= 0)
        glUniform1f(locFogHeightRange, fogHeightRange);
}

void ShaderUniformManager::setPointLights(const vector<vec3> &positions,
                                          const vector<vec3> &colors,
                                          const vector<float> &intensities,
                                          const vector<float> &constants,
                                          const vector<float> &linears,
                                          const vector<float> &quadratics,
                                          const vector<float> &ranges)
{
    if (locNumPointLights >= 0)
    {
        int numLights = std::min((int)positions.size(), 4);
        glUniform1i(locNumPointLights, numLights);

        for (int i = 0; i < numLights; i++)
        {
            string base = "pointLights[" + to_string(i) + "].";
            glUniform3fv(glGetUniformLocation(shaderProgram, (base + "position").c_str()), 1, value_ptr(positions[i]));
            glUniform3fv(glGetUniformLocation(shaderProgram, (base + "color").c_str()), 1, value_ptr(colors[i]));
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "intensity").c_str()), intensities[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "constant").c_str()), constants[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "linear").c_str()), linears[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "quadratic").c_str()), quadratics[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "range").c_str()), ranges[i]);
        }
    }
}

void ShaderUniformManager::setSpotLights(const vector<vec3> &positions,
                                         const vector<vec3> &directions,
                                         const vector<vec3> &colors,
                                         const vector<float> &intensities,
                                         const vector<float> &constants,
                                         const vector<float> &linears,
                                         const vector<float> &quadratics,
                                         const vector<float> &cutoffs,
                                         const vector<float> &outerCutoffs,
                                         const vector<float> &ranges)
{
    if (locNumSpotLights >= 0)
    {
        int numLights = std::min((int)positions.size(), 2);
        glUniform1i(locNumSpotLights, numLights);

        for (int i = 0; i < numLights; i++)
        {
            string base = "spotLights[" + to_string(i) + "].";
            glUniform3fv(glGetUniformLocation(shaderProgram, (base + "position").c_str()), 1, value_ptr(positions[i]));
            glUniform3fv(glGetUniformLocation(shaderProgram, (base + "direction").c_str()), 1, value_ptr(directions[i]));
            glUniform3fv(glGetUniformLocation(shaderProgram, (base + "color").c_str()), 1, value_ptr(colors[i]));
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "intensity").c_str()), intensities[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "constant").c_str()), constants[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "linear").c_str()), linears[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "quadratic").c_str()), quadratics[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "cutoff").c_str()), cutoffs[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "outerCutoff").c_str()), outerCutoffs[i]);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "range").c_str()), ranges[i]);
        }
    }
}

void ShaderUniformManager::setTime(float time)
{
    if (locTime >= 0)
        glUniform1f(locTime, time);
}

void ShaderUniformManager::setUseHeatShimmer(bool use)
{
    if (locUseHeatShimmer >= 0)
        glUniform1i(locUseHeatShimmer, use ? 1 : 0);
}

void ShaderUniformManager::setHeatShimmerIntensity(float intensity)
{
    if (locHeatShimmerIntensity >= 0)
        glUniform1f(locHeatShimmerIntensity, intensity);
}

bool ShaderUniformManager::isValid() const
{
    return locModel >= 0 && locView >= 0 && locProj >= 0 && locViewPos >= 0 && locAmbient >= 0 && locSpecularStrength >= 0 && locShininess >= 0 && locLightDir >= 0 && locLightColor >= 0;
}

void ShaderUniformManager::cleanup()
{
    shaderProgram = 0;
    locModel = -1;
    locView = -1;
    locProj = -1;
    locViewPos = -1;
    locAmbient = -1;
    locSpecularStrength = -1;
    locShininess = -1;
    locLightDir = -1;
    locLightColor = -1;
    locFogColor = -1;
    locFogDensity = -1;
    locFogStart = -1;
    locFogEnd = -1;
    locTime = -1;
    locUseHeatShimmer = -1;
    locHeatShimmerIntensity = -1;
}
