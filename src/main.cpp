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
#include "internal/scene.h"
#include "internal/pixelartfx.h"
#include "internal/paletteparser.h"

// #include <iostream>

void animate_light(SpotLight& light, cyGLSLProgram& mesh_program);

void compile_palette_into_fragshader(char** argv) {
    std::string palette_str = PaletteParser::generate_code_insert(argv[1]);
    std::ifstream inFile("./shaders/outline.frag");
    std::ofstream outFile("./shaders/outline-compiled.frag");

    if (inFile.is_open() && outFile.is_open()) {
        std::string line;
        while (getline(inFile, line)) {
            size_t pos = line.find("/*!AUTO GENERATED PALETTE CONSTANT!*/");
            if (pos != std::string::npos) {
                line.replace(pos, 37, palette_str);
            }
            outFile << line << std::endl;
        }
        inFile.close();
        outFile.close();
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr
            << "Must provide a path to some txt file of color palette hex values."
            << std::endl;
        exit(1);
    }
    compile_palette_into_fragshader(argv);

    GLFWwindow* window = initAndCreateWindow();
    ShaderPrograms programs = build_programs();
    Scene scene(programs, window);

    // Copy the outline fragment shader with the auto-generated palette constants

    PixelArtEffect
        pixel_effect(window, 9, programs.outline, programs.screen, scene);

    while (!glfwWindowShouldClose(window)) {
        process_input(window, pixel_effect);
        update_camera(window, programs);

        animate_light(scene.light, programs.mesh);

        scene.drawShadowMap();

        pixel_effect.setFramebufferSize();
        pixel_effect.beginRender();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.drawMeshes();
        pixel_effect.endRender();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void animate_light(SpotLight& light, cyGLSLProgram& mesh_program) {
    double time = glfwGetTime() / 5.0;
    double time2 = glfwGetTime() * 4;

    light.origin =
        cyVec3f(sin(time) * 60, cos(time) * 60, 25 + 10 * cos(time2));
    light.updateMVP();
    light.updateUniforms();
}
