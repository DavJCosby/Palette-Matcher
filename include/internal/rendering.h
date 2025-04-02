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

void build_programs(ShaderPrograms& programs);

struct MeshData load_mesh(cyGLSLProgram& prog, char* path);

void cleanup_mesh_data(MeshData& meshData);

void load_texture(
    cyGLSLProgram& prog,
    std::basic_string<char> filepath,
    char* attrib_name,
    unsigned tex_id
);

struct MeshData
bind_mesh_vertex_attributes(cyGLSLProgram& prog, cyTriMesh& mesh);

void bind_material_properties(cyGLSLProgram& prog, cyTriMesh& mesh);

void draw_mesh(cyGLSLProgram& prog, struct MeshData& meshData);
