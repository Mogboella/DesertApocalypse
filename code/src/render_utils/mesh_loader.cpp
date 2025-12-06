#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh_loader.h"
#include "texture_loader.h"
#include "glm_compat.h"
#include "transform_utils.h"

using namespace glm;
using namespace std;

namespace
{
    string getDirectory(const string &path)
    {
        size_t slash = path.find_last_of("/\\");
        if (slash == string::npos)
        {
            return "";
        }
        return path.substr(0, slash + 1);
    }

    bool isAbsolutePath(const string &path)
    {
        if (path.empty())
            return false;
        if (path[0] == '/' || path[0] == '\\')
            return true;
        if (path.size() > 1 && path[1] == ':')
            return true;
        return false;
    }

    string buildTexturePath(const string &modelPath, const string &texturePath)
    {
        if (texturePath.empty() || isAbsolutePath(texturePath))
        {
            return texturePath;
        }
        string directory = getDirectory(modelPath);
        return directory + texturePath;
    }

    GLuint loadEmbeddedTexture(const aiScene *scene, const string &texPath)
    {
        if (!scene)
            return 0;

        const aiTexture *embedded = scene->GetEmbeddedTexture(texPath.c_str());
        if (!embedded)
        {
            return 0;
        }

        if (embedded->mHeight == 0)
        {
            const unsigned char *data = reinterpret_cast<const unsigned char *>(embedded->pcData);
            size_t dataSize = static_cast<size_t>(embedded->mWidth);
            return LoadTextureFromMemory(data, dataSize, true, false, nullptr, nullptr, nullptr);
        }
        else
        {
            size_t pixelCount = static_cast<size_t>(embedded->mWidth) * static_cast<size_t>(embedded->mHeight);
            vector<unsigned char> pixels(pixelCount * 4);
            memcpy(pixels.data(), embedded->pcData, pixels.size());
            return UploadTextureFromPixels(pixels.data(), embedded->mWidth, embedded->mHeight, 4, true);
        }
    }

    GLuint loadMaterialTexture(const aiScene *scene,
                               const aiMaterial *material,
                               const string &modelPath)
    {
        if (!scene || !material)
            return 0;

        const aiTextureType textureTypes[] = {
            aiTextureType_DIFFUSE,
            aiTextureType_BASE_COLOR};

        for (aiTextureType type : textureTypes)
        {
            aiString texPath;
            if (material->GetTexture(type, 0, &texPath) != AI_SUCCESS)
            {
                continue;
            }

            string texPathStr = texPath.C_Str();
            if (!texPathStr.empty() && texPathStr[0] == '*')
            {
                GLuint tex = loadEmbeddedTexture(scene, texPathStr);
                if (tex != 0)
                {
                    return tex;
                }
                continue;
            }

            string fullPath = buildTexturePath(modelPath, texPathStr);
            GLuint tex = LoadTexture(fullPath.c_str(), true, true, nullptr, nullptr, nullptr);
            if (tex != 0)
            {
                return tex;
            }

            cerr << "loadMaterialTexture: failed to load external texture '"
                 << fullPath << "' referenced by '" << modelPath << "'\n";
        }

        return 0;
    }
}

static mat4 convertAiMatrix(const aiMatrix4x4 &aiMat)
{
    mat4 to;
    memcpy(glm::value_ptr(to), &aiMat, sizeof(mat4));
    return glm::transpose(to);
}

static int countNodes(aiNode *node)
{
    int count = 1;
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        count += countNodes(node->mChildren[i]);
    }
    return count;
}

static HierarchicalNode *buildNodeHierarchy(aiNode *aiNode, HierarchicalNode *parent,
                                            std::vector<HierarchicalNode> &nodeStorage,
                                            bool isRoot = false)
{
    HierarchicalNode node;
    node.name = std::string(aiNode->mName.C_Str());
    node.parent = parent;

    mat4 localTransform = convertAiMatrix(aiNode->mTransformation);
    node.localTransform = localTransform;
    node.currentTransform = localTransform;

    node.basePosition = vec3(0.0f, 0.0f, 0.0f);
    node.baseRotation = vec3(0.0f, 0.0f, 0.0f);
    node.baseScale = vec3(1.0f, 1.0f, 1.0f);
    node.animRotation = vec3(0.0f, 0.0f, 0.0f);
    node.animTranslation = vec3(0.0f, 0.0f, 0.0f);

    node.meshIndices.clear();

    for (unsigned int i = 0; i < aiNode->mNumMeshes; i++)
    {
        node.meshIndices.push_back(aiNode->mMeshes[i]);
    }

    nodeStorage.push_back(node);
    HierarchicalNode *storedNode = &nodeStorage.back();

    storedNode->children.clear();
    storedNode->children.reserve(aiNode->mNumChildren);

    for (unsigned int i = 0; i < aiNode->mNumChildren; i++)
    {
        HierarchicalNode *child = buildNodeHierarchy(
            aiNode->mChildren[i],
            storedNode,
            nodeStorage,
            false);
        storedNode->children.push_back(child);
    }

    return storedNode;
}

