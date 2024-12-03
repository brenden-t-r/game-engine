#ifndef GAMEENGINE_DIRECTX_H
#define GAMEENGINE_DIRECTX_H

// Include constants
#include "../../constants.h"
#include "../platform.h"
#include "sprite.h"

// Windows/DirectX imports
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <iostream>

// Link necessary d3d11 libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


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
        HWND hwnd = CreateWindow(
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
    }

    void Run(const std::function<void()>& func) override {
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

                func();
//                DisplaySprite();
//                DrawTriangle();

                // Present the back buffer to the screen
                swapChain->Present(1, 0);
            }
        }
    }

    void DisplaySprite() {
        // Set the blend state
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        d3dContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

        Sprite spriteBackground{};
        spriteBackground.SetPosition(-1.0, 1.0f, WINDOW_WIDTH, WINDOW_HEIGHT);
        spriteBackground.CreateBuffer(d3dDevice);
        spriteBackground.LoadTexture(d3dDevice, L"assets/sprites/background.png");
        spriteBackground.Draw(d3dContext, vertexShader, pixelShader);
    }

    void DrawTriangle() override {
        // "Unset" the blend state
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        d3dContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

        // Define a triangle with 3 vertices
        Vertex vertices[3] = {
                { DirectX::XMFLOAT3(0.5f,  -0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
                { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.5f, 0.0f) },
                { DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT2(0.25f, 0.5f) }
        };

            // Create the vertex buffer
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(vertices);
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;

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

        // Set the shaders
        d3dContext->VSSetShader(vertexShader, nullptr, 0);
        d3dContext->PSSetShader(pixelShader, nullptr, 0);
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        d3dContext->PSSetShaderResources(0, 1, nullSRV);

        // Draw the triangle
        d3dContext->Draw(3, 0); // Draw 3 vertices

        // Release resources (if necessary, depending on your resource management strategy)
        vertexBuffer->Release();
    }

    void Shutdown() override {
        CleanD3D();
    }

private:
    // Entry-point args
    HINSTANCE hInstance = nullptr;
    HINSTANCE hPrevInstance = nullptr;
    LPSTR lpCmdLine = nullptr;
    int nCmdShow = 0;

    // DirectX variables
    IDXGISwapChain* swapChain = nullptr;
    ID3D11Device* d3dDevice = nullptr;
    ID3D11DeviceContext* d3dContext = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3D11ShaderResourceView* textureView = nullptr;
    ID3D11BlendState* blendState =nullptr;

    // Window procedure function
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
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

    void LoadShaders() override {
        const WCHAR * SHADER_TEXTURE = L"assets/shaders/TextureShader.hlsl";

        // Compile and create the vertex shader
        ID3DBlob* vsBlob = nullptr;
        D3DCompileFromFile(SHADER_TEXTURE, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr);
        d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);

        // Compile and create the pixel shader
        ID3DBlob* psBlob = nullptr;
        D3DCompileFromFile(SHADER_TEXTURE, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, nullptr);
        d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

        // Define the input layout (add texture coordinates)
        D3D11_INPUT_ELEMENT_DESC layout[] =
                {
                        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
                };

        d3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
        d3dContext->IASetInputLayout(inputLayout);

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
        vertexBuffer->Release();
        inputLayout->Release();
        vertexShader->Release();
        pixelShader->Release();
        renderTargetView->Release();
        swapChain->Release();
        d3dDevice->Release();
        d3dContext->Release();
    }
};

// Entrypoint
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    auto platform = new PlatformDirectX(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    return RealMain(platform);
}

#endif //GAMEENGINE_DIRECTX_H
