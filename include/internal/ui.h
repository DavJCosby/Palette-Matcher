#pragma once

#include <GLFW/glfw3.h>

#include "cy/cyCore.h"
#include "cy/cyGL.h"
#include "rendering.h"
#include <cy/cyVector.h>
#include <cy/cyMatrix.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

GLFWwindow* initAndCreateWindow();

void processInput(GLFWwindow* window);

void updateCamera(GLFWwindow* window, ShaderPrograms& programs);

cyMatrix4f modelView(cyVec3f translation, float pitch, float yaw, float roll);

void cursor_position_callback(GLFWwindow* window, double xPos, double yPos);
