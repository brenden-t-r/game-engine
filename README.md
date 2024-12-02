# Game Engine

## Windows

DirectX backend must be run with MingGW toolchain
OpenGL backend must be run with VS Studio toolchain

Run with one of the following CMake options to choose a backend
```bash
-DCMAKE_CXX_FLAGS="-DBACKEND_DIRECTX"
-DCMAKE_CXX_FLAGS="-DBACKEND_OPENGL"
```