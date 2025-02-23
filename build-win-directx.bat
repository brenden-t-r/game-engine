@echo off

mkdir build-directx
pushd build-directx
cmake .. -G "Visual Studio 17 2022" -DBACKEND_DIRECTX=ON
cmake --build .
::"Debug/GameEngine.exe"
popd
