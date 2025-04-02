#pragma once
#include "glad/glad.h"

#include <cy/cyCore.h>
#include <cy/cyGL.h>
#include <cy/cyTriMesh.h>
#include <cy/cyMatrix.h>
#include <cy/cyVector.h>

class SpotLight {
  public:
    cyVec3f origin, lookat;
    float fov;
    uint width, height;

  private:
    cyGLSLProgram& shadow_program;
    cyGLSLProgram& mesh_program;
    cyMatrix4f projection;
    cy::GLRenderDepth2D shadow_map;
    GLuint depth_map;

  public:
    SpotLight(
        cyVec3f origin,
        cyVec3f lookat,
        float fov,
        cyGLSLProgram& shadow_program,
        cyGLSLProgram& mesh_program,
        uint width,
        uint height
    );

    void Bind();

    void Unbind();

    void updateMVP();

    GLuint getTextureID();

    cyMatrix4f getLightSpaceMatrix() const;

    void updateUniforms();
};
