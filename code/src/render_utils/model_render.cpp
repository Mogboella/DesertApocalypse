#include <iostream>
#include <cmath>
#include <map>

#include "model_render.h"
#include "mesh_loader.h"
#include "scene_manager.h"
#include "transform_utils.h"
#include "glm_compat.h"

using namespace std;
using namespace glm;

ModelRenderer::ModelRenderer(GLuint shaderProgramID)
{
    shaderProgram = shaderProgramID;
    uniforms = new ShaderUniformManager();
    uniforms->initialize(shaderProgramID);
}

ModelRenderer::~ModelRenderer()
{
    if (uniforms)
    {
        uniforms->cleanup();
        delete uniforms;
        uniforms = nullptr;
    }
}

void ModelRenderer::setViewProjection(const mat4 &view, const mat4 &proj, const vec3 &cameraPos)
{
    uniforms->setViewProjection(view, proj, cameraPos);
}

void ModelRenderer::setMaterial(float ambientR, float ambientG, float ambientB, float specularStrength, float shininess)
{
    uniforms->setMaterial(ambientR, ambientG, ambientB, specularStrength, shininess);
}

void ModelRenderer::setLighting(const vec3 &lightDir, const vec3 &lightColor)
{
    uniforms->setLighting(lightDir, lightColor);
}

void ModelRenderer::setFog(const vec3 &fogColor, float fogDensity, float fogStart, float fogEnd, float fogHeight, float fogHeightRange)
{
    if (uniforms)
    {
        uniforms->setFog(fogColor, fogDensity, fogStart, fogEnd, fogHeight, fogHeightRange);
    }
}

bool ModelRenderer::isValidMatrix(const mat4 &m) const
{
    const float *ptr = glm::value_ptr(m);
    for (int i = 0; i < 16; i++)
    {
        if (isnan(ptr[i]) || isinf(ptr[i]))
        {
            return false;
        }
    }
    return true;
}

mat4 ModelRenderer::buildModelMatrix(const vec3 &pos, const vec3 &rot, const vec3 &scale) const
{
    mat4 model = identity_mat4();
    model = translate(model, pos);
    model = rotate_x_deg(model, rot.x);
    model = rotate_y_deg(model, rot.y);
    model = rotate_z_deg(model, rot.z);
    model = glm::scale(model, scale);
    return model;
}

mat4 ModelRenderer::buildNodeTransform(const HierarchicalNode *node, const mat4 &parentGlobal) const
{
    if (!node)
        return parentGlobal;

    mat4 nodeTransform;
    if (node->hasAnimationTransform)
    {
        nodeTransform = node->animationTransform;
    }
    else
    {
        vec3 baseT = extractTranslation(node->localTransform);
        vec3 baseS = extractScale(node->localTransform);

        if (length(node->animTranslation) > 0.0001f ||
            length(node->animRotation) > 0.0001f)
        {
            mat4 animLocal = identity_mat4();
            animLocal = translate(animLocal, baseT + node->animTranslation);
            animLocal = rotate_x_deg(animLocal, node->animRotation.x);
            animLocal = rotate_y_deg(animLocal, node->animRotation.y);
            animLocal = rotate_z_deg(animLocal, node->animRotation.z);
            animLocal = glm::scale(animLocal, baseS); // Preserve scale from original

            nodeTransform = animLocal;
        }
        else
        {
            nodeTransform = node->localTransform;
        }
    }

    return parentGlobal * nodeTransform;
}

void ModelRenderer::renderMesh(const MeshInstance &mesh, const mat4 &modelMatrix, const mat4 &view, const mat4 &proj)
{
    if (!isValidMatrix(modelMatrix))
    {
        cerr << "WARNING: Invalid model matrix in renderMesh" << endl;
        return;
    }

    if (mesh.hasBones)
    {
        uniforms->setHasBones(true);

        GLint locBoneMatrices = glGetUniformLocation(shaderProgram, "finalBonesMatrices");
        if (locBoneMatrices >= 0)
        {
            glUniformMatrix4fv(locBoneMatrices, mesh.boneMatrices.size(), GL_FALSE, value_ptr(mesh.boneMatrices[0]));
        }
    }

    uniforms->setModelMatrix(modelMatrix);
    mesh.draw(shaderProgram, modelMatrix, view, proj);
}

