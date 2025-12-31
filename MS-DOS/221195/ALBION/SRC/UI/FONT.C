/************
 * NAME     : FONT.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-8-1994
 * PROJECT  : Font managing functions
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : TEXT.H
 ************/

/* includes */

#include <string.h>

#include <BBDEF.H>

#include <XLOAD.H>
#include <TEXT.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <XFTYPES.H>
#include <COLOURS.H>

/* global variables */

static MEM_HANDLE Normal_font_handle;
static MEM_HANDLE Techno_font_handle;

/* Normal font data */
static UNCHAR Normal_kerning_table[] =
"\0\0"
"FaFcFdFeFgFmFnFoFpFqFrFsFuFvFwFxFyFz"
"F.F:F,F;F'F+F-F=F<"
"F0F1F2F3F4F5F6F7F8F9"
"LaLcLeLgLjLoLqLtLuLvLy"
"PaPcPdPePgPmPnPoPpPqPsPuPvPwPxPyPz"
"P.P:P,P;P'P+P-P=P<"
"P0P1P2P3P4P5P6P7P8P9"
"TaTcTdTeTgTmTnToTpTqTrTsTuTvTwTxTyTz"
"T.T:T,T;T'T+T-T=T<"
"T0T1T2T3T4T5T6T7T8T9"
"YaYcYdYeYgYmYnYoYpYqYrYsYuYvYwYxYyYz"
"Y.Y:Y,Y;Y'Y+Y-Y=Y<"
"Y0Y1Y2Y3Y4Y5Y6Y7Y8Y9"
"ajbjcjdjejfjgjhjijjjkjljmjnj"
"ojpjrjsjtjujvjwjxjyjzjAjBj"
"CjDjEjFjGjHjIjKjLjMjNjOjPj"
"QjRjSjTjUjVjWjXjYjZj0j1j2j"
"3j4j5j6j7j8j9j„j”jj"
".j:j'j\"j?j!j/j(j)j+j-j=j"
">j<j"
"lalcldlelllolqltlulvlwly"
"l'l\"l?l+l-l<"
"l0l1l2l3l4l5l6l7l8l9"
"\0\0";

static UNBYTE Normal_translation_table[224] = {
	0,78,76,82,91,83,85,74,				/* spc !"#$%&' $ = male	*/
	80,81,84,86,72,87,70,79,			/* ()*+,-./		*/
	62,53,54,55,56,57,58,59,60,61,	/* 0123456789	*/
	71,73,90,88,89,77,92,				/* :;<=>?@		@ = female */
	27,28,29,30,31,32,33,34,35,36,	/* ABCDEFGHIJ	*/
	37,38,39,40,41,42,43,44,45,46,	/* KLMNOPQRST	*/
	47,48,49,50,51,52,  					/* UVWXYZ		*/
	0,78,0,0,0,0,							/* [\]^_`		*/
	1,2,3,4,5,6,7,8,9,10,		 		/* abcdefghij	*/
	11,12,13,14,15,16,17,18,19,20,	/* klmnopqrst	*/
	21,22,23,24,25,26,					/* uvwxyz		*/
	0,0,0,0,0,								/* ... (standard ASCII) */
	0,
	67,0,0,63,0,0,0,						/* ..„ (Atari, PC) */
	0,0,0,0,0,0,
	64,0,0,0,0,0,							/* Ž (Atari, PC) */
	65,0,0,0,0,66,68,						/* ”....™š (Atari, PC) */
	0,0,0,69,0,								/* á (Atari) */
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,69										/* á (PC) */
};

static UNBYTE Normal_width_table[] = {
	0,
	4,4,4,4,4,2,4,4,1,2,					/* abcdefghij	*/
	4,2,7,4,4,4,4,3,4,2,					/* klmnopqrst	*/
	4,4,7,4,4,3,							/* uvwxyz		*/
	4,4,4,4,4,4,4,4,1,4,					/* ABCDEFGHIJ	*/
	5,4,5,4,4,4,4,4,4,5,             /* KLMNOPQRST	*/
	4,5,5,5,5,5,                     /* UVWXYZ		*/
	2,4,4,5,4,4,4,4,4,4,					/* 1234567890	*/
	4,4,4,4,4,4,4,1,1,1,					/* „Ž”™šá.:,	*/
	1,1,1,3,4,1,							/* ;'."?!		*/
	4,2,2,5,4,5,5,3,3,3,					/* /()#%*&+-=	*/
	3,3,5,5  								/* ><$@			*/
};

static UNBYTE Normal_big_width_table[] = {
	0,
	8,8,8,8,8,4,8,8,2,4,					/* abcdefghij	*/
	8,4,14,8,8,8,8,6,8,4,				/* klmnopqrst	*/
	8,8,14,8,8,6,							/* uvwxyz		*/
	8,8,8,8,8,8,8,8,2,8,					/* ABCDEFGHIJ	*/
	10,8,10,8,8,8,8,8,8,10,				/* KLMNOPQRST	*/
	8,10,10,10,10,10,						/* UVWXYZ		*/
	4,8,8,10,8,8,8,8,8,8,				/* 1234567890	*/
	8,8,8,8,8,8,8,2,2,2,					/* „Ž”™šá.:,	*/
	2,2,2,6,8,2,							/* ;'."?!		*/
	8,4,4,10,8,10,10,6,6,6,				/* /()#%*&+-=	*/
	6,6,10,10  								/* ><$@			*/
};

