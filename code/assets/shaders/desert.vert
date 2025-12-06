#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 vertexColor;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool hasBones;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec3 VertexColor;

void main()
{
    vec4 localPos = vec4(position, 1.0);
    vec3 localNormal = normal;

    vec4 skinnedPos = vec4(0.0);
    vec3 skinnedNorm = vec3(0.0);

    if (hasBones)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            int id = boneIds[i];
            float w = weights[i];

            if (w <= 0.0) continue;
            if (id < 0) continue;
            if (id >= MAX_BONES) continue;

            mat4 bone = finalBonesMatrices[id];

            skinnedPos += (bone * localPos) * w;
            skinnedNorm += mat3(bone) * localNormal * w;
        }
    }
    else
    {
        skinnedPos = localPos;
        skinnedNorm = localNormal;
    }

    vec4 worldPosition = model * skinnedPos;
    FragPos = worldPosition.xyz;

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * skinnedNorm);

    TexCoord = texCoord;
    VertexColor = vertexColor;
    gl_Position = projection * view * worldPosition;
}
