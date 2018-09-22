/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: move.h,v 1.2 2004/08/09 05:06:00 sirdan Exp $ */
#ifndef MAKEMAZE_RAND_DIRS_H
#define MAKEMAZE_RAND_DIRS_H	1

#include "common.h"

/* Get the pointer to an adjacent square. */
square *adjacent_square (square *, direction, long pitch);

/* Return direction to a non-visited adjacent square. */
direction possible_next_move (square *, long pitch);

/* Opens a path between a square and the square in the given direction. */
void make_open (square *, direction, long pitch);

/* Finds a neighbor NOT_VISITED, VISITED_ONCE or VISITED_TWICE */
/* in that order. */
square *least_visited_neighbor (square *, long pitch);

/* Number of directions available from a square. */
int num_openings (square *);

#endif /* MAKEMAZE_RAND_DIRS_H */
