#include <vector>
#include <glad/gl.h>

class Terrain
{
public:
  Terrain(int gridSize, float worldSize,
          GLuint heightmapTex,
          float heightScale,
          GLuint diffuseTex);

  ~Terrain();

  void draw() const;
  void setHeightScale(float s) { heightScale = s; }
  float getHeightScale() const { return heightScale; }
  void setDiffuseTexture(GLuint tex) { diffuseTex = tex; }
  GLuint getDiffuseTexture() const { return diffuseTex; }

private:
  GLuint vao = 0, vbo = 0, ibo = 0;
  GLsizei indexCount = 0;
  int gridSize;
  float worldSize;
  GLuint heightmapTex;
  float heightScale;
  GLuint diffuseTex;
};
