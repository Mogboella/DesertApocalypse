#define STB_IMAGE_IMPLEMENTATION
#include "texture_loader.h"
#include <iostream>
#include <string>

#include "stb_image.h"

using namespace std;

namespace
{
bool deriveFormats(int channels, bool srgb, GLenum &format, GLenum &internalFormat)
{
    switch (channels)
    {
    case 1:
        format = GL_RED;
        internalFormat = GL_R8;
        return true;
    case 2:
        format = GL_RG;
        internalFormat = GL_RG8;
        return true;
    case 3:
        format = GL_RGB;
        internalFormat = srgb ? GL_SRGB8 : GL_RGB8;
        return true;
    case 4:
        format = GL_RGBA;
        internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        return true;
    default:
        return false;
    }
}
}

GLuint UploadTextureFromPixels(const unsigned char *pixels,
                               int width,
                               int height,
                               int channels,
                               bool srgb)
{
    if (!pixels || width <= 0 || height <= 0 || channels <= 0)
    {
        cerr << "UploadTextureFromPixels: invalid pixel data\n";
        return 0;
    }

    GLenum format = GL_RGB;
    GLenum internalFormat = GL_RGB;
    if (!deriveFormats(channels, srgb, format, internalFormat))
    {
        cerr << "UploadTextureFromPixels: unsupported channel count (" << channels << ")\n";
        return 0;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internalFormat,
                 width,
                 height,
                 0,
                 format,
                 GL_UNSIGNED_BYTE,
                 pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

GLuint LoadTexture(const char *path,
                   bool srgb,
                   bool flipVertically,
                   int *outWidth,
                   int *outHeight,
                   int *outChannels)
{
    if (!path)
    {
        cerr << "LoadTexture: null path provided\n";
        return 0;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);
    stbi_uc *data = stbi_load(path, &width, &height, &channels, 0);
    stbi_set_flip_vertically_on_load(0);
    if (outWidth)
        *outWidth = width;
    if (outHeight)
        *outHeight = height;
    if (outChannels)
        *outChannels = channels;
    if (!data)
    {
        cerr << "LoadTexture: failed to load image '" << path << "': " << stbi_failure_reason() << "\n";
        return 0;
    }

    GLuint tex = UploadTextureFromPixels(data, width, height, channels, srgb);
    stbi_image_free(data);
    return tex;
}

GLuint LoadTextureFromMemory(const unsigned char *buffer,
                             size_t bufferSize,
                             bool srgb,
                             bool flipVertically,
                             int *outWidth,
                             int *outHeight,
                             int *outChannels)
{
    if (!buffer || bufferSize == 0)
    {
        cerr << "LoadTextureFromMemory: invalid buffer\n";
        return 0;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);
    stbi_uc *data = stbi_load_from_memory(buffer, static_cast<int>(bufferSize), &width, &height, &channels, 0);
    stbi_set_flip_vertically_on_load(0);
    if (outWidth)
        *outWidth = width;
    if (outHeight)
        *outHeight = height;
    if (outChannels)
        *outChannels = channels;
    if (!data)
    {
        cerr << "LoadTextureFromMemory: failed to parse texture: " << stbi_failure_reason() << "\n";
        return 0;
    }

    GLuint tex = UploadTextureFromPixels(data, width, height, channels, srgb);
    stbi_image_free(data);
    return tex;
}
