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

/*
 ** Defines ****************************************************************
 */

#define PA_MAX				(12)
#define TEXTSTYLES_MAX	(8)

#define MINIMUM_SPACE	(2)

#define STANDARD_TEXT_HEIGHT	(8)

#define LINE_LENGTH_MAX	(180)
#define TPB_SIZE			(512)		/* Includes sizeof(struct TPB) */

/* Special text codes */

#define CR					'^'
#define COMSTART_CHAR	'{'
#define COMEND_CHAR		'}'

#define SPACE				' '
#define SOLID_SPACE		'_'

#define HYPHEN1			'-'		/* MUST be REAL hyphen ! */
#define HYPHEN2			'/'
#define SEPARATOR			'|'
#define EOTPB				'~'

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
	struct Textstyle PA_textstyle;
};

/* Text commands */
struct Text_command {
	UNCHAR Command[4];
	void (*Command_handler) (UNCHAR*);
};

/* Kerning pairs */
struct Kerning_pair {
	UNBYTE First;
	UNBYTE Second;
};

/* Fonts */
struct Font {
	struct Fontstyle* Styles[STYLES_MAX];
};

/* Font styles */
struct Fontstyle {
	UNSHORT Raw_width;			/* Width of characters in pixels */
	UNSHORT Raw_height;			/* Height of characters in pixels */
	UNSHORT Raw_size;				/* Size of one character in bytes */
	UNSHORT Between_width;		/* Number of pixels between characters */
	UNSHORT Between_height;		/* Number of pixels between lines */
	UNSHORT Base_line;  			/* Y-position of base line */
	UNSHORT Width_of_space;		/* Number of pixels per space */
	UNCHAR *Kerning_table;		/* Address of paired kerning table / NULL */
	UNBYTE *Translation;			/* Address of ASCII-to-character-index table */
	UNBYTE *Width_table;			/* Address of character width table */
	MEM_HANDLE Graphics_handle;	/* Memory handle of font graphics */
	UNLONG Graphics_offset;		/* Offset to font graphics */
	void (*Print_char)(struct OPM *, SISHORT, SISHORT, struct Fontstyle *,
	 UNSHORT, UNSHORT, UNSHORT);	/* Address of character print function */
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

/* Processed texts */
struct Processed_text {
	UNSHORT Nr_of_lines;
	UNLONG Text_height;
	MEM_HANDLE First_handle;
};

/* Text processing blocks */
struct TPB {
	MEM_HANDLE Next_handle;
	UNSHORT Height;
};

/*
 ** Prototypes *************************************************************
 */

void Text_error(UNSHORT Error_code);
void Text_print_error(UNCHAR *buffer, UNBYTE *data);

void Reset_textstyle_stack(void);
void Push_textstyle(struct Textstyle *New);
void Pop_textstyle(void);
void Change_textstyle(struct Textstyle *New);
void Init_textstyle(void);
void Exit_textstyle(void);

void Reset_PA_stack(void);
void Push_PA(struct PA *New);
void Pop_PA(void);
void Change_PA(struct PA *New);
void Init_PA(struct PA *PA);
void Exit_PA(struct PA *PA);

void Display_text(struct OPM *OPM, UNCHAR *Text);
void Display_text_list(struct OPM *OPM, UNCHAR **Text_list);

void Destroy_processed_text(struct Processed_text *Processed);
void Display_processed_text(struct OPM *OPM, struct Processed_text *Processed);
void Refresh_text_window(struct OPM *OPM, SISHORT Top, SISHORT Bottom,
 struct Processed_text *Processed);

void Erase_print_area(struct OPM *OPM);
void Prepare_print_area(struct OPM *OPM);

void Print_string(struct OPM *OPM, SISHORT X, SISHORT Y, UNCHAR *String);
void Print_processed_string(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNCHAR *String);
void Print_normal_string(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNCHAR *String);
void Print_justified_string(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNCHAR *String);
BOOLEAN Prepare_justification(struct Line_info *Line);

void Print_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow);
void Print_fat_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow);
void Print_high_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow);

UNSHORT Get_line_width(UNCHAR *Text);
void Get_line_size(UNCHAR *Text, UNSHORT *Width, UNSHORT *Height);

UNCHAR *LLC_handler(UNCHAR *Text, struct Text_command *Command_list);
UNSHORT Get_LLC_nr(UNCHAR *Text);

void LLC_Set_ink(UNCHAR *Text);
void LLC_Set_shadow(UNCHAR *Text);
void LLC_No_shadow(UNCHAR *Text);
void LLC_Set_normal_style(UNCHAR *Text);
void LLC_Set_fat_style(UNCHAR *Text);
void LLC_Set_high_style(UNCHAR *Text);
void LLC_Set_fat_high_style(UNCHAR *Text);
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

void Process_text(UNCHAR *Text, struct Processed_text *Processed);
void Process_text_list(UNCHAR **Text_list, struct Processed_text *Processed);
UNCHAR *Find_wrap_position(UNCHAR *Text);
void Write_processed_line(UNCHAR *End, struct Processed_text *Processed);
UNCHAR *Analyze_line(UNCHAR *Text, struct Processed_text *Processed);
void Read_raw_line(void);

void Put_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour);
void Put_fat_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour);
void Put_high_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour);

#endif

