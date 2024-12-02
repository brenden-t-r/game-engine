//#include "platform/platform.h"

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
