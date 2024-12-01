#ifndef DIRECTX11_WINDOWS_H
#define DIRECTX11_WINDOWS_H

// Include constants
#include "../constants.h"

// Windows/DirectX imports
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

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

// Entrypoint
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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
            //RenderFrame();
        }
    }

    return 0;
}


#endif //DIRECTX11_WINDOWS_H
