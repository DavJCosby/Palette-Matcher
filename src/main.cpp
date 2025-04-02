#include <cmath>
#define GL_SILENCE_DEPRECATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cy/cyCore.h>
#include <cy/cyGL.h>
#include <cy/cyTriMesh.h>
#include <cy/cyVector.h>
#include <cy/cyMatrix.h>

#include "internal/rendering.h"
#include "internal/ui.h"
#include "internal/spotlight.h"

#include <iostream>

void animate_light(SpotLight& light, cyGLSLProgram& mesh_program) {
    double time = glfwGetTime();
    light.origin = cyVec3f(sin(time) * 40, cos(time) * 40, 40);
    light.updateMVP();
    light.setupForMeshProgram(mesh_program);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr
            << "insufficient arguments. Please provide a path to an obj to load."
            << std::endl;
        return -1;
    }

    GLFWwindow* window = initAndCreateWindow();
    glEnable(GL_DEPTH_TEST);

    ShaderPrograms programs;
    build_programs(programs);
    programs.mesh.Bind();

    struct MeshData objMeshData = load_mesh(programs.mesh, argv[1]);
    struct MeshData planeMeshData =
        load_mesh(programs.mesh, (char*)"./assets/plane.obj");

    SpotLight light(
        cyVec3f(0.0, -50.0, 40.0),
        cyVec3f(0.0, 0.0, 0.0),
        45.0,
        programs.shadow,
        1024,
        1024
    );
    light.setupForMeshProgram(programs.mesh);

    update_camera(window, programs);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        update_viewport_size(window);
        animate_light(light, programs.mesh);
        process_input(window);
        update_camera(window, programs);

        // draw shadow map
        programs.shadow.Bind();
        light.Bind();
        draw_mesh(programs.shadow, objMeshData);
        light.Unbind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        programs.mesh.Bind();
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, light.getTextureID());

        bind_material_properties(programs.mesh, objMeshData.mesh);
        draw_mesh(programs.mesh, objMeshData);
        bind_material_properties(programs.mesh, planeMeshData.mesh);
        draw_mesh(programs.mesh, planeMeshData);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup_mesh_data(objMeshData);
    cleanup_mesh_data(planeMeshData);

    glfwTerminate();
    return 0;
}
