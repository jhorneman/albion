/************
 * NAME     : TEXT.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 8-7-1994
 * PROJECT  : Text functions
 * NOTES    :
 * SEE ALSO : TEXT.C
 ************/

#ifndef TEXT_H
#define TEXT_H

/*
 ** Defines ****************************************************************
 */

#define FONTS_MAX		(3)
#define PA_MAX			(12)

#define MINIMUM_SPACE	(2)

#define STANDARD_TEXT_HEIGHT	(8)
#define FONT_HEIGHT_MAX			(20)
#define LINE_BUFFER_SIZE		(FONT_HEIGHT_MAX*360+32)

#define LINE_LENGTH_MAX	(180)
#define TPB_MAX			(50)
#define TPB_SIZE			(512)

/* Special text codes */

#define CR					((UNCHAR) '^')
#define COMSTART_CHAR	((UNCHAR) '{')
#define COMEND_CHAR		((UNCHAR) '}')

#define SPACE				((UNCHAR) ' ')
#define SOLID_SPACE		((UNCHAR) '$')

#define HYPHEN1			((UNCHAR) '-')
#define HYPHEN2			((UNCHAR) '/')
#define SEPARATOR			((UNCHAR) '|')
#define EOTPB				((UNCHAR) '~')

/* Text styles */

#define NORMAL_STYLE		(0)
#define FAT_STYLE			(1)
#define HIGH_STYLE		(2)
#define FAT_HIGH_STYLE	(3)
#define STYLES_MAX		(4)

/* Justification types */

#define PRINT_LEFT		(0)
#define PRINT_CENTERED	(1)
#define PRINT_RIGHT		(2)
#define PRINT_JUSTIFIED	(3)

/* Error codes */

#define TEXTERR_TEXT_TOO_LONG				(1)
#define TEXTERR_FONT_STACK_OVERFLOW		(2)
#define TEXTERR_FONT_STACK_UNDERFLOW	(3)

#define TEXTERR_MAX							(3)

/*
 ** Type definitions *******************************************************
 */

typedef void (*LLC_command) (UNCHAR*, UNSHORT*, UNSHORT*);
typedef void (*LLCA_command) (UNCHAR*, UNSHORT*, UNSHORT*, struct Line_info *);
typedef void (*HLC_command) (UNCHAR*);

/*
 ** Structure definitions **************************************************
 */

struct Textstyle {
	UNSHORT Ink;
	UNSHORT Shadow;
	UNSHORT Paper;
	struct Font *Font;
	UNBYTE Justification;
	UNBYTE Style;
};

struct PA {
	struct BBRECT Area;
	struct Textstyle PA_textstyle;
	MEM_HANDLE Background_handle;
};

struct Text_command {
	UNLONG Command;				/* This is actually UNCHAR Command[4] */
	union {
		LLC_command xLLC;
		LLCA_command xLLCA;
		HLC_command xHLC;
	} xCommand_handler;
};

#define LLC		xCommand_handler.xLLC
#define LLCA	xCommand_handler.xLLCA
#define HLC		xCommand_handler.xHLC

struct Kerning_pair {
	UNBYTE First;
	UNBYTE Second;
};

struct Font {
	struct Fontstyle* Styles[STYLES_MAX];
};

struct Fontstyle {
	UNSHORT Raw_char_height;	/* Height of characters in pixels */
	UNSHORT Raw_char_size;		/* Number of bytes per character in font */
	UNSHORT Between_width;		/* Number of pixels between characters */
	UNSHORT Between_height;		/* Number of pixels between lines */
	UNSHORT Base_line;  			/* Y-position of base line */
	UNSHORT Width_of_space;		/* Number of pixels per space */
	UNBYTE *Font_graphics;		/* Address of font graphics */
	struct Kerning_pair *Kerning_table;	/* Address of paired kerning table / NULL */
	UNBYTE *Translation;			/* Address of ASCII-to-character-index table */
	UNBYTE *Width_table;			/* Address of character width table */
	void (*Insert_char)(void);	/* Address of character insertion function */
};

struct Line_info {
	struct Textstyle Initial_style;
	UNSHORT Line_width;
	UNSHORT Width_without_spaces;
	UNBYTE Line_height;
	UNBYTE Line_skip;
	UNBYTE Base_line;
	UNBYTE Nr_of_spaces;
	UNBYTE String_length;
};

struct Processed_text {
	UNSHORT Nr_of_lines;
	UNSHORT Text_height;
	UNSHORT Nr_processed_TPBs;
	MEM_HANDLE Processed_TPB_handles[TPB_MAX];
};

/*
 ** Prototypes *************************************************************
 */

void Text_error(UNSHORT Error_code);
void Text_print_error(UNCHAR *buffer, UNBYTE *data);

void Reset_font_stack(void)
void Push_font (struct Font *New);
void Pop_font (void);
void Change_font (struct Font *New);
void Init_font (struct Font *Font);

void Process_text(UNCHAR *Text, struct Processed_text *Processed);
void Process_text_list(UNCHAR **Text_list, struct Processed_text *Processed);
UNCHAR *Find_wrap_position(UNCHAR *Text);
void Write_processed_line(UNCHAR *End, struct Processed_text *Processed);
UNCHAR *Analyze_line(UNCHAR *Text, struct Line_info *Current, struct Processed_text *Processed);
void Read_raw_line(void);
UNCHAR *LLC_handler(UNCHAR *Text, struct Text_command *Command_list,
 UNSHORT *Screen_X, UNSHORT *Buffer_X);
UNCHAR *LLCA_handler(UNCHAR *Text, UNSHORT *Line_width, UNSHORT *Line_width2,
 struct Line_info *Current);

#endif

