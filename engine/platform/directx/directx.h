#ifndef GAMEENGINE_DIRECTX_H
#define GAMEENGINE_DIRECTX_H

//region: Imports
#include "../../constants.h"
#include "../platform.h"
#include "file_util.h"
#include "../../engine/texture.h"
#include "../../engine/material.h"

#include <Windows.h>
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <iostream>
#include <Xinput.h>
#include "math.h"

#define MINIAUDIO_IMPLEMENTATION
#include "../../dependencies/miniaudio.h"
static ma_engine g_engine;

// Link necessary d3d11 libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxguid.lib")
//endregion

//region: Input helpers/declarations
static USHORT GetWindowsKey(KeyCode keyCode) {
    switch (keyCode) {
        case KeyCode::Up:       return VK_UP;
        case KeyCode::Down:     return VK_DOWN;
        case KeyCode::Left:     return VK_LEFT;
        case KeyCode::Right:    return VK_RIGHT;
        case KeyCode::W:        return 'W';
        case KeyCode::A:        return 'A';
        case KeyCode::S:        return 'S';
        case KeyCode::D:        return 'D';
        default:                return -1;
    }
}
static KeyCode GetKeyCode(USHORT keyCode) {
    switch (keyCode) {
        case VK_UP:       return KeyCode::Up;
        case VK_DOWN:     return KeyCode::Down;
        case VK_LEFT:     return KeyCode::Left;
        case VK_RIGHT:    return KeyCode::Right;
        case 'W':         return KeyCode::W;
        case 'A':         return KeyCode::A;
        case 'S':         return KeyCode::S;
        case 'D':         return KeyCode::D;
        default:          return KeyCode::Unknown;
    }
}
static USHORT GetWindowsGamepadButton(GamepadButton button) {
    switch (button) {
        case GamepadButton::North: return XINPUT_GAMEPAD_Y;
        case GamepadButton::South: return XINPUT_GAMEPAD_A;
        case GamepadButton::East: return XINPUT_GAMEPAD_B;
        case GamepadButton::West: return XINPUT_GAMEPAD_X;
        case GamepadButton::RB: return XINPUT_GAMEPAD_RIGHT_SHOULDER;
        case GamepadButton::LB: return XINPUT_GAMEPAD_LEFT_SHOULDER;
        case GamepadButton::R3: return XINPUT_GAMEPAD_RIGHT_THUMB;
        case GamepadButton::L3: return XINPUT_GAMEPAD_LEFT_THUMB;
        case GamepadButton::Start: return XINPUT_GAMEPAD_START;
        case GamepadButton::Select: return XINPUT_GAMEPAD_BACK;
        case GamepadButton::DLeft: return XINPUT_GAMEPAD_DPAD_LEFT;
        case GamepadButton::DRight: return XINPUT_GAMEPAD_DPAD_RIGHT;
        case GamepadButton::DUp: return XINPUT_GAMEPAD_DPAD_UP;
        case GamepadButton::DDown: return XINPUT_GAMEPAD_DPAD_DOWN;
        default: return -1;
    }
}
GamepadButton GAMEPAD_BUTTONS[] = {GamepadButton::North, GamepadButton::South, GamepadButton::East, GamepadButton::West,
                                 GamepadButton::RB, GamepadButton::LB, GamepadButton::R3, GamepadButton::L3,
                                 GamepadButton::Start, GamepadButton::Select,
                                 GamepadButton::DLeft, GamepadButton::DRight, GamepadButton::DUp, GamepadButton::DDown};
void static(*keyUpCallback)(KeyCode, void*);
static void* keyCallbackContext;
void static(*mouseUpCallback)(MouseButton, void*, vec3);
static void* mouseCallbackContext;
void static(*gamepadUpCallback)(GamepadButton, void*);
static void* gamepadCallbackContext;
//endregion

//region: Helper functions
static const WCHAR* convertToWCHAR(const char* str) {
    if (!str) return nullptr;
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    auto* wstr = new WCHAR[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, size_needed);
    return wstr; // Remember to free with `delete[]` when done.
}
//endregion

