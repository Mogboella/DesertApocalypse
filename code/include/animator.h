#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "mesh_loader.h"

using namespace glm;
using namespace std;

class Animator
{
private:
    HierarchicalModel *model;

    int currentAnimationIndex;
    int nextAnimationIndex;
    int queueAnimationIndex;
    float currentTime;
    bool interpolating;
    float haltTime;
    float interTime;
    float speedMultiplier;


    vector<vector<mat4>> finalBoneMatricesPerMesh;

    map<size_t, mat4> globalTransforms;

    void calculateBoneTransform(
        HierarchicalNode *node,
        const mat4 &parentTransform,
        int animationIndex,
        float animTime);

    void calculateBoneTransition(
        HierarchicalNode *node,
        const mat4 &parentTransform,
        int prevAnimationIndex,
        int nextAnimationIndex,
        float haltTime,
        float currentInterTime,
        float transitionTime);

    vec3 interpolateVectorKey(const vector<VectorKey> &keys, float time, bool loop = true);
    quat interpolateQuatKey(const vector<QuaternionKey> &keys, float time, bool loop = true);

    void buildFinalBoneMatrices();

public:
    Animator(HierarchicalModel *hmodel);

    void updateAnimation(float deltaTime);

    void playAnimation(int animationIndex, bool repeat = true);

    const vector<vector<mat4>> &getFinalBoneMatrices() const { return finalBoneMatricesPerMesh; }

    int getCurrentAnimation() const { return currentAnimationIndex; }
    float getCurrentTime() const { return currentTime; }

    void setActiveAnimation(int animationIndex);

    void setSpeedMultiplier(float speed) { speedMultiplier = speed; }
    float getSpeedMultiplier() const { return speedMultiplier; }
};

#endif
