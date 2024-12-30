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

#define STB_IMAGE_IMPLEMENTATION
#include "opengl/stb_image.h"

// Shader compilation helper function
GLuint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

GLuint LoadTexture(const char* path);

class PlatformOpenGL : public Platform {
public:
    PlatformOpenGL() = default;
    ~PlatformOpenGL() override = default;

    void Init() override {
        /* Initialize the library */
        if (!glfwInit()) {
            fprintf(stderr, "Failed to initialize GLFW\n");
            return;
        }

        glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

        glfwSwapInterval(1); // Enables V-Sync

        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", nullptr, nullptr);
        if (window == nullptr) {
            fprintf(stderr,
                    "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(window);

        // Initialize glew
        if (glewInit() != GLEW_OK) {
            fprintf(stderr, "Failed to initialize GLEW\n");
            glfwTerminate();
            return;
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
//        glEnableVertexAttribArray(0);
    }

    void LoadShaders() override {
        auto vertexSource = getFileContent("assets/shaders/TextureShader.vertexshader");
        auto fragmentSource = getFileContent("assets/shaders/TextureShader.fragmentshader");

        // Compile shaders
        GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

        // Create program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // Check for linking errors
        GLint success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        }

        // Cleanup
        glDetachShader(shaderProgram, vertexShader);
        glDetachShader(shaderProgram, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glUseProgram(shaderProgram);
    }

    void Run(const std::function<void()>& func) override{
        do{
            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//            glGenVertexArrays(1, &triangleVAO);
//            glBindVertexArray(triangleVAO);
//            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
//            glEnableVertexAttribArray(0);

//            func();
//            DrawTriangle();
            auto texture = LoadTexture("assets/sprites/background.png");
            DrawSprite(texture);
//              DrawSprite();

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

    class SpriteGL : public Sprite {
    public:
        ~SpriteGL() {
            glDeleteBuffers(1, &vertexBufferObject);
        }

        void SetTexture(const char* path) {
            GLuint spriteTexture = LoadTexture(path);

            if (spriteTexture == 0) {
                std::cerr << "Failed to load sprite texture!" << std::endl;
                return;
            }
        }

        void Update() override {
            Sprite::Update();
            GLfloat newVertices[] = {
                    vertex1.x, vertex1.y, 0.0f,
                    vertex2.x, vertex2.y, 0.0f,
                    vertex3.x, vertex3.y, 0.0f,
                    vertex4.x, vertex4.y, 0.0f,
            };
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newVertices), newVertices);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
//            glUseProgram(shaderProgram);
//            glBindVertexArray(triangleVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glDisableVertexAttribArray(0);
        }

        GLuint vertexBufferObject = 0;
        const GLfloat vertices[12] {
            0.5f,  0.5f, 0.0f,  // top right
            0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f,  0.5f, 0.0f   // top left
        };
        unsigned int indices[6] {
            // note that we start from 0!
            0, 1, 3,   // first triangle
            1, 2, 3    // second triangle
        };
    };

    GameObject* CreateSprite() override {
        auto gameObject = new SpriteGL();
        gameObject->SetTexture("assets/sprites/background.png");
        glGenBuffers(1, &gameObject->vertexBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, gameObject->vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gameObject->vertices), gameObject->vertices, GL_STATIC_DRAW);
        unsigned int elementBufferObject;
        glGenBuffers(1, &elementBufferObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gameObject->indices), gameObject->indices, GL_STATIC_DRAW);
        return gameObject;
    }

    void DrawSprite(GLuint textureID) {
        float m[9];
        m[0] = 1.0f; m[3] = 0.0f; m[6] = 0.0f;
        m[1] = 0.0f; m[4] = 1.0f; m[7] = 0.0f;
        m[2] = 0.0f; m[5] = 0.0f; m[8] = 1.0f;

        // Vertex data for a quad (position and texture coordinates)
        float vertices[] = {
                // Pos      // Tex
                -1.0f, 1.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,

                -1.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 1.0f, 0.0f
        };

        // Create and bind VBO
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Set vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Assuming you have a basic shader program that accepts these uniforms
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, m);
        glUniform4f(glGetUniformLocation(shaderProgram, "spriteColor"), 1.0, 1.0, 1.0, 1.0);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Draw the sprite
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Cleanup
        glDeleteBuffers(1, &VBO);
    }

    void DrawSprite() {
        static const GLfloat vertices[] = {
                1.0f,  1.0f, 0.0f,  // top right
                1.0f, -1.0f, 0.0f,  // bottom right
                -1.0f, -1.0f, 0.0f,  // bottom left
                -1.0f,  1.0f, 0.0f   // top left
        };
        GLuint vertexBufferObject = 0;
        glGenBuffers(1, &vertexBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        unsigned int indices[] = {  // note that we start from 0!
                0, 1, 3,   // first triangle
                1, 2, 3    // second triangle
        };

        unsigned int elementBufferObject;
        glGenBuffers(1, &elementBufferObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        glUseProgram(shaderProgram);
        glBindVertexArray(triangleVAO);


        // Assuming you have a basic shader program that accepts these uniforms
        float m[9];
        m[0] = 1.0f; m[3] = 0.0f; m[6] = 0.0f;
        m[1] = 0.0f; m[4] = 1.0f; m[7] = 0.0f;
        m[2] = 0.0f; m[5] = 0.0f; m[8] = 1.0f;
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, m);
        glUniform4f(glGetUniformLocation(shaderProgram, "spriteColor"), 1.0, 1.0, 1.0, 1.0);
        // Bind texture
        auto textureID = LoadTexture("assets/sprites/background.png");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glDisableVertexAttribArray(0);

        // Swap buffers
//        glfwSwapBuffers(window);

        glDeleteBuffers(1, &vertexBufferObject);
    }

private:
    GLFWwindow *window = nullptr;
    GLuint triangleVAO = 0;
    GLuint shaderProgram = 0;

    static char* getFileContent(const char* fileName)
    {
        FILE *fp;
        long size = 0;
        char* shaderContent;

        /* Read File to get size */
        fp = fopen(fileName, "rb");
        if(fp == nullptr) {
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

GLuint LoadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Load image data
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); // OpenGL expects texture coordinates to start from bottom-left
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

    if (data) {
        GLenum format;
        switch (channels) {
            case 1:
                format = GL_RED;
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
            default:
                format = GL_RGBA;
                break;
        }

        // Bind and set texture parameters
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Free image data
        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

// Entrypoint
#if PLATFORM_WINDOWS
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    auto platform = new PlatformOpenGL();
    return RealMain(platform);
}
#else
int main() {
    auto platform = new PlatformOpenGL();
    return RealMain(platform);
}
#endif


#endif //GAMEENGINE_OPENGL_H
