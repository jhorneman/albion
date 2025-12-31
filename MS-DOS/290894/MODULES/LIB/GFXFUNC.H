/************
 * NAME     : GFXFUNC.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 20-7-1994
 * PROJECT  : Graphics functions
 * NOTES    :
 * SEE ALSO : GFXFUNC.C
 ************/

#ifndef GFXFUNC_H
#define GFXFUNC_H

#include <BBOPM.H>

/*
 ** Prototypes *************************************************************
 */

void Get_block(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Buffer);

void Put_masked_block(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr);

void Put_unmasked_block(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr);

void Put_silhouette(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNBYTE Colour);

void Put_recoloured_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr, UNBYTE *Recolour_table);

void Put_recoloured_box(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Recolour_table);

void Scroll_block(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, SISHORT Delta_X, SISHORT Delta_Y);

#endif

