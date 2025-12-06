#include <iostream>

#include "mesh_loader.h"
#include "glm_compat.h"
#include "scene_manager.h"
#include "hierarchy_utils.h"

using namespace std;
using namespace glm;

static GLuint shaderProgram = 0;

vector<MeshInstance> meshes;
vector<HierarchicalModel> hierarchicalModels;
vector<ModelRange> modelRanges;
vector<MeshTransform> meshTransforms;
vector<MeshTransform> localTransforms;

void setupScene(GLuint shaderProgramID)
{
    shaderProgram = shaderProgramID;
    meshes.clear();
    meshTransforms.clear();
    modelRanges.clear();
}

bool getModelRange(int modelIndex, int &start, int &count)
{
    if (modelIndex < 0 || modelIndex >= (int)modelRanges.size())
        return false;
    start = (int)modelRanges[modelIndex].start;
    count = (int)modelRanges[modelIndex].count;
    return true;
}

void addMesh(const char *filePath)
{
    vector<MeshInstance> loaded = load_mesh(filePath);
    if (loaded.empty())
    {
        cerr << "addMesh: failed to load '" << filePath << "'" << endl;
        return;
    }

    size_t start = meshes.size();

    for (auto &mesh : loaded)
    {
        meshes.push_back(mesh);

        MeshTransform t{};
        t.position = vec3(0.0f, 0.0f, 0.0f);
        t.rotation = vec3(0.0f, 0.0f, 0.0f);
        t.scale = vec3(1.0f, 1.0f, 1.0f);
        t.animated = false;
        t.animTime = 0.0f;
        meshTransforms.push_back(t);
    }

    size_t count = loaded.size();
    modelRanges.push_back({start, count});
}

void addHierarchicalMesh(const char *filePath)
{
    HierarchicalModel hmodel = load_mesh_hierarchical(filePath);
    if (hmodel.meshes.empty() || !hmodel.rootNode)
    {
        cerr << "addHierarchicalMesh: failed to load '" << filePath << "'" << endl;
        return;
    }

    hmodel.modelIndex = hierarchicalModels.size();

    hmodel.useOrbitalMotion = false;
    hmodel.orbitalParentIdx = -1;
    hmodel.orbitalChildIdx = -1;

    vector<pair<string, string>> parentChildPairs;
    string rootName = "";
    collectHierarchy(hmodel.rootNode, parentChildPairs, rootName);

    hierarchicalModels.push_back(hmodel);
    rebuildNodePointers(hierarchicalModels.back(), parentChildPairs, rootName);
}

void setMeshTransform(int modelIndex, vec3 position, vec3 rotation, vec3 scale, bool animated)
{
    if (modelIndex < 0 || modelIndex >= (int)modelRanges.size())
        return;
    ModelRange r = modelRanges[modelIndex];
    for (size_t i = r.start; i < r.start + r.count; ++i)
    {
        meshTransforms[i].position = position;
        meshTransforms[i].rotation = rotation;
        meshTransforms[i].scale = scale;
        meshTransforms[i].animated = animated;
    }
}

void setSubmeshTransform(int meshGlobalIndex, vec3 position, vec3 rotation, vec3 scale, bool animated)
{
    if (meshGlobalIndex < 0 || meshGlobalIndex >= (int)localTransforms.size())
        return;
    localTransforms[meshGlobalIndex].position = position;
    localTransforms[meshGlobalIndex].rotation = rotation;
    localTransforms[meshGlobalIndex].scale = scale;
    localTransforms[meshGlobalIndex].animated = animated;
}

void setHierarchicalMeshTransform(int hierarchicalIndex, vec3 position, vec3 rotation, vec3 scale, bool animated)
{
    if (hierarchicalIndex < 0 || hierarchicalIndex >= (int)hierarchicalModels.size())
    {
        cerr << "setHierarchicalMeshTransform: invalid index " << hierarchicalIndex << endl;
        return;
    }

    hierarchicalModels[hierarchicalIndex].worldPosition = position;
    hierarchicalModels[hierarchicalIndex].worldRotation = rotation;
    hierarchicalModels[hierarchicalIndex].worldScale = scale;
    hierarchicalModels[hierarchicalIndex].animated = animated;
}

void setOrbitalMotion(int parentIndex, int childIndex)
{
    if (parentIndex >= 0 && parentIndex < (int)hierarchicalModels.size() &&
        childIndex >= 0 && childIndex < (int)hierarchicalModels.size())
    {
        hierarchicalModels[parentIndex].useOrbitalMotion = true;
        hierarchicalModels[parentIndex].orbitalParentIdx = parentIndex;
        hierarchicalModels[parentIndex].orbitalChildIdx = childIndex;

        hierarchicalModels[childIndex].useOrbitalMotion = true;
        hierarchicalModels[childIndex].orbitalParentIdx = parentIndex;
        hierarchicalModels[childIndex].orbitalChildIdx = childIndex;
    }
}

void cleanupScene()
{

    for (MeshInstance &mesh : meshes)
    {
        cleanupMesh(mesh);
    }

    for (HierarchicalModel &model : hierarchicalModels)
    {
        cleanupHierarchicalModel(model);
    }

    meshes.clear();
    meshTransforms.clear();
    hierarchicalModels.clear();
    modelRanges.clear();
    localTransforms.clear();
    shaderProgram = 0;
    cout << "Scene cleaned up." << endl;
}
