#ifndef GAMEENGINE_DIRECTX_H
#define GAMEENGINE_DIRECTX_H

// Include constants
#include "../../constants.h"
#include "../platform.h"

// Windows/DirectX imports
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <iostream>
#include "file_util.h"
#include "math.h"
#include <unordered_map>

// Link necessary d3d11 libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

class InputState {
public:
    bool LButtonDown;
    bool RButtonDown;
    bool MButtonDown;
    double posX;
    double posY;
    bool keyDown[10];

    bool isButtonDown(MouseButton btn) const {
        switch (btn) {
            case MouseButton::Left: return LButtonDown;
            case MouseButton::Right: return RButtonDown;
            case MouseButton::Middle: return MButtonDown;
            default: return false;
        }
    }

    bool isKeyDown(KeyCode key) const {
        int index = (int)key;
        return keyDown[index];
    }
};
static InputState inputState;
void static(*keyUpCallback)(KeyCode, void*);
static void* keyCallbackContext;
void static(*mouseUpCallback)(MouseButton, void*);
static void* mouseCallbackContext;

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
        hwnd = CreateWindow(
                wc.lpszClassName,
                "Direct3D 11 Triangle",
                WS_OVERLAPPEDWINDOW,
                100, 100,
                WINDOW_WIDTH, WINDOW_HEIGHT,
                nullptr, nullptr, wc.hInstance, nullptr
        );
        ShowWindow(hwnd, nCmdShow);

        // Initialize Direct3D
        InitD3D(hwnd);

        // Initialize graphics pipeline
        InitPipeline();

        // Input init
        RegisterRawInput(hwnd);
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
                // Clear the back buffer
                float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
                d3dContext->ClearRenderTargetView(renderTargetView, clearColor);

                func(ctx);

                // Present the back buffer to the screen
                swapChain->Present(1, 0);
            }
        }
    }

    bool IsKeyPressed(KeyCode key) override {
        auto keyCode = GetWindowsKey(key);
        return inputState.isKeyDown(key);
    }
    bool IsMousePressed(MouseButton button) override {
        return inputState.isButtonDown(button);
    }
    vector3 GetMousePos() override {
        RECT rect;
        GetClientRect(hwnd, &rect);  // Get window size
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        POINT cursorPos;
        if (GetCursorPos(&cursorPos)) {
            ScreenToClient(hwnd, &cursorPos);  // Convert to client space

            float ndcX = (2.0f * cursorPos.x) / width - 1.0f;
            float ndcY = 1.0f - (2.0f * cursorPos.y) / height;  // Flip Y axis
            return {ndcX, ndcY};
        }
    }

    class TriangleD3D : public Triangle {
    public:
        explicit TriangleD3D(ID3D11DeviceContext* d3dContext, ID3D11Device* d3dDevice, ID3D11InputLayout* inputLayout,
                             ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader) {
            this->d3dContext = d3dContext;
            this->d3dDevice = d3dDevice;
            this->inputLayout = inputLayout;
            this->vertexShader = vertexShader;
            this->pixelShader = pixelShader;
            Init();
        }
        ~TriangleD3D() {
            this->vertexBuffer->Release();
        }

        void Update() override {
            Triangle::Update();
            vertices[0].position.x = vertex1.x;
            vertices[0].position.y = vertex1.y;
            vertices[1].position.x = vertex2.x;
            vertices[1].position.y = vertex2.y;
            vertices[2].position.x = vertex3.x;
            vertices[2].position.y = vertex3.y;

            // "Unset" the blend state
            float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            d3dContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

            // Map the buffer to update it
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = d3dContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (SUCCEEDED(hr)) {
                memcpy(mappedResource.pData, vertices, sizeof(vertices));
                d3dContext->Unmap(vertexBuffer, 0);
            }

            // Set the vertex buffer
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Set the shaders
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            d3dContext->VSSetShader(vertexShader, nullptr, 0);
            d3dContext->PSSetShader(pixelShader, nullptr, 0);
            d3dContext->PSSetShaderResources(0, 1, nullSRV);

            // Draw the triangle
            d3dContext->Draw(3, 0); // Draw 3 vertices
        }
    private:
        ID3D11DeviceContext* d3dContext = nullptr;
        ID3D11Device* d3dDevice = nullptr;
        ID3D11Buffer* vertexBuffer = nullptr;
        ID3D11InputLayout* inputLayout = nullptr;
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        Vertex vertices[3] {
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
            bufferDesc.ByteWidth = sizeof(vertices); // Size of your vertex data

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = vertices;

            HRESULT hr = d3dDevice->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
            if (FAILED(hr)) {
                // Handle the error (e.g., log it)
            }

            // Set the vertex buffer
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }
    };

    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleD3D(d3dContext, d3dDevice, inputLayoutSimple, vertexShaderSimple, pixelShaderSimple);
        return gameObject;
    }

    class SpriteD3D : public Sprite {
    public:
        SpriteD3D(ID3D11Device *d3DDevice, ID3D11DeviceContext *d3DContext, ID3D11BlendState* blendState,
                  ID3D11InputLayout* inputLayout, ID3D11VertexShader *vertexShader, ID3D11PixelShader *pixelShader) :
                  d3dDevice(d3DDevice), d3dContext(d3DContext), blendState(blendState), inputLayout(inputLayout),
                  vertexShader(vertexShader), pixelShader(pixelShader) {}

        ~SpriteD3D() = default;

        void SetTexture(const WCHAR * path) {
            LoadTextureFromFile(d3dDevice, path, &textureView);
        }

        void CreateBuffer() {
            // Create the vertex buffer (same as before)
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.ByteWidth = sizeof(vertices);
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = vertices;

            d3dDevice->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
        }

        void Update() override {
            Sprite::Update();
            vertices[0].position.x = vertex1.x;
            vertices[0].position.y = vertex1.y;
            vertices[1].position.x = vertex2.x;
            vertices[1].position.y = vertex2.y;
            // Bottom left and bottom right are flipped in DirectX; order matters
            vertices[3].position.x = vertex3.x;
            vertices[3].position.y = vertex3.y;
            vertices[2].position.x = vertex4.x;
            vertices[2].position.y = vertex4.y;

            if (useAtlas) {
                vertices[0].texCoord.x = atlasCellSize * (float)atlasColumn;
                vertices[1].texCoord.x = atlasCellSize * (float)atlasColumn + atlasCellSize;
                vertices[2].texCoord.x = atlasCellSize * (float)atlasColumn;
                vertices[3].texCoord.x = atlasCellSize * (float)atlasColumn + atlasCellSize;
                vertices[0].texCoord.y = atlasCellSize * (float)atlasRow;
                vertices[1].texCoord.y = atlasCellSize * (float)atlasRow;
                vertices[2].texCoord.y = atlasCellSize * (float)atlasRow + atlasCellSize;
                vertices[3].texCoord.y = atlasCellSize * (float)atlasRow + atlasCellSize;
            }

            // Set the blend state
            float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            d3dContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

            // Map the buffer to update it
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = d3dContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (SUCCEEDED(hr)) {
                memcpy(mappedResource.pData, vertices, sizeof(vertices));
                d3dContext->Unmap(vertexBuffer, 0);
            }

            // Set the vertex buffer
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

            // Set the shaders
            d3dContext->VSSetShader(vertexShader, nullptr, 0);
            d3dContext->PSSetShader(pixelShader, nullptr, 0);
            d3dContext->PSSetShaderResources(0, 1, &textureView);
            d3dContext->IASetInputLayout(inputLayout);

            // Draw
            d3dContext->Draw(4, 0);
        }

        ID3D11Device* d3dDevice = nullptr;
        ID3D11DeviceContext* d3dContext = nullptr;
        ID3D11BlendState* blendState = nullptr;
        ID3D11InputLayout* inputLayout = nullptr;
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        ID3D11ShaderResourceView* textureView = nullptr;
        ID3D11Buffer* vertexBuffer = nullptr;

        Vertex vertices[4] {
                // Order matters
                { DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) }, // Top left
                { DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }, // Top Right
                { DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) }, // Bottom Left
                { DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }, // Bottom Right
        };
    };

    static const WCHAR* convertToWCHAR(const char* str) {
        if (!str) return nullptr;

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
        auto* wstr = new WCHAR[size_needed];
        MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, size_needed);
        return wstr; // Remember to free with `delete[]` when done.
    }

    Sprite* CreateSprite(const char * path) override {
        auto gameObject = new SpriteD3D(d3dDevice, d3dContext, blendState, inputLayoutTexture, vertexShaderTexture, pixelShaderTexture);
        gameObject->CreateBuffer();
        auto wchar = convertToWCHAR(path);
        gameObject->SetTexture(wchar);
        delete[] wchar;
        return gameObject;
    }

    void Shutdown() override {
        CleanD3D();
    }

    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {
        keyUpCallback = func;
        keyCallbackContext = context;
    }
    void SetMouseReleasedCallback(void (*func)(MouseButton, void*), void* context) override {
        mouseUpCallback = func;
        mouseCallbackContext = context;
    }

