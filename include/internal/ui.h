#pragma once

#include <GLFW/glfw3.h>

#include "cy/cyCore.h"
#include "cy/cyGL.h"
#include "rendering.h"
#include <cy/cyVector.h>
#include <cy/cyMatrix.h>
#include "internal/pixelartfx.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

GLFWwindow* initAndCreateWindow();

void process_input(GLFWwindow* window, PixelArtEffect& pixel_art_effect);

void update_camera(GLFWwindow* window, ShaderPrograms& programs);

cyMatrix4f model_view(cyVec3f translation, float pitch, float yaw, float roll);

void cursor_position_callback(GLFWwindow* window, double xPos, double yPos);
