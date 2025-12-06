// This is a file to translate functions left behind from lab4_a to GLM equivalents.

#ifndef GLM_COMPAT_H
#define GLM_COMPAT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

#define ONE_DEG_IN_RAD (3.14159265f / 180.0f)

inline glm::mat4 identity_mat4() { return glm::mat4(1.0f); }

inline void print(const glm::mat4 &m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            std::cout << m[j][i] << " ";
        }
        std::cout << "\n";
    }
}

using std::max;
using std::min;

inline glm::mat4 rotate_x_deg(const glm::mat4 &m, float degrees)
{
    return glm::rotate(m, glm::radians(degrees), glm::vec3(1.0f, 0.0f, 0.0f));
}
inline glm::mat4 rotate_y_deg(const glm::mat4 &m, float degrees)
{
    return glm::rotate(m, glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
}
inline glm::mat4 rotate_z_deg(const glm::mat4 &m, float degrees)
{
    return glm::rotate(m, glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
}

#endif
