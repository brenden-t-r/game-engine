cmake_minimum_required(VERSION 3.26)
project(GameEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set sources
set(SOURCES main.cpp)

# Add platform-specific sources
if (WIN32)
    # DirectX
    list(APPEND SOURCES platform/directx/directx.h)
    list(APPEND SOURCES platform/directx/file_util.h)
    list(APPEND SOURCES platform/directx/math.h)
    # (Optional) OpenGL
    list(APPEND SOURCES platform/opengl/opengl.h)
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/GLFW/include)
    link_directories(${PROJECT_SOURCE_DIR}/dependencies/GLFW/lib-vc2019)
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/GLEW/include)
    link_directories(${PROJECT_SOURCE_DIR}/dependencies/GLEW/lib/Release/x64)
    add_definitions(-DGLEW_STATIC)
elseif (UNIX AND NOT APPLE) # For Linux
    #OpenGL
    list(APPEND SOURCES platform/opengl/opengl.h)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
    include_directories(${GLFW3_INCLUDE_DIRS})
    include_directories(/usr/include/GL)
    link_directories(${GLFW3_LIBRARY_DIRS})
    link_directories(/usr/lib/x86_64-linux-gnu)
    add_definitions(-DGLEW_STATIC)
elseif (APPLE)
    # GLFW
    set(GLFW_INCLUDE_DIR /opt/homebrew/Cellar/glfw/3.4/include)
    set(GLFW_LIBRARY /opt/homebrew/Cellar/glfw/3.4/lib/libglfw.3.dylib)
    include_directories(${GLFW_INCLUDE_DIR})
    set(GLEW_INCLUDE_DIR /opt/homebrew/Cellar/glew/2.2.0_1/include)
    set(GLEW_LIBRARY /opt/homebrew/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.0.dylib)
    include_directories(${GLEW_INCLUDE_DIR})
#    list(APPEND SOURCES platform/metal/mtl_implementation.cpp platform/metal/main.mm platform/metal/stbi_image.cpp)
    list(APPEND SOURCES platform/opengl/opengl.h)
endif()

# Add executable
if (WIN32)
    option(BACKEND_DIRECTX "Use DirectX backend" OFF)
    option(BACKEND_OPENGL "Use OpenGL backend" OFF)
    if (BACKEND_DIRECTX)
        add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
        target_compile_definitions(${PROJECT_NAME} PRIVATE BACKEND_DIRECTX)
    elseif (BACKEND_OPENGL)
        add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
        target_compile_definitions(${PROJECT_NAME} PRIVATE BACKEND_OPENGL)
    else()
        message(FATAL_ERROR "No backend specified! Use -DBACKEND_DIRECTX or -DBACKEND_OPENGL.")
    endif()
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

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
    message(STATUS "Building for Linux")
    #OpenGL
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES} GLEW GL X11 pthread Xrandr Xi dl)

    # Set platform pre-processor macro
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_LINUX)
elseif (APPLE)
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/dependencies/metal-cpp)
    target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARY} ${GLEW_LIBRARY}
            "-framework Metal"
            "-framework Foundation"
            "-framework QuartzCore"
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BACKEND_OPENGL)
endif()

file(GLOB ASSETS_FOLDER
        "assets/*"
)
file(COPY ${ASSETS_FOLDER} DESTINATION assets)
