#ifndef ZOOMFUNC_H
#define ZOOMFUNC_H

/************
 * NAME     :  ZOOMFUNC.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */

#include <BBDEF.H>

/* Defines */
#define ZOOMMODE_NORMAL 0
#define ZOOMMODE_SILHOUETTE 1
#define ZOOMMODE_RECOLOUR_BACK 2
#define ZOOMMODE_RECOLOUR 3
#define ZOOMMODE_TRANSPARENT 4

/* Prototypes */
extern void Put_zoomed_block(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Source_width,
 UNSHORT Source_height, UNSHORT Target_width, UNSHORT Target_height,
 UNBYTE *Graphics_ptr);

extern void Put_zoomed_silhouette(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNSHORT Target_width,
 UNSHORT Target_height, UNBYTE *Graphics_ptr, UNSHORT Colour);

extern void Put_zoomed_recoloured_silhouette(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNSHORT Target_width,
 UNSHORT Target_height, UNBYTE *Graphics_ptr, UNBYTE *Recolouring_table);

extern void Put_zoomed_recoloured_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNSHORT Target_width,
 UNSHORT Target_height, UNBYTE *Graphics_ptr, UNBYTE *Recolouring_table);

extern void Put_zoomed_transparent_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNSHORT Target_width,
 UNSHORT Target_height, UNBYTE *Graphics_ptr, UNBYTE *Recolouring_table);

/* externe Assemblerfunktionen */
extern void _ZOOM_BLOCK(UNBYTE *src,UNBYTE *dest,UNLONG srcnextypos,UNLONG srcwidth,UNLONG srcheight,UNLONG dstnextypos,UNLONG dstwidth,UNLONG dstheight,SILONG destx,SILONG desty,SILONG color,SILONG zoommode);
#pragma aux _ZOOM_BLOCK parm caller [] modify [EAX EBX ECX EDX ESI EDI]; // Put values directly in registers

#endif
