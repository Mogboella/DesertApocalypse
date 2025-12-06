#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "animator.h"
#include "transform_utils.h"
#include "glm_compat.h"
#include "mesh_loader.h"

using namespace glm;
using namespace std;

Animator::Animator(HierarchicalModel *hmodel)
    : model(hmodel), currentAnimationIndex(-1), nextAnimationIndex(-1), queueAnimationIndex(-1), currentTime(0.0f), interpolating(false), haltTime(0.0f), interTime(0.0f), speedMultiplier(1.0f)
{
    if (model && !model->meshes.empty())
    {
        finalBoneMatricesPerMesh.resize(model->meshes.size());
        for (size_t i = 0; i < model->meshes.size(); i++)
        {
            if (model->meshes[i].hasBones)
            {
                finalBoneMatricesPerMesh[i].resize(
                    model->meshes[i].boneMatrices.size(),
                    mat4(1.0f));
            }
        }

        if (model->hasEmbeddedAnimation &&
            model->activeAnimation >= 0 &&
            model->activeAnimation < (int)model->animationClips.size())
        {
            currentAnimationIndex = model->activeAnimation;
            currentTime = 0.0f;
        }
    }
}

void Animator::updateAnimation(float deltaTime)
{
    if (!model || currentAnimationIndex < 0 ||
        currentAnimationIndex >= (int)model->animationClips.size())
    {
        return;
    }

    const Animation &currentAnim = model->animationClips[currentAnimationIndex];
    float ticksPerSecond = currentAnim.ticksPerSecond > 0.0f
                               ? currentAnim.ticksPerSecond
                               : 25.0f;

    currentTime = fmod(currentTime + ticksPerSecond * deltaTime * speedMultiplier, currentAnim.duration);

    float transitionTime = ticksPerSecond * 0.2f;

    if (interpolating && interTime <= transitionTime)
    {
        interTime += ticksPerSecond * deltaTime;

        globalTransforms.clear();
        mat4 rootParentTransform = model->originalRootTransform;
        calculateBoneTransition(
            model->rootNode,
            rootParentTransform,
            currentAnimationIndex,
            nextAnimationIndex,
            haltTime,
            interTime,
            transitionTime);

        buildFinalBoneMatrices();
        return;
    }
    else if (interpolating)
    {
        if (queueAnimationIndex >= 0)
        {
            currentAnimationIndex = nextAnimationIndex;
            haltTime = 0.0f;
            nextAnimationIndex = queueAnimationIndex;
            queueAnimationIndex = -1;
            currentTime = 0.0f;
            interTime = 0.0f;
            return;
        }

        interpolating = false;
        currentAnimationIndex = nextAnimationIndex;
        currentTime = 0.0f;
        interTime = 0.0f;
    }

    globalTransforms.clear();
    mat4 rootParentTransform = model->originalRootTransform;
    calculateBoneTransform(
        model->rootNode,
        rootParentTransform,
        currentAnimationIndex,
        currentTime);

    buildFinalBoneMatrices();
}

void Animator::playAnimation(int animationIndex, bool repeat)
{
    if (!model || animationIndex < 0 ||
        animationIndex >= (int)model->animationClips.size())
    {
        return;
    }

    if (currentAnimationIndex < 0)
    {
        currentAnimationIndex = animationIndex;
        currentTime = 0.0f;
        return;
    }

    if (interpolating)
    {
        if (animationIndex != nextAnimationIndex)
        {
            queueAnimationIndex = animationIndex;
        }
    }
    else
    {
        if (animationIndex != currentAnimationIndex)
        {
            interpolating = true;
            haltTime = fmod(currentTime, model->animationClips[currentAnimationIndex].duration);
            nextAnimationIndex = animationIndex;
            currentTime = 0.0f;
            interTime = 0.0f;
        }
    }
}

