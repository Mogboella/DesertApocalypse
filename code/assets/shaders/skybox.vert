#version 330

layout(location = 0) in vec3 position;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = position;
    mat4 fin_view = mat4(mat3(view));
    vec4 pos = projection * fin_view * vec4(position, 1.0);

    gl_Position = pos.xyww;
}
