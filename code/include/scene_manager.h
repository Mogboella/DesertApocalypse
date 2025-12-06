#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "mesh_loader.h"

using namespace std;
using namespace glm;

struct MeshTransform
{
    vec3 position;
    vec3 rotation;
    vec3 scale;
    bool animated;
    float animTime;
};

struct ModelRange
{
    size_t start;
    size_t count;
};

extern vector<MeshInstance> meshes;
extern vector<MeshTransform> meshTransforms;
extern vector<HierarchicalModel> hierarchicalModels;

void setupScene(GLuint shaderProgramID);
bool getModelRange(int modelIndex, int &start, int &count);
void addMesh(const char *filePath);
void addHierarchicalMesh(const char *filePath);
void setMeshTransform(int modelIndex, vec3 position, vec3 rotation, vec3 scale, bool animated = false);
void setSubmeshTransform(int meshGlobalIndex, vec3 position, vec3 rotation, vec3 scale, bool animated = false);
void setHierarchicalMeshTransform(int hierarchicalIndex, vec3 position, vec3 rotation, vec3 scale, bool animated = false);
void setOrbitalMotion(int parentIndex, int childIndex);

#endif