static void buildNodeNameMap(HierarchicalModel &result)
{
    for (size_t i = 0; i < result.nodes.size(); i++)
    {
        result.nodeNameMap[result.nodes[i].name] = i;
    }
}

static MeshInstance processMesh(const aiMesh *mesh, const aiScene *scene, const char *filePath)
{
    MeshInstance instance{};

    for (unsigned int v = 0; v < mesh->mNumVertices; v++)
    {
        instance.vertices.push_back(vec3(
            mesh->mVertices[v].x,
            mesh->mVertices[v].y,
            mesh->mVertices[v].z));

        if (mesh->HasNormals())
        {
            instance.normals.push_back(vec3(
                mesh->mNormals[v].x,
                mesh->mNormals[v].y,
                mesh->mNormals[v].z));
        }

        if (mesh->HasTextureCoords(0))
        {
            instance.texcoords.push_back(vec2(
                mesh->mTextureCoords[0][v].x,
                mesh->mTextureCoords[0][v].y));
        }
        else
        {
            instance.texcoords.push_back(vec2(0.0f, 0.0f));
        }

        if (mesh->HasVertexColors(0))
        {
            instance.colors.push_back(vec3(
                mesh->mColors[0][v].r,
                mesh->mColors[0][v].g,
                mesh->mColors[0][v].b));
        }
        else
        {
            instance.colors.push_back(vec3(1.0f, 1.0f, 1.0f));
        }
    }

    for (unsigned int f = 0; f < mesh->mNumFaces; f++)
    {
        aiFace face = mesh->mFaces[f];
        for (unsigned int i = 0; i < face.mNumIndices; i++)
        {
            instance.indices.push_back(face.mIndices[i]);
        }
    }

    instance.hasBones = false;
    instance.boneIds.resize(instance.vertices.size(), ivec4(-1, -1, -1, -1));
    instance.boneWeights.resize(instance.vertices.size(), vec4(0.0f, 0.0f, 0.0f, 0.0f));
    instance.boneMatrices.clear();
    instance.boneNameToIndex.clear();
    instance.VBO_boneIds = 0;
    instance.VBO_boneWeights = 0;

    const aiMaterial *material = (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < static_cast<int>(scene->mNumMaterials))
                                     ? scene->mMaterials[mesh->mMaterialIndex]
                                     : nullptr;
    instance.diffuseTexture = loadMaterialTexture(scene, material, filePath);
    instance.hasDiffuseTexture = instance.diffuseTexture != 0;

    return instance;
}

static void processBones(MeshInstance &instance, const aiMesh *mesh)
{
    if (!mesh->HasBones())
    {
        return;
    }

    instance.hasBones = true;

    for (unsigned int b = 0; b < mesh->mNumBones; b++)
    {
        const aiBone *bone = mesh->mBones[b];
        string boneName = string(bone->mName.C_Str());

        int boneIndex;
        auto it = instance.boneNameToIndex.find(boneName);
        if (it == instance.boneNameToIndex.end())
        {
            boneIndex = static_cast<int>(instance.boneMatrices.size());
            instance.boneNameToIndex[boneName] = boneIndex;
            instance.boneMatrices.push_back(convertAiMatrix(bone->mOffsetMatrix));
        }
        else
        {
            boneIndex = it->second;
        }

        for (unsigned int w = 0; w < bone->mNumWeights; w++)
        {
            const aiVertexWeight &weight = bone->mWeights[w];
            unsigned int vertexId = weight.mVertexId;
            float weightValue = weight.mWeight;

            if (vertexId >= instance.vertices.size())
                continue;

            for (int i = 0; i < MeshInstance::MAX_BONE_INFLUENCES; i++)
            {
                if (instance.boneWeights[vertexId][i] == 0.0f)
                {
                    instance.boneIds[vertexId][i] = boneIndex;
                    instance.boneWeights[vertexId][i] = weightValue;
                    break;
                }
            }
        }
    }

    for (size_t v = 0; v < instance.vertices.size(); v++)
    {
        float sum = instance.boneWeights[v].x + instance.boneWeights[v].y +
                    instance.boneWeights[v].z + instance.boneWeights[v].w;
        if (sum > 0.0f)
        {
            instance.boneWeights[v] /= sum;
        }
    }
}

