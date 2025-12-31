/************
 * NAME     : FONT.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-8-1994
 * PROJECT  : Font managing functions
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : TEXT.H
 ************/

/* includes */

#include <BBDEF.H>

#include <XLOAD.H>
#include <TEXT.H>

#include <ALBION.H>
#include <FONT.H>
#include <XFTYPES.H>

/* global variables */

MEM_HANDLE Normal_font_handle;
MEM_HANDLE Techno_font_handle;

UNCHAR Normal_kerning_table[] =
"\0\0\
FaFcFdFeFgFmFnFoFpFqFrFsFuFvFwFxFyFz\
F‰FˆF¸F.F:F,F;F'F+F-F=F<\
F0F1F2F3F4F5F6F7F8F9\
LaLcLeLgLjLoLqLtLuLvLy\
PaPcPdPePgPmPnPoPpPqPrPsPuPvPwPxPyPz\
P‰Pˆ\
P.P:P,P;P'P+P-P=P<\
P0P1P2P3P4P5P6P7P8P9\
TaTcTdTeTgTmTnToTpTqTrTsTuTvTwTxTyTz\
T‰TˆT¸T.T:T,T;T'T+T-T=T<\
T0T1T2T3T4T5T6T7T8T9\
YaYcYdYeYgYmYnYoYpYqYrYsYuYvYwYxYyYz\
Y‰YˆY¸Y.Y:Y,Y;Y'Y+Y-Y=Y<\
Y0Y1Y2Y3Y4Y5Y6Y7Y8Y9\
ajbjcjdjejfjgjhjijkjljmjnj\
ojpjrjsjtjujvjwjxjyjzjAjBj\
CjDjEjFjGjHjIjKjLjMjNjOjPj\
QjRjSjTjUjVjWjXjYjZj0j1j2j\
3j4j5j6j7j8j9j‰jƒjˆj÷j¸j‹j\
ﬂj.j:j'j\"j?j!j/j(j)j+j-j=j\
>j<j\
lalcldlelflllolqltlulvlwly\
l'l\"l?l+l-l<\
l0l1l2l3l4l5l6l7l8l9";

UNBYTE Normal_translation_table[224] = {
	0,78,76,82,92,83,85,74,				/* spc !"#$%&' 	$ = male	*/
	80,81,84,86,72,87,70,79,			/* ()*+,-./		*/
	62,53,54,55,56,57,58,59,60,61,	/* 0123456789	*/
	71,73,90,88,89,77,93,					/* :;<=>?@		@ = female */
	27,28,29,30,31,32,33,34,35,36,	/* ABCDEFGHIJ	*/
	37,38,39,40,41,42,43,44,45,46,	/* KLMNOPQRST	*/
	47,48,49,50,51,52,  					/* UVWXYZ		*/
	0,78,0,0,0,0,							/* [\]^_`		*/
	1,2,3,4,5,6,7,8,9,10,		 		/* abcdefghij	*/
	11,12,13,14,15,16,17,18,19,20,	/* klmnopqrst	*/
	21,22,23,24,25,26,					/* uvwxyz		*/
	0,0,0,0,0,								/* ... (standard ASCII) */
	0,
	68,0,0,63,0,0,0,						/* Å..Ñ (Atari, PC) */
	0,0,0,0,0,0,
	64,0,0,0,0,0,							/* é (Atari, PC) */
	65,0,0,0,0,66,67,						/* î....ôö (Atari, PC) */
	0,0,0,69,0,								/* · (Atari) */
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,69										/* · (PC) */
};

UNBYTE Normal_width_table[] = {
	0,
	4,4,4,4,4,2,4,4,1,2,
	4,2,7,4,4,4,4,3,4,2,
	4,4,7,4,4,3,
	4,4,4,4,4,4,4,4,1,4,
	5,4,5,4,4,4,4,4,4,5,
	4,5,5,5,5,5,
	2,4,4,5,4,4,4,4,4,4,
	4,4,4,4,4,4,4,1,1,1,
	1,1,3,4,1,4,
	2,2,5,4,5,5,3,3,3,3,
	3,5,5,5
};

struct Fontstyle Normal_fontstyle = {
	8, 8, 64,
	1, 1,
	6, 3,
	&Normal_kerning_table,
	&Normal_translation_table,
	&Normal_width_table,
	NULL, 0,
	Print_character
};

