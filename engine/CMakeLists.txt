project(GameEngine)

SET(ENGINE_SOURCES entry.cpp)

# Add backend-specific source files
if (WIN32)
    if (BACKEND_DIRECTX)
        list(APPEND ENGINE_SOURCES platform/directx/directx.h)
        list(APPEND ENGINE_SOURCES platform/directx/file_util.h)
        list(APPEND ENGINE_SOURCES platform/directx/math.h)
    elseif (BACKEND_OPENGL)
        list(APPEND ENGINE_SOURCES platform/opengl/opengl.h)
    else()
        message(FATAL_ERROR "No backend specified! Use -DBACKEND_DIRECTX or -DBACKEND_OPENGL.")
    endif()
elseif (UNIX AND NOT APPLE)
    list(APPEND ENGINE_SOURCES platform/opengl/opengl.h)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
    include_directories(${GLFW3_INCLUDE_DIRS})
    include_directories(/usr/include/GL)
    link_directories(${GLFW3_LIBRARY_DIRS})
    link_directories(/usr/lib/x86_64-linux-gnu)
    add_definitions(-DGLEW_STATIC)
elseif (APPLE AND NOT PLATFORM_IOS)
    if (BACKEND_METAL)
        list(APPEND ENGINE_SOURCES platform/metal/main.mm platform/metal/stb_image.cpp)
    elseif (BACKEND_OPENGL)
        set(GLFW_INCLUDE_DIR /opt/homebrew/Cellar/glfw/3.4/include)
        set(GLFW_LIBRARY /opt/homebrew/Cellar/glfw/3.4/lib/libglfw.3.dylib)
        include_directories(${GLFW_INCLUDE_DIR})
        set(GLEW_INCLUDE_DIR /opt/homebrew/Cellar/glew/2.2.0_1/include)
        set(GLEW_LIBRARY /opt/homebrew/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.0.dylib)
        include_directories(${GLEW_INCLUDE_DIR})
        list(APPEND ENGINE_SOURCES platform/opengl/opengl.h)
    else()
        message(FATAL_ERROR "No backend specified! Use -DBACKEND_METAL or -DBACKEND_OPENGL.")
    endif()
elseif (PLATFORM_IOS)
    list(APPEND ENGINE_SOURCES platform/ios/main.mm)
    set(CMAKE_SYSTEM_NAME iOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "17.0" CACHE STRING "Minimum iOS deployment version")
    # set(CMAKE_OSX_SYSROOT iphonesimulator)
    set(CMAKE_OSX_SYSROOT iphoneos)
    set(CMAKE_OSX_ARCHITECTURES arm64)
endif()

message(STATUS "Engine sources:")
foreach(source_file ${ENGINE_SOURCES})
    message(STATUS "  ${source_file}")
endforeach()

add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# Backend and platform-specific linking and definitions
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_WINDOWS)
    if (BACKEND_DIRECTX)
        target_compile_definitions(${PROJECT_NAME} PUBLIC BACKEND_DIRECTX)
        target_link_libraries(${PROJECT_NAME} PUBLIC d3d11 dxgi D3DCompiler XInput)
    elseif (BACKEND_OPENGL)
        target_compile_definitions(${PROJECT_NAME} PUBLIC BACKEND_OPENGL)
        target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/GLFW/include)
        target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/GLFW/lib-vc2019)
        target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/GLEW/include)
        target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/GLEW/lib/Release/x64)
        target_compile_definitions(${PROJECT_NAME} PUBLIC GLEW_STATIC)
        target_link_libraries(${PROJECT_NAME} PUBLIC glew32s glfw3 opengl32)
    endif()
elseif (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES} GLEW GL X11 pthread Xrandr Xi dl)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_LINUX)
elseif (APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_APPLE)
    if (BACKEND_METAL)
        target_compile_definitions(${PROJECT_NAME} PUBLIC BACKEND_METAL)
        target_link_libraries(${PROJECT_NAME}
                PUBLIC
                "-framework Metal"
                "-framework Foundation"
                "-framework MetalKit"
                "-framework IOKit"
                "-framework QuartzCore"
                "-framework CoreVideo"
                "-framework GameController"
        )
        if (PLATFORM_IOS)
            target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_IOS)
            target_link_libraries(${PROJECT_NAME} PUBLIC
                "-framework UIKit"
                "-framework CoreGraphics"
                "-framework CoreFoundation"
                "-framework CoreAudio"
                "-framework AudioToolbox"
                "-framework AVFoundation"
            )
        else()
            target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_MACOS)
            target_link_libraries(${PROJECT_NAME} PUBLIC
                "-framework OpenGL"
                "-framework Cocoa"
                "-framework AppKit"
            )
        endif ()
    elseif (BACKEND_OPENGL)
        target_compile_definitions(${PROJECT_NAME} PUBLIC BACKEND_OPENGL)
        target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARY} ${GLEW_LIBRARY})
    endif()
endif()
