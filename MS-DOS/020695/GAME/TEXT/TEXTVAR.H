/************
 * NAME     : TEXTVAR.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 11-7-1994
 * PROJECT  : Text functions
 * NOTES    :
 * SEE ALSO : TEXTVAR.C, TEXT.C, TEXT.H
 ************/

/*
 ** Global variable external definitions ***********************************
 */

extern UNSHORT PA_width;

/* Stacks : */
extern struct PA *PA_stack[PA_MAX];
extern UNSHORT PA_stack_index;

extern struct Textstyle *Textstyle_stack[];
extern UNSHORT Textstyle_stack_index;

/* For text commands : */
extern struct Textstyle Current_text_style;
extern SISHORT Start_X, Current_X;
extern struct Line_info Current_line_info;
extern UNSHORT Width_without_spaces;

/* Text command lists : */
extern struct LLC_Text_command LLC_commands[];
extern struct LLC_Text_command LLCW_commands[];
extern struct LLC_Text_command LLCA_commands[];

/* Error output : */
extern UNCHAR Text_library_name[];
extern UNCHAR *Text_error_strings[];

