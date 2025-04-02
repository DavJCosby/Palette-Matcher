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
    cyMatrix4f projection;
    cy::GLRenderDepth2D shadowMap;
    GLuint depthMap;
    GLuint depthMapFBO; // Framebuffer object

  public:
    SpotLight(
        cyVec3f origin,
        cyVec3f lookat,
        float fov,
        cyGLSLProgram& shadow_program,
        uint width,
        uint height
    );

    void Bind();

    void Unbind();

    void setupForMeshProgram(cyGLSLProgram& meshProgram);

    void updateMVP();

    GLuint getTextureID();

    cyMatrix4f getLightSpaceMatrix() const;
};
