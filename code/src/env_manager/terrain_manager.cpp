#include <iostream>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "renderer.h"
#include "glm_compat.h"
#include "terrain_manager.h"
#include "texture_loader.h"

using namespace std;
using namespace glm;

TerrainManager::TerrainManager()
    : terrain(nullptr), heightmapTex(0), diffuseTex(0), heightmapData(nullptr),
      gridSize(0), worldSize(0.0f), heightScale(0.0f),
      cellSize(0.0f), textureScale(8.0f),
      heightmapWidth(0), heightmapHeight(0), heightmapChannels(0),
      isInitialized(false)
{
}

TerrainManager::~TerrainManager()
{
    cleanup();
}

bool TerrainManager::initialize(int gSize, float wSize, float hScale,
                                const char *heightmapPath, const char *diffusePath)
{
    if (isInitialized)
    {
        cerr << "TerrainManager already initialized!\n";
        return false;
    }

    gridSize = gSize;
    worldSize = wSize;
    heightScale = hScale;
    cellSize = worldSize / static_cast<float>(gridSize);

    heightmapTex = LoadTexture(heightmapPath, false, false,
                               &heightmapWidth, &heightmapHeight);
    if (heightmapTex == 0)
    {
        cerr << "Failed to load heightmap: " << heightmapPath << "\n";
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, heightmapTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    heightmapData = stbi_load(heightmapPath, &heightmapWidth,
                              &heightmapHeight, &heightmapChannels, 0);
    if (!heightmapData)
    {
        cerr << "Warning: Failed to load heightmap data for CPU sampling\n";
    }


    diffuseTex = LoadTexture(diffusePath, true, true);
    if (diffuseTex == 0)
    {
        cerr << "Failed to load terrain diffuse: " << diffusePath << "\n";
        cleanup();
        return false;
    }

    terrain = new Terrain(gridSize, worldSize, heightmapTex,
                          heightScale, diffuseTex);

    isInitialized = true;

    return true;
}

void TerrainManager::render(GLuint shaderProgram, const mat4 &model,
                            const mat4 &view, const mat4 &proj,
                            const vec3 &cameraPos, float timeOfDay, float deltaTime)
{
    if (!isInitialized || !terrain)
    {
        cerr << "TerrainManager not initialized!\n";
        return;
    }

    glUseProgram(shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, diffuseTex);

    glUniform1i(glGetUniformLocation(shaderProgram, "uHeightmap"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "uDiffuse"), 1);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1,
                       GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uView"), 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProj"), 1,
                       GL_FALSE, glm::value_ptr(proj));

    glUniform1f(glGetUniformLocation(shaderProgram, "uHeightScale"), heightScale);
    glUniform2f(glGetUniformLocation(shaderProgram, "uTexelSize"),
                1.0f / static_cast<float>(heightmapWidth),
                1.0f / static_cast<float>(heightmapHeight));
    glUniform1f(glGetUniformLocation(shaderProgram, "uCellSize"), cellSize);
    glUniform1f(glGetUniformLocation(shaderProgram, "uTextureScale"), textureScale);

    DayNightParams dayNight = calculateDayNightCycle(timeOfDay);

    glUniform3f(glGetUniformLocation(shaderProgram, "uLightDirection"),
                dayNight.lightDir.x, dayNight.lightDir.y, dayNight.lightDir.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "uLightColor"),
                (dayNight.lightColor * dayNight.lightIntensity).x,
                (dayNight.lightColor * dayNight.lightIntensity).y,
                (dayNight.lightColor * dayNight.lightIntensity).z);
    glUniform3f(glGetUniformLocation(shaderProgram, "uAmbientColor"),
                dayNight.ambientColor.x, dayNight.ambientColor.y, dayNight.ambientColor.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "uCameraPos"),
                cameraPos.x, cameraPos.y, cameraPos.z);

    glUniform1f(glGetUniformLocation(shaderProgram, "uSpecularStrength"), 0.25f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uShininess"), 32.0f);

    static float heatDelta = 0.0f;
    heatDelta += deltaTime;
    glUniform1f(glGetUniformLocation(shaderProgram, "uTime"), heatDelta);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseHeatShimmer"), 1);
    glUniform1f(glGetUniformLocation(shaderProgram, "uHeatShimmerIntensity"), 3.5f);

    terrain->draw();
}

float TerrainManager::getHeightAt(float worldX, float worldZ) const
{
    if (!heightmapData || heightmapWidth <= 0 || heightmapHeight <= 0)
    {
        return 0.0f;
    }

    float u = (worldX + worldSize * 0.5f) / worldSize;
    float v = (worldZ + worldSize * 0.5f) / worldSize;

    return sampleHeightmap(u, v);
}

float TerrainManager::sampleHeightmap(float u, float v) const
{
    u = std::max(0.0f, std::min(1.0f, u));
    v = std::max(0.0f, std::min(1.0f, v));

    int px = static_cast<int>(u * (heightmapWidth - 1));
    int py = static_cast<int>(v * (heightmapHeight - 1));

    px = std::max(0, std::min(heightmapWidth - 1, px));
    py = std::max(0, std::min(heightmapHeight - 1, py));

    int pixelIndex = py * heightmapWidth + px;
    int byteIndex = pixelIndex * heightmapChannels;

    if (byteIndex < 0 || byteIndex >= heightmapWidth * heightmapHeight * heightmapChannels)
    {
        return 0.0f;
    }

    float gray;
    if (heightmapChannels == 1)
    {
        gray = static_cast<float>(heightmapData[byteIndex]);
    }
    else
    {
        unsigned char r = heightmapData[byteIndex];
        gray = static_cast<float>(r);
    }

    return (gray / 255.0f) * heightScale;
}

void TerrainManager::cleanup()
{
    if (terrain)
    {
        delete terrain;
        terrain = nullptr;
    }

    if (heightmapTex)
    {
        glDeleteTextures(1, &heightmapTex);
        heightmapTex = 0;
    }

    if (diffuseTex)
    {
        glDeleteTextures(1, &diffuseTex);
        diffuseTex = 0;
    }

    if (heightmapData)
    {
        stbi_image_free(heightmapData);
        heightmapData = nullptr;
    }

    isInitialized = false;
}
