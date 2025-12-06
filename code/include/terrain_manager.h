#ifndef TERRAIN_MANAGER_H
#define TERRAIN_MANAGER_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>

#include "terrain_loader.h"

using namespace glm;

class TerrainManager
{
public:
    TerrainManager();
    ~TerrainManager();

    bool initialize(int gridSize, float worldSize, float heightScale,
                    const char *heightmapPath, const char *diffusePath);
    void render(GLuint shaderProgram, const mat4 &model,
                const mat4 &view, const mat4 &proj,
                const vec3 &cameraPos, float timeOfDay, float deltaTime = 0.016f);
    float getHeightAt(float worldX, float worldZ) const;
    void cleanup();
    float getWorldSize() const { return worldSize; }
    float getHeightScale() const { return heightScale; }
    int getGridSize() const { return gridSize; }

private:
    Terrain *terrain;
    GLuint heightmapTex;
    GLuint diffuseTex;
    unsigned char *heightmapData;

    int gridSize;
    float worldSize;
    float heightScale;
    float cellSize;
    float textureScale;

    int heightmapWidth;
    int heightmapHeight;
    int heightmapChannels;

    bool isInitialized;

    float sampleHeightmap(float u, float v) const;
};

#endif
