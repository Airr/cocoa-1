/*
 *   R : A Computer Language for Statistical Data Analysis
 *   Copyright (C) 1997-2012   The R Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/* <UTF8> This does byte-level access, e.g. isspace, but is OK. */

/* ------------------- process .Renviron files in C -----------------
 *  Formerly part of ../unix/sys-common.c.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h> /* for setenv or putenv */
#include <Defn.h> /* for PATH_MAX */
#include <Rinterface.h>
#include <Fileio.h>
#include <ctype.h>		/* for isspace */

/* remove leading and trailing space */
static char *rmspace(char *s)
{
  return 0;
}

/* look for ${FOO-bar} or ${FOO:-bar} constructs, recursively.
static char *subterm(char *s)
{
  return 0;
}

/* skip along until we find an unmatched right brace */
static char *findRbrace(char *s)
{
  return 0;
}

#define BUF_SIZE 10000
static char *findterm(char *s)
{
  return 0;
}

static void Putenv(char *a, char *b)
{
  return;
}


#define MSG_SIZE 2000
static int process_Renviron(const char *filename)
{
  return 0;
}


/* try system Renviron: R_HOME/etc/Renviron.  Unix only. */
void process_system_Renviron()
{
  return;
}

#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for access, R_OK */
#endif

/* try site Renviron: R_ENVIRON, then R_HOME/etc/Renviron.site. */
void process_site_Renviron ()
{
  return;
}

/* try user Renviron: ./.Renviron, then ~/.Renviron */
void process_user_Renviron()
{
  return;
}

SEXP attribute_hidden do_readEnviron(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
