#include "opengl.h"

// Entrypoint
#if PLATFORM_WINDOWS
#include <windows.h>
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