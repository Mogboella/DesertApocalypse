#include <cmath>
#include <algorithm>
#include <iostream>

#include "animate.h"
#include "animator.h"
#include "mesh_loader.h"
#include "glm_compat.h"
#include "scene_manager.h"
#include "hierarchy_utils.h"

using namespace std;
using namespace glm;

void animateNodeRecursive(HierarchicalNode *node, float animTime, int depth, const vector<HierarchicalNode> &allNodes)
{
    if (!node)
        return;

    if (!isValidNodePointer(node, allNodes))
        return;

    if (depth > 50)
        return;

    bool isRoot = node->name.find("_rootJoint") != string::npos;
    bool planeBody = (node->name == "body");
    bool isWings = node->name.find("wing") != string::npos;

    if (isRoot)
    {
        node->animRotation = vec3(0.0f, 0.0f, 0.0f);
    }
    else if (isWings) {
        float flapSpeed = 10.0f;
        float flapAngle = 25.0f;
        float phase = animTime * flapSpeed;

        bool isWing2 = node->name.find("wing2") != string::npos;
        float direction = isWing2 ? -1.0f : 1.0f;
        float xRot = sin(phase) * flapAngle * direction;
        node->animRotation = vec3(xRot, 0.0f, 0.0f);
    }
    else
    {
        node->animRotation = vec3(0.0f, 0.0f, 0.0f);
    }

    size_t childCount = node->children.size();
    for (size_t i = 0; i < childCount; i++)
    {
        if (i >= node->children.size())
            break;

        HierarchicalNode *child = node->children[i];
        if (child && isValidNodePointer(child, allNodes))
        {
            animateNodeRecursive(child, animTime, depth + 1, allNodes);
        }
    }
};

void setHierarchicalActiveAnimation(int hierarchicalIndex, int animationIndex, float speedMultiplier)
{
    if (hierarchicalIndex < 0 || hierarchicalIndex >= (int)hierarchicalModels.size())
        return;

    HierarchicalModel &hmodel = hierarchicalModels[hierarchicalIndex];

    if (animationIndex < 0 || animationIndex >= (int)hmodel.animationClips.size())
        return;

    // Use animator if available, otherwise fall back to direct setting
    static map<size_t, Animator *> animators;

    if (animators.find(hierarchicalIndex) == animators.end())
    {
        animators[hierarchicalIndex] = new Animator(&hmodel);
    }

    animators[hierarchicalIndex]->setActiveAnimation(animationIndex);
    animators[hierarchicalIndex]->setSpeedMultiplier(speedMultiplier);

    hmodel.activeAnimation = animationIndex;
    hmodel.currentAnimationTime = 0.0f;
};

template <typename T>
int setAnimationKeyframeIndex(const vector<T> &keys, float time, bool loop)
{
    if (keys.empty())
        return -1;
    if (keys.size() == 1)
        return 0;

    float duration = keys.back().time;
    if (loop && duration > 0.0f)
    {
        time = fmod(time, duration);
    }

    for (size_t i = 0; i < keys.size() - 1; i++)
    {
        if (time >= keys[i].time && time <= keys[i + 1].time)
        {
            return static_cast<int>(i);
        }
    }

    return static_cast<int>(keys.size() - 2);
};

vec3 setPositionKeyframe(const vector<VectorKey> &keys, float time, bool loop)
{
    if (keys.empty())
        return vec3(0.0f);
    if (keys.size() == 1)
        return keys[0].value;

    int idx = setAnimationKeyframeIndex(keys, time, loop);
    if (idx < 0 || idx >= static_cast<int>(keys.size() - 1))
        return keys.back().value;

    const VectorKey &k0 = keys[idx];
    const VectorKey &k1 = keys[idx + 1];

    float t = (time - k0.time) / (k1.time - k0.time);
    return mix(k0.value, k1.value, t); // Linear interpolation
};

quat setRotationKeyframe(const vector<QuaternionKey> &keys, float time, bool loop)
{
    if (keys.empty())
        return quat(1.0f, 0.0f, 0.0f, 0.0f);
    if (keys.size() == 1)
        return keys[0].value;

    int idx = setAnimationKeyframeIndex(keys, time, loop);
    if (idx < 0 || idx >= static_cast<int>(keys.size() - 1))
        return keys.back().value;

    const QuaternionKey &k0 = keys[idx];
    const QuaternionKey &k1 = keys[idx + 1];

    float t = (time - k0.time) / (k1.time - k0.time);
    return slerp(k0.value, k1.value, t); // Spherical linear interpolation
};

vec3 setScaleKeyframe(const vector<VectorKey> &keys, float time, bool loop)
{
    if (keys.empty())
        return vec3(1.0f);
    if (keys.size() == 1)
        return keys[0].value;

    int idx = setAnimationKeyframeIndex(keys, time, loop);
    if (idx < 0 || idx >= static_cast<int>(keys.size() - 1))
        return keys.back().value;

    const VectorKey &k0 = keys[idx];
    const VectorKey &k1 = keys[idx + 1];

    float t = (time - k0.time) / (k1.time - k0.time);
    return mix(k0.value, k1.value, t);
};
