project(Game)

# Game sources
set(GAME_SOURCES main.cpp)

# Add executable with platform-specific options
if (WIN32)
    if (BACKEND_DIRECTX OR BACKEND_OPENGL)
        add_executable(${PROJECT_NAME} WIN32 ${GAME_SOURCES})
    else()
        message(FATAL_ERROR "No backend specified! Use -DBACKEND_DIRECTX or -DBACKEND_OPENGL.")
    endif()
elseif (APPLE)
    if (BACKEND_METAL)
        set(BUNDLE_ID_PREFIX "games.gluttonous")
        string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${GAME_SOURCES})
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE YES)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUNDLE_ID_PREFIX}.${PROJECT_NAME_LOWER}"
        )
        if (PLATFORM_IOS)
            set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "17.0")
            configure_file("${CMAKE_SOURCE_DIR}/engine/platform/ios/Info.plist" "${CMAKE_BINARY_DIR}/Info.plist" @ONLY)
        else()
            configure_file("${CMAKE_SOURCE_DIR}/engine/platform/metal/Info.plist" "${CMAKE_BINARY_DIR}/Info.plist" @ONLY)
        endif()
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/Info.plist)
    elseif (BACKEND_OPENGL)
        add_executable(${PROJECT_NAME} ${GAME_SOURCES})
    else()
        message(FATAL_ERROR "No backend specified! Use -DBACKEND_METAL or -DBACKEND_OPENGL.")
    endif()
else()
    add_executable(${PROJECT_NAME} ${GAME_SOURCES})
endif()

# Static link engine library
target_link_libraries(${PROJECT_NAME} PRIVATE GameEngine)

# Copy game assets
if (NOT BACKEND_METAL)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/game/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
else()
    file(GLOB_RECURSE ASSET_FILES "${CMAKE_SOURCE_DIR}/game/assets/*")
    foreach(ASSET ${ASSET_FILES})
        file(RELATIVE_PATH ASSET_RELATIVE_PATH "${CMAKE_SOURCE_DIR}/game" ${ASSET})
        get_filename_component(ASSET_DIR ${ASSET_RELATIVE_PATH} DIRECTORY)
        message(STATUS "Asset: ${ASSET} will be placed at Resources/${ASSET_RELATIVE_PATH}")
        set_source_files_properties(${ASSET} PROPERTIES
                MACOSX_PACKAGE_LOCATION "Resources/${ASSET_DIR}")
    endforeach()
    target_sources(${PROJECT_NAME} PRIVATE ${ASSET_FILES})
    if (NOT PLATFORM_IOS)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>"
        )
    endif()
endif()
