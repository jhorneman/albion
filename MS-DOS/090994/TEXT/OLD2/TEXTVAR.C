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

/* global variables */

struct PA Default_PA;
struct Font *Default_font;
struct Textstyle *Default_textstyle;
struct OPM Text_OPM;
MEM_HANDLE Text_OPM_handle;

UNSHORT PA_width;
UNSHORT Ink_colour, Shadow_colour;

UNBYTE Current_justification;
UNBYTE Current_text_style;

UNSHORT Raw_TPB_index;				/* RTPB currently being read */
UNSHORT Processed_text_index; 	/* Processed characters written to current RTPB */
UNSHORT Raw_text_index;				/* Raw characters read in current RTPB */
UNSHORT Processing_buffer_index;	/* Processed characters in buffer */

UNSHORT Nr_of_raw_TPBs;

UNBYTE Processing_buffer[LINE_LENGTH_MAX];

MEM_HANDLE Raw_TPB_handles[TPB_MAX];

struct Font *Font_stack[FONTS_MAX];
UNSHORT Font_stack_index;

struct Textstyle *Textstyle_stack[TEXTSTYLES_MAX];
UNSHORT Textstyle_stack_index;

struct Text_command HLC_commands[] = {
	(UNLONG) 'SELF', Process_text};
struct Text_command LLC1_commands[] = {
	(UNLONG) 'SELF', Process_text};
struct Text_command LLC2_commands[] = {
	(UNLONG) 'SELF', Process_text};
struct Text_command LLCA_commands[] = {
	(UNLONG) 'SELF', Process_text};

UNCHAR Text_library_name[] = "Text functions";

UNCHAR *Text_error_strings[] = {
	"Illegal error code.",		  	/* Error code was 0 or > TEXTERR_MAX. */
	"Text is too long.",		    	/* Text was too long for processing. */
	"Font stack overflow.",
	"Font stack underflow.",
	"Text-style stack overflow.",
	"Text-style stack underflow.",
	"PA stack overflow.",
	"PA stack underflow."
};

