#pragma once

#include <vector>
#include <string>

#include "glad/glad.h"

#include <cy/cyCore.h>
#include <cy/cyGL.h>
#include <cy/cyTriMesh.h>
#include <GLFW/glfw3.h>

using std::string;

struct MaterialData {};

struct MeshData {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    int unsigned numFaces;
    cyTriMesh mesh;
};

struct ShaderPrograms {
    cyGLSLProgram mesh;
    cyGLSLProgram shadow;
};

void buildPrograms(ShaderPrograms& programs);

struct MeshData loadMesh(cyGLSLProgram& prog, char* path);

void cleanupMeshData(MeshData& meshData);

void loadTexture(
    cyGLSLProgram& prog,
    std::basic_string<char> filepath,
    char* attrib_name,
    unsigned tex_id
);

struct MeshData bindMeshVertexAttributes(cyGLSLProgram& prog, cyTriMesh& mesh);

void bindMaterialProperties(cyGLSLProgram& prog, cyTriMesh& mesh);

void drawMesh(cyGLSLProgram& prog, struct MeshData& meshData);
