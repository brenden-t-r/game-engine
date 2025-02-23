@echo off
setlocal enabledelayedexpansion

:: Set default paths (override with environment variables if already set)
if "%VS_PATH%"=="" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
if "%CMAKE_PATH%"=="" set "CMAKE_PATH=C:\Program Files\JetBrains\CLion 2023.2.1\bin\cmake\win\x64\bin\cmake.exe"
if "%NINJA_PATH%"=="" set "NINJA_PATH=C:\Program Files\JetBrains\CLion 2023.2.1\bin\ninja\win\x64\ninja.exe"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Debug"
if "%BUILD_DIR%"=="" set "BUILD_DIR=%CD%\cmake-build-debug-opengl"
if "%SOURCE_DIR%"=="" set "SOURCE_DIR=%CD%"
if "%BACKEND%"=="" set "BACKEND=DIRECTX"

:: Print configuration
echo Using Visual Studio Path: %VS_PATH%
echo Using CMake Path: %CMAKE_PATH%
echo Using Ninja Path: %NINJA_PATH%
echo Build Type: %BUILD_TYPE%
echo Source Dir: %SOURCE_DIR%
echo Build Dir: %BUILD_DIR%
echo Backend: %BACKEND%

:: Set up MSVC environment
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo Failed to setup MSVC environment.
    exit /b 1
)

:: Run CMake configuration
if "%BACKEND%"=="DIRECTX" (
    echo Building DirectX
    "%CMAKE_PATH%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_MAKE_PROGRAM="%NINJA_PATH%" -DBACKEND_DIRECTX=ON -G Ninja -S "%SOURCE_DIR%" -B "%BUILD_DIR%"
) else (
    echo Building OpenGL
    "%CMAKE_PATH%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_MAKE_PROGRAM="%NINJA_PATH%" -DBACKEND_OPENGL=ON -G Ninja -S "%SOURCE_DIR%" -B "%BUILD_DIR%"
)
if errorlevel 1 (
    echo CMake configuration failed.
    exit /b 1
)

:: Build project
"%CMAKE_PATH%" --build "%BUILD_DIR%" --target GameEngine -j %NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build successful!
exit /b 0
