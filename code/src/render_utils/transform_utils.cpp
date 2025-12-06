#include "transform_utils.h"

using namespace glm;

vec3 extractTranslation(const mat4 &m)
{
    // column-major: last column is translation
    return vec3(m[3][0], m[3][1], m[3][2]);
}

quat extractRotation(const mat4 &m)
{
    vec3 scale = extractScale(m);

    mat4 rotMat = m;
    rotMat[0] /= scale.x;
    rotMat[1] /= scale.y;
    rotMat[2] /= scale.z;

    rotMat[3] = vec4(0, 0, 0, 1);

    return glm::quat_cast(rotMat);
}

vec3 extractScale(const mat4 &m)
{
    // column vectors for X, Y, Z axes
    vec3 col0 = vec3(m[0][0], m[0][1], m[0][2]);
    vec3 col1 = vec3(m[1][0], m[1][1], m[1][2]);
    vec3 col2 = vec3(m[2][0], m[2][1], m[2][2]);

    float sx = length(col0);
    float sy = length(col1);
    float sz = length(col2);

    return vec3(sx, sy, sz);
}
