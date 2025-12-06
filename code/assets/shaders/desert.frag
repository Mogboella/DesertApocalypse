#version 330

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 VertexColor;

out vec4 FragColor;

uniform sampler2D diffuseTexture;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 viewPos;
uniform float specularStrength;
uniform float shininess;
uniform bool hasTexture;

uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogStart;
uniform float fogEnd;
uniform float fogHeight;
uniform float fogHeightRange;

uniform float time;
uniform bool useHeatShimmer;
uniform float heatShimmerIntensity;

uniform int numPointLights;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    float range;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    float cutoff;
    float outerCutoff;
    float range;
};

uniform PointLight pointLights[4];
uniform int numSpotLights;
uniform SpotLight spotLights[2];

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);

    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    attenuation = clamp(attenuation, 0.0, 1.0);

    if (distance > light.range) {
        attenuation = 0.0;
    }

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * baseColor * light.intensity;

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * light.color * light.intensity;

    return (diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);

    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    attenuation = clamp(attenuation, 0.0, 1.0);

    if (distance > light.range) {
        attenuation = 0.0;
    }

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * baseColor * light.intensity * intensity;

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * light.color * light.intensity * intensity;

    return (diffuse + specular) * attenuation;
}

void main() {
    vec2 distortedTexCoord = TexCoord;
    if (useHeatShimmer) {
        float heightFactor = 1.0 - clamp((FragPos.y - viewPos.y) / 50.0, 0.0, 1.0);
        float distortionX = sin(FragPos.y * 0.1 + time * 2.0) * 0.02 * heatShimmerIntensity * heightFactor;
        float distortionY = cos(FragPos.x * 0.1 + time * 1.5) * 0.02 * heatShimmerIntensity * heightFactor;
        distortedTexCoord = TexCoord + vec2(distortionX, distortionY);
    }

    vec3 baseColor = hasTexture ? texture(diffuseTexture, distortedTexCoord).rgb : VertexColor;
    vec3 ambient = ambientColor * baseColor;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDirNorm = normalize(-lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor * baseColor;

    // Calculate specular for directional light
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = ambient + diffuse + specular;

    for (int i = 0; i < numPointLights; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, baseColor);
    }

    for (int i = 0; i < numSpotLights; i++) {
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir, baseColor);
    }

    vec3 finalColor = result;
    float distance = length(viewPos - FragPos);

    float heightAboveGround = FragPos.y - fogHeight;
    float fogFactor = 0.0;

    if (heightAboveGround >= 0.0 && heightAboveGround < fogHeightRange) {
        float heightFactor = 1.0 - clamp(heightAboveGround / fogHeightRange, 0.0, 1.0);
        heightFactor = pow(heightFactor, 2.0);

        float distanceFactor = 0.0;
        if (distance > fogStart) {
            distanceFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
            distanceFactor = 1.0 - distanceFactor;
        }

        fogFactor = heightFactor * (1.0 - distanceFactor * 0.3);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
    }

    finalColor = mix(fogColor, finalColor, 1.0 - fogFactor);
    FragColor = vec4(finalColor, 1.0);
}
