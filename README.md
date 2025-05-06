# Game Engine

## Windows

Run with one of the following CMake options to choose a backend
```bash
-DBACKEND_OPENGL=ON
-DBACKEND_DIRECTX=ON
```

Build and run
```bash
win-build-directx.bat
win-build-opengl.bat
```

## MacOS

Prerequisites
```bash
brew install cmake
# If using OpenGL backend
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