private:
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
    ID3D11InputLayout* inputLayoutSimple = nullptr;
    ID3D11InputLayout* inputLayoutTexture = nullptr;
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11BlendState* blendState = nullptr;
    ID3D11VertexShader* vertexShaderTexture = nullptr;
    ID3D11PixelShader* pixelShaderTexture = nullptr;
    ID3D11VertexShader* vertexShaderSimple = nullptr;
    ID3D11PixelShader* pixelShaderSimple = nullptr;

    // Window procedure function
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_INPUT: {
                UINT dwSize;
                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
                std::vector<BYTE> buffer(dwSize);

                if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
                    break;

                auto raw = (RAWINPUT*)buffer.data();
                if (raw->header.dwType == RIM_TYPEKEYBOARD) {
                    RAWKEYBOARD& rawKb = raw->data.keyboard;
                    bool isKeyDown = (rawKb.Message == WM_KEYDOWN || rawKb.Message == WM_SYSKEYDOWN);
                    bool isKeyUp = (rawKb.Message == WM_KEYUP || rawKb.Message == WM_SYSKEYUP);

                    int key = rawKb.VKey;

                    // Update your key state table
                    if (isKeyDown) {
                        inputState.keyDown[(int)GetKeyCode(key)] = true;
                    } else if (isKeyUp) {
                        if (keyUpCallback) {
                            keyUpCallback(GetKeyCode(key), keyCallbackContext);
                        }
                        inputState.keyDown[(int)GetKeyCode(key)] = false;
                    }
                } else if (raw->header.dwType == RIM_TYPEMOUSE) {
                    RAWMOUSE& rawM = raw->data.mouse;

                    // Check mouse movement
                    int dx = rawM.lLastX;
                    int dy = rawM.lLastY;

                    inputState.posX = dx;
                    inputState.posY = dy;

                    // Check button states
                    if (rawM.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
                        inputState.LButtonDown = true;
                    }
                    if (rawM.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
                        inputState.LButtonDown = false;
                        if (mouseUpCallback) {
                            mouseUpCallback(MouseButton::Left, mouseCallbackContext);
                        }
                    }
                    if (rawM.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
                        inputState.RButtonDown = true;
                    }
                    if (rawM.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
                        inputState.RButtonDown = false;
                        if (mouseUpCallback) {
                            mouseUpCallback(MouseButton::Right, mouseCallbackContext);
                        }
                    }
                    if (rawM.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
                        inputState.MButtonDown = true;
                    }
                    if (rawM.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
                        inputState.MButtonDown = false;
                        if (mouseUpCallback) {
                            mouseUpCallback(MouseButton::Middle, mouseCallbackContext);
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

    // Initialize Direct3D
    void InitD3D(HWND hwnd)
    {
        // Swap chain descriptor
        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferCount = 1;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hwnd;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.BufferDesc.Width = WINDOW_WIDTH;
        scd.BufferDesc.Height = WINDOW_HEIGHT;

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

    void LoadShaders() override {
        const WCHAR * SHADER_TEXTURE = L"assets/shaders/TextureShader.hlsl";

        // Compile and create the vertex shader
        ID3DBlob* vsBlob = nullptr;
        D3DCompileFromFile(SHADER_TEXTURE, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr);
        d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShaderTexture);

        // Compile and create the pixel shader
        ID3DBlob* psBlob = nullptr;
        D3DCompileFromFile(SHADER_TEXTURE, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, nullptr);
        d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShaderTexture);

        // Define the input layout (add texture coordinates)
        D3D11_INPUT_ELEMENT_DESC layout[] =
                {
                        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
                };

        d3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayoutTexture);
        d3dContext->IASetInputLayout(inputLayoutTexture);

        // Clean up shader blobs
        vsBlob->Release();
        psBlob->Release();

        LoadSimpleShader();
    }

    void LoadSimpleShader() {
        const WCHAR * shader = L"assets/shaders/SimpleShader.hlsl";

        // Compile and create the vertex shader
        ID3DBlob* vsBlob = nullptr;
        D3DCompileFromFile(shader, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr);
        d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShaderSimple);

        // Compile and create the pixel shader
        ID3DBlob* psBlob = nullptr;
        D3DCompileFromFile(shader, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, nullptr);
        d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShaderSimple);

        // Define the input layout (add texture coordinates)
        D3D11_INPUT_ELEMENT_DESC layout[] =
                {
                        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                };

        d3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayoutSimple);
        d3dContext->IASetInputLayout(inputLayoutSimple);

        // Clean up shader blobs
        vsBlob->Release();
        psBlob->Release();
    }

    // Initialize graphics pipeline
    void InitPipeline() {
        // Create a sampler state
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        ID3D11SamplerState* samplerState;
        d3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
        d3dContext->PSSetSamplers(0, 1, &samplerState);

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

    // Clean up Direct3D objects
    void CleanD3D()
    {
        if (vertexBuffer) {
            vertexBuffer->Release();
            vertexBuffer = nullptr;
        }
        inputLayoutSimple->Release();
        vertexShaderTexture->Release();
        pixelShaderTexture->Release();
        renderTargetView->Release();
        swapChain->Release();
        d3dDevice->Release();
        d3dContext->Release();
    }

    // Keyboard input map
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

};

// Entrypoint
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    auto platform = new PlatformDirectX(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    return RealMain(platform);
}

#endif //GAMEENGINE_DIRECTX_H
