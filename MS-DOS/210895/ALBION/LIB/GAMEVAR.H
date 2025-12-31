/************
 * NAME     : GAMEVAR.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GAMEVAR_H
#define GAMEVAR_H

#include <BBDEF.H>
#include <GAME.H>

/*
 ** Defines ****************************************************************
 */

#define Error(a) Report_error(a, 0, NULL)

#define MAX_SYSTEM_TEXTS		(600)

/* Error codes */
#define ERROR_OUT_OF_MEMORY					(1)
#define ERROR_NO_OPM								(2)
#define ERROR_NO_SCREEN							(3)
#define ERROR_FILE_LOAD							(4)
#define ERROR_ITEM_NO_SWAP_BACK	 			(5)
#define ERROR_AUTOMAP_FILE						(6)
#define ERROR_CLONE_SAME_MAPS					(7)
#define ERROR_CLONE_MAPS_NOT_SAME_SIZE		(8)
#define ERROR_CLONE_MAPS_NOT_SAME_TYPE		(9)
#define ERROR_SCROLL_BAR_WRONG_PARAMETERS	(10)
#define ERROR_ILLEGAL_PROJECTILE_TYPE		(11)
#define ERROR_OUT_OF_COMOBS					(12)
#define ERROR_NULL_COMOB_PASSED				(13)
#define ERROR_OUT_OF_COMOB_BEHAVIOURS		(14)
#define ERROR_3D_COLOUR_CYCLING_IN_2D_MAP	(15)
#define ERROR_BIT_ARRAY_ACCESS_TYPE			(16)
#define ERROR_BIT_ARRAY_TYPE					(17)
#define ERROR_BIT_ARRAY_OUT_OF_RANGE		(18)
#define ERROR_ILLEGAL_DIALOGUE_RECURSION	(19)
#define ERROR_ILLEGAL_COMBAT_RECURSION		(20)
#define ERROR_NO_PLACE_IN_PARTY				(21)
#define ERROR_BIT_ARRAY_ILLEGAL_ARRAY		(22)
#define ERROR_EMPTY_MONSTER_GROUP			(23)
#define ERROR_STRANGE_LBM						(24)
#define ERROR_ILLEGAL_COMOB_FRAME			(25)

/*
 ** Structure definitions **************************************************
 */

struct Error_message {
	UNSHORT Code;
	UNCHAR Message[40];
};

struct Error_report {
	UNSHORT Code;
	UNSHORT Value1, Value2;
	UNSHORT Line_nr;
	UNCHAR *Filename;
	struct Error_message *Messages;
};

/*
 ** External global variables **********************************************
 */

extern struct OPM Main_OPM;
extern struct OPM Slab_OPM;
extern struct OPM Slab_backup_OPM;

extern struct SCREENPORT Screen;

extern BOOLEAN Quit_program;
extern BOOLEAN Terminal_condition;
extern BOOLEAN Cheat_mode;
extern BOOLEAN Diagnostic_mode;
extern BOOLEAN Super_VGA;
extern BOOLEAN Bumped;
extern BOOLEAN Moved;

extern UNSHORT Nr_auto_notes;

extern UNSHORT Language;

extern UNSHORT Screen_width, Screen_height, Panel_Y;
extern UNSHORT Forbidden_X, Forbidden_Y;

extern UNSHORT Earth_object;

extern const double PI;

/*
 ** Prototypes *************************************************************
 */

void BBMAIN(UNSHORT argc, const UNCHAR **argv);
void Exit_program(void);

void Evaluate_game_arguments(UNSHORT argc, UNCHAR **argv);

BOOLEAN Init_screen(void);
void Exit_screen(void);

/* Error functions */
void Report_error(UNSHORT Error_code, int Line_nr, char *Filename);
void Print_error(UNCHAR *buffer, UNBYTE *data);

#endif
