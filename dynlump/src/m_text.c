#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "w_wad.h"
#include "z_zone.h"

#define M_DYN_PATCH_W 96
#define M_DYN_PATCH_H 15
#define M_DYN_PATCH_LO 0 /* left offset */
#define M_DYN_PATCH_TO 0 /* top offset */

#define PATCH_HEADER_SIZE   8
#define PATCH_POINTERS_SIZE (M_DYN_PATCH_W*4)
#define PATCH_COLUMN_SIZE   (1 /* row offset  */ + \
                            1 /* num. pixels */ + \
                            M_DYN_PATCH_H /* pixel data */ + \
                            2 /* junk/padding */ + \
                            1 /* terminating 0xff */)

#define BACKGROUND_COLOR 47 /* a dark red */

void M_InitDynLump() {
    int index;

    index = numlumps;
    numlumps++;
    lumpinfo = realloc(lumpinfo, numlumps*sizeof(lumpinfo_t));

    strncpy(lumpinfo[index].name, "M_GENERL", 9);
    lumpinfo[index].source = source_dyn;
    lumpinfo[index].wadfile = NULL;
    lumpinfo[index].li_namespace = ns_global;
    /* assume that a graphic lump is WxH bytes in size */
    /* I don't have a copy of uds.txt with me right now */
    lumpinfo[index].size =
        PATCH_HEADER_SIZE +
        PATCH_POINTERS_SIZE +
        M_DYN_PATCH_W * PATCH_COLUMN_SIZE;
}

const void* W_CacheLumpNumDyn(int lump) { /* choose a better name */
    const char* foo;
    unsigned short *sp;
    unsigned long *lp;
    unsigned char *cp;
    int i;

    /* TODO: change ot PU_CACHE, which means specifying an owner */
    foo = Z_Malloc(W_LumpLength(lump), PU_STATIC, 0);
    /* first, an 8 byte header composed of 4 short ints */
    /* width, height, left offs, top offs */
    /* a number of long int pointers */
    /* then the pixel/color data */

    /* XXX: assuming 32 bit here. hmmph. */
    sp = (unsigned short *)foo;
    *sp++ = M_DYN_PATCH_W;
    *sp++ = M_DYN_PATCH_H;
    *sp++ = M_DYN_PATCH_LO;
    *sp++ = M_DYN_PATCH_TO;
    /* pointers to columns */
    /* first column will be at offset header(8) + pointers(w*4) */
    lp = (unsigned long *)sp;
    for(i = 0; i < M_DYN_PATCH_W; ++i) {
        *lp++ = 8 + (M_DYN_PATCH_W*4) + (i*PATCH_COLUMN_SIZE);
    }
    /* now initialize the columns */
    cp = (unsigned char *)lp;
    for(i = 0; i < M_DYN_PATCH_W; ++i) {
        int j;
        *cp++ = 0; /* row offset */
        *cp++ = M_DYN_PATCH_H; /* num, pixels */
        *cp++ = 0; /* junk byte */
        for(j = 0; j < M_DYN_PATCH_H; ++j)
            *cp++ = BACKGROUND_COLOR; /* pixel data */
        *cp++ = 0; /* junk byte */
        *cp++ = 0xff; /* end the column */
    }

    return (void *)foo;
}