static void createMeshBuffers(MeshInstance &instance)
{
    glGenVertexArrays(1, &instance.VAO);
    glBindVertexArray(instance.VAO);

    // Vertices
    glGenBuffers(1, &instance.VBO_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_vertices);
    glBufferData(GL_ARRAY_BUFFER,
                 instance.vertices.size() * sizeof(vec3),
                 instance.vertices.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Normals
    if (!instance.normals.empty())
    {
        glGenBuffers(1, &instance.VBO_normals);
        glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_normals);
        glBufferData(GL_ARRAY_BUFFER,
                     instance.normals.size() * sizeof(vec3),
                     instance.normals.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
    }

    // Texture coordinates
    if (!instance.texcoords.empty())
    {
        glGenBuffers(1, &instance.VBO_texcoords);
        glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_texcoords);
        glBufferData(GL_ARRAY_BUFFER,
                     instance.texcoords.size() * sizeof(vec2),
                     instance.texcoords.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(2);
    }

    // Colors
    if (!instance.colors.empty())
    {
        glGenBuffers(1, &instance.VBO_colors);
        glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_colors);
        glBufferData(GL_ARRAY_BUFFER,
                     instance.colors.size() * sizeof(vec3),
                     instance.colors.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(3);
    }

    // Bone IDs and weights
    if (instance.hasBones && !instance.boneIds.empty())
    {
        glGenBuffers(1, &instance.VBO_boneIds);
        glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_boneIds);
        glBufferData(GL_ARRAY_BUFFER,
                     instance.boneIds.size() * sizeof(ivec4),
                     instance.boneIds.data(),
                     GL_STATIC_DRAW);
        glVertexAttribIPointer(5, 4, GL_INT, 0, nullptr);
        glEnableVertexAttribArray(5);

        glGenBuffers(1, &instance.VBO_boneWeights);
        glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_boneWeights);
        glBufferData(GL_ARRAY_BUFFER,
                     instance.boneWeights.size() * sizeof(vec4),
                     instance.boneWeights.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(6);
    }

    // Indices
    glGenBuffers(1, &instance.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 instance.indices.size() * sizeof(unsigned int),
                 instance.indices.data(),
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
    instance.ready = true;
}

void MeshInstance::draw(GLuint shaderID, const mat4 &model, const mat4 &view, const mat4 &proj) const
{
    if (!ready)
        return;

    GLint locSampler = glGetUniformLocation(shaderID, "diffuseTexture");
    GLint locHasTexture = glGetUniformLocation(shaderID, "hasTexture");
    if (hasDiffuseTexture && diffuseTexture != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);
        if (locSampler >= 0)
        {
            glUniform1i(locSampler, 0);
        }
        if (locHasTexture >= 0)
        {
            glUniform1i(locHasTexture, 1);
        }
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (locHasTexture >= 0)
        {
            glUniform1i(locHasTexture, 0);
        }
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

static void loadAnimations(HierarchicalModel &result, const aiScene *scene)
{
    result.hasEmbeddedAnimation = scene->mNumAnimations > 0;
    result.activeAnimation = -1;
    result.currentAnimationTime = 0.0f;

    if (scene->mNumAnimations == 0)
    {
        cout << "No animations found" << endl;
        return;
    }

    for (unsigned int a = 0; a < scene->mNumAnimations; a++)
    {
        const aiAnimation *animation = scene->mAnimations[a];
        Animation clip;
        clip.name = animation->mName.C_Str();
        clip.duration = static_cast<float>(animation->mDuration);
        clip.ticksPerSecond = static_cast<float>(animation->mTicksPerSecond);

        for (unsigned int n = 0; n < animation->mNumChannels; n++)
        {
            const aiNodeAnim *nodeAnim = animation->mChannels[n];
            NodeAnimation nodeAnimClip;
            nodeAnimClip.nodeName = nodeAnim->mNodeName.C_Str();

            // Position keys
            for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++)
            {
                VectorKey key;
                key.time = static_cast<float>(nodeAnim->mPositionKeys[k].mTime);
                key.value = vec3(nodeAnim->mPositionKeys[k].mValue.x,
                                 nodeAnim->mPositionKeys[k].mValue.y,
                                 nodeAnim->mPositionKeys[k].mValue.z);
                nodeAnimClip.positionKeys.push_back(key);
            }

            // Rotation keys
            for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; k++)
            {
                QuaternionKey key;
                key.time = static_cast<float>(nodeAnim->mRotationKeys[k].mTime);
                key.value = quat(nodeAnim->mRotationKeys[k].mValue.w,
                                 nodeAnim->mRotationKeys[k].mValue.x,
                                 nodeAnim->mRotationKeys[k].mValue.y,
                                 nodeAnim->mRotationKeys[k].mValue.z);
                nodeAnimClip.rotationKeys.push_back(key);
            }

            // Scale keys
            for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; k++)
            {
                VectorKey key;
                key.time = static_cast<float>(nodeAnim->mScalingKeys[k].mTime);
                key.value = vec3(nodeAnim->mScalingKeys[k].mValue.x,
                                 nodeAnim->mScalingKeys[k].mValue.y,
                                 nodeAnim->mScalingKeys[k].mValue.z);
                nodeAnimClip.scaleKeys.push_back(key);
            }

            clip.nodeAnimations.push_back(nodeAnimClip);
        }
        result.animationClips.push_back(clip);
    }

    if (!result.animationClips.empty())
    {
        result.activeAnimation = 0;
        result.animated = true;
    }
}

