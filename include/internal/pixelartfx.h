#pragma once

#include <glad/glad.h>

#include <cy/cyCore.h>
#include <cy/cyGL.h>

#include <GLFW/glfw3.h>

class PixelArtEffect {
  private:
    unsigned int framebuffer_ID;
    unsigned int texture_ID;
    unsigned int rbo_ID;
    int width;
    int height;
    GLFWwindow* window;
    cyGLSLProgram upscale_program;

    // Simple quad for drawing the texture
    unsigned int quadVAO, quadVBO;

    void createFramebuffer(int w, int h);
    void setupQuad();

  public:
    int downscale_factor;

    PixelArtEffect(
        GLFWwindow* window,
        int downscale_factor,
        cyGLSLProgram& upscale_program
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
