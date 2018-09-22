/* Copyright (C) Daniel Hipschman, Aug 2004 */
/* $Id: xmemory.c,v 1.2 2004/08/04 11:48:02 sirdan Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"

void *
xmalloc (size_t n)
{
  /* Some old systems have problems with malloc (0). */
  void *p = malloc (n ? n : 1);
  if (!p)
    fatal ("Memory failure!");
  return p;
}

void *
xcalloc (size_t n)
{
  /* Look Mom, one line! */
  return memset (xmalloc (n), 0, n);
}

void
xfree (void *p)
{
  /* Some old systems have problems with free (NULL). */
  if (p)
    free (p);
}

void
fatal (const char *format, ...)
{
  va_list args;
  va_start (args, format);

  fprintf (stderr, "%s:fatal error: ", program);
  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
  exit (EXIT_FAILURE);
}
