cmake_minimum_required(VERSION 3.26)
project(GameEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set sources
set(SOURCES main.cpp
        constants.h
        platform/platform.h
        engine/file.h
        engine/vector.h
)

# Add platform-specific sources
if (WIN32)

    # DirectX
    list(APPEND SOURCES platform/directx/directx.h)
    list(APPEND SOURCES platform/directx/file_util.h)
    list(APPEND SOURCES platform/directx/math.h)
    list(APPEND SOURCES platform/directx/scene.h)
    list(APPEND SOURCES platform/directx/sprite.h)

    # (Optional) OpenGL
    list(APPEND SOURCES platform/opengl.h)
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/GLFW/include)
    link_directories(${PROJECT_SOURCE_DIR}/dependencies/GLFW/lib-vc2019)
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/GLEW/include)
    link_directories(${PROJECT_SOURCE_DIR}/dependencies/GLEW/lib/Release/x64)
    add_definitions(-DGLEW_STATIC)


elseif (UNIX AND NOT APPLE) # For Linux
    #OpenGL
    list(APPEND SOURCES platform/opengl.h)
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/GLFW/include)
    link_directories(${PROJECT_SOURCE_DIR}/dependencies/GLFW/lib-vc2019)
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/GLEW/include)
    link_directories(${PROJECT_SOURCE_DIR}/dependencies/GLEW/lib/Release/x64)
    add_definitions(-DGLEW_STATIC)
elseif (APPLE)
endif()

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link platform-specific libraries
if (WIN32)
    # DirectX
    target_link_libraries(${PROJECT_NAME} PRIVATE
            d3d11 dxgi D3DCompiler
    )

    # (Optional) OpenGL
    target_link_libraries(${PROJECT_NAME} PRIVATE glew32s glfw3 opengl32)

    # Set platform pre-processor macro
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WINDOWS)
elseif (UNIX AND NOT APPLE) # For Linux
    #OpenGL
    target_link_libraries(${PROJECT_NAME} PRIVATE glew32s glfw3 opengl32)

    # Set platform pre-processor macro
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_LINUX)
elseif (APPLE)
    # TODO: Link OSX, Metal libraries
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_MACOS)
endif()

file(GLOB ASSETS_FOLDER
        "assets/*"
)
file(COPY ${ASSETS_FOLDER} DESTINATION assets)
