#include "internal/pixelartfx.h"
#include <iostream>
#include "internal/scene.h"

PixelArtEffect::PixelArtEffect(
    GLFWwindow* window,
    int downscale_factor,
    cyGLSLProgram& outline_program,
    cyGLSLProgram& upscale_program,
    Scene& scene
) :
    downscale_framebuffer_ID(0),
    downscale_texture_ID(5),
    outline_framebuffer_ID(0),
    outline_texture_ID(7),
    rbo_ID(0),
    width(0),
    height(0),
    window(window),
    outline_program(outline_program),
    upscale_program(upscale_program),
    scene(scene),
    downscale_factor(downscale_factor) {
    setupQuad();
}

PixelArtEffect::~PixelArtEffect() {
    if (downscale_framebuffer_ID)
        glDeleteFramebuffers(1, &downscale_framebuffer_ID);
    if (downscale_texture_ID)
        glDeleteTextures(1, &downscale_texture_ID);
    if (outline_framebuffer_ID)
        glDeleteFramebuffers(1, &outline_framebuffer_ID);
    if (outline_texture_ID)
        glDeleteTextures(1, &outline_texture_ID);
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
    // Delete old framebuffers if they exist
    if (downscale_framebuffer_ID) {
        glDeleteFramebuffers(1, &downscale_framebuffer_ID);
        glDeleteTextures(1, &downscale_texture_ID);
        glDeleteRenderbuffers(1, &rbo_ID);
    }
    if (outline_framebuffer_ID) {
        glDeleteFramebuffers(1, &outline_framebuffer_ID);
        glDeleteTextures(1, &outline_texture_ID);
    }

    width = w;
    height = h;

    // Create downscale framebuffer
    glGenFramebuffers(1, &downscale_framebuffer_ID);
    glBindFramebuffer(GL_FRAMEBUFFER, downscale_framebuffer_ID);

    // Create color texture for the downscale framebuffer
    glGenTextures(1, &downscale_texture_ID);
    glBindTexture(GL_TEXTURE_2D, downscale_texture_ID);
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
        downscale_texture_ID,
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
        std::cout << "ERROR: Downscale framebuffer not complete!" << std::endl;
    }

    // Create outline framebuffer
    glGenFramebuffers(1, &outline_framebuffer_ID);
    glBindFramebuffer(GL_FRAMEBUFFER, outline_framebuffer_ID);

    // Create color texture for the outline framebuffer
    glGenTextures(1, &outline_texture_ID);
    glBindTexture(GL_TEXTURE_2D, outline_texture_ID);
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
        outline_texture_ID,
        0
    );

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: Outline framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PixelArtEffect::setFramebufferSize() {
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    // Check if size changed or downscale factor changed
    int newWidth = fbWidth / downscale_factor;
    int newHeight = fbHeight / downscale_factor;

    if (newWidth != width || newHeight != height) {
        createFramebuffer(newWidth, newHeight);
    }
}

void PixelArtEffect::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, downscale_framebuffer_ID);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PixelArtEffect::endRender() {
    // We finished rendering the scene to our low-res framebuffer

    // STEP 1: Apply outline post-processing
    glBindFramebuffer(GL_FRAMEBUFFER, outline_framebuffer_ID);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up the outline shader
    outline_program.Bind();

    // Bind our scene texture to texture unit 0
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, downscale_texture_ID);
    outline_program.SetUniform("screenTexture", 5);

    //If your outline shader also needs a depth texture
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, scene.depth_texture.GetTextureID());
    outline_program.SetUniform("depthTexture", 6);

    // Draw the fullscreen quad with the outline shader
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // STEP 2: Render to screen with upscaling
    // Switch to default framebuffer
    // Use the upscaling shader
    upscale_program.Bind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Get window dimensions for viewport
    int window_width, window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the outlined texture to texture unit 7
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, outline_texture_ID);
    upscale_program.SetUniform("screenTexture", 7);

    // Draw fullscreen quad with nearest neighbor sampling for pixelated look
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Re-enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
}
