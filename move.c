/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: move.c,v 1.2 2004/08/09 05:06:00 sirdan Exp $ */

#include <stdlib.h>
#include "maze.h"
#include "move.h"

static void
shift_down (direction *array, int index)
{
  while (index < 3) {
    array [index] = array [index + 1];
    ++index;
  }
}

static int
small_rand (int bound)
{
  return rand () % (bound + 1);
}

static void
permute (direction *dirs)
{
  /* Return a random permutation of directions */
  direction pool [4] = {UP, DOWN, LEFT, RIGHT};
  int i;

  for (i = 3; i >= 0; --i) {
    int index = small_rand (i);
    *dirs++ = pool [index];
    shift_down (pool, index);
  }
}

#define new_possible_direction() (possible_direction (true))
#define next_possible_direction() (possible_direction (false))

static direction
possible_direction (bool newdir)
{
  /* With the help of a macro, this function implements */
  /* new_possible_direction () and next_possible_direction (). */
  static direction dirs [5];
  static int index;

  if (newdir) {
    permute (dirs);
    /* Flag the last direction. */
    dirs [4] = 0;
    index = 0;
  }
  else
    ++index;

  return dirs [index];
}

square *
adjacent_square (square *s, direction dir, long pitch)
{
  switch (dir) {
  case UP:
    return s - pitch;
  case DOWN:
    return s + pitch;
  case LEFT:
    return s - 1;
  case RIGHT:
    return s + 1;
  }
  /* Should never get here. */
  return NULL;
}

direction
possible_next_move (square *s, long pitch)
{
  /* Check all directions in a random order */
  /* until we find an adjacent square that has not been visited. */
  direction dir;
  for (dir = new_possible_direction (); dir; dir = next_possible_direction ())
    if (!is_visited (*adjacent_square (s, dir, pitch)))
      break;
  return dir;
}

void 
make_open (square *s, direction dir, long pitch)
{
  direction opposite = 0;

  switch (dir) {
  case UP: opposite = DOWN; break;
  case DOWN: opposite = UP; break;
  case LEFT: opposite = RIGHT; break;
  case RIGHT: opposite = LEFT; break;
  }

  /* Setting the direction flag makes it "open". */
  *s |= dir;
  *adjacent_square (s, dir, pitch) |= opposite;
}

/* If this is true, make some optimizations based on it. */
#if VISITED_ONCE < VISITED_TWICE
# define visited_level(s) ((s) & (VISITED_ONCE | VISITED_TWICE))
#else
static int
visited_level (square *s)
{
  square test = *s;
  return test & VISITED_TWICE ? 2 :
    test & VISITED_ONCE ? 1 : 0;
}
#endif

square *
least_visited_neighbor (square *s, long pitch)
{
  /* Check all directions in a random order */
  /* until we find the least visited neighbor. */
  square *possible, *neighbor = NULL;
  direction dir;

  for (dir = new_possible_direction (); dir; dir = next_possible_direction ()) {
    possible = adjacent_square (s, dir, pitch);
    if (*s & dir
	&& (neighbor == NULL
	    || visited_level (*neighbor) > visited_level (*possible))) 
      neighbor = possible;
  }
  return neighbor;
}

int
num_openings (square *s)
{
  direction dirs [4] = {UP, DOWN, LEFT, RIGHT};
  int i, num = 0;
  square test = *s;

  for (i = 0; i < 4; ++i)
    if (test & dirs [i])
      ++num;

  return num;
}