void Animator::setActiveAnimation(int animationIndex)
{
    if (!model || animationIndex < 0 ||
        animationIndex >= (int)model->animationClips.size())
    {
        return;
    }

    currentAnimationIndex = animationIndex;
    currentTime = 0.0f;
    interpolating = false;
    nextAnimationIndex = -1;
    queueAnimationIndex = -1;
}

void Animator::calculateBoneTransform(
    HierarchicalNode *node,
    const mat4 &parentTransform,
    int animationIndex,
    float animTime)
{
    if (!node || !model || animationIndex < 0)
        return;

    const Animation &anim = model->animationClips[animationIndex];
    string nodeName = node->name;

    mat4 nodeTransform = node->localTransform;

    const NodeAnimation *nodeAnim = nullptr;
    for (const auto &channel : anim.nodeAnimations)
    {
        if (channel.nodeName == nodeName)
        {
            nodeAnim = &channel;
            break;
        }
    }

    if (nodeAnim)
    {
        vec3 position = interpolateVectorKey(nodeAnim->positionKeys, animTime);
        quat rotation = interpolateQuatKey(nodeAnim->rotationKeys, animTime);
        vec3 scale = interpolateVectorKey(nodeAnim->scaleKeys, animTime);

        mat4 T = glm::translate(mat4(1.0f), position);
        mat4 R = glm::mat4_cast(rotation);
        mat4 S = glm::scale(mat4(1.0f), scale);

        nodeTransform = T * R * S;
    }

    mat4 globalTransform = parentTransform * nodeTransform;

    auto it = model->nodeNameMap.find(nodeName);
    if (it != model->nodeNameMap.end())
    {
        size_t nodeIdx = it->second;
        globalTransforms[nodeIdx] = globalTransform;
    }

    for (HierarchicalNode *child : node->children)
    {
        if (child)
        {
            calculateBoneTransform(child, globalTransform, animationIndex, animTime);
        }
    }
}

void Animator::calculateBoneTransition(
    HierarchicalNode *node,
    const mat4 &parentTransform,
    int prevAnimationIndex,
    int nextAnimationIndex,
    float haltTime,
    float currentInterTime,
    float transitionTime)
{
    if (!node || !model || prevAnimationIndex < 0 || nextAnimationIndex < 0)
        return;

    const Animation &prevAnim = model->animationClips[prevAnimationIndex];
    const Animation &nextAnim = model->animationClips[nextAnimationIndex];
    string nodeName = node->name;

    mat4 nodeTransform = node->localTransform;

    const NodeAnimation *prevNodeAnim = nullptr;
    const NodeAnimation *nextNodeAnim = nullptr;

    for (const auto &channel : prevAnim.nodeAnimations)
    {
        if (channel.nodeName == nodeName)
        {
            prevNodeAnim = &channel;
            break;
        }
    }

    for (const auto &channel : nextAnim.nodeAnimations)
    {
        if (channel.nodeName == nodeName)
        {
            nextNodeAnim = &channel;
            break;
        }
    }

    if (prevNodeAnim && nextNodeAnim)
    {
        vec3 prevPos = interpolateVectorKey(prevNodeAnim->positionKeys, haltTime);
        quat prevRot = interpolateQuatKey(prevNodeAnim->rotationKeys, haltTime);
        vec3 prevScale = interpolateVectorKey(prevNodeAnim->scaleKeys, haltTime);

        vec3 nextPos = interpolateVectorKey(nextNodeAnim->positionKeys, 0.0f);
        quat nextRot = interpolateQuatKey(nextNodeAnim->rotationKeys, 0.0f);
        vec3 nextScale = interpolateVectorKey(nextNodeAnim->scaleKeys, 0.0f);

        float t = glm::clamp(currentInterTime / transitionTime, 0.0f, 1.0f);

        vec3 position = glm::mix(prevPos, nextPos, t);
        quat rotation = glm::slerp(prevRot, nextRot, t);
        vec3 scale = glm::mix(prevScale, nextScale, t);

        mat4 T = glm::translate(mat4(1.0f), position);
        mat4 R = glm::mat4_cast(rotation);
        mat4 S = glm::scale(mat4(1.0f), scale);

        nodeTransform = T * R * S;
    }

    mat4 globalTransform = parentTransform * nodeTransform;

    auto it = model->nodeNameMap.find(nodeName);
    if (it != model->nodeNameMap.end())
    {
        size_t nodeIdx = it->second;
        globalTransforms[nodeIdx] = globalTransform;
    }

    for (HierarchicalNode *child : node->children)
    {
        if (child)
        {
            calculateBoneTransition(
                child, globalTransform,
                prevAnimationIndex, nextAnimationIndex,
                haltTime, currentInterTime, transitionTime);
        }
    }
}

