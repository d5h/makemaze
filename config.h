/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: config.h,v 1.1 2004/08/06 02:23:10 sirdan Exp $ */
#ifndef MAKEMAZE_CONFIG_H
#define MAKEMAZE_CONFIG_H	1

/* NOTE: */
/* Don't define USE_SDL here, if you want SDL support, */
/* define it on the command line with "-DUSE_SDL=1". */

/* Default dimensions if no argument is given. */
#if USE_SDL
/* Default dimensions for windowed mode. */
# define DEFAULT_SDL_X_DIM 20
# define DEFAULT_SDL_Y_DIM 20
#endif  /* USE_SDL */
/* Default dimensions for stdout. */
# define DEFAULT_STDOUT_X_DIM 26
# define DEFAULT_STDOUT_Y_DIM 11

/* Maximum allowed argument to --radius option. */
#if USE_SDL
# define RADIUS_MAX 18
# define DEFAULT_RADIUS 10
#endif /* USE_SDL */

#endif /* MAKEMAZE_CONFIG_H */

