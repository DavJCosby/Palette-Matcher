#pragma once
#include "glad/glad.h"
#include <cy/cyCore.h>
#include <cy/cyGL.h>

#include "internal/spotlight.h"
#include "internal/rendering.h"

#include <vector>
using std::vector;

class Scene {
  public:
    SpotLight light;
    vector<Mesh> meshes;
    ShaderPrograms& programs;
    cyGLRenderDepth2D depth_texture;

    Scene(ShaderPrograms& programs, GLFWwindow* window);
    ~Scene();

    void drawShadowMap();

    void drawMeshes();
};
