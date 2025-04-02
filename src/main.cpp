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
    buildPrograms(programs);
    programs.mesh.Bind();

    struct MeshData objMeshData = loadMesh(programs.mesh, argv[1]);
    struct MeshData planeMeshData =
        loadMesh(programs.mesh, (char*)"./assets/plane.obj");

    SpotLight light(
        cyVec3f(0.0, -50.0, 40.0),
        cyVec3f(0.0, 0.0, 0.0),
        45.0,
        programs.shadow,
        1024,
        1024
    );
    light.setupForMeshProgram(programs.mesh);

    updateCamera(window, programs);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        double time = glfwGetTime();
        light.origin = cyVec3f(sin(time) * 40, cos(time) * 40, 40);
        light.updateMVP();
        light.setupForMeshProgram(programs.mesh);

        processInput(window);
        updateCamera(window, programs);

        programs.shadow.Bind();
        light.Bind();
        //drawMesh(programs.shadow, planeMeshData);
        drawMesh(programs.shadow, objMeshData);
        light.Unbind();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        programs.mesh.Bind();
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, light.getTextureID());
        programs.mesh.SetUniform(6, 4);

        bindMaterialProperties(programs.mesh, objMeshData.mesh);
        drawMesh(programs.mesh, objMeshData);
        bindMaterialProperties(programs.mesh, planeMeshData.mesh);
        drawMesh(programs.mesh, planeMeshData);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanupMeshData(objMeshData);
    cleanupMeshData(planeMeshData);

    glfwTerminate();
    return 0;
}
