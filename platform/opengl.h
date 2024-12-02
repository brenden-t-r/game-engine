#ifndef DIRECTX11_OPENGL_H
#define DIRECTX11_OPENGL_H

// Include constants
#include "../constants.h"
#include "platform.h"

// OpenGL
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>

class PlatformOpenGL : public Platform {
public:
    PlatformOpenGL() = default;
    ~PlatformOpenGL() override = default;

    void Init() override {
        /* Initialize the library */
        if (!glfwInit()) {
            fprintf(stderr, "Failed to initialize GLFW\n");
//            return -1;
            return;
        }

        glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", NULL, NULL);
        if (window == NULL) {
            fprintf(stderr,
                    "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
            glfwTerminate();
            return;
//            return -1;
        }
        glfwMakeContextCurrent(window);

        // Initialize glew
        if (glewInit() != GLEW_OK) {
            fprintf(stderr, "Failed to initialize GLEW\n");
            glfwTerminate();
//            return -1;
        }

        printf("%s\n", glGetString(GL_VERSION));

        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

        // Dark blue background
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

        // Dark blue background
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    }

    void Run() override{
        do{
            // Clear the screen
            glClear( GL_COLOR_BUFFER_BIT );

            glfwPollEvents();
        }
        // Check if the ESC key was pressed or the window was closed
        while(
                glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
                glfwWindowShouldClose(window) == 0
        );
    }

    void Shutdown() override{}

private:
    GLFWwindow *window = nullptr;
};

int main() {
    Platform* platform = new PlatformOpenGL();
    return PlatformMain(platform);
}

#endif //DIRECTX11_OPENGL_H
