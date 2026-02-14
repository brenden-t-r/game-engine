#ifndef GAMEENGINE_OPENGL_H
#define GAMEENGINE_OPENGL_H

//region Imports
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

#define MINIAUDIO_IMPLEMENTATION
#include "../../dependencies/miniaudio.h"
static ma_engine g_engine;
//endregion

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
        default: return -1;
    }
}
static int GetGLFWGamepadButton(GamepadButton button) {
    switch (button) {
        case GamepadButton::North: return GLFW_GAMEPAD_BUTTON_Y;
        case GamepadButton::South: return GLFW_GAMEPAD_BUTTON_A;
        case GamepadButton::East: return GLFW_GAMEPAD_BUTTON_B;
        case GamepadButton::West: return GLFW_GAMEPAD_BUTTON_X;
        case GamepadButton::RB: return GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;
        case GamepadButton::LB: return GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;
        case GamepadButton::R3: return GLFW_GAMEPAD_BUTTON_RIGHT_THUMB;
        case GamepadButton::L3: return GLFW_GAMEPAD_BUTTON_LEFT_THUMB;
        case GamepadButton::Start: return GLFW_GAMEPAD_BUTTON_START;
        case GamepadButton::Select: return GLFW_GAMEPAD_BUTTON_BACK;
        case GamepadButton::DLeft: return GLFW_GAMEPAD_BUTTON_DPAD_LEFT;
        case GamepadButton::DRight: return GLFW_GAMEPAD_BUTTON_DPAD_RIGHT;
        case GamepadButton::DUp: return GLFW_GAMEPAD_BUTTON_DPAD_UP;
        case GamepadButton::DDown: return GLFW_GAMEPAD_BUTTON_DPAD_DOWN;
        default: return -1;
    }
}
GamepadButton GAMEPAD_BUTTONS[] = {GamepadButton::North, GamepadButton::South, GamepadButton::East, GamepadButton::West,
                                   GamepadButton::RB, GamepadButton::LB, GamepadButton::R3, GamepadButton::L3,
                                   GamepadButton::Start, GamepadButton::Select,
                                   GamepadButton::DLeft, GamepadButton::DRight, GamepadButton::DUp, GamepadButton::DDown};
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
void static(*mouseUpCallback)(MouseButton, void*, vec3);
static void* mouseCallbackContext;
void static(*gamepadUpCallback)(GamepadButton, void*);
static void* gamepadCallbackContext;
static vec3 get_mouse_pos(GLFWwindow* window) {
    double cursorX, cursorY;
    int width, height;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    glfwGetWindowSize(window, &width, &height);
    float ndcX = 2.0f * (float)cursorX / (float)width - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)cursorY) / (float)height;  // Flip Y axis
    return {ndcX, ndcY, 0};
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    vec3 mousePos = get_mouse_pos(window);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && mouseUpCallback) {
        mouseUpCallback(MouseButton::Left, mouseCallbackContext, mousePos);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && mouseUpCallback) {
        mouseUpCallback(MouseButton::Right, mouseCallbackContext, mousePos);
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE && mouseUpCallback) {
        mouseUpCallback(MouseButton::Middle, mouseCallbackContext, mousePos);
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

        // Init miniaudio
        ma_result result;
        result = ma_engine_init(nullptr, &g_engine);
        if (result != MA_SUCCESS) {
            printf("Failed to initialize audio engine.");
        }
        assert(result == MA_SUCCESS);
    }

    void Run(void (*func)(void*), void* ctx) override{
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetKeyCallback(window, key_callback);

        do{
            GLFWgamepadstate state;
            if (glfwGetGamepadState(0, &state)) {
                gamepadStateB = gamepadStateA;
                gamepadStateA = state;
                for (auto & i : GAMEPAD_BUTTONS) {
                    auto glfwButton = GetGLFWGamepadButton(i);
                    if (gamepadStateA.buttons[glfwButton] == GLFW_RELEASE && gamepadStateB.buttons[glfwButton] == GLFW_PRESS) {
                        if (gamepadUpCallback && gamepadCallbackContext) {
                            gamepadUpCallback(i, gamepadCallbackContext);
                        }
                    }
                }
            }

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
    bool IsGamepadButtonPressed(GamepadButton button) override {
        return false;
        if (!glfwJoystickPresent(0)) return false;
        GLFWgamepadstate state;
        if (!glfwGetGamepadState(0, &state)) return false;
        auto btn = GetGLFWGamepadButton(button);
        return state.buttons[btn] == GLFW_PRESS;
    }
    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {
        keyUpCallback = func;
        keyCallbackContext = context;
    }
    void SetMouseReleasedCallback(void (*func)(MouseButton, void*, vec3), void* context) override {
        mouseUpCallback = func;
        mouseCallbackContext = context;
    }
    void SetGamepadReleasedCallback(void (*func)(GamepadButton, void*), void* context) override {
        gamepadUpCallback = func;
        gamepadCallbackContext = context;
    }
    vec3 GetMousePos() override {
        return get_mouse_pos(window);
    }
    //endregion

    //region GameObjects
    //region Triangle
    class TriangleGL : public Triangle{
    public:
        void Update() override {
            Triangle::Update();
            GLfloat newVertices[] = {
                    vertices[0].x, vertices[0].y, 0.0f,
                    vertices[1].x, vertices[1].y, 0.0f,
                    vertices[2].x, vertices[2].y, 0.0f,
            };
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Bind shaders and uniforms
            auto shader = (ShaderGL*)material->shader;
            glUseProgram(shader->shaderProgram);
            auto mat = (Material*)material;
            mat->BindConstantBuffer(shader->shaderProgram);

            // Bind VAO, VBO
            glBindVertexArray(vertexArrayObject);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newVertices), newVertices);

            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        GLuint vertexArrayObject = 0;
        GLuint vertexBufferObject = 0;
    };
    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleGL();
        gameObject->material = new MaterialColor(colorShader);
        gameObject->vertexArrayObject = triangleVAO;
        gameObject->vertexBufferObject = triangleVBO;
        return gameObject;
    }
    //endregion
    //region Texture
    class TextureGL : public Texture {
    public:
        TextureGL(const char *path, GLuint textureID) : Texture(path), textureID(textureID) {}
        ~TextureGL() override {
            glDeleteTextures(1, &textureID);
        }
        GLuint textureID;
    };
    TextureGL* CreateTexture(const char* path) override {
        GLuint texture = loadTexture(path);
        assert(texture != 0);
        return new TextureGL(nullptr, texture);
    }
    //endregion
    //region Sprite
    class SpriteGL : public Sprite {
    public:
        void Update() override {
            Sprite::Update();
            float newVertices[] = {
                    // Positions                 // Texture Coords
                    vertices[0].x, vertices[0].y, 0.0f,  0.0f, 1.0f, // Top-left
                    vertices[1].x, vertices[1].y, 0.0f,  1.0f, 1.0f, // Top-right
                    vertices[2].x, vertices[2].y, 0.0f,  1.0f, 0.0f, // Bottom-right
                    vertices[3].x, vertices[3].y, 0.0f,  0.0f, 0.0f  // Bottom-left
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

            auto shader = (ShaderGL*)material->shader;
            auto tex = (TextureGL*)((MaterialSprite*)material)->texture;

            glUseProgram(shader->shaderProgram);
            glBindVertexArray(vertexArrayObject);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
            glBindTexture(GL_TEXTURE_2D, tex->textureID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newVertices), newVertices);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }
        Texture* GetTexture() override {
            return (TextureGL*)((MaterialSprite*)material)->texture;
        }
        GLuint vertexArrayObject = 0;
        GLuint vertexBufferObject = 0;
    };
    Sprite* CreateSprite(const char* path) override {
        auto texture = CreateTexture(path);
        return CreateSprite(texture);
    }
    Sprite* CreateSprite(Texture* texture) override {
        auto gameObject = new SpriteGL();
        gameObject->material = new MaterialSprite(textureShader, texture);
        gameObject->vertexArrayObject = quadVAO;
        gameObject->vertexBufferObject = quadVBO;
        return gameObject;
    }
    //endregion
    //region Audio
    class MiniAudioSound : public Sound {
    public:
        ~MiniAudioSound() override{
            ma_sound_uninit(&sound);
        };

        void Init(const char* filePath) {
            ma_result result = ma_sound_init_from_file(&g_engine, filePath, MA_SOUND_FLAG_DECODE, nullptr, nullptr, &sound);
            if (result != MA_SUCCESS) {
                printf("Failed to initialize audio sound.");
            }
            assert(result == MA_SUCCESS);
        }

        void Play() override {
            ma_sound_start(&sound);
        }

        void Stop() override {
            ma_sound_stop(&sound);
        }

        void Reset() override {
            ma_sound_seek_to_pcm_frame(&sound, 0);
        }

        ma_sound sound{};
    };
    Sound* CreateSound(const char* path) override {
        auto sound = new MiniAudioSound();
        sound->Init(path);
        return sound;
    }
    //endregion
    //endregion

    // region: Shaders
    class ShaderGL: public Shader{
    public:
        GLuint shaderProgram;
    };
    ShaderGL* colorShader = nullptr;
    ShaderGL* textureShader = nullptr;
    void LoadShaders() override {
        colorShader = LoadShader(
                "assets/shaders/color.glsl.vert",
                "assets/shaders/color.glsl.frag"
        );
        textureShader = LoadShader(
                "assets/shaders/texture.glsl.vert",
                "assets/shaders/texture.glsl.frag"
        );
    }
    Shader* LoadShader(ShaderDef shaderDef) override {
        return LoadShader(shaderDef.vertexPath, shaderDef.fragmentPath);
    }
    static ShaderGL* LoadShader(const char* vertexPath, const char* fragmentPath) {
        auto vertexSource = getFileContent(vertexPath);
        auto fragmentSource = getFileContent(fragmentPath);

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

        // Create shader
        auto* shader = new ShaderGL();
        shader->shaderProgram = program;
        return shader;
    }
    static GLuint compileShader(const char* source, GLenum type) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
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

    void Shutdown() override {
        glDisableVertexAttribArray(0);
        glDeleteVertexArrays(1, &triangleVAO);
        glDeleteProgram(colorShader->shaderProgram);
        glDeleteProgram(textureShader->shaderProgram);

        // Close OpenGL window and terminate GLFW
        glfwTerminate();
    }

private:

    GLFWwindow *window = nullptr;
    GLuint triangleVAO = 0;
    GLuint triangleVBO = 0;
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    GLFWgamepadstate gamepadStateA = {};
    GLFWgamepadstate gamepadStateB = {};

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
    static char* getFileContent(const char* fileName)
    {
        FILE *fp;
        long size = 0;
        char* shaderContent;

        /* Read File to get size */
        fp = fopen(fileName, "rb");
        if(fp == nullptr) {
            printf("Error reading %s\n", fileName);
            assert(false);
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

#endif //GAMEENGINE_OPENGL_H
