/************
 * NAME     : FINDCOL.H
 * AUTHOR   : Christian Jungen, BlueByte
 * START    : 21-7-1994
 * PROJECT  : Fast colour finding
 * NOTES    :
 * SEE ALSO : FINDCOL.C
 ************/

#ifndef FINDCOL_H
#define FINDCOL_H

/*
 ** Prototypes *************************************************************
 */

void init_find_closest_color_looktable(void);
void install_color_in_color_looktable(UNLONG color_nr,UNLONG r_val,
 UNLONG g_val,UNLONG b_val);
void fill_up_color_looktable(void);
UNLONG  find_closest_color(UNLONG r_val,UNLONG g_val,UNLONG b_val);

#endif

