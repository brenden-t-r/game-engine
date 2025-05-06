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
    # TODO: Opengl?
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${GAME_SOURCES})
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE YES)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_IDENTIFIER "com.mycompany.mygame")
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/platform/ios/Info.plist)
else()
    add_executable(${PROJECT_NAME} ${GAME_SOURCES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE GameEngine)

# Game assets
if (NOT BACKEND_METAL)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/game/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
else()
    file(GLOB_RECURSE ASSET_FILES "${CMAKE_SOURCE_DIR}/assets/*")
    foreach(ASSET ${ASSET_FILES})
        file(RELATIVE_PATH ASSET_RELATIVE_PATH "${CMAKE_SOURCE_DIR}" ${ASSET})
        get_filename_component(ASSET_DIR ${ASSET_RELATIVE_PATH} DIRECTORY)
        message(STATUS "Asset: ${ASSET} will be placed at Resources/${ASSET_RELATIVE_PATH}")
        set_source_files_properties(${ASSET} PROPERTIES
                MACOSX_PACKAGE_LOCATION "Resources/${ASSET_DIR}")
    endforeach()
    target_sources(${PROJECT_NAME} PRIVATE ${ASSET_FILES})
endif()
