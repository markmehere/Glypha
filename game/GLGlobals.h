#ifndef GLGLOBALS_H
#define GLGLOBALS_H

#ifdef EMSCRIPTEN
#include <GL/gl.h>
#include <gl4esinit.h>
#elif defined(__ANDROID__)
#include "../gl4es/include/GL/gl.h"
#include "../gl4es/include/gl4esinit.h"
#endif

#endif