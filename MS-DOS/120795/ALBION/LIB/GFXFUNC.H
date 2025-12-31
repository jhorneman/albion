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

/* externe Assemblerfunktionen */
extern void _OPM_ASS_ChangeCol(UNBYTE *dest,UNBYTE *tab,SILONG mod,UNLONG width,UNLONG height);
#pragma aux _OPM_ASS_ChangeCol parm caller [EDI] [ESI] [EBX] [ECX] [EDX] modify [EAX]; // Put values directly in registers

/*
 ** Structure definitions **************************************************
 */

/* Default graphics header */
struct Gfx_header {
	UNSHORT Width, Height;
	UNBYTE Depth;
	UNBYTE Nr_frames;
};

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

void Put_masked_X_mirrored_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr);

void Put_unmasked_X_mirrored_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr);

void Put_recoloured_X_mirrored_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr, UNBYTE *Recolour_table);

UNSHORT Get_nr_frames(MEM_HANDLE Handle);

#endif

