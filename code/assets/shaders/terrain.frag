#version 330

in vec2 vUV;
in vec3 vWorldPos;
in vec3 vNormal;
out vec4 FragColor;

uniform sampler2D uDiffuse;
uniform float uTextureScale;
uniform vec3 uLightDirection;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uCameraPos;
uniform float uSpecularStrength;
uniform float uShininess;

uniform float uTime;
uniform bool uUseHeatShimmer;
uniform float uHeatShimmerIntensity;

void main() {
    vec2 tiledUV = vUV * uTextureScale;

    vec2 distortedUV = tiledUV;
    if (uUseHeatShimmer) {
        float heightFactor = 1.0 - clamp((vWorldPos.y - uCameraPos.y) / 30.0, 0.0, 1.0);
        float distortionX = sin(vWorldPos.y * 0.15 + uTime * 3.0) * 0.05 * uHeatShimmerIntensity * heightFactor;
        float distortionY = cos(vWorldPos.x * 0.15 + uTime * 2.5) * 0.05 * uHeatShimmerIntensity * heightFactor;
        distortedUV = tiledUV + vec2(distortionX, distortionY);
    }

    vec3 albedo = texture(uDiffuse, distortedUV).rgb;

    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDirection);
    vec3 V = normalize(uCameraPos - vWorldPos);

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uLightColor * albedo;

    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), uShininess) * uSpecularStrength;
    vec3 specular = spec * uLightColor;

    vec3 ambient = uAmbientColor * albedo;

    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);
}
