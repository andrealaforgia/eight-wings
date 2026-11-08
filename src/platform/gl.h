/* OpenGL header shim. Fixed-function (GL 2.1 compatibility) is used on purpose: the 256x224 pixel-art
   renderer needs only textured quads, and it runs everywhere SDL2 can create a legacy GL context. */
#ifndef PLATFORM_GL_H_
#define PLATFORM_GL_H_
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION 1
#include <OpenGL/gl.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#endif
#endif
