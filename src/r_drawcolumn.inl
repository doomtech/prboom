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
 *-----------------------------------------------------------------------------*/


#if ((R_DRAWCOLUMN_PIPELINE & RDC_STANDARD) || (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLATED))

#if (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLATED)
#define GETCOL8_MAPPED(col) (dcvars.translation[(col)])
#else
#define GETCOL8_MAPPED(col) (col)
#endif
void R_DRAWCOLUMN_FUNCNAME(void)
{
  int              count;
  register byte    *dest;            // killough
  register fixed_t frac;            // killough

  // leban 1/17/99:
  // removed the + 1 here, adjusted the if test, and added an increment
  // later.  this helps a compiler pipeline a bit better.  the x86
  // assembler also does this.

  count = dcvars.yh - dcvars.yl;

  // leban 1/17/99:
  // this case isn't executed too often.  depending on how many instructions
  // there are between here and the second if test below, this case could
  // be moved down and might save instructions overall.  since there are
  // probably different wads that favor one way or the other, i'll leave
  // this alone for now.
  if (count < 0)    // Zero length, column does not exceed a pixel.
    return;

  count++;

#ifdef RANGECHECK
  if ((unsigned)dcvars.x >= (unsigned)SCREENWIDTH
      || dcvars.yl < 0
      || dcvars.yh >= SCREENHEIGHT)
    I_Error("R_DrawColumn: %i to %i at %i", dcvars.yl, dcvars.yh, dcvars.x);
#endif

  // Framebuffer destination address.
   // SoM: MAGIC
   {
      // haleyjd: reordered predicates
      if(temp_x == 4 ||
         (temp_x && (temptype != COL_OPAQUE || temp_x + startx != dcvars.x)))
         R_FlushColumns();

      if(!temp_x)
      {
         ++temp_x;
         startx = dcvars.x;
         *tempyl = commontop = dcvars.yl;
         *tempyh = commonbot = dcvars.yh;
         temptype = COL_OPAQUE;
         R_FlushWholeColumns = R_FlushWholeOpaque;
         R_FlushHTColumns    = R_FlushHTOpaque;
         R_FlushQuadColumn   = R_FlushQuadOpaque;
         dest = &tempbuf[dcvars.yl << 2];
      } else {
         tempyl[temp_x] = dcvars.yl;
         tempyh[temp_x] = dcvars.yh;
   
         if(dcvars.yl > commontop)
            commontop = dcvars.yl;
         if(dcvars.yh < commonbot)
            commonbot = dcvars.yh;
      
         dest = &tempbuf[(dcvars.yl << 2) + temp_x++];
      }
   }

  // Determine scaling, which is the only mapping to be done.
#define  fracstep dcvars.iscale
  frac = dcvars.texturemid + (dcvars.yl-centery)*fracstep;

  // Inner loop that does the actual texture mapping,
  //  e.g. a DDA-lile scaling.
  // This is as fast as it gets.       (Yeah, right!!! -- killough)
  //
  // killough 2/1/98: more performance tuning

    if (dcvars.texheight == 128) {
        while(count--)
        {
                *dest = dcvars.colormap[GETCOL8_MAPPED(dcvars.source[(frac>>FRACBITS)&127])];
                dest += 4;
                frac += fracstep;
        }
    } else if (dcvars.texheight == 0) {
  /* cph - another special case */
  while (count--) {
    *dest = dcvars.colormap[GETCOL8_MAPPED(dcvars.source[frac>>FRACBITS])];
    dest += 4;
    frac += fracstep;
  }
    } else {
     register unsigned heightmask = dcvars.texheight-1; // CPhipps - specify type
     if (! (dcvars.texheight & heightmask) )   // power of 2 -- killough
     {
         while (count>0)   // texture height is a power of 2 -- killough
           {
             *dest = dcvars.colormap[GETCOL8_MAPPED(dcvars.source[(frac>>FRACBITS) & heightmask])];
             dest += 4;
             frac += fracstep;
            count--;
           }
     }
     else
     {
         heightmask++;
         heightmask <<= FRACBITS;

         if (frac < 0)
           while ((frac += heightmask) <  0);
         else
           while (frac >= (int)heightmask)
             frac -= heightmask;

         while(count>0)
           {
             // Re-map color indices from wall texture column
             //  using a lighting/special effects LUT.

             // heightmask is the Tutti-Frutti fix -- killough

             *dest = dcvars.colormap[GETCOL8_MAPPED(dcvars.source[frac>>FRACBITS])];
             dest += 4;
             if ((frac += fracstep) >= (int)heightmask)
               frac -= heightmask;
            count--;
           }
     }
    }
}
#undef fracstep
#endif

