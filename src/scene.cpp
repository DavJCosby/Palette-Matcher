#include "internal/spotlight.h"
#include "internal/rendering.h"
#include "internal/scene.h"

#include <iostream>

Scene::Scene(ShaderPrograms& programs) :
    light(
        cyVec3f(0.0, -50.0, 40.0),
        cyVec3f(0.0, 0.0, 0.0),
        45.0,
        programs.shadow,
        programs.mesh,
        1024,
        1024
    ),
    programs(programs) {
    programs.mesh.Bind();

    Mesh teapot(
        load_mesh(programs.mesh, (char*)"./assets/teapot/teapot.obj"),
        true
    );
    Mesh plane(load_mesh(programs.mesh, (char*)"./assets/plane.obj"), false);

    meshes.push_back(teapot);
    meshes.push_back(plane);
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
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, light.getTextureID());
    for (Mesh& mesh : meshes) {
        mesh.bindMaterialProperties(programs.mesh);
        mesh.draw();
    }
}