void ModelRenderer::renderNode(HierarchicalNode *node,
                               const mat4 &parentGlobal,
                               const HierarchicalModel &model,
                               const mat4 &view,
                               const mat4 &proj)
{
    if (!node)
        return;

    mat4 global = buildNodeTransform(node, parentGlobal);

    if (!isValidMatrix(global))
    {
        cerr << "ERROR: Invalid matrix at node '" << node->name << "'" << endl;
        return;
    }

    uniforms->setModelMatrix(global);

    // Draw all meshes on this node
    for (unsigned int meshIdx : node->meshIndices)
    {
        if (meshIdx < model.meshes.size())
        {
            const MeshInstance &mesh = model.meshes[meshIdx];

            if (mesh.hasBones && meshIdx < model.boneMatricesPerMesh.size() &&
                !model.boneMatricesPerMesh[meshIdx].empty())
            {

                uniforms->setHasBones(true);

                const auto &boneMatrices = model.boneMatricesPerMesh[meshIdx];
                const GLsizei MAX_BONES_CPU = 100;
                GLsizei boneCount = static_cast<GLsizei>(
                    std::min(boneMatrices.size(), static_cast<size_t>(MAX_BONES_CPU)));

                GLint locBoneMatrices = glGetUniformLocation(shaderProgram, "finalBonesMatrices");
                if (locBoneMatrices >= 0)
                {
                    glUniformMatrix4fv(locBoneMatrices,
                                       boneCount,
                                       GL_FALSE,
                                       value_ptr(boneMatrices[0]));
                }
            }
            else
            {
                uniforms->setHasBones(false);
            }

            model.meshes[meshIdx].draw(shaderProgram, global, view, proj);
        }
    }

    for (HierarchicalNode *child : node->children)
    {
        if (child)
        {
            renderNode(child, global, model, view, proj);
        }
    }
}

void ModelRenderer::renderHierarchicalModel(HierarchicalModel &model,
                                            const mat4 &worldTransform,
                                            const mat4 &view,
                                            const mat4 &proj)
{
    if (!model.rootNode)
        return;

    mat4 rootGlobal = buildNodeTransform(model.rootNode, worldTransform);

    // uniforms->setModelMatrix(worldTransform);

    for (unsigned int meshIdx : model.rootNode->meshIndices)
    {
        if (meshIdx < model.meshes.size())
        {
            const MeshInstance &mesh = model.meshes[meshIdx];

            if (mesh.hasBones && meshIdx < model.boneMatricesPerMesh.size() &&
                !model.boneMatricesPerMesh[meshIdx].empty())
            {

                uniforms->setHasBones(true);

                GLint locBoneMatrices = glGetUniformLocation(shaderProgram, "finalBonesMatrices");
                if (locBoneMatrices >= 0)
                {
                    glUniformMatrix4fv(locBoneMatrices,
                                       model.boneMatricesPerMesh[meshIdx].size(),
                                       GL_FALSE,
                                       value_ptr(model.boneMatricesPerMesh[meshIdx][0]));
                }
            }
            else
            {
                uniforms->setHasBones(false);
            }

            model.meshes[meshIdx].draw(shaderProgram, rootGlobal, view, proj);
        }
    }

    // Render children recursively
    for (HierarchicalNode *child : model.rootNode->children)
    {
        if (child)
        {
            renderNode(child, rootGlobal, model, view, proj);
        }
    }
}

void ModelRenderer::renderMeshes(const vector<MeshInstance> &meshes,
                                 vector<MeshTransform> &transforms,
                                 float delta,
                                 const mat4 &view,
                                 const mat4 &proj)
{
    uniforms->setHasBones(false);

    for (size_t i = 0; i < meshes.size() && i < transforms.size(); i++)
    {
        MeshTransform &transform = transforms[i];

        if (transform.animated)
        {
            transform.animTime += delta;

            static map<size_t, float> originalY;
            if (originalY.find(i) == originalY.end())
            {
                originalY[i] = transform.position.y;
            }

            float floatAmount = sin(transform.animTime * 0.8f) * 0.3f;
            transform.position.y = originalY[i] + floatAmount;
        }

        mat4 modelMatrix = buildModelMatrix(transform.position, transform.rotation, transform.scale);

        if (!isValidMatrix(modelMatrix))
        {
            cerr << "WARNING: Invalid matrix values for mesh " << i << endl;
            continue;
        }

        uniforms->setModelMatrix(modelMatrix);
        meshes[i].draw(shaderProgram, modelMatrix, view, proj);

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            cerr << "GL error after draw " << i << ": 0x" << hex << err << dec << "\n";
        }
    }
}

void ModelRenderer::setPointLights(const vector<vec3> &positions, const vector<vec3> &colors,
                                   const vector<float> &intensities,
                                   const vector<float> &constants,
                                   const vector<float> &linears,
                                   const vector<float> &quadratics,
                                   const vector<float> &ranges)
{
    if (uniforms)
    {
        uniforms->setPointLights(positions, colors, intensities, constants, linears, quadratics, ranges);
    }
}

void ModelRenderer::setSpotLights(const vector<vec3> &positions, const vector<vec3> &directions,
                                  const vector<vec3> &colors, const vector<float> &intensities,
                                  const vector<float> &constants,
                                  const vector<float> &linears,
                                  const vector<float> &quadratics,
                                  const vector<float> &cutoffs, const vector<float> &outerCutoffs,
                                  const vector<float> &ranges)
{
    if (uniforms)
    {
        uniforms->setSpotLights(positions, directions, colors, intensities, constants, linears, quadratics, cutoffs, outerCutoffs, ranges);
    }
}

void ModelRenderer::setTime(float time)
{
    if (uniforms)
    {
        uniforms->setTime(time);
    }
}

void ModelRenderer::setUseHeatShimmer(bool use)
{
    if (uniforms)
    {
        uniforms->setUseHeatShimmer(use);
    }
}

void ModelRenderer::setHeatShimmerIntensity(float intensity)
{
    if (uniforms)
    {
        uniforms->setHeatShimmerIntensity(intensity);
    }
}
