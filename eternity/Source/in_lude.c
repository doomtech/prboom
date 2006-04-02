// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2002 James Haley
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//--------------------------------------------------------------------------
//
// DESCRIPTION:
// 
// Shared intermission code
//
// haleyjd: This code has been moved here from wi_stuff.c to
// provide a consistent interface to intermission code and to
// enable better code reuse.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "doomstat.h"
#include "m_random.h"
#include "p_tick.h"
#include "in_lude.h"
#include "v_video.h"
#include "r_main.h"
#include "s_sound.h"
#include "d_gi.h"
#include "p_enemy.h"
#include "e_edf.h"

// Globals

// used for timing
int intertime;

// used to accelerate or skip a stage
int acceleratestage; // killough 3/28/98: made global

static giinterfuncs_t *InterFuncs = NULL;

//
// Intermission Camera
//

// is this just some boring picture, or a view of the level?
static int realbackdrop = 1;
camera_t   intercam;

MobjCollection_t camerathings;
mobj_t *wi_camera;

//
// IN_AddCameras
//
// haleyjd: new solution for intermission cameras. Uses
// P_CollectThings to remove limit, and actually uses spawned
// things of type 5003 rather than just storing the mapthing_t's.
//
void IN_AddCameras(void)
{
   int cameratype = E_ThingNumForDEHNum(MT_CAMERA);

   P_ReInitMobjCollection(&camerathings, cameratype);

   if(cameratype == NUMMOBJTYPES || demo_version < 331)
      return;
   
   P_CollectThings(&camerathings);
}

// set up the intermissions camera

void IN_StartCamera(void)
{
   int i;
   
   if(!P_CollectionIsEmpty(&camerathings))
   {
      realbackdrop = 1;

      // pick a camera at random
      wi_camera = P_CollectionGetRandom(&camerathings, pr_misc);
      
      // centre the view
      players[displayplayer].updownangle = 0;
      
      // remove the player mobjs (look silly in camera view)
      for(i=0; i<MAXPLAYERS; i++)
      {
         if(!playeringame[i]) continue;
         // this is strange. the monsters can still
         // see the player Mobj, (and even kill it!)
         // even tho it has been removed from the
         // level. I make it unshootable first so
         // they lose interest.
         players[i].mo->flags &= ~MF_SHOOTABLE;
         P_RemoveMobj(players[i].mo);
      }
            
      intercam.x = wi_camera->x;
      intercam.y = wi_camera->y;
      intercam.angle = wi_camera->angle;
      intercam.updownangle = 0;
      {
         // haleyjd: camera deep water HOM bug fix
         subsector_t *subsec =
            R_PointInSubsector(intercam.x, intercam.y);
         
         intercam.z = subsec->sector->floorheight + 41*FRACUNIT;
         intercam.heightsec = subsec->sector->heightsec;
      }
      R_SetViewSize(11);     // force fullscreen
   }
   else            // no camera, boring interpic
   {
      realbackdrop = 0;
      wi_camera = NULL;
   }
}

// ====================================================================
// IN_slamBackground
// Purpose: Put the full-screen background up prior to patches
// Args:    none
// Returns: void
//
void IN_slamBackground(void)
{
   if(realbackdrop)
      R_RenderPlayerView(players+displayplayer, &intercam);
   else
      V_CopyRect(0, 0, 1, SCREENWIDTH, SCREENHEIGHT, 0, 0, 0);  // killough 11/98
}

// ====================================================================
// IN_checkForAccelerate
// Purpose: See if the player has hit either the attack or use key
//          or mouse button.  If so we set acceleratestage to 1 and
//          all those display routines above jump right to the end.
// Args:    none
// Returns: void
//
void IN_checkForAccelerate(void)
{
   int   i;
   player_t  *player;
   
   // check for button presses to skip delays
   for(i = 0, player = players ; i < MAXPLAYERS ; i++, player++)
   {
      if(playeringame[i])
      {
         if(player->cmd.buttons & BT_ATTACK)
         {
            if(!player->attackdown)
               acceleratestage = 1;
            player->attackdown = true;
         }
         else
            player->attackdown = false;
         
         if (player->cmd.buttons & BT_USE)
         {
            if(!player->usedown)
               acceleratestage = 1;
            player->usedown = true;
         }
         else
            player->usedown = false;
      }
   }
}

void IN_Ticker(void)
{
   // counter for general background animation
   intertime++;  
   
   DEBUGMSG("in_ticker\n");

   // intermission music
   if(intertime == 1)
      S_ChangeMusicNum(gameModeInfo->interMusNum, true);

   IN_checkForAccelerate();

   InterFuncs->Ticker();

   // keep the level running when using an intermission camera
   if(realbackdrop)
      P_Ticker();
}

void IN_Drawer(void)
{
   InterFuncs->Drawer();
}

void IN_DrawBackground(void)
{
   gameModeInfo->interfuncs->DrawBackground();
}

void IN_Start(wbstartstruct_t *wbstartstruct)
{
   IN_StartCamera();  //set up camera

   InterFuncs = gameModeInfo->interfuncs;

   InterFuncs->Start(wbstartstruct);
}

// EOF

