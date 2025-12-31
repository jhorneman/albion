/************
 * NAME     : AUTOMAP.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef AUTOMAP_H
#define AUTOMAP_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define AUTOMAP_PAL_NR	(21)

#define AUTOMAP_BUFFER_WIDTH		(45)	/* Scroll margin not included ! */
#define AUTOMAP_BUFFER_HEIGHT		(24)

/* Automap playfield graphics */
#define NR_AUTOMAP_TOP_EDGE		(4)
#define NR_AUTOMAP_BOTTOM_EDGE	(4)
#define NR_AUTOMAP_LEFT_EDGE		(3)
#define NR_AUTOMAP_RIGHT_EDGE		(3)
#define NR_AUTOMAP_SURFACE			(5)

/* Automap playfield graphics offsets (measured in TILES) */
#define AUTOMAP_TOP_LEFT_CORNER		(0)
#define AUTOMAP_TOP_RIGHT_CORNER		(AUTOMAP_TOP_LEFT_CORNER + 64)
#define AUTOMAP_BOTTOM_LEFT_CORNER	(AUTOMAP_TOP_RIGHT_CORNER + 64)
#define AUTOMAP_BOTTOM_RIGHT_CORNER	(AUTOMAP_BOTTOM_LEFT_CORNER + 64)
#define AUTOMAP_TOP_EDGE				(AUTOMAP_BOTTOM_RIGHT_CORNER + 64)
#define AUTOMAP_BOTTOM_EDGE			(AUTOMAP_TOP_EDGE + NR_AUTOMAP_TOP_EDGE * 16)
#define AUTOMAP_LEFT_EDGE				(AUTOMAP_BOTTOM_EDGE + NR_AUTOMAP_BOTTOM_EDGE * 16)
#define AUTOMAP_RIGHT_EDGE				(AUTOMAP_LEFT_EDGE + NR_AUTOMAP_LEFT_EDGE * 16)
#define AUTOMAP_SURFACE					(AUTOMAP_RIGHT_EDGE + NR_AUTOMAP_RIGHT_EDGE * 16)
#define AUTOMAP_WALLS					(AUTOMAP_SURFACE + NR_AUTOMAP_SURFACE * 16)

/* Automap graphics offsets (measured in BYTES) */
#define AUTOMAP_ICONS					((AUTOMAP_WALLS + 16) * 64)
#define AUTOMAP_YAH_TOP					(AUTOMAP_ICONS + (45 * 256))
#define AUTOMAP_YAH_BASE				(AUTOMAP_YAH_TOP + 256)
#define AUTOMAP_YAH_ARROWS				(AUTOMAP_YAH_BASE + 256)

/* Automapper icons */
#define WALL_AUTO_ICON				(1)
#define RIDDLEMOUTH_AUTO_ICON		(2)
#define TELEPORTER_AUTO_ICON		(3)
#define SPINNER_AUTO_ICON			(4)
#define TRAP_AUTO_ICON				(5)
#define TRAPDOOR_AUTO_ICON			(6)
#define SPECIAL_AUTO_ICON			(7)
#define MONSTER_AUTO_ICON			(8)
#define CLOSED_DOOR_AUTO_ICON		(9)
#define OPEN_DOOR_AUTO_ICON		(10)
#define MERCHANT_AUTO_ICON			(11)
#define INN_AUTO_ICON				(12)
#define CLOSED_CHEST_AUTO_ICON	(13)
#define MAP_EXIT_AUTO_ICON			(14)
#define OPEN_CHEST_AUTO_ICON		(15)
#define TRASHPILE_AUTO_ICON		(16)
#define PERSON_AUTO_ICON			(17)
#define GOTO_POINT_AUTO_ICON		(18)

#define MAX_AUTO_ICONS				(18)

/* Automapper function flags */
#define SHOW_NPCS_FUNCTION			(1<<0)
#define SHOW_MONSTERS_FUNCTION 	(1<<1)
#define SHOW_TRAPS_FUNCTION 		(1<<2)
#define SHOW_HIDDEN_FUNCTION 		(1<<3)

/* Automapper CA parameters */
#define AMCA_OFFSET			(5)
#define AMCA_WIDTH			(2 * AMCA_OFFSET + 1)

/* Automapper CA values */
#define AMCA_VOID			(0)		/* Raw void : before CA initialization */
#define AMCA_EMPTY		(1)		/* Not blocking vision, not checked */
#define AMCA_OUTSIDE		(2)		/* Outside of the map */
#define AMCA_BLOCKED		(3)		/* Blocking vision, not checked */
#define AMCA_VISIBLE		(4)		/* Visible, checked */
#define AMCA_EDGE			(5)		/* Edge, blocking vision, checked */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Enter automap function */
void Enter_Automap(void);
BOOLEAN Do_enter_automap(struct Event_action *Action);

/* Automap module functions */
void Automap_ModInit(void);
void Automap_ModExit(void);
void Init_Automap_display(void);
void Exit_Automap_display(void);
void Display_automap(void);

/* Automap methods */
void Touched_Automap_object(struct Object *Object, union Method_parms *P);
void Right_Automap_object(struct Object *Object, union Method_parms *P);

/* Automap support functions */
UNSHORT Get_Automap_mouse_state(SISHORT X, SISHORT Y);
void Move_automap(SISHORT dX, SISHORT dY);
UNSHORT Find_goto_point(SISHORT X, SISHORT Y);

/* Automap scroll update function */
void Update_automap_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y);

/* Automap management functions */
void Init_automap(void);
void Exit_automap(void);
void Save_automap(void);
void Update_automap(void);

/* Automap access functions */
BOOLEAN Get_automap_bit(SISHORT X, SISHORT Y);
void Set_automap_bit(SISHORT X, SISHORT Y);

/* Automap display preparation functions */
void Create_automap_parchment_layer(void);

void Prepare_automap_floors(void);
void Shrink_floor(struct BBPALETTE *Source_pal, UNBYTE *Input, UNBYTE *Output);

void Insert_automap_walls_and_floors(void);
void Insert_automap_events(void);
void Insert_automap_NPCs(void);
void Insert_automap_goto_points(void);

void Insert_automap_wall(SISHORT X, SISHORT Y);
void Insert_automap_icon(SISHORT X, SISHORT Y, UNSHORT Icon_nr);

#endif