/* Techno font data */
static UNBYTE Techno_translation_table[224] = {
	0,78,76,82,91,83,85,74,				/* spc !"#$%&' $ = male	*/
	80,81,84,86,72,87,70,79,			/* ()*+,-./		*/
	62,53,54,55,56,57,58,59,60,61,	/* 0123456789	*/
	71,73,90,88,89,77,92,				/* :;<=>?@		@ = female */
	27,28,29,30,31,32,33,34,35,36,	/* ABCDEFGHIJ	*/
	37,38,39,40,41,42,43,44,45,46,	/* KLMNOPQRST	*/
	47,48,49,50,51,52,  					/* UVWXYZ		*/
	0,78,0,0,0,0,							/* [\]^_`		*/
	1,2,3,4,5,6,7,8,9,10,		 		/* abcdefghij	*/
	11,12,13,14,15,16,17,18,19,20,	/* klmnopqrst	*/
	21,22,23,24,25,26,					/* uvwxyz		*/
	0,0,0,0,0,								/* ... (standard ASCII) */
	0,
	67,0,0,63,0,0,0,						/* ..„ (Atari, PC) */
	0,0,0,0,0,0,
	64,0,0,0,0,0,							/* Ž (Atari, PC) */
	65,0,0,0,0,66,68,						/* ”....™š (Atari, PC) */
	0,0,0,69,0,								/* á (Atari) */
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,69										/* á (PC) */
};

static UNBYTE Techno_width_table[] = {
	0,
	8,8,8,8,8,8,8,8,8,8,					/* abcdefghij	*/
	8,8,8,8,8,8,8,8,8,8,					/* klmnopqrst	*/
	8,8,8,8,8,8,         				/* uvwxyz		*/
	8,8,8,8,8,8,8,8,8,8,					/* ABCDEFGHIJ	*/
	8,8,8,8,8,8,8,8,8,8,             /* KLMNOPQRST	*/
	8,8,8,8,8,8,                     /* UVWXYZ		*/
	8,8,8,8,8,8,8,8,8,8,					/* 1234568890	*/
	8,8,8,8,8,8,8,8,8,8,					/* „Ž”™šá.:,	*/
	8,8,8,8,8,8,         				/* ;'."?!		*/
	8,8,8,8,8,8,8,8,8,8,					/* /()#%*&+-=	*/
	8,8,8,8 	           					/* ><$@			*/
};

static UNBYTE Techno_big_width_table[] = {
	0,
	16,16,16,16,16,16,16,16,16,16,		/* abcdefghij	*/
	16,16,16,16,16,16,16,16,16,16,		/* klmnopqrst	*/
	16,16,16,16,16,16,         			/* uvwxyz		*/
	16,16,16,16,16,16,16,16,16,16,		/* ABCDEFGHIJ	*/
	16,16,16,16,16,16,16,16,16,16,      /* KLMNOPQRST	*/
	16,16,16,16,16,16,                  /* UVWXYZ		*/
	16,16,16,16,16,16,16,16,16,16,		/* 1234567890	*/
	16,16,16,16,16,16,16,16,16,16,		/* „Ž”™šá.:,	*/
	16,16,16,16,16,16,         			/* ;'."?!		*/
	16,16,16,16,16,16,16,16,16,16,		/* /()#%*&+-=	*/
	16,16,16,16	            				/* ><$@			*/
};

/* Normal font styles */
static struct Fontstyle Normal_fontstyle = {
	8, 8, 64,
	1, 1,
	6, 3,
	Normal_kerning_table,
	Normal_translation_table,
	Normal_width_table,
	NULL, 0,
	Print_character
};

static struct Fontstyle Normal_fat_fontstyle = {
	8, 8, 64,
	2, 1,
	6, 4,
	Normal_kerning_table,
	Normal_translation_table,
	Normal_width_table,
	NULL, 0,
	Print_fat_character
};

static struct Fontstyle Normal_high_fontstyle = {
	8, 16, 64,
	1, 2,
	13, 4,
	Normal_kerning_table,
	Normal_translation_table,
	Normal_width_table,
	NULL, 0,
	Print_high_character
};

static struct Fontstyle Normal_fat_high_fontstyle = {
	8, 16, 64,
	2, 2,
	13, 4,
	Normal_kerning_table,
	Normal_translation_table,
	Normal_width_table,
	NULL, 0,
	Print_fat_high_character
};

static struct Fontstyle Normal_big_fontstyle = {
	16, 16, 64,
	2, 2,
	13, 6,
	Normal_kerning_table,
	Normal_translation_table,
	Normal_big_width_table,
	NULL, 0,
	Print_big_character
};

/* Techno font styles */
static struct Fontstyle Techno_fontstyle = {
	8, 8, 64,
	1, 1,
	6, 8,
	NULL,
	Techno_translation_table,
	Techno_width_table,
	NULL, 0,
	Print_character
};

