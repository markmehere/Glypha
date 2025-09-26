# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.5)

project("glypha")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DD3SOUND")

set(SOURCES
    game/GLGameLock.h
    game/GLGameState.h
    game/GLGame.cpp
    game/GLGame.h
    game/GLImage.cpp
    game/GLImage_Android.cpp
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
    game/GLCursor.cpp
    game/GLCursor.h
    game/GLDataToWave.cpp
    game/GLDataToWave.h
    game/GLSounds_Android.cpp
    game/GLSounds.h
    game/GLFont.cpp
    game/GLFont.h
    game/GLPoint.h
    game/GLPrefs.cpp
    game/GLPrefs.h
    game/GLMenu.cpp
    game/GLMenu.h
)

set(RESOURCES
    resources/enemyFly_png.cpp
    resources/enemyWalk_png.cpp
    resources/background_png.cpp
    resources/birdback_png.cpp
    resources/hand_png.cpp
    resources/numbers_png.cpp
    resources/obelisks_png.cpp
    resources/platforms_png.cpp
    resources/player_png.cpp
    resources/playerIdle_png.cpp
    resources/torches_png.cpp
    resources/about_png.cpp
    resources/boom1_wav.cpp
    resources/music_wav.cpp
    resources/bonus_wav.cpp
    resources/boom2_wav.cpp
    resources/spawn_wav.cpp
    resources/splash_wav.cpp
    resources/bird_wav.cpp
    resources/flap_wav.cpp
    resources/flap2_wav.cpp
    resources/grate_wav.cpp
    resources/lightning_wav.cpp
    resources/scrape2_wav.cpp
    resources/screech_wav.cpp
    resources/walk_wav.cpp
    resources/egg_png.cpp
    resources/eye_png.cpp
    resources/help_android_png.cpp
    resources/font11_png.cpp
    resources/font11_fnt.cpp
)

set(GL4ES_SOURCES
    gl4es/src/gl/arbconverter.c
    gl4es/src/gl/arbgenerator.c
    gl4es/src/gl/arbhelper.c
    gl4es/src/gl/arbparser.c
    gl4es/src/gl/array.c
    gl4es/src/gl/blend.c
    gl4es/src/gl/blit.c
    gl4es/src/gl/buffers.c
    gl4es/src/gl/build_info.c
    gl4es/src/gl/debug.c
    gl4es/src/gl/decompress.c
    gl4es/src/gl/depth.c
    gl4es/src/gl/directstate.c
    gl4es/src/gl/drawing.c
    gl4es/src/gl/enable.c
    gl4es/src/gl/envvars.c
    gl4es/src/gl/eval.c
    gl4es/src/gl/face.c
    gl4es/src/gl/fog.c
    gl4es/src/gl/fpe.c
    gl4es/src/gl/fpe_cache.c
    gl4es/src/gl/fpe_shader.c
    gl4es/src/gl/framebuffers.c
    gl4es/src/gl/gl_lookup.c
    gl4es/src/gl/getter.c
    gl4es/src/gl/gl4es.c
    gl4es/src/gl/glstate.c
    gl4es/src/gl/hint.c
    gl4es/src/gl/init.c
    gl4es/src/gl/light.c
    gl4es/src/gl/line.c
    gl4es/src/gl/list.c
    gl4es/src/gl/listdraw.c
    gl4es/src/gl/listrl.c
    gl4es/src/gl/loader.c
    gl4es/src/gl/logs.c
    gl4es/src/gl/matrix.c
    gl4es/src/gl/matvec.c
    gl4es/src/gl/oldprogram.c
    gl4es/src/gl/pixel.c
    gl4es/src/gl/planes.c
    gl4es/src/gl/pointsprite.c
    gl4es/src/gl/preproc.c
    gl4es/src/gl/program.c
    gl4es/src/gl/queries.c
    gl4es/src/gl/raster.c
    gl4es/src/gl/render.c
    gl4es/src/gl/samplers.c
    gl4es/src/gl/shader.c
    gl4es/src/gl/shaderconv.c
    gl4es/src/gl/shader_hacks.c
    gl4es/src/gl/stack.c
    gl4es/src/gl/stencil.c
    gl4es/src/gl/string_utils.c
    gl4es/src/gl/stubs.c
    gl4es/src/gl/texenv.c
    gl4es/src/gl/texgen.c
    gl4es/src/gl/texture.c
    gl4es/src/gl/texture_compressed.c
    gl4es/src/gl/texture_params.c
    gl4es/src/gl/texture_read.c
    gl4es/src/gl/texture_3d.c
    gl4es/src/gl/uniform.c
    gl4es/src/gl/vertexattrib.c
    gl4es/src/gl/wrap/gl4eswraps.c
    gl4es/src/gl/wrap/gles.c
    gl4es/src/gl/wrap/glstub.c
    gl4es/src/gl/math/matheval.c
    gl4es/src/glx/hardext.c
    gl4es/src/glx/glx.c
    gl4es/src/glx/lookup.c
    gl4es/src/glx/gbm.c
    gl4es/src/glx/streaming.c
)

add_library(gl4es_static STATIC
    ${GL4ES_SOURCES}
)

target_include_directories(gl4es_static PUBLIC
    gl4es/include
)

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(glypha SHARED
    main.cpp
    AndroidOut.cpp
    GL4ESRenderer.cpp
    ${SOURCES}
    ${RESOURCES}
)

target_include_directories(glypha PRIVATE
    game
    includes
)

target_compile_definitions(gl4es_static PUBLIC
    BCMHOST
    NOX11
    NO_GBM
    DEFAULT_ES=2
    GL4ES_COMPILE_FOR_USE_IN_SHARED_LIB
    NO_INIT_CONSTRUCTOR
)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

# Support 16 KB page sizes
target_link_options(glypha PRIVATE "-Wl,-z,max-page-size=16384")

# Configure libraries CMake uses to link your target library.
target_link_libraries(glypha
    gl4es_static
    game-activity::game-activity
    EGL
    GLESv2
    jnigraphics
    android
    log
    aaudio)
