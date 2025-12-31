#ifndef FNTINTRN_H
#define FNTINTRN_H

/************
 * NAME     :  OPMINTRN.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */

#include <BBDEF.H>

/* prototypes */

extern void _OPM_ASS_init_font(int x1,int y1,int x2,int y2,UNBYTE *opm_bitmap_ptr,int screen_modulo);
#pragma aux _OPM_ASS_init_font parm caller [] modify [EAX EBX ECX EDX ESI EDI];
extern void _OPM_ASS_draw_text(int x_pos,int y_pos,int color,int font_size,char *text_ptr);
#pragma aux _OPM_ASS_draw_text parm caller [] modify [EAX EBX ECX EDX ESI EDI];
extern void _OPM_ASS_draw_char(int draw_color,int x_pos,int y_pos,int char_width,int char_height,char *char_ptr);
#pragma aux _OPM_ASS_draw_char parm caller [] modify [EAX EBX ECX EDX ESI EDI];

#endif
