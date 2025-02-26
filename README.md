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
cd assets/shaders
xcrun -sdk macosx metal -o triangle.air -c triangle.metal
xcrun -sdk macosx metallib -o triangle.metallib triangle.air
xcrun -sdk macosx metal -o square.air -c square.metal
xcrun -sdk macosx metallib -o square.metallib square.air
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
