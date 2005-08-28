/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
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
 *      Renderer main interface.
 *
 *-----------------------------------------------------------------------------*/

#ifndef R_MAIN_H
#define R_MAIN_H

#include "d_player.h"
#include "r_data.h"
#include "p_chase.h"

#ifdef __GNUG__
#pragma interface
#endif

extern int screenblocks;

//
// POV related.
//

extern fixed_t  viewcos;
extern fixed_t  viewsin;
extern int      viewwidth;
extern int      viewheight;
extern int      viewwindowx;
extern int      viewwindowy;
extern int      centerx;
extern int      centery;
extern fixed_t  centerxfrac;
extern fixed_t  centeryfrac;
extern fixed_t  projection;
// proff 11/06/98: Added for high-res
extern fixed_t  projectiony;
extern int      validcount;
extern int      linecount;
extern int      loopcount;
extern int      viewheightsec;
extern camera_t *viewcamera;

//
// Rendering stats
//

extern int rendered_visplanes, rendered_segs, rendered_vissprites;
extern boolean rendering_stats;

//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.

#define LIGHTLEVELS       16
#define LIGHTSEGSHIFT      4
#define MAXLIGHTSCALE     48
#define LIGHTSCALESHIFT   12
#define MAXLIGHTZ        128
#define LIGHTZSHIFT       20

// killough 3/20/98: Allow colormaps to be dynamic (e.g. underwater)
extern lighttable_t *(*zlight)[MAXLIGHTZ];
extern lighttable_t *fullcolormap;
extern int numcolormaps;    // killough 4/4/98: dynamic number of maps
extern lighttable_t **colormaps;
// killough 3/20/98, 4/4/98: end dynamic colormaps

extern int          extralight;
extern lighttable_t *fixedcolormap;

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.

#define NUMCOLORMAPS 32

//
// Function pointer to switch refresh/drawing functions.
//

extern void (*colfunc)(); // Removed void parameter - POPE

//
// Utility functions.
//

CONSTFUNC int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node);
CONSTFUNC int R_PointOnSegSide(fixed_t x, fixed_t y, const seg_t *line);
angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);

//
// REFRESH - the actual rendering functions.
//

void R_RenderPlayerView(player_t *player, camera_t* viewcamera);
void R_InitColFunc();
void R_Init(void);                           // Called by startup code.
void R_SetViewSize(int blocks);              // Called by M_Responder.
void R_ExecuteSetViewSize(void);             // cph - called by D_Display to complete a view resize
angle_t R_WadToAngle(int wadangle);

#endif // R_MAIN_H
