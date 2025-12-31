/************
 * NAME     : COLOURS.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 13-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COLOURS_H
#define COLOURS_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/* Standard colours */
#define BLACK			(192)
#define BLINK			(193)
#define WHITE			(194)
#define BRIGHTEST		(195)
#define BRIGHTER		(196)
#define BRIGHT			(197)
#define NORMAL			(198)
#define DARK			(199)
#define DARKER			(200)
#define DARKEST		(201)
#define INDIGO			(202)
#define PURPLE			(205)
#define RED				(208)
#define GREEN			(213)
#define GOLD			(219)
#define TURQUOISE		(224)
#define BLUE			(228)
#define SKIN			(232)
#define MARMOR			(240)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern UNSHORT Current_palette_nr;

extern struct BBPALETTE Palette;

extern UNBYTE Recolour_tables[10][256];
extern UNBYTE Red_table[256];

/*
 ** Prototypes *************************************************************
 */

/* Palette management functions */
void Init_palette(void);
BOOLEAN Load_palette(UNSHORT Palette_nr);

void Restore_palette(void);

void Update_palette(UNSHORT Start, UNSHORT Size);

void Cycle_colours(UNSHORT Palette_nr);
void Colour_cycle_forward(UNSHORT Start, UNSHORT Size);

/* Palette support functions */
void Calculate_recolouring_table(UNBYTE *Table, UNSHORT Start, UNSHORT Size,
 UNSHORT Target_R, UNSHORT Target_G, UNSHORT Target_B, UNSHORT Percentage);

void Recolour_palette(UNSHORT Start, UNSHORT Size, UNSHORT Target_R,
 UNSHORT Target_G, UNSHORT Target_B, UNSHORT Percentage);

/* Fade functions */
void Fade_to_black(void);
void Fade_to_white(void);
void Fade_from_black(void);
void Fade_from_white(void);

#endif