static struct Fontstyle Techno_fat_fontstyle = {
	8, 8, 64,
	2, 1,
	6, 9,
	NULL,
	Techno_translation_table,
	Techno_width_table,
	NULL, 0,
	Print_fat_character
};

static struct Fontstyle Techno_high_fontstyle = {
	8, 16, 64,
	1, 2,
	13, 8,
	NULL,
	Techno_translation_table,
	Techno_width_table,
	NULL, 0,
	Print_high_character
};

static struct Fontstyle Techno_fat_high_fontstyle = {
	8, 16, 64,
	2, 2,
	13, 9,
	NULL,
	Techno_translation_table,
	Techno_width_table,
	NULL, 0,
	Print_fat_high_character
};

static struct Fontstyle Techno_big_fontstyle = {
	16, 16, 64,
	2, 2,
	13, 4,
	NULL,
	Techno_translation_table,
	Techno_big_width_table,
	NULL, 0,
	Print_big_character
};

/* Fonts */
struct Font Normal_font = {
	&Normal_fontstyle,
	&Normal_fat_fontstyle,
	&Normal_high_fontstyle,
	&Normal_fat_high_fontstyle,
	&Normal_big_fontstyle
};

struct Font Techno_font = {
	&Techno_fontstyle,
	&Techno_fat_fontstyle,
	&Techno_high_fontstyle,
	&Techno_fat_high_fontstyle,
	&Techno_big_fontstyle
};

struct Textstyle Default_text_style = {
	GOLD_TEXT, SHADOW_TEXT, 0xFFFF,
	&Normal_font,
	PRINT_JUSTIFIED, NORMAL_STYLE
};

struct PA Default_PA;

UNBYTE Text_colours[][7] = {
	{BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},	/* Shadow */
	{194,194,195,196,197,198,199},	/* Silver */
	{194,219,220,221,222,223,236},	/* Gold */
	{194,195,232,233,234,235,236},	/* Bronze */
	{219,219,220,213,214,215,216},	/* Green */
	{219,220,208,209,210,211,212},	/* Orange-red */
	{219,220,208,208,209,210,211},	/* Red */
	{WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE},	/* All white */
	{196,197,198,199,199,199,199},	/* Dark silver */
};

#if FALSE
	{194,195,196,197,198,199,200},	/* Silver */
	{194,219,220,221,222,223,236},	/* Gold */
	{194,232,233,234,235,236,237},	/* Bronze */
	{194,219,220,213,214,215,216},	/* Green */
	{219,220,208,209,210,211,212},	/* Orange-red */
	{220,208,209,210,211,212,213},	/* Red */
#endif

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
	Normal_font_handle = Load_subfile(FONTS, 1);
	Techno_font_handle = Load_subfile(FONTS, 2);

	Normal_fontstyle.Graphics_handle				= Normal_font_handle;
	Normal_fat_fontstyle.Graphics_handle		= Normal_font_handle;
	Normal_high_fontstyle.Graphics_handle		= Normal_font_handle;
	Normal_fat_high_fontstyle.Graphics_handle	= Normal_font_handle;
	Normal_big_fontstyle.Graphics_handle		= Normal_font_handle;

	Techno_fontstyle.Graphics_handle				= Techno_font_handle;
	Techno_fat_fontstyle.Graphics_handle		= Techno_font_handle;
	Techno_high_fontstyle.Graphics_handle		= Techno_font_handle;
	Techno_fat_high_fontstyle.Graphics_handle	= Techno_font_handle;
	Techno_big_fontstyle.Graphics_handle		= Techno_font_handle;

	/* Prepare fonts */
	Prepare_font(&Normal_font);
	Prepare_font(&Techno_font);

	/* Make default PA */
	Default_PA.Area.left		= 3;
	Default_PA.Area.top		= 3;
	Default_PA.Area.width	= Screen_width - 6;
	Default_PA.Area.height	= Screen_height - 6;

	/* Reset text stacks */
	Reset_textstyle_stack();
	Reset_PA_stack();

	/* Destroy text cache */
	Destroy_text_cache();
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
	MEM_Free_memory(Techno_font_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_font
 * FUNCTION  : Initialize a font.
 * FILE      : FONT.C
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
	for (i=0;i<MAX_STYLES;i++)
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

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_ink
 * FUNCTION  : Set the current printing ink colour.
 * FILE      : FONT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 12:33
 * LAST      : 20.10.94 12:33
 * INPUTS    : UNSHORT Ink - Ink colour.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will change the ink colour in the default
 *              text style, so the new ink colour will only be used if this
 *              text style is the current one.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_ink(UNSHORT Ink)
{
	Default_text_style.Ink = Ink;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_style
 * FUNCTION  : Set the current printing ink style.
 * FILE      : FONT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 09:27
 * LAST      : 27.10.94 09:27
 * INPUTS    : UNSHORT Style - Printing style.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will change the style in the default
 *              text style, so the new style will only be used if this
 *              text style is the current one.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_style(UNSHORT Style)
{
	Default_text_style.Style = Style;
}

