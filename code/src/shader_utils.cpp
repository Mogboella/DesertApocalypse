#include "shader_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

static char *readShaderSource(const char *shaderFile)
{
    FILE *shader_file = fopen(shaderFile, "rb");
    if (!shader_file)
    {
        cerr << "readShaderSource: failed to open shader file: " << shaderFile << endl;
        return nullptr;
    }
    fseek(shader_file, 0, SEEK_END);
    long size = ftell(shader_file);
    fseek(shader_file, 0, SEEK_SET);
    char *buffer_val = new char[size + 1];
    fread(buffer_val, 1, size, shader_file);
    buffer_val[size] = '\0';
    fclose(shader_file);
    return buffer_val;
}

static void AddShader(GLuint program, const char *file, GLenum type)
{
    char *src = readShaderSource(file);
    GLuint sh = glCreateShader(type);

    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLint success;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLchar log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        cerr << "Shader compile error (" << file << "): " << log << endl;
        exit(1);
    }

    glAttachShader(program, sh);

    delete[] src;
}

GLuint CompileShaders(const char *vertex_shader, const char *fragment_shader)
{
    GLuint program = glCreateProgram();
    AddShader(program, vertex_shader, GL_VERTEX_SHADER);
    AddShader(program, fragment_shader, GL_FRAGMENT_SHADER);

    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        GLchar log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        cerr << "Link error: " << log << endl;
        exit(1);
    }

    GLint linkStatus = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus)
    {
        GLchar infoLog[10240];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        cerr << "Shader link error:\n"
             << infoLog << endl;
    }

    glUseProgram(program);

    return program;
}
