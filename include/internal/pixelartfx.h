#pragma once

#include <glad/glad.h>

#include <cy/cyCore.h>
#include <cy/cyGL.h>
#include "internal/scene.h"

#include <GLFW/glfw3.h>

class PixelArtEffect {
  private:
    GLuint downscale_framebuffer_ID;
    GLuint downscale_texture_ID;
    GLuint outline_framebuffer_ID;
    GLuint outline_texture_ID;
    GLuint rbo_ID;
    int width;
    int height;
    GLFWwindow* window;

  public:
    cyGLSLProgram outline_program;
    cyGLSLProgram upscale_program;

  private:
    Scene& scene;

    // Simple quad for drawing the texture
    unsigned int quadVAO, quadVBO;

    void createFramebuffer(int w, int h);
    void setupQuad();

  public:
    int downscale_factor;

    PixelArtEffect(
        GLFWwindow* window,
        int downscale_factor,
        cyGLSLProgram& outline_program,
        cyGLSLProgram& upscale_program,
        Scene& scene
    );
    ~PixelArtEffect();

    void setFramebufferSize();
    void beginRender();
    void endRender();

    int GetWidth() const {
        return width;
    }

    int GetHeight() const {
        return height;
    }
};
