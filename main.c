/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: main.c,v 1.4 2004/08/06 02:35:44 sirdan Exp $ */

#include "config.h"

/* This program should compile just fine using only the C standard library. */
/* You can compile with SDL, however, by defining USE_SDL to non-zero. */
#if USE_SDL
# include <SDL/SDL.h>
const char *interface = "GUI (SDL)/Text";
#else  /* !USE_SDL */
const char *interface = "Text";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "maze.h"
#include "move.h"

const char *program = "Make Maze";
const char *author = "Copyright (C) Daniel Hipschman, Aug 2004";

/* Define external variable. */
prog_options options;

static void
usage (bool err)
{
  FILE *stream;
  int exit_status;

  if (err) {
    stream = stderr;
    exit_status = EXIT_FAILURE;
  }
  else {
    stream = stdout;
    exit_status = EXIT_SUCCESS;
  }

  fprintf (stream,
	   "%s %s %s,\n"
	   "%s.\n"
	   "Usage: makemaze [options] [MxN]\n"
	   " where M and N are the dimensions of the maze.\n",
	   program, interface, VERSION, author);
  if (!err) {
    /* Print extended help if the user asks for it. */
    fprintf (stream,
	     "Options:\n"
	     "  --help        Prints this help.\n"
#if USE_SDL
	     "  --radius=N    Sets the size of the window when --stdout is not used.\n"
	     "                The actual size of the window is (2*radius+1) tiles.\n"
	     "                This determines how much of the maze can be seen at once.\n"
#endif /* USE_SDL */
	     "  --seed=N      Specify a seed for the random number generator.\n"
	     "                Along with the dimensions, this number uniquely\n"
	     "                identifies a maze on a given system.\n"
	     "  --stdout      Print the maze to standard output and quits.\n"
	     "  --version     Prints the version number.\n");
  }
  exit (exit_status);
}

static void
get_options (int argc, char **argv)
{
  bool got_dimensions = false;

  while (--argc) {
    ++argv;
    if (strcmp (*argv, "--stdout") == 0)
      options.stdout_flag = 1;
    else if (strncmp (*argv, "--seed", 6) == 0) {
      if (options.seed_flag)
	fatal ("Multiple --seed option.");
      if (sscanf (*argv, "--seed=%u", &options.seed) != 1)
	fatal ("Malformed --seed argument: %s", *argv + 6);
      options.seed_flag = 1;
    }
#if USE_SDL
    else if (strncmp (*argv, "--radius", 8) == 0) {
      if (options.radius_flag)
	fatal ("Multiple --radius argument.");
      if (sscanf (*argv, "--radius=%li", &options.radius) != 1)
	fatal ("Malformed --radius argument: %s", *argv + 8);
      /* Make sure radius is positive. */
      if (options.radius < 1)
	fatal ("Radius must be positive.");
      if (options.radius > RADIUS_MAX)
	fatal ("Radius cannot exceed %d.", RADIUS_MAX);
      options.radius_flag = 1;
    }
#endif /* USE_SDL */
    else if (isdigit (**argv)) {
      char x;
      if (got_dimensions)
	fatal ("One set of dimensions at a time.");
      if (sscanf (*argv, "%lu%c%lu",
		  &options.x_dim, &x, &options.y_dim) != 3
	  || (x != 'x' && x != 'X'))
	fatal ("Malformed dimensions: %s", *argv);
      /* Can only handle one set of dimensions. */ 
      got_dimensions = true;
    }
    else if (strcmp (*argv, "--help") == 0)
      usage (false);
    else if (strcmp (*argv, "--version") == 0) {
      printf ("%s %s %s\n%s\n", program, interface, VERSION, author);
      exit (EXIT_SUCCESS);
    }
    else
      fatal ("Unknown option: %s\nTry the --help option.\n", *argv);
  }    

  /* Default dimensions. */
  if (!got_dimensions) {
#if USE_SDL
    if (options.stdout_flag) {
      options.x_dim = DEFAULT_STDOUT_X_DIM;
      options.y_dim = DEFAULT_STDOUT_Y_DIM;
    }
    else {
      options.x_dim = DEFAULT_SDL_X_DIM;
      options.y_dim = DEFAULT_SDL_Y_DIM;
    }
#else  /* !USE_SDL */
    options.x_dim = DEFAULT_STDOUT_X_DIM;
    options.y_dim = DEFAULT_STDOUT_Y_DIM;
#endif /* USE_SDL */
  }

  /* Default seed, make it really random. */
  if (!options.seed_flag)
    options.seed = time (NULL);

#if USE_SDL
  /* Check compatibility of some flags. */
  if (options.radius_flag && options.stdout_flag)
    fatal ("Options --radius and --stdout are incompatible.");
  /* Default radius. */
  if (!options.radius_flag)
    options.radius = DEFAULT_RADIUS;
#endif /* USE_SDL */
}

#if USE_SDL
static void
init_sdl (void)
{
  if (SDL_Init (SDL_INIT_VIDEO) == -1)
    fatal ("Could not initialize SDL: %s", SDL_GetError ());
  if (atexit (SDL_Quit)) {
    /* I don't know why this would happen, but just to be safe. */
    SDL_Quit ();
    fatal ("Couldn't register `atexit'.");
  }

  SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}
#endif /* USE_SDL */

int
main (int argc, char **argv)
{
  maze m;

  get_options (argc, argv);
  srand (options.seed);

  make_maze (&m, options.x_dim, options.y_dim);

#if USE_SDL
  /* If we are building with SDL, make it the default. */
  if (options.stdout_flag) {
    print_maze (&m);	
  }
  else {
    /* Don't try to use SDL until the last minute, because */
    /* if SDL cannot be initialized, they can still use stdout. */
    init_sdl ();
    play_maze (&m);
  }
#else  /* !USE_SDL */
  /* If not building with SDL, always use stdout, */
  /* regardless of if it is set as an option. */
  print_maze (&m);
#endif /* USE_SDL */

  free_maze (&m);

  return 0;
}

