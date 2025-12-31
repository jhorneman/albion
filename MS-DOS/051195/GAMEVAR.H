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
#include <BBMEM.H>
#include <GAME.H>

/*
 ** Macro definitions ******************************************************
 */

#ifdef DEBUG

#define Error(a) Report_error(a, __LINE__, __FILE__)

#else

#define Error(a) Report_error(a, 0, NULL)

#endif

/*
 ** Defines ****************************************************************
 */

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
#define ERROR_ILLEGAL_COMOB					(13)
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
#define ERROR_ILLEGAL_SAVE_GAME				(26)
#define ERROR_ILLEGAL_SAVE_GAME_VERSION	(27)
#define ERROR_TOO_MANY_ITEMS_IN_PACKET		(28)
#define ERROR_SETUP_INI_ACCESS_ERROR		(29)
#define ERROR_SAVED_GAME_ACCESS_ERROR		(30)
#define ERROR_TOO_MANY_3D_WALLS				(31)
#define ERROR_TOO_MANY_3D_FLOORS				(32)
#define ERROR_TOO_MANY_3D_OBJECTS			(33)
#define ERROR_TOO_MANY_3D_TEXTURES			(34)
#define ERROR_ILLEGAL_X_COORDINATE			(35)
#define ERROR_ILLEGAL_Y_COORDINATE			(36)

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

extern BOOLEAN Quit_program_flag;
extern BOOLEAN Loading_game;
extern BOOLEAN Restart_game_flag;
extern BOOLEAN Game_over_flag;
extern BOOLEAN Cheat_mode;
extern BOOLEAN Diagnostic_mode;
extern BOOLEAN Super_VGA;
extern BOOLEAN No_load_flag;

extern UNSHORT Language;

extern UNSHORT Load_game_nr;

extern UNSHORT Screen_width, Screen_height, Panel_Y;
extern UNSHORT Forbidden_X, Forbidden_Y;

extern UNSHORT Earth_object;

extern const double PI;

extern struct File_type Background_buffer_ftype;

/*
 ** Prototypes *************************************************************
 */

void BBMAIN(UNSHORT argc, const UNCHAR **argv);

void Exit_program(void);

BOOLEAN Init_screen(void);
void Exit_screen(void);

void Determine_screen_size(void);

BOOLEAN Open_screen(void);
void Close_screen(void);

/* Error functions */
void Report_error(UNSHORT Error_code, int Line_nr, char *Filename);
void Print_error(UNCHAR *buffer, UNBYTE *data);

#endif
