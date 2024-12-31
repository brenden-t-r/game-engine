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
//        glGenVertexArrays(1, &triangleVAO);
//        glBindVertexArray(triangleVAO);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
//        glEnableVertexAttribArray(0);
    }

    void LoadShaders() override {
        textureShader = loadShaderProgram(
                "assets/shaders/TextureShader.vertexshader",
                "assets/shaders/TextureShader.fragmentshader"
        );
    }

    void Run(const std::function<void()>& func) override{
        auto triangleShader = loadShaderProgram(
                "assets/shaders/SimpleVertexShader.vertexshader",
                "assets/shaders/SimpleFragmentShader.fragmentshader"
        );

        // Triangle Data
        float triangleVertices[] = {
                // Positions
                0.0f,  0.5f, 0.0f, // Top
                -0.5f, -0.5f, 0.0f, // Left
                0.5f, -0.5f, 0.0f  // Right
        };

        // Quad Data
        float quadVertices[] = {
                // Positions      // Texture Coords
                -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // Top-left
                1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // Top-right
                1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // Bottom-right
                -1.0f, -1.0f, 0.0f,  0.0f, 0.0f  // Bottom-left
        };
        unsigned int quadIndices[] = {
                0, 1, 2, // First Triangle
                0, 2, 3  // Second Triangle
        };
        auto texture = LoadTexture("assets/sprites/background.png");

        // VAO and VBO for Triangle
        unsigned int triangleVAO, triangleVBO;
        glGenVertexArrays(1, &triangleVAO);
        glGenBuffers(1, &triangleVBO);

        // VAO, VBO, and EBO for Quad
        unsigned int quadVAO, quadVBO, quadEBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);

        glBindVertexArray(triangleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0); // Unbind VAO

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0); // Unbind VAO

        float x = 0;
        float y = 0;

        do{
            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render Textured Quad
            glUseProgram(textureShader); // Use appropriate shader
            glBindTexture(GL_TEXTURE_2D, texture);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Render Triangle
            glUseProgram(triangleShader); // Use appropriate shader
            glBindVertexArray(triangleVAO);
            glBindBuffer(GL_ARRAY_BUFFER, triangleVBO); // Bind the triangle's VBO
            x += 0.01f;
            GLfloat newVertices[] = {
                    // Positions
                    x + 0.0f, y+ 0.5f, 0.0f, // Top
                    x -0.5f, y-0.5f, 0.0f, // Left
                    x + 0.5f, y-0.5f, 0.0f  // Right
            };
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newVertices), newVertices);
            glDrawArrays(GL_TRIANGLES, 0, 3);

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
        glDeleteProgram(textureShader);

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
//            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newVertices), newVertices);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        GLuint vertexBufferObject = 0;
        GLfloat vertices[6]{
                0.5f,  -0.5f, //0.0f,
                -0.5f, -0.5f, //0.0f,
                0.0, 0.5f//, 0.0f,
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
//            glUseProgram(textureShader);
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
//        // Vertex data for a quad (position and texture coordinates)
//        float vertices[] = {
//                // Pos      // Tex
//                -1.0f, 1.0f, 0.0f, 1.0f,
//                1.0f, -1.0f, 1.0f, 0.0f,
//                -1.0f, -1.0f, 0.0f, 0.0f,
//
//                -1.0f, 1.0f, 0.0f, 1.0f,
//                1.0f, 1.0f, 1.0f, 1.0f,
//                1.0f, -1.0f, 1.0f, 0.0f
//        };
//        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

//        float m[9];
//        m[0] = 1.0f; m[3] = 0.0f; m[6] = 0.0f;
//        m[1] = 0.0f; m[4] = 1.0f; m[7] = 0.0f;
//        m[2] = 0.0f; m[5] = 0.0f; m[8] = 1.0f;
//        glUniformMatrix3fv(glGetUniformLocation(textureShader, "transform"), 1, GL_FALSE, m);

        static const GLfloat vertices[] = {
                // Pos         // Tex
                0.0f,  1.0f,   1.0f, 1.0f,  // top right
                0.0f, -1.0f,   1.0f, 0.0f,  // bottom right
                -1.0f, -1.0f,  0.0f, 0.0f,  // bottom left
                -1.0f,  1.0f,  0.0f, 1.0f   // top left
        };

        // Create and bind VBO
        GLuint vertexBufferObject = 0;
        glGenBuffers(1, &vertexBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        unsigned int indices[] = {
                0, 1, 3,   // first triangle
                1, 2, 3    // second triangle
        };
        unsigned int elementBufferObject;
        glGenBuffers(1, &elementBufferObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Set vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        // Cleanup
        glDeleteBuffers(1, &vertexBufferObject);
    }

private:
    GLFWwindow *window = nullptr;
    GLuint triangleVAO = 0;
    GLuint textureShader = 0;

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

    static GLuint loadShaderProgram(const char* vertex, const char* fragment) {
        auto vertexSource = getFileContent(vertex);
        auto fragmentSource = getFileContent(fragment);

        // Compile shaders
        GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

        // Create program
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Check for linking errors
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        }

        // Cleanup
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
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
