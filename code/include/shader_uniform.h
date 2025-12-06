#ifndef SHADER_UNIFORM_H
#define SHADER_UNIFORM_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh_loader.h"

using namespace std;
using namespace glm;

class ShaderUniformManager
{
private:
    GLuint shaderProgram;
    GLint locModel;
    GLint locView;
    GLint locProj;
    GLint locViewPos;
    GLint locAmbient;
    GLint locSpecularStrength;
    GLint locShininess;
    GLint locLightDir;
    GLint locLightColor;
    GLint locHasBones;
    GLint locFogColor;
    GLint locFogDensity;
    GLint locFogStart;
    GLint locFogEnd;
    GLint locNumPointLights;
    GLint locNumSpotLights;
    GLint locPointLights;
    GLint locSpotLights;
    GLint locTime;
    GLint locUseHeatShimmer;
    GLint locHeatShimmerIntensity;
    GLint locFogHeight;
    GLint locFogHeightRange;

public:
    void initialize(GLuint shaderProgramID);

    void setViewProjection(const mat4 &view, const mat4 &proj, const vec3 &cameraPos);

    void setLighting(const vec3 &lightDir, const vec3 &lightColor);

    void setMaterial(float ambientR, float ambientG, float ambientB,
                     float specularStrength, float shininess);

    void setHasBones(bool hasBones);
    void setFog(const vec3 &fogColor, float fogDensity, float fogStart, float fogEnd, float fogHeight, float fogHeightRange);
    void setModelMatrix(const mat4 &model);

    void setPointLights(const vector<vec3> &positions, const vector<vec3> &colors,
                        const vector<float> &intensities,
                        const vector<float> &constants,
                        const vector<float> &linears,
                        const vector<float> &quadratics,
                        const vector<float> &ranges);
    void setSpotLights(const vector<vec3> &positions, const vector<vec3> &directions,
                       const vector<vec3> &colors, const vector<float> &intensities,
                       const vector<float> &constants,
                       const vector<float> &linears,
                       const vector<float> &quadratics,
                       const vector<float> &cutoffs, const vector<float> &outerCutoffs,
                       const vector<float> &ranges);

    void setTime(float time);
    void setUseHeatShimmer(bool use);
    void setHeatShimmerIntensity(float intensity);
    bool isValid() const;
    void cleanup();
};

#endif
