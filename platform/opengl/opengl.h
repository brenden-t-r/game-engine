#ifndef GAMEENGINE_OPENGL_H
#define GAMEENGINE_OPENGL_H

#include "../../constants.h"
#include "../platform.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//region Input Helpers / Callbacks
static int GetGLFWKey(KeyCode keyCode) {
    switch (keyCode) {
        case KeyCode::Up:       return GLFW_KEY_UP;
        case KeyCode::Down:     return GLFW_KEY_DOWN;
        case KeyCode::Left:     return GLFW_KEY_LEFT;
        case KeyCode::Right:    return GLFW_KEY_RIGHT;
        case KeyCode::W:        return GLFW_KEY_W;
        case KeyCode::A:        return GLFW_KEY_A;
        case KeyCode::S:        return GLFW_KEY_S;
        case KeyCode::D:        return GLFW_KEY_D;
        default:                return -1;
    }
}
static int GetGLFWMouseButton(MouseButton button) {
    switch(button) {
        case MouseButton::Left: return GLFW_MOUSE_BUTTON_LEFT;
        case MouseButton::Right: return GLFW_MOUSE_BUTTON_RIGHT;
        case MouseButton::Middle: return GLFW_MOUSE_BUTTON_MIDDLE;
        default: -1;
    }
}
static KeyCode GetKeyCode(int glfwKey) {
    switch (glfwKey) {
        case GLFW_KEY_UP:       return KeyCode::Up;
        case GLFW_KEY_DOWN:     return KeyCode::Down;
        case GLFW_KEY_LEFT:     return KeyCode::Left;
        case GLFW_KEY_RIGHT:    return KeyCode::Right;
        case GLFW_KEY_W:        return KeyCode::W;
        case GLFW_KEY_A:        return KeyCode::A;
        case GLFW_KEY_S:        return KeyCode::S;
        case GLFW_KEY_D:        return KeyCode::D;
        default:                return KeyCode::Unknown;
    }
}
void static(*keyUpCallback)(KeyCode, void*);
static void* keyCallbackContext;
void static(*mouseUpCallback)(MouseButton, void*);
static void* mouseCallbackContext;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && mouseUpCallback) {
        mouseUpCallback(MouseButton::Left, mouseCallbackContext);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && mouseUpCallback) {
        mouseUpCallback(MouseButton::Right, mouseCallbackContext);
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE && mouseUpCallback) {
        mouseUpCallback(MouseButton::Middle, mouseCallbackContext);
    }
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE && keyUpCallback) {
        keyUpCallback(GetKeyCode(key), keyCallbackContext);
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
//endregion

class PlatformOpenGL : public Platform {
public:
    PlatformOpenGL() = default;
    ~PlatformOpenGL() override = default;

    void Init() override {
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

        // Set the resize callback
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // Initialize glew
        if (glewInit() != GLEW_OK) {
            fprintf(stderr, "Failed to initialize GLEW\n");
            glfwTerminate();
            return;
        }

        printf("%s\n", glGetString(GL_VERSION));

        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

        glClearColor(0.2, 0.4, 0.6, 1.0);

        // Setup VAOs
        setupVAOs();
    }

    void LoadShaders() override {
        triangleShader = loadShaderProgram(
                "assets/shaders/SimpleVertexShader.vertexshader",
                "assets/shaders/SimpleFragmentShader.fragmentshader"
        );
        textureShader = loadShaderProgram(
                "assets/shaders/TextureShader.vertexshader",
                "assets/shaders/TextureShader.fragmentshader"
        );
    }

    void Run(void (*func)(void*), void* ctx) override{
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetKeyCallback(window, key_callback);

        do{
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glfwPollEvents();

            func(ctx);

            glfwSwapBuffers(window);
        }
        // Check if the ESC key was pressed or the window was closed
        while(
                glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
                glfwWindowShouldClose(window) == 0
        );
    }

    //region Input Handling
    bool IsKeyPressed(KeyCode key) override {
        auto keyCode = GetGLFWKey(key);
        int state = glfwGetKey(window, keyCode);
        return state == GLFW_PRESS;
    }
    bool IsMousePressed(MouseButton button) override {
        auto btn = GetGLFWMouseButton(button);
        int state = glfwGetMouseButton(window, btn);
        return state == GLFW_PRESS;
    }
    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {
        keyUpCallback = func;
        keyCallbackContext = context;
    }
    void SetMouseReleasedCallback(void (*func)(MouseButton, void*), void* context) override {
        mouseUpCallback = func;
        mouseCallbackContext = context;
    }
    vector3 GetMousePos() override {
        double cursorX, cursorY;
        int width, height;
        glfwGetCursorPos(window, &cursorX, &cursorY);
        glfwGetWindowSize(window, &width, &height);
        float ndcX = 2.0f * (float)cursorX / (float)width - 1.0f;
        float ndcY = 1.0f - (2.0f * (float)cursorY) / (float)height;  // Flip Y axis
        return {ndcX, ndcY, 0};
    }
    //endregion

    //region GameObjects
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
            glUseProgram(shaderProgram); // Use appropriate shader
            glBindVertexArray(vertexArrayObject);
            glBindBuffer(GL_ARRAY_BUFFER, vertexArrayObject); // Bind the triangle's VBO
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newVertices), newVertices);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        GLuint shaderProgram = 0;
        GLuint vertexArrayObject = 0;
        GLuint vertexBufferObject = 0;
    };

    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleGL();
        gameObject->shaderProgram = triangleShader;
        gameObject->vertexArrayObject = triangleVAO;
        gameObject->vertexBufferObject = triangleVBO;
        return gameObject;
    }

    class SpriteGL : public Sprite {
    public:
        ~SpriteGL() {
            glDeleteBuffers(1, &vertexBufferObject);
        }

        void SetTexture(const char* path) {
            texture = loadTexture(path);

            if (texture == 0) {
                std::cerr << "Failed to load sprite texture!" << std::endl;
                return;
            }
        }

        void Update() override {
            Sprite::Update();
            float newVertices[] = {
                    // Positions                 // Texture Coords
                    vertex1.x, vertex1.y, 0.0f,  0.0f, 1.0f, // Top-left
                    vertex2.x, vertex2.y, 0.0f,  1.0f, 1.0f, // Top-right
                    vertex3.x, vertex3.y, 0.0f,  1.0f, 0.0f, // Bottom-right
                    vertex4.x, vertex4.y, 0.0f,  0.0f, 0.0f  // Bottom-left
            };

            int row = atlasNumRows - atlasRow - 1;
            if (useAtlas) {
                newVertices[3] = atlasCellSize * (float)atlasColumn; // Top-left
                newVertices[4] = atlasCellSize * (float)row + atlasCellSize;
                newVertices[8] = atlasCellSize * (float)atlasColumn + atlasCellSize; // Top-right
                newVertices[9] = atlasCellSize * (float)row+ atlasCellSize;
                newVertices[13] = atlasCellSize * (float)atlasColumn + atlasCellSize; // Bottom-right
                newVertices[14] = atlasCellSize * (float)row;
                newVertices[18] = atlasCellSize * (float)atlasColumn;  // Bottom-left
                newVertices[19] = atlasCellSize * (float)row;
            }

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUseProgram(shaderProgram); // Use appropriate shader
            glBindVertexArray(vertexArrayObject);
            glBindBuffer(GL_ARRAY_BUFFER, vertexArrayObject);
            glBindTexture(GL_TEXTURE_2D, texture);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newVertices), newVertices);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }

        GLuint texture = 0;
        GLuint shaderProgram = 0;
        GLuint vertexArrayObject = 0;
        GLuint vertexBufferObject = 0;
    };

    Sprite* CreateSprite(const char* path) override {
        auto gameObject = new SpriteGL();
        gameObject->SetTexture(path);
        gameObject->shaderProgram = textureShader;
        gameObject->vertexArrayObject = quadVAO;
        gameObject->vertexBufferObject = quadVBO;
        return gameObject;
    }
    //endregion

    void Shutdown() override{
        glDisableVertexAttribArray(0);
        glDeleteVertexArrays(1, &triangleVAO);
        glDeleteProgram(textureShader);

        // Close OpenGL window and terminate GLFW
        glfwTerminate();
    }

private:
    GLFWwindow *window = nullptr;
    GLuint triangleVAO = 0;
    GLuint triangleVBO = 0;
    GLuint triangleShader = 0;
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    GLuint textureShader = 0;

    void setupVAOs() {
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

        // VAO and VBO for Triangle
        glGenVertexArrays(1, &triangleVAO);
        glGenBuffers(1, &triangleVBO);

        // VAO, VBO, and EBO for Quad
        unsigned int quadEBO;
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
    }

    //region Shaders
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

    static GLuint compileShader(const char* source, GLenum type) {
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
    //endregion

    static GLuint loadTexture(const char* path) {
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
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
};

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
