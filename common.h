/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: common.h,v 1.4 2004/08/09 05:06:00 sirdan Exp $ */
#ifndef MAKEMAZE_COMMON_H
#define MAKEMAZE_COMMON_H	1

#include <stddef.h>

extern const char *program;
extern const char *author;

typedef struct {
  unsigned int seed_flag : 1;
  unsigned int stdout_flag : 1;
#if USE_SDL
  unsigned int radius_flag : 1;
  long radius;
#endif /* USE_SDL */
  long x_dim, y_dim;
  unsigned int seed;
} prog_options;
extern prog_options options;

/* A square (room) in the maze. Store info in bits. */
typedef unsigned char square;
/* Mainly used for clarity. */
typedef unsigned char direction;

/* Useful boolean stuff. */
#if !defined (__cplusplus)	/* In case someone compiles with C++. */
typedef int bool;
# define true 1
# define false 0
#endif /* true/false */

/* Bits used with square. */
#define VISITED	0x01
/* Directions. We can use zero as an "no direction" indicator. */
#define UP	0x02
#define DOWN	0x04
#define LEFT	0x08
#define RIGHT	0x10

#if USE_SDL
/* Used for interactively playing the maze. */
# define RIGHT_VISITED	0x20
# define DOWN_VISITED	0x40
#endif /* USE_SDL */

/* Used for finding the longest path in the maze. */
#define VISITED_ONCE	VISITED
#define VISITED_TWICE	0x80

/* This seems useful. */
#define is_visited(s) ((s) & VISITED)
#define set_visited(s) ((s) |= VISITED)

/* Memory allocation routines. */
void *xmalloc (size_t);
void *xcalloc (size_t);
void xfree (void *);

/* In case something really bad happens. Call like printf. */
void fatal (const char *, ...);

#endif /* MAKEMAZE_COMMON_H */
