/************
 * NAME     : FONT.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef FONT_H
#define FONT_H

#include <BBDEF.H>
#include <TEXT.H>

/*
 ** Defines ****************************************************************
 */

#define CAPITAL_COLOUR		(194)

/*
 ** External global variables **********************************************
 */

extern struct Font Normal_font;
extern struct Font Techno_font;
extern struct Textstyle Default_text_style;
extern struct PA Default_PA;
extern UNBYTE Text_colours[6][9];

/*
 ** Prototypes *************************************************************
 */

void Init_font(void);
void Exit_font(void);
void Prepare_font(struct Font *Font);

#endif

