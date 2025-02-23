@echo off

mkdir build-opengl
pushd build-opengl
cmake .. -G "Visual Studio 17 2022" -DBACKEND_OPENGL=ON
cmake --build .
::"Debug/GameEngine.exe"
popd
