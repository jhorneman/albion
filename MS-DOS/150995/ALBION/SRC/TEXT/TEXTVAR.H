/************
 * NAME     : TEXTVAR.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 11-7-1994
 * PROJECT  : Text functions
 * NOTES    :
 * SEE ALSO : TEXTVAR.C, TEXT.C, TEXT.H
 ************/

#ifndef TEXTVAR_H
#define TEXTVAR_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define PA_MAX				(4)
#define TEXTSTYLES_MAX	(8)

#define MINIMUM_SPACE	(2)

#define MAX_TEXT_CACHE	(30)

#define TPB_SIZE			(1024)	/* Includes sizeof(struct TPB) */

/* Error codes */
#define TEXTERR_TEXTSTYLE_STACK_OVERFLOW	(1)
#define TEXTERR_TEXTSTYLE_STACK_UNDERFLOW	(2)
#define TEXTERR_PA_STACK_OVERFLOW			(3)
#define TEXTERR_PA_STACK_UNDERFLOW			(4)

#define TEXTERR_MAX								(4)

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* Additional Print Area data */
struct PA_data {
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
};

/* Low-level text commands */
struct LLC_Text_command {
	UNCHAR Command[4];
	void (*Command_handler)(UNCHAR *);
};

/* Line info */
struct Line_info {
	struct Textstyle Style;
	UNSHORT Width;
	UNSHORT Width_without_spaces;
	UNBYTE Height;
	UNBYTE Skip;
	UNBYTE Base_line;
	UNBYTE Nr_of_spaces;
	UNBYTE String_length;
};

/* Text processing blocks */
struct TPB {
	MEM_HANDLE Next_handle;
	UNSHORT Height;
	UNSHORT Nr_of_lines;
};

/* Cached text lines */
struct Cached_text_line {
	UNSHORT Line_nr, Priority;
	UNSHORT Width, Height;
	MEM_HANDLE Line_handle;
};

/*
 ** External global variables **********************************************
 */

extern UNSHORT PA_width;
extern BOOLEAN Process_text_dry_run_flag;

/* Stacks : */
extern struct PA *PA_stack[PA_MAX];
extern struct PA_data PA_data_stack[PA_MAX];
extern UNSHORT PA_stack_index;

extern struct Textstyle *Textstyle_stack[];
extern UNSHORT Textstyle_stack_index;

/* For text commands : */
extern struct Textstyle Current_text_style;

extern SISHORT TEXT_Current_X;

extern struct Line_info Current_line_info;

extern UNSHORT Width_without_spaces;

/* Text command lists : */
extern struct LLC_Text_command LLC_commands[];
extern struct LLC_Text_command LLCW_commands[];
extern struct LLC_Text_command LLCA_commands[];

/* Error output : */
extern UNCHAR Text_library_name[];
extern UNCHAR *Text_error_strings[];

/*
 ** Prototypes *************************************************************
 */

/* Print functions for processed texts */
void Print_processed_string(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNCHAR *String);

/* Low-level text command functions */
UNCHAR *LLC_handler(UNCHAR *Text, struct LLC_Text_command *Command_list);

/* Low-level text commands */
void LLC_Set_ink(UNCHAR *Text);
void LLC_Set_shadow(UNCHAR *Text);
void LLC_No_shadow(UNCHAR *Text);
void LLC_Set_normal_style(UNCHAR *Text);
void LLC_Set_fat_style(UNCHAR *Text);
void LLC_Set_high_style(UNCHAR *Text);
void LLC_Set_fat_high_style(UNCHAR *Text);
void LLC_Set_big_style(UNCHAR *Text);
void LLC_Set_normal_font(UNCHAR *Text);
void LLC_Set_techno_font(UNCHAR *Text);
void LLC_Set_capital(UNCHAR *Text);
void LLC_Unset_capital(UNCHAR *Text);
void LLC_Default(UNCHAR *Text);
void LLC_HJump(UNCHAR *Text);
void LLC_VJump(UNCHAR *Text);
void LLC_Set_centered(UNCHAR *Text);
void LLC_Set_left(UNCHAR *Text);
void LLC_Set_right(UNCHAR *Text);
void LLC_Set_justified(UNCHAR *Text);

/* Graphics functions */
void Put_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour);
void Put_fat_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour);
void Put_high_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour);
void Put_fat_high_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour);
void Put_big_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour);

#endif

