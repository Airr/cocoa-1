/*
 *  R : A Computer Langage for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 2001-4  The R Core Team
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

#include "Defn.h"


/*  mkPRIMSXP - return a builtin function      */
/*              either "builtin" or "special"  */

/*  The value produced is cached do avoid the need for GC protection
    in cases where a .Primitive is produced by unserializing or
    reconstructed after a package has clobbered the value assigned to
    a symbol in the base package. */

SEXP attribute_hidden mkPRIMSXP(int offset, int eval)
{
  return 0;
}

/* This is called by function() {}, where an invalid
 body should be impossible. When called from
 other places (eg do_asfunction) they
 should do this checking in advance */

/*  mkCLOSXP - return a closure with formals f,  */
/*             body b, and environment rho       */
SEXP attribute_hidden mkCLOSXP(SEXP formals, SEXP body, SEXP rho)
{
  return 0;
}

/* mkChar - make a character (CHARSXP) variable -- see Rinlinedfuns.h */

/*  mkSYMSXP - return a symsxp with the string  */
/*             name inserted in the name field  */

static int isDDName(SEXP name)
{
  return 0;
}

SEXP attribute_hidden mkSYMSXP(SEXP name, SEXP value)

{
  return 0;
}
