/*
 * Choose backend
 *  Windows => OpenGL, DirectX
 *  Linux   => OpenGL
 *  Apple   => OpenGL, Metal
 * */
#define BACKEND_DIRECTX
//#define BACKEND_OPENGL
//#define BACKEND_METAL

/*
 * Platform-specific entry-points using preprocessor macro
 */

#ifdef PLATFORM_WINDOWS

// Initialize Window
    #ifdef BACKEND_OPENGL
    #include "platform/opengl.h"
    #endif
    #ifdef BACKEND_DIRECTX
    #include "platform/windows.h"
    #endif

#endif

#ifdef PLATFORM_LINUX
    #include "platform/opengl.h"
#endif

#ifdef PLATFORM_APPLE
#endif

#ifdef DIRECTX_BACKEND_2
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "scene.h"

// Link necessary d3d11 libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

// Global Variables
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

// Constants
const WCHAR * SHADER_TEXTURE = L"TextureShader.hlsl";

// Function Prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitD3D(HWND hwnd);
void CleanD3D();
void RenderFrame();
void InitPipeline();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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

    // Initialize the scene
//    yourScene = new YourScene(d3dDevice, d3dContext, vertexShader, pixelShader);
//    yourScene->Init();

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
            RenderFrame();
        }
    }

//    delete yourScene;

    CleanD3D();
    return 0;
}

// Window procedure function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    // Create the device, device context, and swap chain
    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
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

// Initialize graphics pipeline
void InitPipeline() {
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

    // Clean up shader blobs
    vsBlob->Release();
    psBlob->Release();
}

// Render the frame
void RenderFrame()
{
    // Clear the back buffer
    float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    d3dContext->ClearRenderTargetView(renderTargetView, clearColor);

    // Set the blend state
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    d3dContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

    // YourScene->Update()

    // Present the back buffer to the screen
    swapChain->Present(1, 0);
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
#endif