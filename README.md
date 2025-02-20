# Game Engine

## Windows

Must be run with Visual Studio toolchain.

Run with one of the following CMake options to choose a backend
```bash
-DBACKEND_OPENGL=ON
-DBACKEND_DIRECTX=ON
```

Build and run
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build .
./Debug/GameEngine.exe
```

## MacOS

Compile shaders:

```bash
xcrun -sdk macosc metal -o shader.air -c shader.metal
xcrun -sdk macosc metallib -o shader.metallib shader.air
```

## Linux


