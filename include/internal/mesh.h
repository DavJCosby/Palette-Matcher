#pragma once
#include "glad/glad.h"
#include <cy/cyCore.h>
#include <cy/cyGL.h>

#include "internal/spotlight.h"
#include "internal/rendering.h"

struct MeshData {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    int unsigned numFaces;
    cyTriMesh mesh;
};

class Mesh {
  public:
    struct MeshData mesh_data;
    bool casts_shadow;

    Mesh(MeshData mesh_data, bool casts_shadow);

    void draw();
    void bindMaterialProperties(cyGLSLProgram& program);
    void cleanup();
};
