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

//
// R_FlushWholeOpaque
//
// Flushes the entire columns in the buffer, one at a time.
// This is used when a quad flush isn't possible.
// Opaque version -- no remapping whatsoever.
//
static void R_FlushWholeOpaque(void)
{
   register byte *source;
   register byte *dest;
   register int  count, yl;

   while(--temp_x >= 0)
   {
      yl     = tempyl[temp_x];
      source = &tempbuf[temp_x + (yl << 2)];
      dest   = topleft + yl*screens[0].pitch + startx + temp_x;
      count  = tempyh[temp_x] - yl + 1;
      
      while(--count >= 0)
      {
         *dest = *source;
         source += 4;
         dest += screens[0].pitch;
      }
   }
}

//
// R_FlushHTOpaque
//
// Flushes the head and tail of columns in the buffer in
// preparation for a quad flush.
// Opaque version -- no remapping whatsoever.
//
static void R_FlushHTOpaque(void)
{
   register byte *source;
   register byte *dest;
   register int count, colnum = 0;
   int yl, yh;

   while(colnum < 4)
   {
      yl = tempyl[colnum];
      yh = tempyh[colnum];
      
      // flush column head
      if(yl < commontop)
      {
         source = &tempbuf[colnum + (yl << 2)];
         dest   = topleft + yl*screens[0].pitch + startx + colnum;
         count  = commontop - yl;
         
         while(--count >= 0)
         {
            *dest = *source;
            source += 4;
            dest += screens[0].pitch;
         }
      }
      
      // flush column tail
      if(yh > commonbot)
      {
         source = &tempbuf[colnum + ((commonbot + 1) << 2)];
         dest   = topleft + (commonbot + 1)*screens[0].pitch + startx + colnum;
         count  = yh - commonbot;
         
         while(--count >= 0)
         {
            *dest = *source;
            source += 4;
            dest += screens[0].pitch;
         }
      }         
      ++colnum;
   }
}

static void R_FlushWholeTL(void)
{
   register byte *source;
   register byte *dest;
   register int  count, yl;

   while(--temp_x >= 0)
   {
      yl     = tempyl[temp_x];
      source = &tempbuf[temp_x + (yl << 2)];
      dest   = topleft + yl*screens[0].pitch + startx + temp_x;
      count  = tempyh[temp_x] - yl + 1;

      while(--count >= 0)
      {
         // haleyjd 09/11/04: use temptranmap here
         *dest = temptranmap[(*dest<<8) + *source];
         source += 4;
         dest += screens[0].pitch;
      }
   }
}

static void R_FlushHTTL(void)
{
   register byte *source;
   register byte *dest;
   register int count;
   int colnum = 0, yl, yh;

   while(colnum < 4)
   {
      yl = tempyl[colnum];
      yh = tempyh[colnum];

      // flush column head
      if(yl < commontop)
      {
         source = &tempbuf[colnum + (yl << 2)];
         dest   = topleft + yl*screens[0].pitch + startx + colnum;
         count  = commontop - yl;

         while(--count >= 0)
         {
            // haleyjd 09/11/04: use temptranmap here
            *dest = temptranmap[(*dest<<8) + *source];
            source += 4;
            dest += screens[0].pitch;
         }
      }

      // flush column tail
      if(yh > commonbot)
      {
         source = &tempbuf[colnum + ((commonbot + 1) << 2)];
         dest   = topleft + (commonbot + 1)*screens[0].pitch + startx + colnum;
         count  = yh - commonbot;

         while(--count >= 0)
         {
            // haleyjd 09/11/04: use temptranmap here
            *dest = temptranmap[(*dest<<8) + *source];
            source += 4;
            dest += screens[0].pitch;
         }
      }
      
      ++colnum;
   }
}

static void R_FlushWholeFuzz(void)
{
   register byte *source;
   register byte *dest;
   register int  count, yl;

   while(--temp_x >= 0)
   {
      yl     = tempyl[temp_x];
      source = &tempbuf[temp_x + (yl << 2)];
      dest   = topleft + yl*screens[0].pitch + startx + temp_x;
      count  = tempyh[temp_x] - yl + 1;

      while(--count >= 0)
      {
         // SoM 7-28-04: Fix the fuzz problem.
         *dest = tempfuzzmap[6*256+dest[fuzzoffset[fuzzpos]]];
         
         // Clamp table lookup index.
         if(++fuzzpos == FUZZTABLE) 
            fuzzpos = 0;
         
         source += 4;
         dest += screens[0].pitch;
      }
   }
}

