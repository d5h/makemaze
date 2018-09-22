/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: play_maze.c,v 1.3 2004/08/09 05:06:00 sirdan Exp sirdan $ */

/* This file deals mainly with SDL. However if USE_SDL is not set, */
/* you can still compile this, it'll just be a dummy object file. */
#if USE_SDL

#include <SDL/SDL.h>

#include <stdlib.h>
#include <assert.h>
#include "maze.h"

/* Pixels in the tiles. */
#define TILE_DIMENSION 12
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
# define RED_MASK_3	0xFF0000
# define GREEN_MASK_3	0x00FF00
# define BLUE_MASK_3	0x0000FF
#else  /* !SDL_BIG_ENDIAN */
# define RED_MASK_3	0x0000FF
# define GREEN_MASK_3	0x00FF00
# define BLUE_MASK_3	0xFF0000
#endif /* SDL_BYTEORDER */

SDL_Event event;
bool need_redraw;
/* Main surface. This will be freed by SDL itself. */
SDL_Surface *window;

/* Helpful symbols, if possibly a little ugly, */
/* they make things more effiecient. */
static SDL_Rect static_tile_rect;
static SDL_Rect blit_rect;
static Uint32 white;
static square *maze_upper_left;
static long pitch;

/* Tiles. */
#include "wall.data"
#include "you.data"
#include "trail.data"
#include "finish.data"
#include "down_arrow.data"
#include "left_arrow.data"
#include "right_arrow.data"
#include "up_arrow.data"
SDL_Surface *wall, *you, *trail, *finish;
SDL_Surface *down_arrow, *left_arrow, *right_arrow, *up_arrow;

/* Position in the maze. Since it takes 2 tiles for every square */
/* in the maze, these values indicate a "half step" in their LSB. */
/* To get an index into the maze squares, shift right once. */
typedef struct {
  long x, y;
} position_holder;

position_holder position;
position_holder start_pos;
position_holder finish_pos;

static SDL_Surface *
load_tile_RGB (const char *name, unsigned char *data)
{
  SDL_Surface *s;

  s = SDL_CreateRGBSurfaceFrom (data,
				TILE_DIMENSION, TILE_DIMENSION,
				24, TILE_DIMENSION * 3,
				RED_MASK_3, GREEN_MASK_3, BLUE_MASK_3,
				0);
  if (!s)
    fatal ("Couldn't create \"%s\" tile: %s", name, SDL_GetError ());

  return s;
}

static void
load_tiles (void)
{
  Uint32 tile_white;

  /* If one is NULL, the rest should be anyway. */
  assert (wall == NULL);

  wall = load_tile_RGB ("wall", wall_data);
  trail = load_tile_RGB ("trail", trail_data);
  you = load_tile_RGB ("you", you_data);
  finish = load_tile_RGB ("finsh", finish_data);

  down_arrow = load_tile_RGB ("down_arrow", down_arrow_data);
  tile_white = SDL_MapRGB (down_arrow->format, '\376', '\376', '\376');
  SDL_SetColorKey (down_arrow, SDL_SRCCOLORKEY, tile_white); 

  left_arrow = load_tile_RGB ("left_arrow", left_arrow_data);
  SDL_SetColorKey (left_arrow, SDL_SRCCOLORKEY, tile_white);

  right_arrow = load_tile_RGB ("right_arrow", right_arrow_data);
  SDL_SetColorKey (right_arrow, SDL_SRCCOLORKEY, tile_white);

  up_arrow = load_tile_RGB ("up_arrow", up_arrow_data);
  SDL_SetColorKey (up_arrow, SDL_SRCCOLORKEY, tile_white);

  
  /* Initialize some global symbols. */
  static_tile_rect.x = static_tile_rect. y = 0;
  static_tile_rect.w = static_tile_rect.h = TILE_DIMENSION;
  blit_rect.w = blit_rect.h = TILE_DIMENSION;
  white = SDL_MapRGB (window->format, 255, 255, 255);
}

