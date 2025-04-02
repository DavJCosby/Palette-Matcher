#include "glad/glad.h"
#include "internal/rendering.h"

#include <OpenGL/OpenGL.h>
#include <cy/cyCore.h>
#include <cy/cyGL.h>
#include <cy/cyTriMesh.h>
#include <cy/cyMatrix.h>
#include <cy/cyVector.h>

#include "internal/spotlight.h"

SpotLight::SpotLight(
    cyVec3f origin,
    cyVec3f lookat,
    float fov,
    cyGLSLProgram& shadow_program,
    unsigned int width,
    unsigned int height
) :
    origin(origin),
    lookat(lookat),
    fov(fov),
    width(width),
    height(height),
    shadow_program(shadow_program) {
    // Build shader
    this->shadow_program.BuildFiles(
        "./shaders/shadow.vert",
        "./shaders/shadow.frag"
    );
    this->shadow_program.Bind();
    this->shadow_program.RegisterUniform(0, "MVP");
    updateMVP();

    glBindTexture(GL_TEXTURE_2D, this->depthMap);
    glGenTextures(1, &this->depthMap);

    this->shadowMap
        .Initialize(true, this->width, this->height, GL_DEPTH_COMPONENT24);

    this->shadowMap.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
    this->shadowMap.Bind();

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind framebuffer
}

void SpotLight::updateMVP() {
    this->projection = cyMatrix4f::Perspective(
        this->fov * 3.145 / 180.0, // deg to rad
        (float)this->width / (float)this->height,
        0.1f,
        200.0f
    );
    this->projection *= cyMatrix4f::View(origin, lookat, cyVec3f(0, 0, 1));
    float mvp_array[16] = {0};
    this->projection.Get(mvp_array);
    this->shadow_program.Bind();
    this->shadow_program.SetUniformMatrix4("MVP", mvp_array);
}

void SpotLight::Bind() {
    // Save previous state
    this->shadow_program.Bind();
    this->shadowMap.Bind();
    glViewport(0, 0, this->width, this->height);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
}

void SpotLight::Unbind() {
    this->shadowMap.Unbind();
}

GLuint SpotLight::getTextureID() {
    return this->shadowMap.GetTextureID();
}

cyMatrix4f SpotLight::getLightSpaceMatrix() const {
    cyMatrix4f result =
        cyMatrix4f::Translation(cyVec3f(0.5, 0.5, 0.5 - 0.0000005));
    result *= cyMatrix4f::Scale(0.5);
    result *= this->projection;
    return result;
}

void SpotLight::setupForMeshProgram(cyGLSLProgram& meshProgram) {
    meshProgram.Bind();
    float lightSpaceMatrix[16];
    cyMatrix4f lightspace = this->getLightSpaceMatrix();
    lightspace.Get(lightSpaceMatrix);

    meshProgram.SetUniformMatrix4("LightSpaceMatrix", lightSpaceMatrix);
    meshProgram.SetUniform3("LightPosition", this->origin.Elements());
    meshProgram.SetUniform("LightConeAngle", this->fov);
}
