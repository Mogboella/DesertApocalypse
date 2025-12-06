#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <iostream>
#include <functional>
#include <map>

using namespace std;
using namespace glm;

struct HierarchicalNode;
struct HierarchicalModel;
struct MeshInstance;
struct AnimationRule;

void cleanupMesh(MeshInstance &mesh);
vector<MeshInstance> load_mesh(const char *filePath);

void cleanupHierarchicalModel(HierarchicalModel &model);
HierarchicalModel load_mesh_hierarchical(const char *filePath);

struct VectorKey
{
    float time;
    vec3 value;
};

struct QuaternionKey
{
    float time;
    quat value;
};

struct NodeAnimation
{
    string nodeName;
    vector<VectorKey> positionKeys;
    vector<QuaternionKey> rotationKeys;
    vector<VectorKey> scaleKeys;
};

struct Animation
{
    string name;
    vector<NodeAnimation> nodeAnimations;
    float duration;
    float ticksPerSecond;
};

struct MeshInstance {
    GLuint VAO;
    GLuint VBO_vertices;
    GLuint VBO_normals;
    GLuint VBO_texcoords;
    GLuint EBO;

    vector<vec3> vertices;
    vector<vec3> normals;
    vector<vec2> texcoords;

    vector<unsigned int> indices;

    vector<vec3> colors;
    GLuint VBO_colors;
    GLuint diffuseTexture;
    bool hasDiffuseTexture;

    static const int MAX_BONE_INFLUENCES = 4;
    static const int MAX_BONES = 200;

    vector<ivec4> boneIds;
    vector<vec4> boneWeights;

    vector<mat4> boneMatrices;
    map<string, int> boneNameToIndex;
    bool hasBones;

    GLuint VBO_boneIds;
    GLuint VBO_boneWeights;

    bool ready;

    void draw(GLuint shaderID, const mat4 &model, const mat4 &view, const mat4 &proj) const;
};

struct HierarchicalNode
{
    string name;
    mat4 localTransform;
    mat4 currentTransform;

    vector<unsigned int> meshIndices;
    vector<HierarchicalNode *> children;
    HierarchicalNode *parent;

    // Animation parameters
    vec3 basePosition;
    vec3 baseRotation;
    vec3 baseScale;
    vec3 animRotation;
    vec3 animTranslation;

    mat4 animationTransform;
    bool hasAnimationTransform;
};

struct HierarchicalModel
{
    vector<MeshInstance> meshes;
    vector<HierarchicalNode> nodes;
    HierarchicalNode *rootNode;
    int modelIndex;
    vec3 worldPosition;
    vec3 worldRotation;
    vec3 worldScale;
    bool animated;

    vector<Animation> animationClips;
    int activeAnimation;
    float currentAnimationTime;
    bool hasEmbeddedAnimation;

    mat4 globalInverseTransform;
    mat4 originalRootTransform;

    vector<vector<mat4>> boneMatricesPerMesh;

    map<string, size_t> nodeNameMap;

    bool useOrbitalMotion;
    int orbitalParentIdx;
    int orbitalChildIdx;
};

#endif
