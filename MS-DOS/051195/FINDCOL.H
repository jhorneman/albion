/************
 * NAME     : FINDCOL.H
 * AUTHOR   : Jurie Horneman, BlueByte & Dave Thielen
 * START    : 11-11-1994
 * PROJECT  : Find closest colour function
 * NOTES    :
 * SEE ALSO : FINDCOL.C
 ************/

#ifndef FINDCOL_H
#define FINDCOL_H

#include <BBDEF.H>

/*
 ** Prototypes *************************************************************
 */

void Prepare_colour_find(struct BBPALETTE *Pal, UNSHORT Start, UNSHORT Number);

UNBYTE Find_closest_colour(UNSHORT Red, UNSHORT Green, UNSHORT Blue);

void Clear_protected_colours(void);

void Add_protected_colours(UNSHORT Start, UNSHORT Size);
void Remove_protected_colours(UNSHORT Start, UNSHORT Size);

#endif

