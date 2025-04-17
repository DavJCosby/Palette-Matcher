#include "cy/cyVector.h"
#include "glad/glad.h"
#include "internal/rendering.h"
#include "internal/ui.h"

static double lastMouseX;
static double lastMouseY;

static float camRotX = 1.57;
static float camRotY = 1.97;
static float camDist = 400;

static bool plusKeyDebounce = true;
static bool minusKeyDebounce = true;
static bool tKeyDebounce = true;
static bool togglePalette = true;

static float dither = 0.003;

static double lastKey = 0;

const double KEY_REPEAT_INTERVAL = 0.25;

const float CAM_MIN_DIST = 200;
const float CAM_MAX_DIST = 1200;

float deg2rad(float deg) {
    return deg * 3.145 / 180.0;
}

GLFWwindow* initAndCreateWindow() {
    if (!glfwInit())
        return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8); // MSAA 8x

    GLFWwindow* window =
        glfwCreateWindow(960, 720, "Final Project", NULL, NULL);

    if (!window) {
        std::cout << "Failed to create the window!" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to load opengl function pointers!" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);

    glClearColor(64.0 / 255.0, 6.0 / 255.0, 191.0 / 255.0, 1.0f);
    return window;
}

void process_input(GLFWwindow* window, PixelArtEffect& pixel_art_effect) {
    // ESCAPE TO CLOSE WINDOW

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // I/O TO ZOOM IN/OUT

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        camDist -= 0.5;
        if (camDist < CAM_MIN_DIST) {
            camDist = CAM_MIN_DIST;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        camDist += 0.5;
        if (camDist > CAM_MAX_DIST) {
            camDist = CAM_MAX_DIST;
        }
    }

    double time = glfwGetTime();

    // PLUS/MINUS TO CHANGE PIXELATION SCALE

    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_RELEASE
        || time - lastKey > KEY_REPEAT_INTERVAL) {
        plusKeyDebounce = true;
    }

    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_RELEASE
        || time - lastKey > KEY_REPEAT_INTERVAL) {
        minusKeyDebounce = true;
    }

    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) { // plus key
        if (plusKeyDebounce) {
            pixel_art_effect.downscale_factor -= 1;

            if (pixel_art_effect.downscale_factor < 1) {
                pixel_art_effect.downscale_factor = 1;
            }
            plusKeyDebounce = false;
            lastKey = time;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
        if (minusKeyDebounce) {
            pixel_art_effect.downscale_factor += 1;
            minusKeyDebounce = false;
            lastKey = time;
        }
    }

    // T TO TOGGLE PALETTE MATCHING

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (tKeyDebounce) {
            togglePalette = !togglePalette;
            pixel_art_effect.outline_program.SetUniform(
                "TogglePalette",
                togglePalette ? 1 : 0
            );
            tKeyDebounce = false;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        tKeyDebounce = true;
    }

    // COMMA/PERIOD TO INCREASE/DECREASE DITHER AMOUNT

    if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {
        dither -= 0.00005;
        if (dither < 0.0) {
            dither = 0.0;
        }
        pixel_art_effect.outline_program.SetUniform("Dither", dither);
        std::cout << "Dithering Factor: " << dither << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
        dither += 0.00005;
        if (dither > 0.025) {
            dither = 0.025;
        }
        pixel_art_effect.outline_program.SetUniform("Dither", dither);
        std::cout << "Dithering Factor: " << dither << std::endl;
    }
}

void cursor_position_callback(GLFWwindow* window, double xPos, double yPos) {
    double deltaX = xPos - lastMouseX;
    double deltaY = yPos - lastMouseY;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        camRotX += deltaX * 0.01;
        camRotY += deltaY * 0.01;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        camDist += deltaY * 0.5;
        if (camDist < CAM_MIN_DIST) {
            camDist = CAM_MIN_DIST;
        }
        if (camDist > CAM_MAX_DIST) {
            camDist = CAM_MAX_DIST;
        }
    }

    lastMouseX = xPos;
    lastMouseY = yPos;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void update_camera(GLFWwindow* window, ShaderPrograms& programs) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    cyMatrix4f projection = cy::Matrix4f::Perspective(
        deg2rad(5.0),
        (float)width / (float)height,
        1.0f,
        5000.0f
    );

    cyMatrix4f mv =
        model_view(cyVec3f(0, -5, -camDist), camRotY, 3.14, camRotX);
    cyMatrix4f scale = cyMatrix4f::Scale(5);
    cyMatrix4f finalTransform = projection * scale * mv;

    programs.mesh.Bind();
    float mvp_array[16] = {0};
    finalTransform.Get(mvp_array);
    programs.mesh.SetUniformMatrix4("MVP", mvp_array);

    float mv_array[16] = {0};
    mv.Get(mv_array);
    programs.mesh.SetUniformMatrix4("MV", mv_array);
}

cyMatrix4f model_view(cyVec3f translation, float yaw, float pitch, float roll) {
    cyMatrix4f trans = cyMatrix4f::Translation(translation);
    cyMatrix4f rot = cyMatrix4f::RotationZYX(yaw, pitch, roll);

    return trans * rot;
};
