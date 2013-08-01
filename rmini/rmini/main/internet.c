/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2001-12   The R Core Team.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>

#include <Rconnections.h>
#include <Rdynpriv.h>
#include <R_ext/R-ftp-http.h>
#include <Rmodules/Rinternet.h>

static R_InternetRoutines routines, *ptr = &routines;


/*
SEXP Rdownload(SEXP call, SEXP op, SEXP args, SEXP env);
Rconnection R_newurl(char *description, char *mode);
Rconnection R_newsock(char *host, int port, int server, char *mode, int timeout);


Next 6 are for use by libxml, only

void *R_HTTPOpen(const char *url);
int   R_HTTPRead(void *ctx, char *dest, int len);
void  R_HTTPClose(void *ctx);

void *R_FTPOpen(const char *url);
int   R_FTPRead(void *ctx, char *dest, int len);
void  R_FTPClose(void *ctx);

void Rsockopen(int *port)
void Rsocklisten(int *sockp, char **buf, int *len)
void Rsockconnect(int *port, char **host)
void Rsockclose(int *sockp)
void Rsockread(int *sockp, char **buf, int *maxlen)
void Rsockwrite(int *sockp, char **buf, int *start, int *end, int *len)

int Rsockselect(int nsock, int *insockfd, int *ready, int *write,
		double timeout)

int R_HTTPDCreate(const char *ip, int port);
void R_HTTPDStop(void);
 */

static int initialized = 0;

R_InternetRoutines *
R_setInternetRoutines(R_InternetRoutines *routines)
{
  return 0;
}

#ifdef Win32
extern Rboolean UseInternet2;
#endif

static void internet_Init(void)
{
  return;
}

SEXP Rdownload(SEXP args)
{
  return 0;
}

#ifdef Win32
SEXP attribute_hidden do_setInternet2(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
#endif

Rconnection attribute_hidden R_newurl(const char *description,
				      const char * const mode)
{
  return 0;
}

Rconnection attribute_hidden
R_newsock(const char *host, int port, int server, const char * const mode,
	  int timeout)
{
  return 0;
}

void *R_HTTPOpen(const char *url)
{
  return 0;
}

int   R_HTTPRead(void *ctx, char *dest, int len)
{
  return 0;
}

void  R_HTTPClose(void *ctx)
{
  return;
}

void *R_FTPOpen(const char *url)
{
  return 0;
}

int   R_FTPRead(void *ctx, char *dest, int len)
{
  return 0;
}

void  R_FTPClose(void *ctx)
{
  return;
}

int extR_HTTPDCreate(const char *ip, int port)
{
  return 0;
}

void extR_HTTPDStop(void)
{
  return;
}


SEXP Rsockconnect(SEXP sport, SEXP shost)
{
  return 0;
}

SEXP Rsockread(SEXP ssock, SEXP smaxlen)
{
  return 0;
}

SEXP Rsockclose(SEXP ssock)
{
  return 0;
}

SEXP Rsockopen(SEXP sport)
{
  return 0;
}

SEXP Rsocklisten(SEXP ssock)
{
  return 0;
}

SEXP Rsockwrite(SEXP ssock, SEXP sstring)
{
  return 0;
}


attribute_hidden
int Rsockselect(int nsock, int *insockfd, int *ready, int *write,
		double timeout)
{
  return 0;
}
