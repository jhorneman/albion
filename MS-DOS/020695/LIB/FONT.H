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

#define SHADOW_TEXT		(0)
#define SILVER_TEXT		(1)
#define GOLD_TEXT			(2)
#define BRONZE_TEXT		(3)
#define GREEN_TEXT		(4)
#define ORANGE_TEXT		(5)
#define RED_TEXT			(6)
#define WHITE_TEXT		(7)

#define CAPITAL_COLOUR	(6)

/*
 ** External global variables **********************************************
 */

extern struct Font Normal_font;
extern struct Font Techno_font;
extern struct Textstyle Default_text_style;
extern struct PA Default_PA;
extern UNBYTE Text_colours[8][7];

/*
 ** Prototypes *************************************************************
 */

void Init_font(void);
void Exit_font(void);
void Prepare_font(struct Font *Font);
void Set_ink(UNSHORT Ink);
void Set_style(UNSHORT Style);

#endif