vec3 Animator::interpolateVectorKey(const vector<VectorKey> &keys, float time, bool loop)
{
    if (keys.empty())
        return vec3(0.0f);

    if (keys.size() == 1)
        return keys[0].value;

    float animTime = loop ? fmod(time, keys.back().time) : time;

    for (size_t i = 0; i < keys.size() - 1; i++)
    {
        if (animTime <= keys[i + 1].time)
        {
            float deltaTime = keys[i + 1].time - keys[i].time;
            if (deltaTime < 0.001f)
                return keys[i].value;

            float factor = (animTime - keys[i].time) / deltaTime;
            return glm::mix(keys[i].value, keys[i + 1].value, factor);
        }
    }

    return keys.back().value;
}

quat Animator::interpolateQuatKey(const vector<QuaternionKey> &keys, float time, bool loop)
{
    if (keys.empty())
        return quat(1.0f, 0.0f, 0.0f, 0.0f);

    if (keys.size() == 1)
        return keys[0].value;

    float animTime = loop ? fmod(time, keys.back().time) : time;

    for (size_t i = 0; i < keys.size() - 1; i++)
    {
        if (animTime <= keys[i + 1].time)
        {
            float deltaTime = keys[i + 1].time - keys[i].time;
            if (deltaTime < 0.001f)
                return keys[i].value;

            float factor = (animTime - keys[i].time) / deltaTime;
            return glm::slerp(keys[i].value, keys[i + 1].value, factor);
        }
    }

    return keys.back().value;
}

void Animator::buildFinalBoneMatrices()
{
    if (!model)
        return;

    finalBoneMatricesPerMesh.resize(model->meshes.size());

    for (size_t meshIdx = 0; meshIdx < model->meshes.size(); meshIdx++)
    {
        MeshInstance &mesh = model->meshes[meshIdx];

        if (!mesh.hasBones || mesh.boneMatrices.empty())
        {
            finalBoneMatricesPerMesh[meshIdx].clear();
            continue;
        }

        finalBoneMatricesPerMesh[meshIdx].resize(
            mesh.boneMatrices.size(),
            mat4(1.0f));

        for (const auto &pair : mesh.boneNameToIndex)
        {
            const string &boneName = pair.first;
            int boneIndex = pair.second;

            if (boneIndex < 0 || boneIndex >= (int)mesh.boneMatrices.size())
                continue;

            auto nodeIt = model->nodeNameMap.find(boneName);
            if (nodeIt == model->nodeNameMap.end())
                continue;

            size_t nodeIdx = nodeIt->second;
            auto globalIt = globalTransforms.find(nodeIdx);
            if (globalIt == globalTransforms.end())
                continue;

            mat4 globalBoneTransform = globalIt->second;
            mat4 inverseBindPose = mesh.boneMatrices[boneIndex];

            finalBoneMatricesPerMesh[meshIdx][boneIndex] =
                model->globalInverseTransform * globalBoneTransform * inverseBindPose;
        }
    }

    model->boneMatricesPerMesh = finalBoneMatricesPerMesh;
}
