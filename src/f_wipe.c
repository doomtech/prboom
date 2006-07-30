/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Mission begin melt/wipe screen special effect.
 *
 *-----------------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "z_zone.h"
#include "doomdef.h"
#include "i_video.h"
#include "v_video.h"
#include "m_random.h"
#include "f_wipe.h"

//
// SCREEN WIPE PACKAGE
//

// CPhipps - macros for the source and destination screens
#define SRC_SCR 2
#define DEST_SCR 3

static screeninfo_t wipe_scr_start;
static screeninfo_t wipe_scr_end;
static screeninfo_t wipe_scr;

static void wipe_shittyColMajorXform(short *array, int width, int height)
{
  short *dest = Z_Malloc(width*height*sizeof(short), PU_STATIC, 0);
  int x, y;

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
      dest[x*height+y] = array[y*width+x];
  memcpy(array, dest, width*height*sizeof(short));
  Z_Free(dest);
}

static int *y;

static int wipe_initMelt(int width, int height, int ticks)
{
  int i;

  // copy start screen to main screen
  memcpy(wipe_scr.data, wipe_scr_start.data, width*height);

  // makes this wipe faster (in theory)
  // to have stuff in column-major format
  wipe_shittyColMajorXform((short*)wipe_scr_start.data, width/2, height);
  wipe_shittyColMajorXform((short*)wipe_scr_end.data, width/2, height);

  // setup initial column positions (y<0 => not ready to scroll yet)
  y = (int *) malloc(width*sizeof(int));
  y[0] = -(M_Random()%16);
  for (i=1;i<width;i++)
    {
      int r = (M_Random()%3) - 1;
      y[i] = y[i-1] + r;
      if (y[i] > 0)
        y[i] = 0;
      else
        if (y[i] == -16)
          y[i] = -15;
    }
  return 0;
}

static int wipe_doMelt(int width, int height, int ticks)
{
  boolean done = true;
  int i;

  width /= 2;

  while (ticks--)
    for (i=0;i<width;i++)
      if (y[i]<0)
        {
          y[i]++;
          done = false;
        }
      else
        if (y[i] < height)
          {
            short *s, *d;
            int j, dy, idx;

            /* cph 2001/07/29 -
             *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
             *  the whole screen, so make the melt rate depend on SCREENHEIGHT
             *  so it takes no longer in high res
             */
            dy = (y[i] < 16) ? y[i]+1 : SCREENHEIGHT/25;
            if (y[i]+dy >= height)
              dy = height - y[i];
            s = &((short *)wipe_scr_end.data)[i*height+y[i]];
            d = &((short *)wipe_scr.data)[y[i]*width+i];
            idx = 0;
            for (j=dy;j;j--)
              {
                d[idx] = *(s++);
                idx += width;
              }
            y[i] += dy;
            s = &((short *)wipe_scr_start.data)[i*height];
            d = &((short *)wipe_scr.data)[y[i]*width+i];
            idx = 0;
            for (j=height-y[i];j;j--)
              {
                d[idx] = *(s++);
                idx += width;
              }
            done = false;
          }
  return done;
}

// CPhipps - modified to allocate and deallocate screens[2 to 3] as needed, saving memory

static int wipe_exitMelt(int width, int height, int ticks)
{
  free(y);
  V_FreeScreen(&wipe_scr_start);
  wipe_scr_start.width = 0;
  wipe_scr_start.height = 0;
  V_FreeScreen(&wipe_scr_end);
  wipe_scr_end.width = 0;
  wipe_scr_end.height = 0;
  // Paranoia
  y = NULL;
  screens[SRC_SCR] = wipe_scr_start;
  screens[DEST_SCR] = wipe_scr_end;
  return 0;
}

int wipe_StartScreen(int x, int y, int width, int height)
{
  wipe_scr_start.width = SCREENWIDTH;
  wipe_scr_start.height = SCREENHEIGHT;
  wipe_scr_start.not_on_heap = false;
  V_AllocScreen(&wipe_scr_start);
  screens[SRC_SCR] = wipe_scr_start;
  V_CopyRect(x, y, 0,       width, height, x, y, SRC_SCR, VPT_NONE ); // Copy start screen to buffer
  return 0;
}

int wipe_EndScreen(int x, int y, int width, int height)
{
  wipe_scr_end.width = SCREENWIDTH;
  wipe_scr_end.height = SCREENHEIGHT;
  wipe_scr_end.not_on_heap = false;
  V_AllocScreen(&wipe_scr_end);
  screens[DEST_SCR] = wipe_scr_end;
  V_CopyRect(x, y, 0,       width, height, x, y, DEST_SCR, VPT_NONE); // Copy end screen to buffer
  V_CopyRect(x, y, SRC_SCR, width, height, x, y, 0       , VPT_NONE); // restore start screen
  return 0;
}

// killough 3/5/98: reformatted and cleaned up
int wipe_ScreenWipe(int x, int y, int width, int height, int ticks)
{
  static boolean go;                               // when zero, stop the wipe
  if (!go)                                         // initial stuff
    {
      go = 1;
      wipe_scr = screens[0];
      wipe_initMelt(width, height, ticks);
    }
  // do a piece of wipe-in
  if (wipe_doMelt(width, height, ticks))     // final stuff
    {
      wipe_exitMelt(width, height, ticks);
      go = 0;
    }
  return !go;
}
