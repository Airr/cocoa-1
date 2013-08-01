/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2008  The R Core Team
 *  Copyright (C) 2002--2005  The R Foundation
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


 *  This is an extensive reworking by Paul Murrell of an original
 *  quick hack by Ross Ihaka designed to give a superset of the
 *  functionality in the AT&T Bell Laboratories GRZ library.
 *
 */

/* This should be regarded as part of the graphics engine */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>
#include <Graphics.h>
#include <GraphicsBase.h> 
#include <R_ext/GraphicsEngine.h>

int baseRegisterIndex = -1;

GPar* dpptr(pGEDevDesc dd) {
  return 0;
}

static SEXP R_INLINE getSymbolValue(SEXP symbol)
{
  return 0;
}

/*
 *  DEVICE FUNCTIONS
 *
 *  R allows there to be (up to 64) multiple devices in
 *  existence at the same time.	 Only one device is the
 *  active device and all drawing occurs in this device
 *
 *  Each device has its own set of graphics parameters
 *  so that switching between devices, switches between
 *  their graphical contexts (e.g., if you set the line
 *  width on one device then switch to another device,
 *  don't expect to be using the line width you just set!)
 *
 *  Each device has additional device-specific graphics
 *  parameters which the device driver (i.e., NOT this
 *  generic graphics code) is wholly responsible for
 *  maintaining (including creating and destroying special
 *  resources such as X11 windows).
 *
 *  Each device has a display list which records every
 *  graphical operation since the last dpptr(dd)->newPage;
 *  this is used to redraw the output on the device
 *  when it is resized and to copy output from one device
 *  to another (this can be disabled, which is the default
 *  for postscript).
 *
 *  NOTE: that graphical operations should only be
 *  recorded in the displayList if they are "guaranteed"
 *  to succeed (to avoid heaps of error messages on a
 *  redraw) which means that the recording should be the
 *  last thing done in a graphical operation (see do_*
 *  in plot.c).
 *
 */

static int R_CurrentDevice = 0;
static int R_NumDevices = 1;
/*
   R_MaxDevices is defined in Rgraphics.h to be 64.  Slots are
   initiialized to be NULL, and returned to NULL when a device is
   removed.

   Slot 0 is the null device, and slot 63 is keep empty as a sentinel
   for over-allocation: if a driver fails to call
   R_CheckDeviceAvailable and uses this slot the device it allocated
   will be killed.

   'active' means has been successfully opened and is not in the
   process of being closed and destroyed.  We do this to allow for GUI
   callbacks starting to kill a device whilst another is being killed.
 */
static pGEDevDesc R_Devices[R_MaxDevices];
static Rboolean active[R_MaxDevices];

/* a dummy description to point to when there are no active devices */

static GEDevDesc nullDevice;

/* In many cases this is used to mean that the current device is
   the null device, and in others to mean that there is no open device.
   The two condiions are currently the same, as no way is provided to
   select the null device (selectDevice(0) immediately opens a device).

   But watch out if you intend to change the logic of any of this.
*/

/* Used in grid */
int NoDevices(void)
{
  return 0;
}

int NumDevices(void)
{
  return 0;
}

pGEDevDesc GEcurrentDevice(void)
{
  return 0;
}

pGEDevDesc GEgetDevice(int i)
{
  return 0;
}

int curDevice(void)
{
  return 0;
}


int nextDevice(int from)
{
  return 0;
}

int prevDevice(int from)
{
  return 0;
}

/* This should be called if you have a pointer to a GEDevDesc
 * and you want to find the corresponding device number
 */

int GEdeviceNumber(pGEDevDesc dd)
{
  return 0;
}

/* This should be called if you have a pointer to a DevDesc
 * and you want to find the corresponding device number
 */
int ndevNumber(pDevDesc dd)
{
  return 0;
}

int selectDevice(int devNum)
{
  return 0;
}

/* historically the close was in the [kK]illDevices.
   only use findNext = FALSE when shutting R dowm, and .Device[s] are not
   updated.
*/
static
void removeDevice(int devNum, Rboolean findNext)
{
  return;
}

void GEkillDevice(pGEDevDesc gdd)
{
  return;
}

void killDevice(int devNum)
{
  return;
}


/* Used by front-ends via R_CleanUp to shutdown all graphics devices
   at the end of a session. Not the same as graphics.off(), and leaves
   .Devices and .Device in an invalid state. */
void KillAllDevices(void)
{
  return;
}

/* A common construction in some graphics devices */
pGEDevDesc desc2GEDesc(pDevDesc dd)
{
  return 0;
}

/* ------- interface for creating devices ---------- */

void R_CheckDeviceAvailable(void)
{
  return;
}

Rboolean R_CheckDeviceAvailableBool(void)
{
  return 0;
}

void GEaddDevice(pGEDevDesc gdd)
{
  return;
}

/* convenience wrapper */
void GEaddDevice2(pGEDevDesc gdd, const char *name)
{
  return;
}

Rboolean Rf_GetOptionDeviceAsk(void); /* from options.c */

/* Create a GEDevDesc, given a pDevDesc
 */
pGEDevDesc GEcreateDevDesc(pDevDesc dev)
{
  return 0;
}


void attribute_hidden InitGraphics(void)
{
  return;
}


void NewFrameConfirm(pDevDesc dd)
{
  return;
}
