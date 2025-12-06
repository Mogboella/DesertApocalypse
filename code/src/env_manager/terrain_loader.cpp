#include <iostream>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "terrain_loader.h"

using namespace std;
using namespace glm;

struct Vertex
{
    vec2 xz;
    vec2 uv;
};

Terrain::Terrain(int gSize, float wrldSize,
                 GLuint hmap,
                 float hScale,
                 GLuint diffuse)
    : gridSize(gSize), worldSize(wrldSize), heightmapTex(hmap), heightScale(hScale), diffuseTex(diffuse)
{

    const int N = gridSize;
    const float step = worldSize / float(N);
    vector<Vertex> verts;
    verts.reserve((N + 1) * (N + 1));

    for (int j = 0; j <= N; ++j)
    {
        for (int i = 0; i <= N; ++i)
        {
            float x = -0.5f * worldSize + i * step;
            float z = -0.5f * worldSize + j * step;
            float u = float(i) / float(N);
            float v = float(j) / float(N);
            verts.push_back({vec2(x, z), vec2(u, v)});
        }
    }

    vector<unsigned> idx;
    idx.reserve(N * N * 6);

    auto id = [N](int i, int j)
    { return j * (N + 1) + i; };
    for (int j = 0; j < N; ++j)
    {
        for (int i = 0; i < N; ++i)
        {
            unsigned a = id(i, j);
            unsigned b = id(i + 1, j);
            unsigned c = id(i + 1, j + 1);
            unsigned d = id(i, j + 1);

            idx.push_back(a);
            idx.push_back(b);
            idx.push_back(c);
            idx.push_back(a);
            idx.push_back(c);
            idx.push_back(d);
        }
    }
    indexCount = (GLsizei)idx.size();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);

    if (!verts.empty())
    {
        cerr << "  First vertex: pos=(" << verts[0].xz.x << ","
             << verts[0].xz.y << ")\n";
    }

    // layout: location 0 = xz, location 1 = uv
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, xz));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

    glBindVertexArray(0);
}

Terrain::~Terrain()
{
    if (ibo)
        glDeleteBuffers(1, &ibo);
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (vao)
        glDeleteVertexArrays(1, &vao);
}

void Terrain::draw() const
{
    static bool once = false;
    if (!once)
    {
        cerr << "Terrain::draw() called: VAO=" << vao
             << " indexCount=" << indexCount << "\n";
        once = true;
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        cerr << "GL error in Terrain::draw(): 0x" << hex << err << dec << "\n";
    }

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, diffuseTex);
}
