# Game Engine

## Windows

Must be run with Visual Studio toolchain.

Run with one of the following CMake options to choose a backend
```bash
-DBACKEND_OPENGL=ON
-DBACKEND_DIRECTX=ON
```

## MacOS

Compile shaders:

```bash
xcrun -sdk macosc metal -o shader.air -c shader.metal
xcrun -sdk macosc metallib -o shader.metallib shader.air
```
