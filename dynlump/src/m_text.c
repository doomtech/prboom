#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "w_wad.h"
#include "z_zone.h"
#include "m_text.h"

#define PATCH_HEADER_SIZE   8
#define PATCH_POINTERS_SIZE(width) (width*4)
#define PATCH_COLUMN_SIZE(height)   (1 /* row offset  */ + \
                            1 /* num. pixels */ + \
                            height /* pixel data */ + \
                            2 /* junk/padding */ + \
                            1 /* terminating 0xff */)

#define BACKGROUND_COLOR 47 /* a dark red */

#define CHARMAP_ENTRIES_LENGTH 29
struct text_charmap_entry charmap_entries[CHARMAP_ENTRIES_LENGTH] = {
        { 'f', "M_SFXVOL", 0, 0, 0, 0 },
        { 'r', "M_SCRNSZ", 0, 0, 0, 0 },
        { 'a', "M_QUITG" , 0, 0, 0, 0 },
        { 'g', "M_SAVEG" , 0, 0, 0, 0 },
        { 's', "M_MUSVOL", 0, 0, 0, 0 },
        { 'o', "M_MUSVOL", 0, 0, 0, 0 },
        { 'u', "M_QUITG" , 0, 0, 0, 0 },
        { 't', "M_QUITG" , 0, 0, 0, 0 },
        { 'A', "M_NEWG"  , 0, 0, 0, 0 },
        { 'm', "M_QUITG" , 0, 0, 0, 0 },
        { 'p', "M_OPTION", 0, 0, 0, 0 },
        { 'C', "M_SKILL",  0, 0, 0, 0 },
        { 'S', "M_SFXVOL", 0, 0, 0, 0 },
        { 'i', "M_QUITG" , 0, 0, 0, 0 },
        { 'n', "M_SVOL"  , 0, 0, 0, 0 },
        { 'l', "M_MUSVOL", 0, 0, 0, 0 },
        { 'y', "M_MSENS",  0, 0, 0, 0 },
        { 'e', "M_QUITG" , 0, 0, 0, 0 },
        { 'G', "M_QUITG" , 0,54,15,70 },
        { 'z', "M_SCRNSZ", 0, 0, 0, 0 },
        { 'd', "M_SVOL"  , 0, 0, 0, 0 },
        { 'M', "M_MUSVOL", 0, 0, 0, 0 },
        { 'H', "M_HURT"  , 0, 0, 0, 0 },
        { 'U', "M_ULTRA" , 0, 0, 0, 0 },
        { 'v', "M_MSENS",  0, 0, 0, 0 },
        { 'c', "M_MUSVOL", 0, 0, 0, 0 },
        { 'w', "M_NGAME" , 0, 0, 0, 0 },
        { 'W', "M_NEWG"  , 0, 0, 0, 0 },
        { 'h', "M_SKILL",  0, 0, 0, 0 },
};
/*
        characters missing from the above (that we need):
        b: missing
        D: missing
        F: missing
        K: missing
        B: missing
        /: missing
        P: missing
*/

#define DYNLUMPS_SIZE 1
struct dynlump dynlumps[DYNLUMPS_SIZE] = {
        { "M_GENERL", "General", 0, 0, 0 },
};

struct text_charmap_entry *find_charmap_entry(char character) {
        int i;
        for(i = 0; i < CHARMAP_ENTRIES_LENGTH; ++i) {
                struct text_charmap_entry *p = &charmap_entries[i];
                if(p->character == character) {
                        return p;
                }
        }
        return NULL;
}
int width_of_char(struct text_charmap_entry *character) {
    int foo = (character->x2 - character->x1);
    fprintf(stderr,"found size %d\n", foo);
    return foo;
}
int height_of_char(struct text_charmap_entry *character) {
    int foo = (character->y2 - character->y1);
    return foo;
}

/*
 * called at prboom startup
 * this function loops through our list of dynlumps and fills 
 * out the width, height and index fields, by looking up the 
 * information in our characters list.
 *
 * we also extend the lumpinfo array to accommodate the lumps
 * that we will dynamically generate and fill in the new entries
 * (including the lump's eventual size). The index into this array
 * is what is saved in the index field of the dynlump struct.
 */
