#ifndef ANIMATE_H
#define ANIMATE_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "mesh_loader.h"

using namespace std;
using namespace glm;

void animateNodeRecursive(HierarchicalNode *node, float animTime, int depth, const vector<HierarchicalNode> &allNodes);
void setHierarchicalActiveAnimation(int hierarchicalIndex, int animationIndex, float speedMultiplier = 1.0f);

template <typename T>
int setAnimationKeyframeIndex(const vector<T> &keys, float time, bool loop);
vec3 setPositionKeyframe(const vector<VectorKey> &keys, float time, bool loop);
quat setRotationKeyframe(const vector<QuaternionKey> &keys, float time, bool loop);
vec3 setScaleKeyframe(const vector<VectorKey> &keys, float time, bool loop);
#endif
