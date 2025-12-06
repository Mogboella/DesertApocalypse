#version 330 core

in vec3 TexCoords;
out vec4 FragColor;

uniform vec3 horizonColor;
uniform vec3 zenithColor;
uniform vec3 sun1Dir;
uniform vec3 sun2Dir;
uniform vec3 sun1Color;
uniform vec3 sun2Color;
uniform float sunSize;

void main() {
    vec3 dir = normalize(TexCoords);

    float t = max(0.0, dir.y);
    vec3 skyColor = mix(horizonColor, zenithColor, t);

    float sun1Intensity = max(0.0, dot(dir, normalize(sun1Dir)));
    sun1Intensity = pow(sun1Intensity, sunSize);
    vec3 sun1Contrib = sun1Color * sun1Intensity;

    float sun2Intensity = max(0.0, dot(dir, normalize(sun2Dir)));
    sun2Intensity = pow(sun2Intensity, sunSize);
    vec3 sun2Contrib = sun2Color * sun2Intensity;

    vec3 finalColor = skyColor + sun1Contrib + sun2Contrib;

    FragColor = vec4(finalColor, 1.0);
}