void M_InitDynLump() {
    int index;
    int i;

    for(i = 0; i < DYNLUMPS_SIZE; ++i) {
            char *p;
            int totwidth = 0;
            int maxheight = 0;
            for(p = dynlumps[i].text; *p != '\0'; ++p) {
                struct text_charmap_entry *tce;
                int thisheight;
                tce = find_charmap_entry(*p);
                if(NULL == tce) {
                    continue; // XXX
                }
                thisheight = height_of_char(tce);
                if(thisheight > maxheight) {
                    maxheight = thisheight;
                }
                totwidth += width_of_char(tce);
            }
            dynlumps[i].width = totwidth;
            dynlumps[i].height = maxheight;
            fprintf(stderr,"ZOMG: totwidth=%d\n", totwidth);
        index = numlumps;
        dynlumps[i].lumpnum = index;
        numlumps++;
        /* todo: pull this realloc out of the loop and do it 
         * just once, but * DYNLUMPS_SIZE bigger. */
        lumpinfo = realloc(lumpinfo, numlumps*sizeof(lumpinfo_t));

        strncpy(lumpinfo[index].name, "M_GENERL", 9);
        lumpinfo[index].source = source_dyn;
        lumpinfo[index].wadfile = NULL;
        lumpinfo[index].li_namespace = ns_global;
        lumpinfo[index].size =
            PATCH_HEADER_SIZE +
            PATCH_POINTERS_SIZE(totwidth) +
            totwidth * PATCH_COLUMN_SIZE(maxheight);
    }
}

void jons_debug_hack(int lump, void *foo) {
    // evil
    FILE *debugfile;
    size_t bytes_written;
    size_t to_write;
    debugfile = fopen("/tmp/zomglol","w");
    if(NULL == debugfile) {
        fprintf(stderr, "shit, bah\n");
        exit(1);
    }
    to_write = W_LumpLength(lump);
    bytes_written = fwrite(foo, to_write, 1, debugfile);
    /* XXX: this is records written, so 1 is good */
    if(to_write != bytes_written) {
        fprintf(stderr, "only %d of %d bytes written\n",
            bytes_written, to_write);
    }
    fclose(debugfile);
}

/*
 * called when a dynamically-generated lump is required.
 */
const void* W_CacheLumpNumDyn(int lump) { /* choose a better name */
    const char* foo;
    unsigned short *sp;
    unsigned long *lp;
    unsigned char *cp;
    int i;
    struct dynlump *dlump = NULL;

    /* find our local dynlump structure */
    for(i = 0; i < DYNLUMPS_SIZE; ++i) {
        if(dynlumps[i].lumpnum == lump) {
            dlump = &dynlumps[i];
            break;
        }
    }
    if(NULL == dlump) {
        fprintf(stderr,"Error: asked to create lump %d but we don't"
            "know anything about it\n", lump);
        exit(1);
    }

    /* TODO: change to PU_CACHE, which means specifying an owner */
    foo = Z_Malloc(W_LumpLength(lump), PU_STATIC, 0);
    /* XXX: malloc failure */

    /* first, the patch header: width/height/left offset/top offset */
    /* XXX: assuming 32 bit here. hmmph. */
    sp = (unsigned short *)foo;
    *sp++ = dlump->width;
    *sp++ = dlump->height;
    *sp++ = 0; /* left offset */
    *sp++ = 0; /* top offset */
    /* next, pointers to columns */
    /* first column will be at offset header(8) + pointers(w*4) */
    /* subsequent columns i will be a column-size worth further on */
    lp = (unsigned long *)sp;
    for(i = 0; i < dlump->width; ++i) {
        *lp++ = 8 + (dlump->width*4) + (i*PATCH_COLUMN_SIZE(dlump->height));
    }
    /* now initialize the columns */
    cp = (unsigned char *)lp;
    for(i = 0; i < dlump->width; ++i) {
        int j;
        *cp++ = 0; /* row offset */
        *cp++ = dlump->height; /* num, pixels */
        *cp++ = 0; /* junk byte */
        for(j = 0; j < dlump->height; ++j)
            *cp++ = BACKGROUND_COLOR; /* pixel data */
        *cp++ = 0; /* junk byte */
        *cp++ = 0xff; /* end the column */
    }
    jons_debug_hack(lump,foo);
    return (void *)foo;
}
