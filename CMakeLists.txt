cmake_minimum_required(VERSION 3.26)
project(DirectX11)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set sources
set(SOURCES main.cpp file_util.h
        sprite.h
        math.h
        constants.h
        scene.h
)

# Add platform-specific sources
if (WIN32)
    # DirectX
    list(APPEND SOURCES platform/windows.h)

    # (Optional) OpenGL
    list(APPEND SOURCES platform/opengl.h)
    include_directories(${PROJECT_SOURCE_DIR}/Dependencies/GLFW/include)
    link_directories(${PROJECT_SOURCE_DIR}/Dependencies/GLFW/lib-vc2019)
    include_directories(${PROJECT_SOURCE_DIR}/Dependencies/GLEW/include)
    link_directories(${PROJECT_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64)
    add_definitions(-DGLEW_STATIC)


elseif (UNIX AND NOT APPLE) # For Linux
    #OpenGL
    list(APPEND SOURCES platform/opengl.h)
    include_directories(${PROJECT_SOURCE_DIR}/Dependencies/GLFW/include)
    link_directories(${PROJECT_SOURCE_DIR}/Dependencies/GLFW/lib-vc2019)
    include_directories(${PROJECT_SOURCE_DIR}/Dependencies/GLEW/include)
    link_directories(${PROJECT_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64)
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

# Ensure the HLSL shader is copied to the output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/TextureShader.hlsl"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/images/texture.png"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
