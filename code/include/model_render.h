#ifndef MODEL_RENDER_H
#define MODEL_RENDER_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

#include "mesh_loader.h"
#include "scene_manager.h"
#include "shader_uniform.h"

using namespace std;
using namespace glm;

class ModelRenderer
{
private:
    GLuint shaderProgram;
    ShaderUniformManager *uniforms;

    bool isValidMatrix(const mat4 &m) const;
    mat4 buildModelMatrix(const vec3 &pos, const vec3 &rot, const vec3 &scale) const;
    mat4 buildNodeTransform(const HierarchicalNode *node, const mat4 &parentGlobal) const;

public:
    ModelRenderer(GLuint shaderProgramID);

    ~ModelRenderer();

    void renderMesh(const MeshInstance &mesh, const mat4 &modelMatrix, const mat4 &view, const mat4 &proj);

    void renderNode(HierarchicalNode *node,
                    const mat4 &parentGlobal,
                    const HierarchicalModel &model,
                    const mat4 &view,
                    const mat4 &proj);

    void renderHierarchicalModel(HierarchicalModel &model,
                                 const mat4 &worldTransform,
                                 const mat4 &view,
                                 const mat4 &proj);

    void renderMeshes(const vector<MeshInstance> &meshes,
                      vector<MeshTransform> &transforms,
                      float delta,
                      const mat4 &view,
                      const mat4 &proj);

    void setViewProjection(const mat4 &view, const mat4 &proj, const vec3 &cameraPos);
    void setMaterial(float ambientR, float ambientG, float ambientB, float specularStrength, float shininess);
    void setLighting(const vec3 &lightDir, const vec3 &lightColor);
    void setFog(const vec3 &fogColor, float fogDensity, float fogStart, float fogEnd, float fogHeight, float fogHeightRange);
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
};

#endif
