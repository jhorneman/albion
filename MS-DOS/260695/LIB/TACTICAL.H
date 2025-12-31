/************
 * NAME     : TACTICAL.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 25-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef TACTICAL_H
#define TACTICAL_H

#include <BBDEF.H>
#include <BBOPM.H>

/*
 ** Defines ****************************************************************
 */

/* Tactical icon size */
#define TACTICAL_ICON_WIDTH		(32)
#define TACTICAL_ICON_HEIGHT		(48)

/* Tactical window square size */
#define TACTICAL_SQUARE_WIDTH		(32)
#define TACTICAL_SQUARE_HEIGHT	(24)

/* Number of columns and rows in tactical array */
#define NR_TACTICAL_COLUMNS		(6)
#define NR_TACTICAL_ROWS			(5)
#define NR_TACTICAL_SQUARES		(NR_TACTICAL_COLUMNS * NR_TACTICAL_ROWS)

/* Tactical window coordinates and size */
#define TACTICAL_WIDTH		(NR_TACTICAL_COLUMNS * TACTICAL_SQUARE_WIDTH + 2 + 14 + 3)
#define TACTICAL_HEIGHT		(NR_TACTICAL_ROWS * TACTICAL_SQUARE_HEIGHT + 2 + 24 + 7 + 3)
#define TACTICAL_X			(COMBAT_X + (COMBAT_WIDTH - TACTICAL_WIDTH) / 2)
#define TACTICAL_Y			((COMBAT_Y + (COMBAT_HEIGHT - (TACTICAL_HEIGHT - 24)) / 2) - 24)

/* Tactical window draw modes */
#define TACTICAL_NORMAL_DRAWMODE	 		(0)
#define TACTICAL_FEEDBACK_DRAWMODE		(1)
#define TACTICAL_HIGHLIGHT_DRAWMODE		(2)

/*
 ** Type definitions *******************************************************
 */

typedef void (*Tactical_display_handler) (UNSHORT X, UNSHORT Y);

typedef void (*Trap_handler) (struct Combat_participant *Victim_part,
 UNSHORT Strength);

/*
 ** Structure definitions **************************************************
 */

/* Tactical square */
struct Tactical_square {
	struct Combat_participant *Part;	/* Pointer to participant data */
	Trap_handler Trap_handler;			/* Pointer to trap handler */
	UNSHORT Trap_strength;				/* Trap strength */
};

/* Tactical square OID*/
struct Tactical_square_OID {
	UNSHORT X, Y;
};

/* Tactical square object */
struct Tactical_square_object {
	struct Object Object;
	UNSHORT X, Y;
};

/*
 ** External global variables **********************************************
 */

extern struct Object_class Tactical_window_Class;

extern struct Tactical_square Combat_matrix[NR_TACTICAL_ROWS][NR_TACTICAL_COLUMNS];

/*
 ** Prototypes *************************************************************
 */

/* Tactical window methods */
UNLONG Init_Tactical_window_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Tactical_window_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Tactical_window_object(struct Object *Object, union Method_parms *P);
//UNLONG Customkeys_Tactical_window_object(struct Object *Object, union Method_parms *P);

/* Tactical square methods */
UNLONG Init_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Right_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Touch_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Tactical_square_object(struct Object *Object, union Method_parms *P);
UNLONG Pop_up_Tactical_square_object(struct Object *Object, union Method_parms *P);

/* Tactical square pop-up menu actions */
void Tactical_square_PUM_evaluator(struct PUM *PUM);
void PUM_Attack_tactical_square(UNLONG Data);
void PUM_Move_tactical_square(UNLONG Data);
void PUM_Cast_spell_tactical_square(UNLONG Data);
void PUM_Use_magic_item_tactical_square(UNLONG Data);
void PUM_Flee_tactical_square(UNLONG Data);
void PUM_Advance_tactical_square(UNLONG Data);
void PUM_Start_round(UNLONG Data);
void PUM_View_combat(UNLONG Data);

/* Tactical window display functions */
void Draw_tactical_window(void);
void Draw_tactical_squares(UNSHORT X, UNSHORT Y);
void Draw_tactical_icons(UNSHORT X, UNSHORT Y);

/* Tactical window background management functions */
void Init_tactical_window_background(void);
void Exit_tactical_window_background(void);
void Get_tactical_window_background(void);
void Restore_tactical_window_background(UNSHORT X, UNSHORT Y, UNSHORT Width,
 UNSHORT Height);

/* Tactical window square select function */
UNSHORT Select_tactical_square(UNSHORT Message_nr,
 Tactical_display_handler Handler, UNLONG Mask, UNLONG Mask2);

void Show_potential_attack_targets(UNSHORT X, UNSHORT Y);
void Show_potential_move_targets(UNSHORT X, UNSHORT Y);
void Show_potential_row_targets(UNSHORT X, UNSHORT Y);

#endif

