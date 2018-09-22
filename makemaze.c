/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: makemaze.c,v 1.2 2004/08/09 05:06:00 sirdan Exp $ */

#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "maze.h"
#include "move.h"

typedef struct {
  square **data, **sentinal, **front, **back;
} queue;

static void
init_queue (queue *q, long size)
{
  q->data = xmalloc (size * sizeof (square *));
  q->sentinal = q->data + size;
  q->front = q->back = q->data;
}

static void
destroy_queue (queue *q)
{
  xfree (q->data);
}

static void
enqueue (queue *q, square *s)
{
  assert (q->back < q->sentinal);
  *q->back++ = s;
}

static square *
dequeue (queue *q)
{
  assert (q->front < q->back);
  return *q->front++;
}

static bool
queue_empty (queue *q)
{
  return q->front == q->back;
}

typedef struct branch_data_ {
  square *branch;
  long length;
  struct branch_data_ *next;
} branch_data;

typedef struct {
  branch_data *top;
  int size;
} branch_stack;

static void
branch_stack_init (branch_stack *s)
{
  s->top = NULL;
  s->size = 0;
}

static void
branch_stack_push (branch_stack *st, square *sq)
{
  branch_data *d = xmalloc (sizeof (branch_data));
  d->branch = sq;
  d->length = 0;
  d->next = st->top;
  st->top = d;
  ++st->size;
}

static void
branch_stack_pop (branch_stack *s)
{
  branch_data *node = s->top;

  assert (node);		/* Else underflow. */
  s->top = node->next;
  xfree (node);
  --s->size;
}

static void
delete_branch_stack (branch_stack *s)
{
  while (s->top)
    branch_stack_pop (s);
}

void
reset_maze (maze *m)
{
  long pitch = m->x_dim + 2;
  square *s, *sentinal;
  /* Clear all visited flags. */
  sentinal = m->data + pitch * (m->y_dim + 2);
  for (s = m->data; s < sentinal; ++s)
    *s &= ~(VISITED | RIGHT_VISITED | DOWN_VISITED
	    | VISITED_ONCE | VISITED_TWICE);
}

static void
visit_row (square *s, long pitch)
{
  square *limit;
  for (limit = s + pitch; s < limit; ++s)
    set_visited (*s);
}

static void
create_perimeter (square *data, long x_dim, long y_dim)
{
  square *row = data, *last_row;
  long x_actual = x_dim + 2, y_actual = y_dim + 2;

  visit_row (row, x_actual);

  last_row = data + x_actual * (y_actual - 1);
  for (row += x_actual; row < last_row; row += x_actual) {
    set_visited (row [0]);
    set_visited (row [x_actual - 1]);
  }

  visit_row (row, x_actual);
}

static bool
design_maze (maze *m, queue *q)
{
  /* Begin building the maze in the top left corner (start). */
  long pitch = m->x_dim + 2;
  square *pos = m->data + pitch + 1;
  direction dir;

  for (;;) {
    /* Mark the current square and save it so that if we get to a dead end, */
    /* we can return to look for branch points. */
    set_visited (*pos);
    enqueue (q, pos);

    for (;;) {
      /* Try to find an unvisited adjacent square. */
      dir = possible_next_move (pos, pitch);
      if (dir) {
	/* Found one. */
	make_open (pos, dir, pitch);
	pos = adjacent_square (pos, dir, pitch);
	break;
      }
      else {
	/* We reached a dead end. Go back and look for a place to branch. */
	if (!queue_empty (q)) {
	  pos = dequeue (q);
	}
	else  {
	  /* Done! Make sure we visited all the squares. */
	  assert (q->back == q->sentinal);
	  return true;
	}
      }
    }
  }
}

static void
choose_branch (branch_stack *s, int n, maze *m)
{
  branch_data max, second_max;
  long total_max_length;
  assert (n > 1 && s->size >= n);

  /* This should ensure that a valid branch is always chosen. */
  max.length = second_max.length = -1;

  while (n--) {
    if (s->top->length > max.length) {
      second_max = max;
      max = *s->top;
    }
    else if (s->top->length > second_max.length)
      second_max = *s->top;

    branch_stack_pop (s);
  }

  /* Check if we found a path longer than the previous longest. */
  total_max_length = max.length + second_max.length + 1;
  if (m->longest < total_max_length) {
    m->start = second_max.branch;
    m->finish = max.branch;
    m->longest = total_max_length;
  }

  /* Repush the max. */
  branch_stack_push (s, max.branch);
  s->top->length = max.length;
}

