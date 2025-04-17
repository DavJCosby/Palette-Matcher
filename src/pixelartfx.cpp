#include "internal/pixelartfx.h"
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
    float quadVertices[] = {
        // POSITIONS       // TEXTURE COORDS
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
    width = w;
    height = h;

    if (downscale_framebuffer_ID) {
        glDeleteFramebuffers(1, &downscale_framebuffer_ID);
        glDeleteTextures(1, &downscale_texture_ID);
        glDeleteRenderbuffers(1, &rbo_ID);
    }
    if (outline_framebuffer_ID) {
        glDeleteFramebuffers(1, &outline_framebuffer_ID);
        glDeleteTextures(1, &outline_texture_ID);
    }

    // RESCALING FRAMEBUFFER
    glGenFramebuffers(1, &downscale_framebuffer_ID);
    glBindFramebuffer(GL_FRAMEBUFFER, downscale_framebuffer_ID);

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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        downscale_texture_ID,
        0
    );

    glGenRenderbuffers(1, &rbo_ID);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_ID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        rbo_ID
    );

    // OUTLINE FRAMEBUFFER
    glGenFramebuffers(1, &outline_framebuffer_ID);
    glBindFramebuffer(GL_FRAMEBUFFER, outline_framebuffer_ID);

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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        outline_texture_ID,
        0
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PixelArtEffect::setFramebufferSize() {
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    int new_width = fb_width / downscale_factor;
    int new_height = fb_height / downscale_factor;

    if (new_width != width || new_height != height) {
        createFramebuffer(new_width, new_height);
    }
}

void PixelArtEffect::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, downscale_framebuffer_ID);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PixelArtEffect::endRender() {
    // OUTLINE POST-PROCESSING
    glBindFramebuffer(GL_FRAMEBUFFER, outline_framebuffer_ID);
    glClear(GL_COLOR_BUFFER_BIT);
    outline_program.Bind();

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, downscale_texture_ID);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, scene.depth_texture.GetTextureID());

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // UPSCALE TO FILL CANVAS
    upscale_program.Bind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, width * downscale_factor, height * downscale_factor);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, outline_texture_ID);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
