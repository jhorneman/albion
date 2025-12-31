/************
 * NAME     : EVENTS.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef EVENTS_H
#define EVENTS_H

#include <BBDEF.H>
#include <EVELOGIC.H>

/*
 ** Defines ****************************************************************
 */

#define SHORTCUT_MAP_NR			(199)

/* Event types */
#define MAP_EXIT_EVENT			(1)
#define DOOR_EVENT				(2)
#define CHEST_EVENT				(3)
#define TEXT_EVENT				(4)
#define SPINNER_EVENT			(5)
#define TRAP_EVENT				(6)
#define CHANGE_USED_ITEM_EVENT	(7)
#define DATACHANGE_EVENT		(8)
#define CHANGE_ICON_EVENT		(9)
#define ENCOUNTER_EVENT			(10)
#define PLACE_ACTION_EVENT		(11)
#define QUERY_EVENT				(12)
#define MODIFY_EVENT				(13)
#define ACTION_EVENT				(14)
#define SIGNAL_EVENT				(15)
#define CLONE_AUTOMAP_EVENT	(16)
#define SOUND_EVENT				(17)
#define START_DIALOGUE_EVENT	(18)
#define CREATE_TRANS_EVENT		(19)
#define EXECUTE_EVENT			(20)
#define REMOVE_MEMBER_EVENT	(21)
#define END_DIALOGUE_EVENT		(22)
#define WIPE_EVENT				(23)
#define PLAY_ANIMATION_EVENT	(24)
#define OFFSET_EVENT				(25)
#define PAUSE_EVENT				(26)
#define SINGLE_ITEM_CHEST_EVENT		(27)
#define ASK_SURRENDER_EVENT	(28)
#define DO_SCRIPT_EVENT			(29)

#define MAX_EVENT_TYPES			(29)

/* Map exit event types */
#define NORMAL_MX_TYPE				(0)
#define TELEPORTER_MX_TYPE			(1)
#define TRAPDOOR_UP_MX_TYPE  		(2)
#define JUMP_MX_TYPE					(3)
#define GAME_OVER_MX_TYPE			(4)
#define TRAPDOOR_DOWN_MX_TYPE		(5)
#define FALL_DOWN_MX_TYPE 			(6)
#define NORMAL_X_REL_MX_TYPE		(8)
#define NORMAL_Y_REL_MX_TYPE		(9)
#define NORMAL_XY_REL_MX_TYPE		(10)

/* Text event types */
#define NORMAL_TEXT_TYPE							(0)
#define VISUAL_TEXT_TYPE							(1)
#define PARTY_SPOKEN_TEXT_TYPE					(2)
#define NPC_SPOKEN_TEXT_TYPE						(3)
#define MULTI_CHOICE_TEXT_TYPE					(4)
#define PLACE_TITLE_TEXT_TYPE						(5)
#define PERMANENT_TEXT_TYPE						(6)
#define COMBAT_TEXT_TYPE							(7)
#define SLOW_COMBAT_TEXT_TYPE						(8)
#define MULTI_CHOICE_NO_STANDARD_TEXT_TYPE	(9)
#define ACTIVE_SPOKEN_TEXT_TYPE					(10)
#define MULTI_CHOICE_DEFAULT_TEXT_TYPE			(11)
#define MULTI_CHOICE_DEFAULT_NO_STANDARD_TEXT_TYPE	(12)
#define REPLACE_DEFAULT_MULTI_CHOICE_TYPE		(13)

/* Sound event types */
#define MAIN_SONG_SOUND_TYPE			(0)
#define EFFECT_SOUND_TYPE				(1)
#define AMBIENT_SONG_SOUND_TYPE		(2)
#define JINGLE_SOUND_TYPE				(3)
#define LOCATED_EFFECT_SOUND_TYPE	(4)

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern UNSHORT Shortcut_map_X;
extern UNSHORT Shortcut_map_Y;

extern Event_handler Event_handlers[MAX_EVENT_TYPES];

/*
 ** Prototypes *************************************************************
 */

#endif

