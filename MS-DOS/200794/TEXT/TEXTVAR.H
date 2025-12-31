/************
 * NAME     : TEXTVAR.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 11-7-1994
 * PROJECT  : Text functions
 * NOTES    :
 * SEE ALSO : TEXTVAR.C, TEXT.C, TEXT.H
 ************/

/* global variable external definitions */

extern struct Font *Font_stack[FONTS_MAX];
extern UNSHORT Font_stack_index;

extern struct Font *Font_stack[FONTS_MAX];
extern UNSHORT Font_stack_index;

extern UNSHORT PA_width;
extern UNSHORT Ink_colour, Shadow_colour;

extern UNBYTE Current_justification;
extern UNBYTE Current_text_style;

extern UNSHORT Raw_TPB_index;				/* RTPB currently being read */
extern UNSHORT Processed_text_index; 	/* Processed characters written to current RTPB */
extern UNSHORT Raw_text_index;				/* Raw characters read in current RTPB */
extern UNSHORT Processing_buffer_index;	/* Processed characters in buffer */

extern UNSHORT Nr_of_raw_TPBs;

extern UNBYTE Processing_buffer[LINE_LENGTH_MAX];

extern MEM_HANDLE Raw_TPB_handles[TPB_MAX];

extern struct Text_command HLC_commands[];
extern struct Text_command LLC1_commands[];
extern struct Text_command LLC2_commands[];
extern struct Text_command LLCA_commands[];

extern UNCHAR Text_library_name[];

extern UNCHAR *Text_error_strings[];