static void R_FlushHTFuzz(void)
{
   register byte *source;
   register byte *dest;
   register int count;
   int colnum = 0, yl, yh;

   while(colnum < 4)
   {
      yl = tempyl[colnum];
      yh = tempyh[colnum];

      // flush column head
      if(yl < commontop)
      {
         source = &tempbuf[colnum + (yl << 2)];
         dest   = topleft + yl*screens[0].pitch + startx + colnum;
         count  = commontop - yl;

         while(--count >= 0)
         {
            // SoM 7-28-04: Fix the fuzz problem.
            *dest = tempfuzzmap[6*256+dest[fuzzoffset[fuzzpos]]];
            
            // Clamp table lookup index.
            if(++fuzzpos == FUZZTABLE) 
               fuzzpos = 0;
            
            source += 4;
            dest += screens[0].pitch;
         }
      }

      // flush column tail
      if(yh > commonbot)
      {
         source = &tempbuf[colnum + ((commonbot + 1) << 2)];
         dest   = topleft + (commonbot + 1)*screens[0].pitch + startx + colnum;
         count  = yh - commonbot;

         while(--count >= 0)
         {
            // SoM 7-28-04: Fix the fuzz problem.
            *dest = tempfuzzmap[6*256+dest[fuzzoffset[fuzzpos]]];
            
            // Clamp table lookup index.
            if(++fuzzpos == FUZZTABLE) 
               fuzzpos = 0;
            
            source += 4;
            dest += screens[0].pitch;
         }
      }
      
      ++colnum;
   }
}

// Begin: Quad column flushing functions.
static void R_FlushQuadOpaque(void)
{
   register int *source = (int *)(&tempbuf[commontop << 2]);
   register int *dest = (int *)(topleft + commontop*screens[0].pitch + startx);
   register int count;
   register int deststep = screens[0].pitch / 4;

   count = commonbot - commontop + 1;

   while(--count >= 0)
   {
      *dest = *source++;
      dest += deststep;
   }
}

static void R_FlushQuadTL(void)
{
   register byte *source = &tempbuf[commontop << 2];
   register byte *dest = topleft + commontop*screens[0].pitch + startx;
   register int count;

   count = commonbot - commontop + 1;

   while(--count >= 0)
   {
      *dest   = temptranmap[(*dest<<8) + *source];
      dest[1] = temptranmap[(dest[1]<<8) + source[1]];
      dest[2] = temptranmap[(dest[2]<<8) + source[2]];
      dest[3] = temptranmap[(dest[3]<<8) + source[3]];
      source += 4;
      dest += screens[0].pitch;
   }
}

static void R_FlushQuadFuzz(void)
{
   register byte *source = &tempbuf[commontop << 2];
   register byte *dest = topleft + commontop*screens[0].pitch + startx;
   register int count;
   int fuzz1, fuzz2, fuzz3, fuzz4;
   fuzz1 = fuzzpos;
   fuzz2 = (fuzz1 + tempyl[1]) % FUZZTABLE;
   fuzz3 = (fuzz2 + tempyl[2]) % FUZZTABLE;
   fuzz4 = (fuzz3 + tempyl[3]) % FUZZTABLE;

   count = commonbot - commontop + 1;

   while(--count >= 0)
   {
      // SoM 7-28-04: Fix the fuzz problem.
      dest[0] = tempfuzzmap[6*256+dest[0 + fuzzoffset[fuzz1]]];
      if(++fuzz1 == FUZZTABLE) fuzz1 = 0;
      dest[1] = tempfuzzmap[6*256+dest[1 + fuzzoffset[fuzz2]]];
      if(++fuzz2 == FUZZTABLE) fuzz2 = 0;
      dest[2] = tempfuzzmap[6*256+dest[2 + fuzzoffset[fuzz3]]];
      if(++fuzz3 == FUZZTABLE) fuzz3 = 0;
      dest[3] = tempfuzzmap[6*256+dest[3 + fuzzoffset[fuzz4]]];
      if(++fuzz4 == FUZZTABLE) fuzz4 = 0;

      source += 4;
      dest += screens[0].pitch;
   }

   fuzzpos = fuzz4;
}
