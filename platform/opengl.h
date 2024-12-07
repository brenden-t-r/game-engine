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

        glfwSwapInterval(1); // Enables V-Sync

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

        // Setup VAOs
        glGenVertexArrays(1, &triangleVAO);
        glBindVertexArray(triangleVAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    }

    void LoadShaders() override {
        // Create and compile our GLSL program from the shaders
//        shaderProgram = LoadShaders(
//                "TextureShader.vertexshader",
//                "TextureShader.fragmentshader"
//        );
        // Use our shader
//        glUseProgram(shaderProgram);
    }

    void Run(const std::function<void()>& func) override{
        do{
            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glGenVertexArrays(1, &triangleVAO);
            glBindVertexArray(triangleVAO);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);

            func();
//            DrawTriangle();
//            DrawSprite();

            glfwPollEvents();


            glfwSwapBuffers(window);
        }
        // Check if the ESC key was pressed or the window was closed
        while(
                glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
                glfwWindowShouldClose(window) == 0
        );
    }

    void Shutdown() override{
        glDisableVertexAttribArray(0);
        glDeleteVertexArrays(1, &triangleVAO);
        glDeleteProgram(shaderProgram);

        // Close OpenGL window and terminate GLFW
        glfwTerminate();
    }

    GameObject* CreateGameObject() override {
        auto gameObject = new GameObject();
        return gameObject;
    }

    class TriangleGL : public Triangle{

    public:
        ~TriangleGL(){
            glDeleteBuffers(1, &vertexBufferObject);
        }

        void Update() override {
            Triangle::Update();
            GLfloat newVertices[] = {
                    vertex1.x, vertex1.y, 0.0f,
                    vertex2.x, vertex2.y, 0.0f,
                    vertex3.x, vertex3.y, 0.0f,
            };
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newVertices), newVertices);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        GLuint vertexBufferObject = 0;
        GLfloat vertices[9]{
                0.5f,  -0.5f, 0.0f,
                -0.5f, -0.5f, 0.0f,
                0.0, 0.5f, 0.0f,
        };
    };

    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleGL();
        glGenBuffers(1, &gameObject->vertexBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, gameObject->vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gameObject->vertices), gameObject->vertices, GL_STATIC_DRAW);
        return gameObject;
    }

//    void DrawSprite() {
//        static const GLfloat vertices[] = {
//                0.5f,  0.5f, 0.0f,  // top right
//                0.5f, -0.5f, 0.0f,  // bottom right
//                -0.5f, -0.5f, 0.0f,  // bottom left
//                -0.5f,  0.5f, 0.0f   // top left
//        };
//        glGenBuffers(1, &vertexBufferObject);
//        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//        unsigned int indices[] = {  // note that we start from 0!
//                0, 1, 3,   // first triangle
//                1, 2, 3    // second triangle
//        };
//
//        unsigned int elementBufferObject;
//        glGenBuffers(1, &elementBufferObject);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//
//        glEnableVertexAttribArray(0);
//        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
//
//        glUseProgram(shaderProgram);
//        glBindVertexArray(triangleVAO);
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
//        glDisableVertexAttribArray(0);
//
//        // Swap buffers
//        glfwSwapBuffers(window);
//
//        glDeleteBuffers(1, &vertexBufferObject);
//    }

private:
    GLFWwindow *window = nullptr;
    GLuint triangleVAO = 0;
    GLuint shaderProgram = 0;

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
