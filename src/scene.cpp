#include "internal/spotlight.h"
#include "internal/rendering.h"
#include "internal/scene.h"
#include <OpenGL/gl.h>

#include <iostream>

Scene::Scene(ShaderPrograms& programs, GLFWwindow* window) :
    light(
        cyVec3f(0.0, -50.0, 40.0),
        cyVec3f(0.0, 0.0, 0.0),
        50.0,
        programs.shadow,
        programs.mesh,
        4096,
        4096
    ),
    programs(programs) {
    programs.mesh.Bind();

    Mesh duck(load_mesh(programs.mesh, (char*)"./assets/duck/duck.obj"), true);

    Mesh teapot(
        load_mesh(programs.mesh, (char*)"./assets/teapot/teapot.obj"),
        true
    );

    Mesh plane(
        load_mesh(programs.mesh, (char*)"./assets/ground/hb1.obj"),
        true
    );

    meshes.push_back(duck);
    meshes.push_back(teapot);
    meshes.push_back(plane);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    depth_texture.Initialize(
        false, // depth comparison texture
        2048, // width
        2048 // height
    );

    depth_texture.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);

    depth_texture.Bind();
}

Scene::~Scene() {
    std::cout << "Cleaning up scene" << std::endl;
    for (Mesh& mesh : meshes) {
        mesh.cleanup();
    }
}

void Scene::drawShadowMap() {
    programs.shadow.Bind();
    light.Bind();
    for (Mesh& mesh : meshes) {
        if (mesh.casts_shadow) {
            mesh.draw();
        }
    }
    light.Unbind();
}

void Scene::drawMeshes() {
    programs.mesh.Bind();
    glActiveTexture(GL_TEXTURE4); // shadow map
    glBindTexture(GL_TEXTURE_2D, light.getTextureID());

    glActiveTexture(GL_TEXTURE6); // depth map
    glBindTexture(GL_TEXTURE_2D, depth_texture.GetTextureID());

    // first render to depth texture
    depth_texture.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (Mesh& mesh : meshes) {
        mesh.bindMaterialProperties(programs.mesh);
        mesh.draw();
    }
    depth_texture.Unbind();

    // then render normal shading
    for (Mesh& mesh : meshes) {
        mesh.bindMaterialProperties(programs.mesh);
        mesh.draw();
    }
}
