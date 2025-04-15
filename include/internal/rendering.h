#pragma once

#include <vector>
#include <string>

#include "glad/glad.h"

#include <cy/cyCore.h>
#include <cy/cyGL.h>
#include <cy/cyTriMesh.h>
#include <GLFW/glfw3.h>

#include "internal/mesh.h"

using std::string;

struct MaterialData {};

struct ShaderPrograms {
    cyGLSLProgram mesh;
    cyGLSLProgram shadow;
    cyGLSLProgram outline;
    cyGLSLProgram screen;
};

ShaderPrograms build_programs();

struct MeshData load_mesh(cyGLSLProgram& prog, char* path);

void load_texture(
    cyGLSLProgram& prog,
    std::basic_string<char> filepath,
    char* attrib_name,
    unsigned tex_id
);

struct MeshData
bind_mesh_vertex_attributes(cyGLSLProgram& prog, cyTriMesh& mesh);
