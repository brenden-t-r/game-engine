cmake_minimum_required(VERSION 3.26)
project(GameProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add options here if needed
option(BACKEND_DIRECTX "Use DirectX backend" OFF)
option(BACKEND_OPENGL "Use OpenGL backend" OFF)
option(BACKEND_METAL "Use Metal backend" OFF)
option(PLATFORM_IOS "Build for iOS" OFF)

if (PLATFORM_IOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "17.0")
endif()

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(game)
