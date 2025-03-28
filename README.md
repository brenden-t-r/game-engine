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
cmake .. -G "Visual Studio 17 2022" -DBACKEND_DIRECTX=ON
cmake --build .
"Debug/GameEngine.exe"
```

## MacOS

Prerequisites
```bash
brew install cmake
brew install glfw
brew install glew
```

Run with one of the following CMake options to choose a backend
```bash
-DBACKEND_METAL=ON
-DBACKEND_OPENGL=ON
```

Build and run
```bash
mkdir build
cd build
cmake .. -DBACKEND_METAL=ON
make
./GameEngine
```

Compile shaders:

```bash
xcrun -sdk macosc metal -o shader.air -c shader.metal
xcrun -sdk macosc metallib -o shader.metallib shader.air
```

## Linux

Build and run
```bash
mkdir build
cd build
cmake ..
make
./GameEngine
```