class PlatformDirectX : public Platform {
public:
    PlatformDirectX(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
        this->hInstance = hInstance;
        this->hPrevInstance = hPrevInstance;
        this->lpCmdLine = lpCmdLine;
        this->nCmdShow = nCmdShow;
    };
    ~PlatformDirectX() override= default;

    void Init() override {
        // Register the window class
        WNDCLASSEX wc = {
                sizeof(WNDCLASSEX),
                CS_CLASSDC,
                WindowProc,
                0L,
                0L,
                GetModuleHandle(nullptr),
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                "Direct3DWindowClass",
                nullptr
        };
        RegisterClassEx(&wc);

        // Disable monitor DPI scaling
#if (_WIN32_WINNT >= 0x0A00) // Windows 10+
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#elif (_WIN32_WINNT >= 0x0603) // Windows 8.1+
        SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
#else
        SetProcessDPIAware(); // Legacy API
#endif

        // Create the window
        RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        hwnd = CreateWindow(
                wc.lpszClassName,
                "Direct3D 11 Triangle",
                WS_OVERLAPPEDWINDOW,
                100, 100,
                rect.right - rect.left,
                rect.bottom - rect.top,
                nullptr, nullptr, wc.hInstance, nullptr
        );
        ShowWindow(hwnd, nCmdShow);

        // Initialize Direct3D
        InitD3D(hwnd);

        // Initialize graphics pipeline
        InitPipeline();

        // Input init
        RegisterRawInput(hwnd);

        // Init miniaudio
        ma_result result;
        result = ma_engine_init(nullptr, &g_engine);
        if (result != MA_SUCCESS) {
            printf("Failed to initialize audio engine.");
        }
        assert(result == MA_SUCCESS);
    }

    void SetGamepadVibration(int amountLeft, int amountRight) override {
        XINPUT_VIBRATION vibration;
        ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
        vibration.wLeftMotorSpeed = amountLeft; // use any value between 0-65535 here
        vibration.wRightMotorSpeed = amountRight; // use any value between 0-65535 here
        XInputSetState(0, &vibration);
    }

