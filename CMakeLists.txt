cmake_minimum_required(VERSION 3.1)

set(EXE_NAME "Glypha III")
string(REPLACE " " "" EXE_NAME_NOSPACES "${EXE_NAME}")
set(EXE_IDENTIFIER "com.kainjow.${EXE_NAME_NOSPACES}")

if(APPLE)
  # must be set before project()
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum macOS deployment version")
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(Glypha)

set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "4")

set(SOURCES
  game/GLGame.cpp
  game/GLGame.h
  game/GLImage.cpp
  game/GLImage.h
  game/GLRect.cpp
  game/GLRect.h
  game/GLRenderer.cpp
  game/GLRenderer.h
  game/GLResources.h
  game/GLUtils.cpp
  game/GLUtils.h
  game/GLBufferReader.cpp
  game/GLBufferReader.h
  game/GLCursor.h
  game/GLSounds.h
  game/GLFont.cpp
  game/GLFont.h
  game/GLPoint.h
  game/GLPrefs.cpp
  game/GLPrefs.h
)

if(HAVE_QT)
  set(CMAKE_AUTOMOC ON)
  list(APPEND SOURCES
    qt/main.cpp
    qt/main.hpp
    game/GLCursorQt.cpp
    game/GLImage_Qt.cpp
    game/GLSoundsQt.cpp
    game/GLSoundsQtImp.cpp
    game/GLSoundsQtImp.h
    game/GLDataToWave.cpp
    game/GLDataToWave.h
  )
elseif(BEOS OR HAIKU)
  list(APPEND SOURCES
    haiku/main.cpp
    game/GLCursor.cpp
    game/GLImage_libpng.cpp
    game/GLSounds.cpp
  )
elseif(WIN32)
  list(APPEND SOURCES
    win/main.cpp
    win/resources.rc
	win/resources.h
    game/GLCursorWin32.cpp
    game/GLImage_Win32.cpp
    game/GLSoundsWin32.cpp
  )
elseif(APPLE)
  set(APPICON ${CMAKE_CURRENT_SOURCE_DIR}/mac/icon.icns)
  set_source_files_properties(${APPICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  list(APPEND SOURCES
    mac/main.mm
    game/GLCursorCocoa.mm
    game/GLImage_CoreImage.cpp
    game/GLSoundsCocoa.mm
    ${APPICON}
  )
endif()

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${EXE_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE game)

if(HAVE_QT)
  find_package(Qt5 COMPONENTS Core Widgets OpenGL Multimedia REQUIRED)
  target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::OpenGL Qt5::Multimedia)
  target_compile_definitions(${PROJECT_NAME} PRIVATE GLYPHA_QT=)
  if(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE
      -std=c++11
    )
  endif()
elseif(BEOS OR HAIKU)
elseif(UNIX AND NOT APPLE)
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -std=c++11
  )
elseif(APPLE)
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
  )
  target_compile_definitions(${PROJECT_NAME} PRIVATE
    GL_SILENCE_DEPRECATION
  )
elseif(WIN32)
  target_include_directories(${PROJECT_NAME} PRIVATE win)
  target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE)
endif()

# Separate so macOS version can build Qt version for testing
if(APPLE)
  target_compile_options(${PROJECT_NAME} PRIVATE
    -std=c++11
    -stdlib=libc++
  )
endif()

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    /WX
    /W4
  )
endif()

list(FIND SOURCES game/GLImage_libpng.cpp HAVE_IMAGE_LIBPNG)
if(NOT HAVE_IMAGE_LIBPNG EQUAL -1)
  include(FindPNG)
  target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})
  target_include_directories(${PROJECT_NAME} PRIVATE ${PNG_INCLUDE_DIRS})
endif()

include(FindOpenGL)
if(NOT OPENGL_FOUND)
  message(FATAL_ERROR "OpenGL not found!")
endif()
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

if(HAVE_QT)
elseif(BEOS OR HAIKU)
  find_library(BE be)
  target_link_libraries(${PROJECT_NAME} ${BE})
elseif(WIN32)
  target_link_libraries(${PROJECT_NAME} shlwapi windowscodecs winmm)
elseif(APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/mac/Info.plist
    MACOSX_BUNDLE_BUNDLE_NAME ${EXE_NAME}
    MACOSX_BUNDLE_GUI_IDENTIFIER ${EXE_IDENTIFIER}
    # Fix Xcode's Archive functionality
    XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
    XCODE_ATTRIBUTE_SKIP_INSTALL "No"
  )
  if("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    set_target_properties(${PROJECT_NAME} PROPERTIES
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Mac Developer"
      XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_LIST_DIR}/mac/Glypha.entitlements"
    )
  endif()
  target_link_libraries(${PROJECT_NAME}
    "-framework Cocoa"
    "-framework CoreVideo"
    "-framework AVFoundation"
  )
endif()

# Separate so macOS version can build Qt version for testing
if(APPLE)
  target_link_libraries(${PROJECT_NAME}
    -stdlib=libc++
  )
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION .)
if(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
endif()
include(CPack)

target_compile_definitions(${PROJECT_NAME} PRIVATE "GL_GAME_NAME=\"${EXE_NAME}\""
  "GL_GAME_VERSION=\"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}\"")
if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE "GL_GAME_NAME_W=L\"${EXE_NAME}\"")
endif()

add_executable(bin-convert bin-convert.c)

add_dependencies(${PROJECT_NAME} bin-convert)

set(RESOURCES
  resources/images/help.png
  resources/images/eye.png
  resources/images/egg.png
  resources/images/enemyFly.png
  resources/images/enemyWalk.png
  resources/images/background.png
  resources/images/hand.png
  resources/images/numbers.png
  resources/images/obelisks.png
  resources/images/platforms.png
  resources/images/player.png
  resources/images/playerIdle.png
  resources/images/torches.png
  resources/images/about.png

  resources/sounds/boom1.wav
  resources/sounds/music.wav
  resources/sounds/bonus.wav
  resources/sounds/boom2.wav
  resources/sounds/spawn.wav
  resources/sounds/splash.wav
  resources/sounds/bird.wav
  resources/sounds/flap.wav
  resources/sounds/flap2.wav
  resources/sounds/grate.wav
  resources/sounds/lightning.wav
  resources/sounds/scrape2.wav
  resources/sounds/screech.wav
  resources/sounds/walk.wav

  resources/fonts/font11.png
  resources/fonts/font11.fnt
)

foreach(rsrc ${RESOURCES})
  get_filename_component(rsrc_filename ${rsrc} NAME)
  string(REPLACE "." "_" rsrc_filename ${rsrc_filename})
  set(input_path ${CMAKE_CURRENT_LIST_DIR}/${rsrc})
  set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/resources)
  set(output_path ${output_dir}/${rsrc_filename}.cpp)
  target_sources(${PROJECT_NAME} PRIVATE ${output_path})
  set_source_files_properties(${PROJECT_NAME} ${output_path}
    PROPERTIES GENERATED TRUE
  )
  add_custom_command(OUTPUT ${output_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${output_dir}
    COMMAND $<TARGET_FILE:bin-convert> ${input_path} ${output_path}
  )
endforeach()

source_group(rsrcs REGULAR_EXPRESSION "/resources/.*.cpp$")
source_group(game REGULAR_EXPRESSION "/game/")
