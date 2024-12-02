
/*
 * Choose backend
 *  Windows => OpenGL, DirectX
 *  Linux   => OpenGL
 *  Apple   => OpenGL, Metal
 * */
//#define BACKEND_DIRECTX
//#define BACKEND_OPENGL
//#define BACKEND_METAL

/*
 * Platform-specific entry-points using preprocessor macro
 */
#if defined(PLATFORM_WINDOWS) and defined(BACKEND_OPENGL)
#include "platform/opengl.h"
#elif defined(PLATFORM_WINDOWS) and defined(BACKEND_DIRECTX)
#include "platform/directx/directx.h"
#elif defined(PLATFORM_LINUX)
#include "platform/opengl.h"
#elif defined(PLATFORM_APPLE)
#else
// No backend selected
int main() {
    return -1;
}
#endif
