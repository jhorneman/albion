/************
 * NAME     : TEXTVAR.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 11-7-1994
 * PROJECT  : Text functions
 * NOTES    :
 * SEE ALSO : TEXT.C, TEXT.H
 ************/

/* includes */

#include <BBDEF.H>
#include <BBERROR.H>
#include <BBMEM.H>

#include <TEXT.H>
#include "TEXTVAR.H"

/* global variables */

UNSHORT PA_width;

BOOLEAN Process_text_dry_run_flag = FALSE;

/* Stacks : */
struct PA *PA_stack[PA_MAX];
struct PA_data PA_data_stack[PA_MAX];

UNSHORT PA_stack_index;

struct Textstyle *Textstyle_stack[TEXTSTYLES_MAX];
UNSHORT Textstyle_stack_index;

/* For text commands : */
struct Textstyle Current_text_style;
SISHORT Start_X, Current_X;
struct Line_info Current_line_info;
UNSHORT Width_without_spaces;

/* For actual printing : */
struct LLC_Text_command LLC_commands[] = {
	{{'I','N','K',' '}, LLC_Set_ink},
	{{'S','H','A','D'}, LLC_Set_shadow},
	{{'N','S','H','A'}, LLC_No_shadow},
	{{'N','O','R','S'}, LLC_Set_normal_style},
	{{'F','A','T',' '}, LLC_Set_fat_style},
	{{'H','I','G','H'}, LLC_Set_high_style},
	{{'F','A','H','I'}, LLC_Set_fat_high_style},
	{{'B','I','G',' '}, LLC_Set_big_style},
	{{'N','O','R','F'}, LLC_Set_normal_font},
	{{'T','E','C','F'}, LLC_Set_techno_font},
	{{'C','A','P','I'}, LLC_Set_capital},
	{{'U','N','C','A'}, LLC_Unset_capital},
	{{'D','F','A','U'}, LLC_Default},
	{{'H','J','M','P'}, LLC_HJump},
	{{0,0,0,0}, NULL}
};

/* For word-wrapping checks : */
struct LLC_Text_command LLCW_commands[] = {
	{{'N','O','R','S'}, LLC_Set_normal_style},
	{{'F','A','T',' '}, LLC_Set_fat_style},
	{{'H','I','G','H'}, LLC_Set_high_style},
	{{'F','A','H','I'}, LLC_Set_fat_high_style},
	{{'B','I','G',' '}, LLC_Set_big_style},
	{{'N','O','R','F'}, LLC_Set_normal_font},
	{{'T','E','C','F'}, LLC_Set_techno_font},
	{{'C','A','P','I'}, LLC_Set_capital},
	{{'U','N','C','A'}, LLC_Unset_capital},
	{{'D','F','A','U'}, LLC_Default},
	{{'H','J','M','P'}, LLC_HJump},
	{{0,0,0,0}, NULL}
};

/* For line analysis : */
struct LLC_Text_command LLCA_commands[] = {
	{{'I','N','K',' '}, LLC_Set_ink},
	{{'S','H','A','D'}, LLC_Set_shadow},
	{{'N','S','H','A'}, LLC_No_shadow},
	{{'N','O','R','S'}, LLC_Set_normal_style},
	{{'F','A','T',' '}, LLC_Set_fat_style},
	{{'H','I','G','H'}, LLC_Set_high_style},
	{{'F','A','H','I'}, LLC_Set_fat_high_style},
	{{'B','I','G',' '}, LLC_Set_big_style},
	{{'N','O','R','F'}, LLC_Set_normal_font},
	{{'T','E','C','F'}, LLC_Set_techno_font},
	{{'C','A','P','I'}, LLC_Set_capital},
	{{'U','N','C','A'}, LLC_Unset_capital},
	{{'D','F','A','U'}, LLC_Default},
	{{'H','J','M','P'}, LLC_HJump},
	{{'V','J','M','P'}, LLC_VJump},
	{{'C','N','T','R'}, LLC_Set_centered},
	{{'L','E','F','T'}, LLC_Set_left},
	{{'R','I','G','H'}, LLC_Set_right},
	{{'J','U','S','T'}, LLC_Set_justified},
	{{0,0,0,0}, NULL}
};

/* Error output : */
UNCHAR Text_library_name[] = "Text functions";

UNCHAR *Text_error_strings[] = {
	"Illegal error code.",		  	/* Error code was 0 or > TEXTERR_MAX. */
	"Text-style stack overflow.",
	"Text-style stack underflow.",
	"PA stack overflow.",
	"PA stack underflow."
};

