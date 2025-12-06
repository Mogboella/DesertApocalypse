#ifndef TRANSFORM_UTILS
#define TRANSFORM_UTILS

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

vec3 extractTranslation(const mat4 &m);
vec3 extractScale(const mat4 &m);
quat extractRotation(const mat4 &m);

#endif