// Here is the version of R_DrawColumn that deals with translucent  // phares
// textures and sprites. It's identical to R_DrawColumn except      //    |
// for the spot where the color index is stuffed into *dest. At     //    V
// that point, the existing color index and the new color index
// are mapped through the TRANMAP lump filters to get a new color
// index whose RGB values are the average of the existing and new
// colors.
//
// Since we're concerned about performance, the 'translucent or
// opaque' decision is made outside this routine, not down where the
// actual code differences are.
#if (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLUCENT)
void R_DRAWCOLUMN_FUNCNAME(void)
{
  int              count;
  register byte    *dest;           // killough
  register fixed_t frac;            // killough

  count = dcvars.yh - dcvars.yl + 1;

  // Zero length, column does not exceed a pixel.
  if (count <= 0)
    return;

#ifdef RANGECHECK
  if ((unsigned)dcvars.x >= (unsigned)SCREENWIDTH
      || dcvars.yl < 0
      || dcvars.yh >= SCREENHEIGHT)
    I_Error("R_DrawTLColumn: %i to %i at %i", dcvars.yl, dcvars.yh, dcvars.x);
#endif

  // Framebuffer destination address.
  // SoM: MAGIC
   {
      // haleyjd: reordered predicates
      if(temp_x == 4 || tranmap != temptranmap ||
         (temp_x && (temptype != COL_TRANS || temp_x + startx != dcvars.x)))
         R_FlushColumns();

      if(!temp_x)
      {
         ++temp_x;
         startx = dcvars.x;
         *tempyl = commontop = dcvars.yl;
         *tempyh = commonbot = dcvars.yh;
         temptype = COL_TRANS;
         temptranmap = tranmap;
         R_FlushWholeColumns = R_FlushWholeTL;
         R_FlushHTColumns    = R_FlushHTTL;
         R_FlushQuadColumn   = R_FlushQuadTL;
         dest = &tempbuf[dcvars.yl << 2];
      } else {
         tempyl[temp_x] = dcvars.yl;
         tempyh[temp_x] = dcvars.yh;
   
         if(dcvars.yl > commontop)
            commontop = dcvars.yl;
         if(dcvars.yh < commonbot)
            commonbot = dcvars.yh;
      
         dest = &tempbuf[(dcvars.yl << 2) + temp_x++];
      }
   }

  // Determine scaling,
  //  which is the only mapping to be done.
#define  fracstep dcvars.iscale
  frac = dcvars.texturemid + (dcvars.yl-centery)*fracstep;

  // Inner loop that does the actual texture mapping,
  //  e.g. a DDA-lile scaling.
  // This is as fast as it gets.       (Yeah, right!!! -- killough)
  //
  // killough 2/1/98, 2/21/98: more performance tuning

  {
    register const byte *source = dcvars.source;
    register const lighttable_t *colormap = dcvars.colormap;
    register unsigned heightmask = dcvars.texheight-1; // CPhipps - specify type
    if (dcvars.texheight & heightmask)   // not a power of 2 -- killough
      {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
          while ((frac += heightmask) <  0);
        else
          while (frac >= (int)heightmask)
            frac -= heightmask;

        do
          {
            // Re-map color indices from wall texture column
            //  using a lighting/special effects LUT.

            // heightmask is the Tutti-Frutti fix -- killough

            *dest = tranmap[(*dest<<8)+colormap[source[frac>>FRACBITS]]]; // phares
            dest += 4;
            if ((frac += fracstep) >= (int)heightmask)
              frac -= heightmask;
          }
        while (--count);
      }
    else
      {
	if (heightmask == -1 && frac < 0) frac = 0;
        while ((count-=2)>=0)   // texture height is a power of 2 -- killough
          {
            *dest = tranmap[(*dest<<8)+colormap[source[(frac>>FRACBITS) & heightmask]]]; // phares
            dest += 4;
            frac += fracstep;
            *dest = tranmap[(*dest<<8)+colormap[source[(frac>>FRACBITS) & heightmask]]]; // phares
            dest += 4;
            frac += fracstep;
          }
        if (count & 1)
          *dest = tranmap[(*dest<<8)+colormap[source[(frac>>FRACBITS) & heightmask]]]; // phares
      }
  }
}
#undef fracstep
#endif

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//
#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
void R_DRAWCOLUMN_FUNCNAME(void)
{
  int      count;

  // Adjust borders. Low...
  if (!dcvars.yl)
    dcvars.yl = 1;

  // .. and high.
  if (dcvars.yh == viewheight-1)
    dcvars.yh = viewheight - 2;

  count = dcvars.yh - dcvars.yl;

  // Zero length.
  if (count < 0)
    return;

#ifdef RANGECHECK
  if ((unsigned) dcvars.x >= (unsigned)SCREENWIDTH
      || dcvars.yl < 0
      || (unsigned)dcvars.yh >= (unsigned)SCREENHEIGHT)
    I_Error("R_DrawFuzzColumn: %i to %i at %i", dcvars.yl, dcvars.yh, dcvars.x);
#endif

   // SoM: MAGIC
   {
      // haleyjd: reordered predicates
      if(temp_x == 4 ||
         (temp_x && (temptype != COL_FUZZ || temp_x + startx != dcvars.x)))
         R_FlushColumns();

      if(!temp_x)
      {
         ++temp_x;
         startx = dcvars.x;
         *tempyl = commontop = dcvars.yl;
         *tempyh = commonbot = dcvars.yh;
         temptype = COL_FUZZ;
         tempfuzzmap = fullcolormap; // SoM 7-28-04: Fix the fuzz problem.
         R_FlushWholeColumns = R_FlushWholeFuzz;
         R_FlushHTColumns    = R_FlushHTFuzz;
         R_FlushQuadColumn   = R_FlushQuadFuzz;
      } else {
         tempyl[temp_x] = dcvars.yl;
         tempyh[temp_x] = dcvars.yh;
   
         if(dcvars.yl > commontop)
            commontop = dcvars.yl;
         if(dcvars.yh < commonbot)
            commonbot = dcvars.yh;
      }
   }
}
#endif

#undef GETCOL8_MAPPED

#undef R_DRAWCOLUMN_FUNCNAME
#undef R_DRAWCOLUMN_PIPELINE
