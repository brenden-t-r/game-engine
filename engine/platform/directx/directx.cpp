#include "directx.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    auto platform = new PlatformDirectX(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    return RealMain(platform);
}