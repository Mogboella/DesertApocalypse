#version 330

layout(location = 0) in vec2 xz;
layout(location = 1) in vec2 uv;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform sampler2D uHeightmap;
uniform float uHeightScale;
uniform vec2 uTexelSize;
uniform float uCellSize;

out vec2 vUV;
out vec3 vWorldPos;
out vec3 vNormal;

float sampleHeight(vec2 texCoord)
{
    return texture(uHeightmap, texCoord).r * uHeightScale;
}

vec3 computeNormal(vec2 texCoord)
{
    vec2 offset = uTexelSize;
    vec2 minUV = vec2(0.0);
    vec2 maxUV = vec2(1.0);

    float hL = sampleHeight(clamp(texCoord - vec2(offset.x, 0.0), minUV, maxUV));
    float hR = sampleHeight(clamp(texCoord + vec2(offset.x, 0.0), minUV, maxUV));
    float hD = sampleHeight(clamp(texCoord - vec2(0.0, offset.y), minUV, maxUV));
    float hU = sampleHeight(clamp(texCoord + vec2(0.0, offset.y), minUV, maxUV));

    float dX = (hR - hL) * 0.5;
    float dZ = (hU - hD) * 0.5;

    vec3 n = normalize(vec3(-dX / uCellSize, 1.0, -dZ / uCellSize));
    return n;
}

void main() {
    vUV = uv;

    float height = sampleHeight(uv);
    vec3 localPos = vec3(xz.x, height, xz.y);

    vec3 normal = computeNormal(uv);
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    vNormal = normalize(normalMatrix * normal);

    vec4 world = uModel * vec4(localPos, 1.0);
    vWorldPos = world.xyz;

    gl_Position = uProj * uView * world;
}
