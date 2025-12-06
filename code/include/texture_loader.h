#ifndef TEXTURE_UTILS_H
#define TEXTURE_UTILS_H

#include <cstddef>
#include <glad/gl.h>

GLuint LoadTexture(const char *path,
                   bool srgb = false,
                   bool flipVertically = false,
                   int *outWidth = nullptr,
                   int *outHeight = nullptr,
                   int *outChannels = nullptr);

GLuint LoadTextureFromMemory(const unsigned char *buffer,
                             size_t bufferSize,
                             bool srgb = false,
                             bool flipVertically = false,
                             int *outWidth = nullptr,
                             int *outHeight = nullptr,
                             int *outChannels = nullptr);

GLuint UploadTextureFromPixels(const unsigned char *pixels,
                               int width,
                               int height,
                               int channels,
                               bool srgb = false);

#endif