static void
free_tiles (void)
{
  if (wall) {
    SDL_FreeSurface (wall);
    wall = NULL;
  }
  if (you) {
    SDL_FreeSurface (you);
    you = NULL;
  }
  if (trail) {
    SDL_FreeSurface (trail);
    trail = NULL;
  }
  if (finish) {
    SDL_FreeSurface (finish);
    finish = NULL;
  }
  if (up_arrow) {
    SDL_FreeSurface (up_arrow);
    up_arrow = NULL;
  }
  if (down_arrow) {
    SDL_FreeSurface (down_arrow);
    down_arrow = NULL;
  }
  if (left_arrow) {
    SDL_FreeSurface (left_arrow);
    left_arrow = NULL;
  }
  if (right_arrow) {
    SDL_FreeSurface (right_arrow);
    right_arrow = NULL;
  }

}

static void
blit_tile (SDL_Surface *s, Sint16 x, Sint16 y)
{
  blit_rect.x = x;
  blit_rect.y = y;
  SDL_BlitSurface (s, &static_tile_rect,
		   window, &blit_rect);
}

static void
blit_blank (Sint16 x, Sint16 y)
{
  blit_rect.x = x;
  blit_rect.y = y;
  SDL_FillRect (window, &blit_rect, white);
}

static void
process_move (SDLKey key)
{
  /* Find our present position. */
  square *sqr = maze_upper_left + pitch * (position.y / 2) + (position.x / 2);

  /* Assume we will need to redraw */
  /* unless we learn that the key pressed is ineffectual. */
  need_redraw = true;

  if (position.x & 1) {
    /* X coordinate is odd, on the right half of a square. */
    /* Only process left/right. */
    switch (key) {
    case SDLK_LEFT:
      --position.x;
      sqr[0] |= VISITED;
      break;
    case SDLK_RIGHT:
      ++position.x;
      sqr[1] |= VISITED;
      break;
    default:
      need_redraw = false;
    }
  }
  else if (position.y & 1) {
    /* Y coordinate is odd, on bottom half of a square. */
    /* Only process up/down. */
    switch (key) {
    case SDLK_UP:
      --position.y;
      sqr[0] |= VISITED;
      break;
    case SDLK_DOWN:
      ++position.y;
      sqr[pitch] |= VISITED;
      break;
    default:
      need_redraw = false;
    }
  }
  else {
    /* On the top-left half of the square, process all four directions, */
    /* and make sure the move is allowed. */
    switch (key) {
    case SDLK_LEFT:
      if (*sqr & LEFT) {
	--position.x;
	sqr[-1] |= RIGHT_VISITED;
      }
      break;
    case SDLK_RIGHT:
      if (*sqr & RIGHT) {
	++position.x;
	sqr[0] |= RIGHT_VISITED;
      }
      break;
    case SDLK_UP:
      if (*sqr & UP) {
	--position.y;
	sqr[-pitch] |= DOWN_VISITED;
      }
      break;
    case SDLK_DOWN:
      if (*sqr & DOWN) {
	++position.y;
	sqr[0] |= DOWN_VISITED;
      }
      break;
    default:
      need_redraw = false;
    }
  }

  /* If position == finish, game won! */
}

static bool
maybe_draw_arrow (long i, long j, Sint16 bx, Sint16 by)
{
  long x_diff = j - position.x, y_diff = i - position.y;
  /* Consider up/down arrows. */
  if (x_diff == 0) {
    if (y_diff == options.radius && finish_pos.y > position.y)
      blit_tile (down_arrow, bx, by);
    else if (y_diff == -options.radius && finish_pos.y < position.y)
      blit_tile (up_arrow, bx, by);
    else
      return false;
  }
  /* Consider left/right arrows. */
  else if (y_diff == 0) {
    if (x_diff == options.radius && finish_pos.x > position.x)
      blit_tile (right_arrow, bx, by);
    else if (x_diff == -options.radius && finish_pos.x < position.x)
      blit_tile (left_arrow, bx, by);
    else
      return false;
  }
  else
    return false;
  return true;
}

/* Used for debugging. Usually set to zero. */
#define ALWAYS_DRAW_ARROWS	0

