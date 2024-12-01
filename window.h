#ifndef DIRECTX11_WINDOW_H
#define DIRECTX11_WINDOW_H

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

// Link necessary d3d11 libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


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


class window {
public:
    void Init(float width, float height, int nCmdShow) {
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
                width, height,
                nullptr, nullptr, wc.hInstance, nullptr
        );
        ShowWindow(hwnd, nCmdShow);
    }
private:


};


#endif //DIRECTX11_WINDOW_H