vector<MeshInstance> load_mesh(const char *filePath)
{
    vector<MeshInstance> result;

    const aiScene *scene = aiImportFile(
        filePath,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_PreTransformVertices);

    if (!scene || !scene->mRootNode)
    {
        cerr << "Assimp error loading '" << filePath << "': "
             << aiGetErrorString() << endl;
        return result;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        const aiMesh *mesh = scene->mMeshes[m];
        MeshInstance instance{};

        for (unsigned int v = 0; v < mesh->mNumVertices; v++)
        {
            instance.vertices.push_back(vec3(
                mesh->mVertices[v].x,
                mesh->mVertices[v].y,
                mesh->mVertices[v].z));

            if (mesh->HasNormals())
            {
                instance.normals.push_back(vec3(
                    mesh->mNormals[v].x,
                    mesh->mNormals[v].y,
                    mesh->mNormals[v].z));
            }

            if (mesh->HasTextureCoords(0))
            {
                instance.texcoords.push_back(vec2(
                    mesh->mTextureCoords[0][v].x,
                    mesh->mTextureCoords[0][v].y));
            }
            else
            {
                instance.texcoords.push_back(vec2(0.0f, 0.0f));
            }

            if (mesh->HasVertexColors(0))
            {
                instance.colors.push_back(vec3(
                    mesh->mColors[0][v].r,
                    mesh->mColors[0][v].g,
                    mesh->mColors[0][v].b));
            }
            else
            {
                instance.colors.push_back(vec3(1.0f, 1.0f, 1.0f));
            }
        }

        for (unsigned int f = 0; f < mesh->mNumFaces; f++)
        {
            aiFace face = mesh->mFaces[f];
            for (unsigned int i = 0; i < face.mNumIndices; i++)
            {
                instance.indices.push_back(face.mIndices[i]);
            }
        }

        glGenVertexArrays(1, &instance.VAO);
        glBindVertexArray(instance.VAO);
        glGenBuffers(1, &instance.VBO_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_vertices);
        glBufferData(GL_ARRAY_BUFFER,
                     instance.vertices.size() * sizeof(vec3),
                     instance.vertices.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        if (!instance.normals.empty())
        {
            glGenBuffers(1, &instance.VBO_normals);
            glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_normals);
            glBufferData(GL_ARRAY_BUFFER,
                         instance.normals.size() * sizeof(vec3),
                         instance.normals.data(),
                         GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(1);
        }

        if (!instance.texcoords.empty())
        {
            glGenBuffers(1, &instance.VBO_texcoords);
            glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_texcoords);
            glBufferData(GL_ARRAY_BUFFER,
                         instance.texcoords.size() * sizeof(vec2),
                         instance.texcoords.data(),
                         GL_STATIC_DRAW);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(2);
        }

        if (!instance.colors.empty())
        {
            glGenBuffers(1, &instance.VBO_colors);
            glBindBuffer(GL_ARRAY_BUFFER, instance.VBO_colors);
            glBufferData(GL_ARRAY_BUFFER,
                         instance.colors.size() * sizeof(vec3),
                         instance.colors.data(),
                         GL_STATIC_DRAW);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(3);
        }

        glGenBuffers(1, &instance.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     instance.indices.size() * sizeof(unsigned int),
                     instance.indices.data(),
                     GL_STATIC_DRAW);

        glBindVertexArray(0);

        const aiMaterial *material = (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < static_cast<int>(scene->mNumMaterials))
                                         ? scene->mMaterials[mesh->mMaterialIndex]
                                         : nullptr;
        instance.diffuseTexture = loadMaterialTexture(scene, material, filePath);
        instance.hasDiffuseTexture = instance.diffuseTexture != 0;

        instance.ready = true;
        result.push_back(instance);
    }

    aiReleaseImport(scene);
    return result;
}