struct Fontstyle Fat_fontstyle = {
	8, 8, 64,
	2, 1,
	6, 4,
	&Normal_kerning_table,
	&Normal_translation_table,
	&Normal_width_table,
	NULL, 0,
	Print_fat_character
};

struct Fontstyle High_fontstyle = {
	8, 16, 64,
	1, 1,
	13, 4,
	&Normal_kerning_table,
	&Normal_translation_table,
	&Normal_width_table,
	NULL, 0,
	Print_high_character
};

struct Font Normal_font = {
	&Normal_fontstyle,
	&Fat_fontstyle,
	&High_fontstyle,
	&Fat_fontstyle
};

struct Font Techno_font = {
	&Normal_fontstyle,
	&Fat_fontstyle,
	&Normal_fontstyle,
	&Fat_fontstyle
};

struct Textstyle Default_text_style = {
	1, BLACK, 0, //xFFFF,
	&Normal_font,
	PRINT_JUSTIFIED, NORMAL_STYLE
};

struct Textstyle Diagnostic_text_style = {
	6, BLACK, 0, //xFFFF,
	&Normal_font,
	PRINT_JUSTIFIED, NORMAL_STYLE
};

struct PA Default_PA;

UNBYTE Text_colours[7][9] = {
	{194,195,196,197,198,199,200,BLACK},	/* Silver */
	{194,219,220,221,222,223,236,BLACK},	/* Gold */
	{194,232,233,234,235,236,237,BLACK},	/* Bronze */
	{194,219,220,213,214,215,216,BLACK},	/* Green */
	{219,220,208,209,210,211,212,BLACK},	/* Orange-red */
	{220,208,209,210,211,212,213,BLACK},		/* Red */
	{WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK}	/* All white */
};

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_text
 * FUNCTION  : Load and prepare fonts.
 * FILE      : FONT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.94 15:26
 * LAST      : 23.08.94 15:26
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_font(void)
{
	/* Load font graphics */
	Normal_font_handle = XLD_Load_subfile(&Xftypes[FONTS], 1);
	// Techno_font_handle = XLD_Load_subfile(&Xftypes[FONTS], 2);

	Normal_fontstyle.Graphics_handle = Normal_font_handle;
	Fat_fontstyle.Graphics_handle = Normal_font_handle;
	High_fontstyle.Graphics_handle = Normal_font_handle;

	/* Prepare font */
	Prepare_font(&Normal_font);

	/* Make default PA */
	Default_PA.Area.left = 3;
	Default_PA.Area.top = 3;
	Default_PA.Area.width = Screen_width - 6;
	Default_PA.Area.height = Screen_height - 6;

	memcpy(&Default_PA.PA_textstyle, &Default_text_style,
	 sizeof(struct Textstyle));

	/* Reset text stacks */
	Reset_textstyle_stack();
	Reset_PA_stack();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_font
 * FUNCTION  : Terminate fonts.
 * FILE      : FONT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.94 15:27
 * LAST      : 23.08.94 15:27
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_font(void)
{
	MEM_Free_memory(Normal_font_handle);
	// MEM_Free_memory(Techno_font_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_font
 * FUNCTION  : Initialize a font.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 16:25
 * LAST      : 11.07.94 16:25
 * INPUTS    : struct Font *Font - Pointer to font.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Prepare_font(struct Font *Font)
{
	UNSHORT i;
	struct Fontstyle *Style;
	struct Kerning_pair *Table;
	UNBYTE *Translation;

	/* Check all font styles */
	for (i=0;i<STYLES_MAX;i++)
	{
		/* Get font style */
		Style = Font->Styles[i];
		/* Kerning table available ? */
		if (Style->Kerning_table)
		{
			/* Yes -> Already converted ? */
			Table = (struct Kerning_pair *) Style->Kerning_table;
			if (*((UNSHORT *) Table) != 0xFFFF)
			{
				/* No -> Convert */
				*((UNSHORT *) Table) = 0xFFFF;
				Table++;

				Translation = Style->Translation;

				/* Until the table has ended */
				while (*(UNSHORT *) Table)
				{
					/* Convert kerning pairs */
					Table->First = Translation[((UNSHORT) Table->First) - 32];
					Table->Second = Translation[((UNSHORT) Table->Second) -32];
					/* Next entry */
					Table++;
				}
			}
		}
	}
}

