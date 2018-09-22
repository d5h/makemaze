/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: maze.h,v 1.3 2004/08/09 05:06:00 sirdan Exp $ */
#ifndef MAKEMAZE_MAZE_H
#define MAKEMAZE_MAZE_H	1

#include "common.h"

typedef struct {
  square *data, *start, *finish;
  long x_dim, y_dim, longest;
} maze;

bool make_maze (maze *, long x_dim, long y_dim);
void print_maze (maze *);
#define free_maze(m) (xfree ((m)->data))

#if USE_SDL
void play_maze (maze *);
#endif /* USE_SDL */

void reset_maze (maze *);

#endif /* MAKEMAZE_MAZE_H */