HierarchicalModel load_mesh_hierarchical(const char *filePath)
{
    HierarchicalModel result;
    result.modelIndex = -1;
    result.worldPosition = vec3(0.0f, 0.0f, 0.0f);
    result.worldRotation = vec3(0.0f, 0.0f, 0.0f);
    result.worldScale = vec3(1.0f, 1.0f, 1.0f);
    result.animated = false;

    const aiScene *scene = aiImportFile(
        filePath,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || !scene->mRootNode)
    {
        cerr << "Assimp error loading hierarchical model '" << filePath << "': "
             << aiGetErrorString() << endl;
        return result;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        const aiMesh *mesh = scene->mMeshes[m];
        MeshInstance instance = processMesh(mesh, scene, filePath);
        processBones(instance, mesh);
        createMeshBuffers(instance);

        result.meshes.push_back(instance);
    }

    int totalNodes = countNodes(scene->mRootNode);
    result.nodes.clear();
    result.nodes.reserve(totalNodes);
    result.rootNode = buildNodeHierarchy(scene->mRootNode, nullptr, result.nodes, true);

    if (result.rootNode)
    {
        mat4 rootTransform = result.rootNode->localTransform;
        result.originalRootTransform = rootTransform;
        result.globalInverseTransform = glm::inverse(rootTransform);
    }
    else
    {
        result.originalRootTransform = mat4(1.0f);
        result.globalInverseTransform = mat4(1.0f);
    }

    loadAnimations(result, scene);
    buildNodeNameMap(result);
    aiReleaseImport(scene);

    return result;
}

void cleanupMesh(MeshInstance &mesh)
{
    if (mesh.VBO_vertices)
        glDeleteBuffers(1, &mesh.VBO_vertices);
    if (mesh.VBO_normals)
        glDeleteBuffers(1, &mesh.VBO_normals);
    if (mesh.VBO_texcoords)
        glDeleteBuffers(1, &mesh.VBO_texcoords);
    if (mesh.VBO_colors)
        glDeleteBuffers(1, &mesh.VBO_colors);
    if (mesh.VBO_boneIds)
        glDeleteBuffers(1, &mesh.VBO_boneIds);
    if (mesh.VBO_boneWeights)
        glDeleteBuffers(1, &mesh.VBO_boneWeights);
    if (mesh.VAO)
        glDeleteVertexArrays(1, &mesh.VAO);
    mesh.boneIds.clear();
    mesh.boneWeights.clear();
    mesh.boneMatrices.clear();
    mesh.boneNameToIndex.clear();
    mesh.hasBones = false;
}

void cleanupHierarchicalModel(HierarchicalModel &model)
{
    for (MeshInstance &mesh : model.meshes)
    {
        cleanupMesh(mesh);
    }
    model.nodes.clear();
    model.nodeNameMap.clear();
    model.animationClips.clear();
    model.activeAnimation = -1;
    model.currentAnimationTime = 0.0f;
}