    void Run(void (*func)(void*), void* ctx) override {
        // Enter the message loop
        MSG msg = { nullptr };
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                XINPUT_STATE state;
                if (XInputGetState(0, &state) == ERROR_SUCCESS) {
                    gamepadStateB = gamepadStateA;
                    gamepadStateA = state;
                    for (auto & i : GAMEPAD_BUTTONS) {
                        auto xButton = GetWindowsGamepadButton(i);
                        if (!(gamepadStateA.Gamepad.wButtons & xButton) && gamepadStateB.Gamepad.wButtons & xButton) {
                            if (gamepadUpCallback && gamepadCallbackContext) {
                                gamepadUpCallback(i, gamepadCallbackContext);
                            }
                        }
                    }
                }

                // Clear the back buffer
                float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
                d3dContext->ClearRenderTargetView(renderTargetView, clearColor);

                func(ctx);

                // Present the back buffer to the screen
                swapChain->Present(1, 0);
            }
        }
    }

    // region Input Handling
    bool IsKeyPressed(KeyCode key) override {
        auto keyCode = GetWindowsKey(key);
        return (GetAsyncKeyState(keyCode) & 0x8000) != 0;
    }
    bool IsMousePressed(MouseButton button) override {
        int btn = -1;
        switch (button) {
            case MouseButton::Left: btn = VK_LBUTTON; break;
            case MouseButton::Right: btn = VK_RBUTTON; break;
            case MouseButton::Middle: btn = VK_MBUTTON; break;
        }
        return (GetAsyncKeyState(btn) & 0x8000) != 0;
    }
    struct GamepadState {
        bool NORTH, SOUTH, EAST, WEST/*, DPADDOWN, DPADUP, DPADLEFT, DPADRIGHT, START, BACK*/;
    };
    bool IsGamepadButtonPressed(GamepadButton button) override {
        XINPUT_STATE state;
        if (XInputGetState(0, &state) != ERROR_SUCCESS) return false;
        auto btn = GetWindowsGamepadButton(button);
        return state.Gamepad.wButtons & btn;
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
    void RemoveAllCallbacks() override {
        keyUpCallback = nullptr;
        mouseUpCallback = nullptr;
        gamepadUpCallback = nullptr;
    }
    vec3 GetMousePos() override {
        return _GetMousePos(hwnd);
    }

    // endregion

    //region GameObjects
    //region Triangle
    class TriangleD3D : public Triangle {
    public:
        explicit TriangleD3D(ID3D11DeviceContext* d3dContext, ID3D11Device* d3dDevice) {
            this->d3dContext = d3dContext;
            this->d3dDevice = d3dDevice;
            Init();
        }
        ~TriangleD3D() override {
            this->vertexBuffer->Release();
        }

        void Update() override {
            Triangle::Update();
            d3d_vertices[0].position.x = vertices[0].x;
            d3d_vertices[0].position.y = vertices[0].y;
            d3d_vertices[1].position.x = vertices[1].x;
            d3d_vertices[1].position.y = vertices[1].y;
            d3d_vertices[2].position.x = vertices[2].x;
            d3d_vertices[2].position.y = vertices[2].y;

            // Map the buffer to update it
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = d3dContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (SUCCEEDED(hr)) {
                memcpy(mappedResource.pData, d3d_vertices, sizeof(d3d_vertices));
                d3dContext->Unmap(vertexBuffer, 0);
            }

            // Set the vertex buffer
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Bind shaders
            auto shader = (ShaderD3D*)material->shader;
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            d3dContext->VSSetShader(shader->vertexShader, nullptr, 0);
            d3dContext->PSSetShader(shader->pixelShader, nullptr, 0);
            d3dContext->PSSetShaderResources(0, 1, nullSRV);

            // Bind constant buffer
            D3D11_MAPPED_SUBRESOURCE mapped{};
            d3dContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            auto* dst = reinterpret_cast<uint8_t*>(mapped.pData);
            material->BindConstantBuffer(dst);
            d3dContext->Unmap(constantBuffer, 0);
            d3dContext->PSSetConstantBuffers(0, 1, &constantBuffer);

            // Draw the triangle
            d3dContext->Draw(3, 0); // Draw 3 vertices
        }
    private:
        ID3D11DeviceContext* d3dContext = nullptr;
        ID3D11Device* d3dDevice = nullptr;
        ID3D11Buffer* vertexBuffer = nullptr;
        ID3D11Buffer* constantBuffer = nullptr;
        Vertex d3d_vertices[3] {
                { DirectX::XMFLOAT3(0.5f,  -0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
                { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.5f, 0.0f) },
                { DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT2(0.25f, 0.5f) }
        };

        void Init() {
            // Create the vertex buffer
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // Required for MAP_WRITE_DISCARD
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // Enables writing
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.ByteWidth = sizeof(d3d_vertices); // Size of your vertex data

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = d3d_vertices;

            HRESULT hr = d3dDevice->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
            if (FAILED(hr)) {
                printf("Error creating buffer: %ldl", hr);
            }

            // Set the vertex buffer
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Constant buffer
            D3D11_BUFFER_DESC cbd = {};
            cbd.ByteWidth = 32;
            cbd.Usage = D3D11_USAGE_DYNAMIC;
            cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            d3dDevice->CreateBuffer(&cbd, nullptr, &constantBuffer);
        }
    };
    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleD3D(d3dContext, d3dDevice);
        gameObject->material = new MaterialColor(colorShader);
        return gameObject;
    }
    //endregion
    //region Texture
    class TextureD3D : public Texture {
    public:
        TextureD3D(const char *path, TextureSettings settings, ID3D11ShaderResourceView* textureView, ID3D11SamplerState* samplerState)
                : Texture(path, settings), textureView(textureView), samplerState(samplerState) {}
        ~TextureD3D() override {
            textureView->Release();
        }
        ID3D11ShaderResourceView* textureView = nullptr;
        ID3D11SamplerState* samplerState = nullptr;
    };
    Texture* CreateTexture(const char* path, TextureSettings textureSettings) override {
        auto wchar = convertToWCHAR(path);
        ID3D11ShaderResourceView* textureView;
        LoadTextureFromFile(d3dDevice, d3dContext, wchar, &textureView);
        delete[] wchar;
        ID3D11SamplerState* samplerState;
        if (textureSettings.filter == TextureFilter::POINT) {
            samplerState = samplerStatePoint;
        } else {
            samplerState = samplerStateLinear;
        }
        auto texture = new TextureD3D(path, textureSettings, textureView, samplerState);
        return texture;
    }
    //endregion
    //region Sprite
    class SpriteD3D : public Sprite {
    public:
        SpriteD3D(ID3D11Device *d3DDevice, ID3D11DeviceContext *d3DContext, ID3D11BlendState* blendState) :
                  d3dDevice(d3DDevice), d3dContext(d3DContext), blendState(blendState) {
            // Create the vertex buffer
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.ByteWidth = sizeof(d3d_vertices);
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = d3d_vertices;
            d3dDevice->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
        }

        ~SpriteD3D() override = default;

        void SetMaterial(Material* mat) override {
            Sprite::SetMaterial(mat);
            CreateConstantBuffer();
        }

        void CreateConstantBuffer() {
            if (constantBuffer != nullptr) {
                constantBuffer->Release();
            }
            D3D11_BUFFER_DESC cbd = {};
            cbd.ByteWidth = ((ShaderD3D*)material->shader)->constantBufferSize;
            cbd.Usage = D3D11_USAGE_DYNAMIC;
            cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            d3dDevice->CreateBuffer(&cbd, nullptr, &constantBuffer);
        }

        void Update() override {
            Sprite::Update();
            d3d_vertices[0].position.x = vertices[0].x;
            d3d_vertices[0].position.y = vertices[0].y;
            d3d_vertices[1].position.x = vertices[1].x;
            d3d_vertices[1].position.y = vertices[1].y;
            // Bottom left and bottom right are flipped in DirectX; order matters
            d3d_vertices[3].position.x = vertices[2].x;
            d3d_vertices[3].position.y = vertices[2].y;
            d3d_vertices[2].position.x = vertices[3].x;
            d3d_vertices[2].position.y = vertices[3].y;

            if (useAtlas && !useGlyph) {
                d3d_vertices[0].texCoord.x = atlasCellSize * (float)atlasColumn;
                d3d_vertices[1].texCoord.x = atlasCellSize * (float)atlasColumn + atlasCellSize;
                d3d_vertices[2].texCoord.x = atlasCellSize * (float)atlasColumn;
                d3d_vertices[3].texCoord.x = atlasCellSize * (float)atlasColumn + atlasCellSize;
                d3d_vertices[0].texCoord.y = atlasCellSize * (float)atlasRow;
                d3d_vertices[1].texCoord.y = atlasCellSize * (float)atlasRow;
                d3d_vertices[2].texCoord.y = atlasCellSize * (float)atlasRow + atlasCellSize;
                d3d_vertices[3].texCoord.y = atlasCellSize * (float)atlasRow + atlasCellSize;
            }
            if (useAtlas && useGlyph) {
                float modifier = 0.0f;
                float u0 = (glyphX + modifier) / (float)atlasWidth;
                float v0 = (glyphY + modifier) / (float)atlasHeight;
                float u1 = (glyphX + glyphW - modifier) / (float)atlasWidth;
                float v1 = (glyphY + glyphH - modifier) / (float)atlasHeight;
                d3d_vertices[0].texCoord.x = u0; d3d_vertices[0].texCoord.y = v0;
                d3d_vertices[1].texCoord.x = u1; d3d_vertices[1].texCoord.y = v0;
                d3d_vertices[2].texCoord.x = u0; d3d_vertices[2].texCoord.y = v1;
                d3d_vertices[3].texCoord.x = u1; d3d_vertices[3].texCoord.y = v1;
            }

            // Set the blend state
            float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            d3dContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

            // Set the sampler state
            auto tex = (TextureD3D*)((MaterialSprite*)material)->texture;
            d3dContext->PSSetSamplers(0, 1, &tex->samplerState);

            // Map the buffer to update it
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = d3dContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (SUCCEEDED(hr)) {
                memcpy(mappedResource.pData, d3d_vertices, sizeof(d3d_vertices));
                d3dContext->Unmap(vertexBuffer, 0);
            }

            // Set the vertex buffer
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

            // Bind shader resources
            auto shader = (ShaderD3D*)material->shader;
            d3dContext->VSSetShader(shader->vertexShader, nullptr, 0);
            d3dContext->PSSetShader(shader->pixelShader, nullptr, 0);
            d3dContext->PSSetShaderResources(0, 1, &tex->textureView);
            d3dContext->IASetInputLayout(shader->inputLayout);

            // Bind constant buffer
            D3D11_MAPPED_SUBRESOURCE mapped{};
            d3dContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            auto* dst = reinterpret_cast<uint8_t*>(mapped.pData);
            memset(mapped.pData, 0, shader->constantBufferSize);
            material->BindConstantBuffer(dst);
            d3dContext->Unmap(constantBuffer, 0);
            d3dContext->PSSetConstantBuffers(0, 1, &constantBuffer);

            // Draw
            d3dContext->Draw(4, 0);
        }

        Texture* GetTexture() override {
            return (TextureD3D*)((MaterialSprite*)material)->texture;
        }

    private:
        ID3D11Device* d3dDevice = nullptr;
        ID3D11DeviceContext* d3dContext = nullptr;
        ID3D11BlendState* blendState = nullptr;
        ID3D11Buffer* vertexBuffer = nullptr;
        ID3D11Buffer* constantBuffer = nullptr;

        Vertex d3d_vertices[4] {
                // Order matters
                { DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) }, // Top left
                { DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }, // Top Right
                { DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) }, // Bottom Left
                { DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }, // Bottom Right
        };
    };
    Sprite* CreateSprite(const char * path) override {
        auto texture = (TextureD3D*)CreateTexture(path, DEFAULT_TEXTURE_SETTINGS);
        return CreateSprite(texture);
    }
    Sprite* CreateSprite(Texture* texture) override {
        auto gameObject = new SpriteD3D(d3dDevice, d3dContext, blendState);
        gameObject->material = new MaterialSprite(textureShader, texture);
        gameObject->CreateConstantBuffer();
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

    //region Shaders
    // Consider making this an abstraction class with dynamic creation
    D3D11_INPUT_ELEMENT_DESC INPUT_LAYOUT_POSITION[1] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    D3D11_INPUT_ELEMENT_DESC INPUT_LAYOUT_POSITION_TEXCOORD[2] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    class ShaderD3D : public Shader {
    public:
        ID3D11VertexShader* vertexShader;
        ID3D11PixelShader* pixelShader;
        ID3D11InputLayout* inputLayout;
        unsigned int constantBufferSize;
    };
    Shader* colorShader = nullptr;
    Shader* textureShader = nullptr;
    void LoadShaders() override {
        colorShader = LoadShader("assets/shaders/color.hlsl", InputLayoutType::POSITION);
        textureShader = LoadShader("assets/shaders/texture.hlsl", InputLayoutType::POSITION_TEXCOORD);
    }
    Shader* LoadShader(ShaderDef shaderDef) override {
        return LoadShader(shaderDef.path, shaderDef.inputLayoutType);
    }
    Shader* LoadShader(const char* path, InputLayoutType inputLayoutType) {
        D3D11_INPUT_ELEMENT_DESC* layout;
        int numElements;
        switch (inputLayoutType) {
            case InputLayoutType::POSITION: layout = INPUT_LAYOUT_POSITION; numElements = 1; break;
            case InputLayoutType::POSITION_TEXCOORD: layout = INPUT_LAYOUT_POSITION_TEXCOORD; numElements = 2; break;
        }
        const WCHAR* shaderPath = convertToWCHAR(path);
        // Compile vertex shader
        ID3D11VertexShader* vertexShader;
        ID3DBlob* vsBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(shaderPath, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr);
        if (!SUCCEEDED(hr)) {
            printf("Failed to compile vertex shader: %ld\n", hr);
            exit(1);
        }
        d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
        // Compile pixel shader
        ID3D11PixelShader* pixelShader;
        ID3DBlob* psBlob = nullptr;
        hr = D3DCompileFromFile(shaderPath, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, nullptr);
        if (!SUCCEEDED(hr)) {
            printf("Failed to compile pixel shader: %ld\n", hr);
            exit(1);
        }
        d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
        // Input layout
        ID3D11InputLayout* inputLayout;
        d3dDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
        d3dContext->IASetInputLayout(inputLayout);

        auto* shader = new ShaderD3D();
        shader->vertexShader = vertexShader;
        shader->pixelShader = pixelShader;
        shader->inputLayout = inputLayout;

        ID3D11ShaderReflection* reflection = nullptr;
        D3DReflect(
                psBlob->GetBufferPointer(),
                psBlob->GetBufferSize(),
                IID_ID3D11ShaderReflection,
                (void**)&reflection
                );
        D3D11_SHADER_DESC shaderDesc;
        reflection->GetDesc(&shaderDesc);
        ID3D11ShaderReflectionConstantBuffer* cb;
        if (shaderDesc.ConstantBuffers > 0) {
            cb = reflection->GetConstantBufferByIndex(0);
            D3D11_SHADER_BUFFER_DESC bufferDesc;
            cb->GetDesc(&bufferDesc);
            shader->constantBufferSize = bufferDesc.Size;
            for (int i = 0; i < bufferDesc.Variables; i++) {
                auto shaderVar = cb->GetVariableByIndex(i);
                D3D11_SHADER_VARIABLE_DESC varDesc;
                shaderVar->GetDesc(&varDesc);
                Shader::UniformFieldOffset field;
                field.name = varDesc.Name;
                field.offset = varDesc.StartOffset;
                shader->uniformFieldOffsets.push_back(field);
            }
        }

        // Clean up shader blobs
        vsBlob->Release();
        psBlob->Release();

        return shader;
    }
    //endregion

    void Shutdown() override {
        CleanD3D();
    }

private:
    // region Private Variables
    // Entry-point args
    HWND hwnd = nullptr;
    HINSTANCE hInstance = nullptr;
    HINSTANCE hPrevInstance = nullptr;
    LPSTR lpCmdLine = nullptr;
    int nCmdShow = 0;

    // DirectX variables
    IDXGISwapChain* swapChain = nullptr;
    ID3D11Device* d3dDevice = nullptr;
    ID3D11DeviceContext* d3dContext = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;
    ID3D11BlendState* blendState = nullptr;
    ID3D11SamplerState* samplerStatePoint = nullptr;
    ID3D11SamplerState* samplerStateLinear = nullptr;

    // State vars
    XINPUT_STATE gamepadStateA = {};
    XINPUT_STATE gamepadStateB = {};
    // endregion

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_INPUT: {
                BYTE rawData[sizeof(RAWINPUT)];
                UINT dataSize = sizeof(rawData);
                if (GetRawInputData((HRAWINPUT) lParam, RID_INPUT, rawData, &dataSize, sizeof(RAWINPUTHEADER)) > 0) {
                    auto raw = reinterpret_cast<RAWINPUT*>(rawData);

                    if (raw->header.dwType == RIM_TYPEKEYBOARD) {
                        RAWKEYBOARD &rawKb = raw->data.keyboard;
                        bool isKeyDown = (rawKb.Message == WM_KEYDOWN || rawKb.Message == WM_SYSKEYDOWN);
                        bool isKeyUp = (rawKb.Message == WM_KEYUP || rawKb.Message == WM_SYSKEYUP);

                        int key = rawKb.VKey;

                        // Update your key state table
                        if (isKeyUp) {
                            if (keyUpCallback) {
                                keyUpCallback(GetKeyCode(key), keyCallbackContext);
                            }
                        }
                    }
                    else if (raw->header.dwType == RIM_TYPEMOUSE) {
                        RAWMOUSE &rawM = raw->data.mouse;

                        // Check button states
                        if (rawM.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
                            if (mouseUpCallback) {
                                mouseUpCallback(MouseButton::Left, mouseCallbackContext, _GetMousePos(hWnd));
                            }
                        }
                        if (rawM.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
                            if (mouseUpCallback) {
                                mouseUpCallback(MouseButton::Right, mouseCallbackContext, _GetMousePos(hWnd));
                            }
                        }
                        if (rawM.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
                            if (mouseUpCallback) {
                                mouseUpCallback(MouseButton::Middle, mouseCallbackContext, _GetMousePos(hWnd));
                            }
                        }
                    }
                }
                break;
            }

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    static void RegisterRawInput(HWND hwnd) {
        RAWINPUTDEVICE rid[2];

        // Register keyboard
        rid[0].usUsagePage = 0x01;  // Generic Desktop Controls
        rid[0].usUsage = 0x06;      // Keyboard
        rid[0].dwFlags = RIDEV_INPUTSINK; // Receive input even if not focused
        rid[0].hwndTarget = hwnd;

        // Register mouse
        rid[1].usUsagePage = 0x01;  // Generic Desktop Controls
        rid[1].usUsage = 0x02;      // Mouse
        rid[1].dwFlags = RIDEV_INPUTSINK; // Receive input even if not focused
        rid[1].hwndTarget = hwnd;

        // Register both devices
        if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
            printf("Failed to register raw input device.");
        }
    }

    static vec3 _GetMousePos(HWND hWnd) {
        RECT rect;
        GetClientRect(hWnd, &rect);  // Get window size
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        POINT cursorPos;
        if (GetCursorPos(&cursorPos)) {
            ScreenToClient(hWnd, &cursorPos);  // Convert to client space

            float ndcX = (2.0f * cursorPos.x) / width - 1.0f;
            float ndcY = 1.0f - (2.0f * cursorPos.y) / height;  // Flip Y axis
            return {ndcX, ndcY};
        } else assert(false);
    }

    void InitD3D(HWND hwnd)
    {
        // Get window dimensions
        RECT rc;
        GetClientRect(hwnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        WINDOW_WIDTH = width;
        WINDOW_HEIGHT = height;

        // Swap chain descriptor
        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferCount = 1;
        scd.BufferDesc.Width = width;
        scd.BufferDesc.Height = height;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hwnd;
        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        // Device creation flags
        UINT createDeviceFlags = 0;
//#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif

        // Create the device, device context, and swap chain
        D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, NULL, 0,
                                      D3D11_SDK_VERSION, &scd, &swapChain, &d3dDevice, NULL, &d3dContext);

        // Create the render target view
        ID3D11Texture2D* backBuffer;
        swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
        d3dDevice->CreateRenderTargetView(backBuffer, NULL, &renderTargetView);
        backBuffer->Release();

        // Set the render target
        d3dContext->OMSetRenderTargets(1, &renderTargetView, NULL);

        // Set the viewport
        D3D11_VIEWPORT viewport = {};
        viewport.Width = WINDOW_WIDTH * 1.0f;
        viewport.Height = WINDOW_HEIGHT * 1.0f;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        d3dContext->RSSetViewports(1, &viewport);
    }

    void InitPipeline() {
        // Linear clamp
        D3D11_SAMPLER_DESC samplerDescLinear = {};
        samplerDescLinear.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDescLinear.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDescLinear.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDescLinear.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDescLinear.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDescLinear.MinLOD = 0;
        samplerDescLinear.MaxLOD = D3D11_FLOAT32_MAX;
        d3dDevice->CreateSamplerState(&samplerDescLinear, &samplerStateLinear);

        // Point clamp
        D3D11_SAMPLER_DESC samplerDescPoint = {};;
        samplerDescPoint.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDescPoint.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDescPoint.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDescPoint.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDescPoint.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDescPoint.MinLOD = 0;
        samplerDescPoint.MaxLOD = D3D11_FLOAT32_MAX;
        d3dDevice->CreateSamplerState(&samplerDescPoint, &samplerStatePoint);

        // Set sampler state to linear
        d3dContext->PSSetSamplers(0, 1, &samplerStateLinear);

        // Update the blend state to handle alpha
        D3D11_BLEND_DESC blendDesc = { 0 };
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        d3dDevice->CreateBlendState(&blendDesc, &blendState);
    }

    void CleanD3D()
    {
        renderTargetView->Release();
        swapChain->Release();
        d3dDevice->Release();
        d3dContext->Release();
    }
};

#endif //GAMEENGINE_DIRECTX_H
