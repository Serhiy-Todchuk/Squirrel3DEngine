#pragma once

#ifdef _WIN32
# pragma	warning (disable:4786)
# pragma	warning (disable:4996)
# define GL_GLEXT_LEGACY
# ifdef WIN32_LEAN_AND_MEAN
#  undef WIN32_LEAN_AND_MEAN
# endif
# include <GL/glee.h>
# include <GL/gl.h>
#elif __APPLE__
# ifdef SUPPORT_GL3
#  include <OpenGL/gl3.h>
# else
#  include <GL/glee.h>
# endif
#endif

#include "macros.h"
