#pragma once

#ifdef FLUX_TARGET_OPENGL
#    define FLUX_OPENGL_FULL_VERSION                                                               \
        FLUX_JOIN(FLUX_OPENGL_VERSION_MAJOR, FLUX_JOIN(FLUX_OPENGL_VERSION_MINOR, 0))
#    define FLUX_GLSL_VERSION "#version " FLUX_TO_STRING(FLUX_OPENGL_FULL_VERSION)
#endif