static void
draw_window (maze *m)
{
  long i, j, x_dim = m->x_dim, y_dim = m->y_dim;
  Sint16 bx, by;

#if !ALWAYS_DRAW_ARROWS
  bool need_arrows;

  /* If the finish is visible, we don't need arrows. */
  need_arrows = abs (position.x - finish_pos.x) > options.radius
    || abs (position.y - finish_pos.y) > options.radius;
#endif /* !ALWAYS_DRAW_ARROWS */

  /* Offset used for blitting. */
  by = 0;
  for (i = position.y - options.radius;
       i <= position.y + options.radius;
       ++i)
    {
      bx = 0;
      for (j = position.x - options.radius;
	   j <= position.x + options.radius;
	   ++j)
	{
	  /* When we are in the middle of the display, draw "you". */
	  if (j == position.x && i == position.y)
	    blit_tile (you, bx, by);

	  /* Check for the finish. */
	  /* If we need arrows, we know the finish is off-screen. */
	  else if (!need_arrows && (j == finish_pos.x && i == finish_pos.y)) 
	    blit_tile (finish, bx, by);

	  /* Handle in-bound cases and top/left border cases. */
	  else if (j >= -1 && j < 2 * x_dim
		   && i >= -1 && i < 2 * y_dim)
	    {
	      /* Top/left boundry cases: */
	      if (i == -1 || j == -1)
		blit_tile (wall, bx, by);

	      /* In-bounds cases: */
	      else {
		square *sqr = maze_upper_left + pitch * (i / 2) + (j / 2);
		SDL_Surface *s;
		/* Determine what kind of tile goes here. */
		if (j & 1) {
		  /* On the right half. */
		  if (i & 1)
		    s = wall;	/* Bottom-right, always a wall. */
		  else {
		    /* Top-right: */
		    if (*sqr & RIGHT)
		      s = *sqr & RIGHT_VISITED ? trail : NULL;
		    else
		      s = wall;
		  }
		}
		else {
		  /* On the left half. */
		  if (i & 1) {
		    /* Bottom-left: */
		    if (*sqr & DOWN)
		      s = *sqr & DOWN_VISITED ? trail : NULL;
		    else
		      s = wall;
		  }
		  else		/* Top-left: */
		    s = *sqr & VISITED ? trail : NULL;
		}
		if (s)
		  blit_tile (s, bx, by);
		else
		  blit_blank (bx, by);
	      }
	    }
	  /* Out-of-bounds case: */
	  else
	    blit_blank (bx, by);

#if ALWAYS_DRAW_ARROWS
	  maybe_draw_arrow (i, j, bx, by);
#else  /* !ALWAYS_DRAW_ARROWS */
	  /* Draw arrows if the finish is not on-screen. */
	  if (need_arrows)
	    maybe_draw_arrow (i, j, bx, by);
#endif /* ALWAYS_DRAW_ARROWS */

	  /* Move to next offset. */
	  bx += TILE_DIMENSION;
	}
      by += TILE_DIMENSION;
    }

  SDL_UpdateRect (window, 0, 0, 0, 0);
  need_redraw = false;
}

void
play_maze (maze *m)
{
  /* Don't call this function twice. */
  assert (window == NULL);

  /* Deal with enormous mazes later. For now be naive. */
  window = SDL_SetVideoMode (TILE_DIMENSION * (2 * options.radius + 1),
			     TILE_DIMENSION * (2 * options.radius + 1),
			     0, SDL_ANYFORMAT);
  if (window == NULL)
    fatal ("Couldn't create window: %s", SDL_GetError ());

  load_tiles ();
  /* Initialize some global symbols. */
  pitch = m->x_dim + 2;
  maze_upper_left = m->data + pitch + 1;
  /* Initialize the maze. Reset all visited squares. */
  reset_maze (m);
  /* Initialize the our position. */
  start_pos.x = ((m->start - m->data) % pitch - 1) * 2;
  start_pos.y = ((m->start - m->data) / pitch - 1) * 2;
  finish_pos.x = ((m->finish - m->data) % pitch - 1) * 2;
  finish_pos.y = ((m->finish - m->data) / pitch - 1) * 2;
  position.x = start_pos.x;
  position.y = start_pos.y;
  /* Make sure this goes after reset_maze. We start here, so its visited. */
  *m->start |= VISITED;
  need_redraw = true;

  /* Event loop. */
  do {
    /* Only redraw when the position changes. */
    if (need_redraw) 
      draw_window (m);
    
    if (!SDL_WaitEvent (&event))
      fatal ("%s", SDL_GetError ());

    /* Handle keyboard events. */
    if (event.type == SDL_KEYDOWN)
      process_move (event.key.keysym.sym);
    
  } while (event.type != SDL_QUIT);

  free_tiles ();
}

#else  /* !USE_SDL */
/* If not using SDL, just define a pointless symbol so that the */
/* object file is not empty. */
extern int I_am_Iron_Man;
int I_am_Iron_Man;
#endif /* USE_SDL */
