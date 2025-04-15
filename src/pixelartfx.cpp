#include "internal/pixelartfx.h"
#include <iostream>

PixelArtEffect::PixelArtEffect(
    GLFWwindow* window,
    int downscale_factor,
    cyGLSLProgram& upscale_program
) :
    framebuffer_ID(0),
    texture_ID(5),
    rbo_ID(0),
    width(0),
    height(0),
    downscale_factor(downscale_factor),
    window(window),
    upscale_program(upscale_program) {
    setupQuad();
}

PixelArtEffect::~PixelArtEffect() {
    if (framebuffer_ID)
        glDeleteFramebuffers(1, &framebuffer_ID);
    if (texture_ID)
        glDeleteTextures(1, &texture_ID);
    if (rbo_ID)
        glDeleteRenderbuffers(1, &rbo_ID);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

void PixelArtEffect::setupQuad() {
    // Vertices for a quad that covers the entire screen
    float quadVertices[] = {
        // positions        // texture coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(quadVertices),
        quadVertices,
        GL_STATIC_DRAW
    );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glBindVertexArray(0);
}

void PixelArtEffect::createFramebuffer(int w, int h) {
    // Delete old framebuffer if it exists
    if (framebuffer_ID) {
        glDeleteFramebuffers(1, &framebuffer_ID);
        glDeleteTextures(1, &texture_ID);
        glDeleteRenderbuffers(1, &rbo_ID);
    }

    width = w;
    height = h;

    // Create framebuffer
    glGenFramebuffers(1, &framebuffer_ID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_ID);

    // Create texture for the framebuffer
    glGenTextures(1, &texture_ID);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        NULL
    );

    // Use nearest neighbor filtering for pixel art effect
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach texture to framebuffer
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        texture_ID,
        0
    );

    // Create renderbuffer for depth and stencil
    glGenRenderbuffers(1, &rbo_ID);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_ID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        rbo_ID
    );

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PixelArtEffect::setFramebufferSize() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    createFramebuffer(width / downscale_factor, height / downscale_factor);
}

void PixelArtEffect::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_ID);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PixelArtEffect::endRender() {
    upscale_program.Bind();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Draw the framebuffer texture on a fullscreen quad
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texture_ID);

    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_DEPTH_TEST);
}
