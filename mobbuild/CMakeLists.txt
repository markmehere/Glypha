cmake_minimum_required(VERSION 3.1)

set(EXE_NAME "Glypha Mobile")
string(TOLOWER "${EXE_NAME}" EXE_LOWER)
string(REPLACE " " "_" EXE_NAME_NOSPACES "${EXE_LOWER}")
set(EXE_IDENTIFIER "com.kainjow.${EXE_NAME_NOSPACES}")

project(Glypha)

set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "4")

find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL_IMAGE REQUIRED)

include_directories(${PNG_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${SDL_IMAGE_INCLUDE_DIR})

set(SOURCES
  ../game/GLGlobals.h
  ../game/GLGame.cpp
  ../game/GLGame.h
  ../game/GLImage_libpng.cpp
  ../game/GLImage.cpp
  ../game/GLImage.h
  ../game/GLRect.cpp
  ../game/GLRect.h
  ../game/GLRenderer.cpp
  ../game/GLRenderer.h
  ../game/GLResources.h
  ../game/GLUtils.cpp
  ../game/GLUtils.h
  ../game/GLDataToWave.cpp
  ../game/GLDataToWave.h
  ../game/GLBufferReader.cpp
  ../game/GLBufferReader.h
  ../game/GLCursor.h
  ../game/GLCursor.cpp
  ../game/GLSounds.h
  ../game/GLSoundsSDL.cpp
  ../game/GLFont.cpp
  ../game/GLFont.h
  ../game/GLPoint.h
  ../game/GLPrefs.cpp
  ../game/GLPrefs.h
  ../web/audio.cpp
  ../web/audio.h
  ../web/main.cpp
  ../build/resources/enemyFly_png.cpp
  ../build/resources/enemyWalk_png.cpp
  ../build/resources/background_png.cpp
  ../build/resources/hand_png.cpp
  ../build/resources/numbers_png.cpp
  ../build/resources/obelisks_png.cpp
  ../build/resources/platforms_png.cpp
  ../build/resources/player_png.cpp
  ../build/resources/playerIdle_png.cpp
  ../build/resources/torches_png.cpp
  ../build/resources/about_png.cpp
  ../build/resources/boom1_wav.cpp
  ../build/resources/music_wav.cpp
  ../build/resources/bonus_wav.cpp
  ../build/resources/boom2_wav.cpp
  ../build/resources/spawn_wav.cpp
  ../build/resources/splash_wav.cpp
  ../build/resources/bird_wav.cpp
  ../build/resources/flap_wav.cpp
  ../build/resources/flap2_wav.cpp
  ../build/resources/grate_wav.cpp
  ../build/resources/lightning_wav.cpp
  ../build/resources/scrape2_wav.cpp
  ../build/resources/screech_wav.cpp
  ../build/resources/walk_wav.cpp
  ../build/resources/egg_png.cpp
  ../build/resources/eye_png.cpp
  ../build/resources/help_png.cpp
  ../build/resources/font11_png.cpp
  ../build/resources/font11_fnt.cpp
)

add_definitions(-DMOBILE)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${EXE_NAME_NOSPACES})

target_include_directories(${PROJECT_NAME} PRIVATE ../game ../web ../webbuild/gl4es/include)

target_compile_definitions(${PROJECT_NAME} PRIVATE "GL_GAME_NAME=\"Glypha III\""
  "GL_GAME_VERSION=\"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}\"")

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/../webbuild/gl4es/lib/libGL.a
    "-sUSE_LIBPNG"
    "-sUSE_SDL=2"
    "-sUSE_SDL_IMAGE=2"
    "-sFULL_ES2"
    "-lidbfs.js"
    "-sEXPORTED_FUNCTIONS=_main,_malloc"
    "-sEXPORTED_RUNTIME_METHODS=stringToUTF8,lengthBytesUTF8"
  )

set(CMAKE_EXECUTABLE_SUFFIX ".html")
