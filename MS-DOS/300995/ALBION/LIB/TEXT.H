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

#include <BBDEF.H>
#include <BBOPM.H>

/*
 ** Defines ****************************************************************
 */

#define STANDARD_TEXT_HEIGHT	(9)

#define MAX_LINE_LENGTH	(180)

/* Text display flags */
#define WAIT_TEXT_DISPLAY_FLAG	(1 << 0)
#define CLEAR_TEXT_DISPLAY_FLAG	(1 << 1)

/* Special text codes */
#define CR					'^'
#define COMSTART_CHAR	'{'
#define COMEND_CHAR		'}'

#define SPACE				' '
#define SOLID_SPACE		'_'
#define PERIOD				'.'
#define HYPHEN1			'-'		/* MUST be REAL hyphen ! */
#define HYPHEN2			'/'
#define SEPARATOR			'|'
#define EOTPB				'~'

/* Text styles */
#define NORMAL_STYLE		(0)
#define FAT_STYLE			(1)
#define HIGH_STYLE		(2)
#define FAT_HIGH_STYLE	(3)
#define BIG_STYLE			(4)
#define MAX_STYLES		(5)

/* Justification types */
#define PRINT_LEFT		(0)
#define PRINT_CENTERED	(1)
#define PRINT_RIGHT		(2)
#define PRINT_JUSTIFIED	(3)

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* High-level text commands */
struct HLC_Text_command {
	UNCHAR Command[4];
	UNCHAR * (*Command_handler)(UNCHAR *);
};

/* Kerning pairs */
struct Kerning_pair {
	UNBYTE First;
	UNBYTE Second;
};

/* Text styles */
struct Textstyle {
	UNSHORT Ink;
	UNSHORT Shadow;
	UNSHORT Paper;
	struct Font *Font;
	UNBYTE Justification;
	UNBYTE Style;
};

/* Print areas */
struct PA {
	struct BBRECT Area;
};

/* Fonts */
struct Font {
	struct Fontstyle *Styles[MAX_STYLES];
};

/* Font styles */
struct Fontstyle {
	UNSHORT Raw_width;			/* Width of characters in pixels */
	UNSHORT Raw_height;			/* Height of characters in pixels */
	UNSHORT Raw_size;				/* Size of one character in bytes */
	UNSHORT Between_width;		/* Number of pixels between characters */
	UNSHORT Between_height;		/* Number of pixels between lines */
	UNSHORT Base_line;  			/* Y-coordinate of base line */
	UNSHORT Width_of_space;		/* Number of pixels per space */
	UNCHAR *Kerning_table;		/* Address of paired kerning table / NULL */
	UNBYTE *Translation;			/* Address of ASCII-to-character-index table */
	UNBYTE *Width_table;			/* Address of character width table */
	MEM_HANDLE Graphics_handle;	/* Memory handle of font graphics / NULL */
	UNLONG Graphics_offset;		/* Offset to font graphics */
	void (*Print_char)(struct OPM *, SISHORT, SISHORT, struct Fontstyle *,
	 UNSHORT, UNSHORT, UNSHORT);	/* Address of character print function */
};

/* Processed texts */
struct Processed_text {
	UNSHORT Nr_of_lines;
	UNLONG Text_height;
	MEM_HANDLE First_handle;
};

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

void Destroy_text_cache(void);

/* Textstyle management functions */
void Reset_textstyle_stack(void);
void Push_textstyle(struct Textstyle *New);
void Pop_textstyle(void);
void Change_textstyle(struct Textstyle *New);

/* Print Area management functions */
void Reset_PA_stack(void);
void Push_PA(struct PA *New);
void Pop_PA(void);
void Change_PA(struct PA *New);
void Init_PA(struct PA *PA);

/* Text analysis and display functions */
void Get_text_size(UNCHAR *Text, UNSHORT *Nr_lines_ptr,
 UNLONG *Text_height_ptr);

void Process_and_display_text(struct OPM *OPM, UNCHAR *Text,
 UNSHORT Display_flags);

#define Display_text(a, b) Process_and_display_text(a, b, 0)
#define Display_text_and_wait(a, b) Process_and_display_text(a, b, WAIT_TEXT_DISPLAY_FLAG)

/* Text display support functions */
void Destroy_processed_text(struct Processed_text *Processed);
void Display_processed_text(struct OPM *OPM,
 struct Processed_text *Processed, UNSHORT Display_flags);
void Refresh_text_window(struct OPM *OPM, SISHORT Top, SISHORT Bottom,
 SILONG Current_text_Y, struct Processed_text *Processed);

/* Print area support functions */
void Erase_print_area(struct OPM *OPM);
void Init_print_area(struct OPM *OPM);
void Exit_print_area(void);

/* Print functions */
void xprintf(struct OPM *OPM, SISHORT X, SISHORT Y, UNCHAR *Format, ...);

void Print_string(struct OPM *OPM, SISHORT X, SISHORT Y, UNCHAR *String);

void Print_centered_string(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNCHAR *String);

void Print_right_string(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNCHAR *String);

void Print_centered_line_string(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Width, UNCHAR *String);

void Print_centered_box_string(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Width, UNSHORT Height, UNCHAR *String);

/* Character printing functions */
void Print_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow);
void Print_fat_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow);
void Print_high_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow);
void Print_fat_high_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow);
void Print_big_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow);

/* Text line analysis functions */
UNSHORT Get_line_width(UNCHAR *Text);
void Get_line_size(UNCHAR *Text, UNSHORT *Width, UNSHORT *Height);

/* Text processing functions */
void Process_text(UNCHAR *Text, struct Processed_text *Processed);
void Process_text_list(UNCHAR **Text_list, struct Processed_text *Processed);

#endif

