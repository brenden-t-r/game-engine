/*
 *       ___           ___           ___                       ___           ___
 *      /\__\         /\  \         /\__\                     /\  \         /\__\
 *     /:/ _/_        \:\  \       /:/ _/_       ___          \:\  \       /:/ _/_
 *    /:/ /\__\        \:\  \     /:/ /\  \     /\__\          \:\  \     /:/ /\__\
 *   /:/ /:/ _/_   _____\:\  \   /:/ /::\  \   /:/__/      _____\:\  \   /:/ /:/ _/_
 *  /:/_/:/ /\__\ /::::::::\__\ /:/__\/\:\__\ /::\  \     /::::::::\__\ /:/_/:/ /\__\
 *  \:\/:/ /:/  / \:\~~\~~\/__/ \:\  \ /:/  / \/\:\  \__  \:\~~\~~\/__/ \:\/:/ /:/  /
 *   \::/_/:/  /   \:\  \        \:\  /:/  /   ~~\:\/\__\  \:\  \        \::/_/:/  /
 *    \:\/:/  /     \:\  \        \:\/:/  /       \::/  /   \:\  \        \:\/:/  /
 *     \::/  /       \:\__\        \::/  /        /:/  /     \:\__\        \::/  /
 *      \/__/         \/__/         \/__/         \/__/       \/__/         \/__/
 *
 *
 *  Platform/Backend Support
 *  ------------------------
 *  Windows => OpenGL, DirectX
 *  Linux   => OpenGL
 *  macOS   => OpenGL, Metal
 *  iOS     => Metal
 *
 */

// Just the declaration; defined within the game code
int RealMain(Platform* platform);

// Run loop boilerplate, called by game
void GameUpdateFn(void* context) {
    ((Game*)(context))->Update();
}
void RunLoop(Platform* platform, Game* game) {
    platform->Run(GameUpdateFn, game);
}

/*
 * Platform-specific entry-points using preprocessor macro
 */
#if defined(PLATFORM_WINDOWS) && defined(BACKEND_OPENGL)
#include "../engine/platform/opengl/opengl.cpp"
#elif defined(PLATFORM_WINDOWS) && defined(BACKEND_DIRECTX)
#include "platform/directx/directx.cpp"
#elif defined(PLATFORM_LINUX)
#include "platform/opengl/opengl.cpp"
#elif defined(PLATFORM_APPLE) && defined(BACKEND_OPENGL)
#include "platform/opengl/opengl.cpp"
#elif defined(PLATFORM_APPLE) && defined(BACKEND_METAL)
#elif defined(PLATFORM_IOS)
#else
// No backend selected
int main() {
    return -1;
}
#endif