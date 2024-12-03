#ifndef GAMEENGINE_OPENGL_H
#define GAMEENGINE_OPENGL_H

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

    void LoadShaders() override {
        VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        // Create and compile our GLSL program from the shaders
        GLuint programID = LoadShaders(
                "SimpleVertexShader.vertexshader",
                "SimpleFragmentShader.fragmentshader"
        );

        // Use our shader
        glUseProgram(programID);

        static const GLfloat g_vertex_buffer_data[] = {
                -1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                0.0f,  1.0f, 0.0f,
        };

        vertexbuffer;
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    }

    void Run(const std::function<void()>& func) override{
        do{
            // Clear the screen
            glClear( GL_COLOR_BUFFER_BIT );

            func();
//            DrawTriangle();
//            DrawSprite();

            glfwPollEvents();
        }
        // Check if the ESC key was pressed or the window was closed
        while(
                glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
                glfwWindowShouldClose(window) == 0
        );
    }

    void Shutdown() override{
        // Cleanup VBO
        glDeleteBuffers(1, &vertexbuffer);
        glDeleteVertexArrays(1, &VertexArrayID);
        glDeleteProgram(programID);

        // Close OpenGL window and terminate GLFW
        glfwTerminate();
    }

    void DrawSprite() {
    }

    void DrawTriangle() override {
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                nullptr             // array buffer offset
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
    }

private:
    GLFWwindow *window = nullptr;
    GLuint VertexArrayID = 0;
    GLuint programID = 0;
    GLuint vertexbuffer = 0;

    GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
        // Create the shaders
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Read vertex shader code from file
        const char* vertexShaderCode = get_shader_content(vertex_file_path);

        // Read fragment shader
        const char* fragmentShaderCode = get_shader_content(fragment_file_path);

        // compile vertex shader
        glShaderSource(VertexShaderID, 1, &vertexShaderCode, NULL);
        glCompileShader(VertexShaderID);

        // compile fragment shader
        glShaderSource(FragmentShaderID, 1, &fragmentShaderCode, NULL);
        glCompileShader(FragmentShaderID);

        // Check vertex shader
        GLint Result = GL_FALSE;
        int InfoLogLength;
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        printf("%i\n", Result);
        if (InfoLogLength > 0){
            char* infoLog = static_cast<char *>(malloc(1024));
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &infoLog[0]);
            printf("%s\n", infoLog);
            free(infoLog);
        }

        // Check fragment shader
        Result = GL_FALSE;
        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        printf("%i\n", Result);
        if (InfoLogLength > 0){
            char* infoLog = static_cast<char *>(malloc(1024));
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &infoLog[0]);
            printf("%s\n", infoLog);
            free(infoLog);
        }


        // Link the program
        printf("Linking program\n");
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, VertexShaderID);
        glAttachShader(ProgramID, FragmentShaderID);
        glLinkProgram(ProgramID);

        // Check the program
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        printf("%i\n", Result);
        printf("%i\n", InfoLogLength);
        if (InfoLogLength > 0){
            char* infoLog = static_cast<char *>(malloc(1024));
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &infoLog[0]);
            printf("%s\n", infoLog);

            GLint maxLength = 0;
            glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &maxLength);
            glGetProgramInfoLog(ProgramID, maxLength, &maxLength, &infoLog[0]);
            printf("%s\n", infoLog);

            free(infoLog);
        }

        glDetachShader(ProgramID, VertexShaderID);
        glDetachShader(ProgramID, FragmentShaderID);

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        return ProgramID;
    }

    char* get_shader_content(const char* fileName)
    {
        FILE *fp;
        long size = 0;
        char* shaderContent;

        /* Read File to get size */
        fp = fopen(fileName, "rb");
        if(fp == NULL) {
            printf("Error reading %s\n", fileName);
            return "";
        }
        fseek(fp, 0L, SEEK_END);
        size = ftell(fp)+1;
        fclose(fp);

        /* Read File for Content */
        fp = fopen(fileName, "r");
        shaderContent = static_cast<char *>(memset(malloc(size), '\0', size));
        fread(shaderContent, 1, size-1, fp);
        fclose(fp);

        return shaderContent;
    }
};

int main() {
    auto platform = new PlatformOpenGL();
    return RealMain(platform);
}


#endif //GAMEENGINE_OPENGL_H