static void
set_start_and_finish (maze *m)
{
  branch_stack s;
  long pitch = m->x_dim + 2;
  square *lvn, *pos = m->data + pitch + 1;

  reset_maze (m);
  m->start = m->finish = pos;
  m->longest = 0;

  branch_stack_init (&s);

  for (;;) {
    for (;;) {
      *pos |= VISITED_ONCE;
      lvn = least_visited_neighbor (pos, pitch);
      if (*lvn & (VISITED_ONCE /* | VISITED_TWICE */))
	break;
      pos = lvn;
    }
    /* If there are no loops, the following assertion should be true. */
    assert (num_openings (pos) == 1);
    branch_stack_push (&s, pos);
    for (;;) {
      lvn = least_visited_neighbor (pos, pitch);
      if (*lvn & VISITED_TWICE) {
	/* Done */
	assert (s.size == num_openings (pos));
	if (s.size == 1)
	  branch_stack_push (&s, pos);
	choose_branch (&s, s.size, m);
	delete_branch_stack (&s);
	return;
      }
      if (*lvn & VISITED_ONCE) {
	int n = num_openings (pos);
	if (n > 2)
	  choose_branch (&s, n - 1, m);
	*pos |= VISITED_TWICE;
	++s.top->length;
	pos = lvn;
      }
      else {
	pos = lvn;
	break;
      }
    }
  }
}

bool
make_maze (maze *m, long x_dim, long y_dim)
{
  queue q;
  /* Do some initialization and cleanup in this function. */

  if (x_dim < 2 || y_dim < 2)
    fatal ("Dimensions must be at least 2x2.");

  m->x_dim = x_dim;
  m->y_dim = y_dim;
  /* To avoid problems with boundries, we allocate the maze */
  /* with a one square wide perimeter set to visited. */
  m->data = xcalloc ((y_dim + 2) * (x_dim + 2) * sizeof (square));
  init_queue (&q, x_dim * y_dim);

  create_perimeter (m->data, x_dim, y_dim);

  design_maze (m, &q);
  destroy_queue (&q);

  set_start_and_finish (m);

  return true;
}

#define PILLAR	'*'
#define HWALL	'='
#define VWALL	'l'

static void
print_top (long dim)
{
  putchar (PILLAR);
  while (dim--) {
    /* Double up the horizontal walls because most fonts are about */
    /* twice as tall as they are wide. It makes it easier to see. */
    putchar (HWALL); putchar (HWALL);
    putchar (PILLAR);
  }
  putchar ('\n');
}

static void
print_row (square *row, long dim, square *start, square *finish)
{
  square *pos, *limit = row + dim + 1;

  putchar (VWALL);
  /* Do vertical walls. */
  for (pos = row + 1; pos < limit; ++pos) {
    if (pos == start) {
      putchar ('$'); putchar ('$');
    }
    else if (pos == finish) {
      putchar ('#'); putchar ('#');
    }
    else {
      putchar (' '); putchar (' ');
    }
    /* Look for a wall to the right. */
    putchar (*pos & RIGHT ? ' ' : VWALL);
  }
  putchar ('\n');

  putchar (PILLAR);
  /* Do horizontal walls. */
  for (pos = row + 1; pos < limit; ++pos) {
    char c = *pos & DOWN ? ' ' : HWALL;
    /* Look for a wall below. */
    putchar (c); putchar (c);
    putchar (PILLAR);
  }
  putchar ('\n');
}

void
print_maze (maze *m)
{
  long x_dim = m->x_dim, pitch = x_dim + 2;
  square *row = m->data, *last_row = row + pitch * (m->y_dim + 1);

  /* Draw the top wall and left wall "by hand". */
  print_top (x_dim);

  for (row += pitch; row < last_row; row += pitch)
    print_row (row, x_dim, m->start, m->finish);
}
