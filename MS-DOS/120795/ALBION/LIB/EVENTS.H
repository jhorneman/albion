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
#define END_SEQUENCE_MX_TYPE		(4)
#define TRAPDOOR_DOWN_MX_TYPE		(5)
#define FALL_DOWN_MX_TYPE 			(6)
#define CHAPTER_SCREEN_MX_TYPE	(7)
#define NORMAL_X_REL_MX_TYPE		(8)
#define NORMAL_Y_REL_MX_TYPE		(9)
#define NORMAL_XY_REL_MX_TYPE		(10)

/* Text event types */
#define NORMAL_TEXT_TYPE				(0)
#define VISUAL_TEXT_TYPE				(1)
#define PARTY_SPOKEN_TEXT_TYPE		(2)
#define NPC_SPOKEN_TEXT_TYPE			(3)
#define MULTI_CHOICE_TEXT_TYPE		(4)
#define PLACE_TITLE_TEXT_TYPE			(5)
#define PERMANENT_TEXT_TYPE			(6)
#define COMBAT_TEXT_TYPE				(7)
#define SLOW_COMBAT_TEXT_TYPE			(8)
#define MULTI_CHOICE_NO_STANDARD_TEXT_TYPE	(9)
#define ACTIVE_SPOKEN_TEXT_TYPE					(10)
#define MULTI_CHOICE_DEFAULT_TEXT_TYPE			(11)

/* Datachange data types */
#define ATTRIBUTE_DC_TYPE			(0)
#define SKILL_DC_TYPE				(1)
#define LP_DC_TYPE					(2)
#define SP_DC_TYPE					(3)
#define APR_DC_TYPE					(4)
#define CONDITIONS_DC_TYPE			(5)
#define LEVEL_DC_TYPE				(6)
#define LANGUAGES_DC_TYPE			(7)
#define EP_DC_TYPE					(8)
#define TP_DC_TYPE					(9)
#define PORTRAIT_DC_TYPE			(10)
#define FULL_BODY_PIC_DC_TYPE		(11)
#define EVENT_SET_1_DC_TYPE		(12)
#define EVENT_SET_2_DC_TYPE		(13)
#define GFX_2D_DC_TYPE				(14)
#define TACTICAL_ICON_DC_TYPE		(15)
#define SPELL_DC_TYPE				(16)
#define MAX_LP_DC_TYPE				(17)
#define MAX_SP_DC_TYPE				(18)
#define ITEMS_DC_TYPE				(19)
#define GOLD_DC_TYPE					(20)
#define FOOD_DC_TYPE					(21)
#define MORALE_DC_TYPE				(22)

/* Datachange change types */
#define ACTIVE_MEMBER_DC			(0)
#define ENTIRE_PARTY_DC				(1)
#define SINGLE_MEMBER_DC			(2)
#define CURRENT_PART_DC				(3)
#define CURRENT_PART_VICTIM_DC	(4)
#define NPC_DC							(5)

/* Sound event types */
#define MAIN_SONG_SOUND_TYPE			(0)
#define EFFECT_SOUND_TYPE				(1)
#define AMBIENT_SONG_SOUND_TYPE		(2)
#define JINGLE_SOUND_TYPE				(3)
#define LOCATED_EFFECT_SOUND_TYPE	(4)

/* Query and Modify types */
#define QUEST_BIT_QM_TYPE					(0)
#define EVENT_SAVE_BIT_QM_TYPE			(1)
#define DOOR_BIT_QM_TYPE					(2)
#define CHEST_BIT_QM_TYPE					(3)
#define CD_BIT_QM_TYPE						(4)
#define CHAR_IN_PARTY_QM_TYPE				(5)
#define ITEM_CARRIED_QM_TYPE				(6)
#define ITEM_USED_QM_TYPE					(7)
#define WORD_BIT_QM_TYPE					(8)
#define SUCCESS_QM_TYPE						(9)
#define INTERNAL_QM_TYPE					(10)
#define LIGHT_STATE_QM_TYPE				(11)
#define DIRECTION_QM_TYPE					(12)
#define WORD_SPOKEN_QM_TYPE				(13)
#define TRIGGERING_NPC_QM_TYPE			(14)
#define GOLD_CARRIED_QM_TYPE				(15)
#define FOOD_CARRIED_QM_TYPE				(16)
#define CHANCE_QM_TYPE						(17)
#define HOUR_QM_TYPE							(18)
#define ACTIVE_LEVEL_QM_TYPE				(19)
#define TRIGGER_MODE_QM_TYPE				(20)
#define MEMBER_CONSCIOUS_QM_TYPE			(21)
#define SEX_IN_PARTY_QM_TYPE				(22)
#define CLASS_IN_PARTY_QM_TYPE			(23)
#define RACE_IN_PARTY_QM_TYPE				(24)
#define ACTIVE_LANGUAGES_QM_TYPE			(25)
#define ACTIVE_MEMBER_QM_TYPE				(26)
#define DAY_QM_TYPE							(27)
#define BYTE_COUNTER_QM_TYPE				(28)
#define MAP_NR_QM_TYPE						(29)
#define STEP_QM_TYPE							(30)
#define REQUESTER_QM_TYPE					(31)
#define CHECK_ACTION_MONSTER_QM_TYPE	(32)
#define SHADING_FACTOR_QM_TYPE			(33)
#define CHECK_DIALOGUE_ACTION_QM_TYPE	(34)
#define BATTLE_SITUATION_QM_TYPE			(35)
#define ITEM_TYPE_USED_QM_TYPE			(36)
#define RELATIVE_WEIGHT_QM_TYPE			(37)
#define PROBE_SKILL_QM_TYPE				(38)
#define PROBE_ATTRIBUTE_QM_TYPE			(39)
#define _2D_MAP_QM_TYPE						(40)
#define X_QM_TYPE								(41)
#define Y_QM_TYPE								(42)
#define NUMBER_REQ_QM_TYPE					(43)
#define VISIBLE_QM_TYPE						(44)

/* Modify modes */
#define CLEAR_MODIFY_MODE			(0)
#define SET_MODIFY_MODE				(1)
#define TOGGLE_MODIFY_MODE	  		(2)
#define LOAD_MODIFY_MODE			(3)
#define INC_MODIFY_MODE				(4)
#define DEC_MODIFY_MODE				(5)
#define INCPERC_MODIFY_MODE		(6)
#define DECPERC_MODIFY_MODE		(7)

/* Query modes */
#define LT_QUERY_MODE	(1)
#define LE_QUERY_MODE	(2)
#define EQ_QUERY_MODE	(3)
#define GE_QUERY_MODE	(4)
#define GT_QUERY_MODE	(5)

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

void Map_Exit_event(void);
void Do_normal_map_exit(UNSHORT New_X, UNSHORT New_Y, UNSHORT New_VD,
 UNSHORT New_map_nr);

void Door_event(void);

void Chest_event(void);

void Text_event(void);

void Spinner_event(void);

void Trap_event(void);
void Do_trap(UNSHORT Member_nr, struct Event_block *Event);

void Change_Used_Item_event(void);

void Datachange_event(void);
void Do_datachange(MEM_HANDLE Char_handle, struct Event_block *Event);

void Change_Icon_event(void);

void Encounter_event(void);

void Place_action_event(void);

void Query_event(void);
BOOLEAN Query_compare(UNSHORT Value, UNSHORT Query_mode, UNSHORT Number);

void Modify_event(void);
UNLONG Modify_amount(UNLONG Amount, UNLONG Maximum, UNSHORT Modify_mode,
 UNLONG Number);

void Signal_event(void);

void Clone_Automap_event(void);

void Sound_event(void);

void Start_Dialogue_event(void);

void Create_Trans_event(void);

void Execute_event(void);

void Remove_Member_event(void);

void End_Dialogue_event(void);

void Wipe_event(void);

void Play_Animation_event(void);

void Offset_event(void);

void Pause_event(void);

void Single_Item_Chest_event(void);

void Ask_Surrender_event(void);

void Do_Script_event(void);

void Modify_carried_items(MEM_HANDLE Char_handle, UNSHORT Item_index,
 UNSHORT Number, UNSHORT Modify_mode);
void Modify_items_carried_by_party(UNSHORT Item_index, UNSHORT Number,
 UNSHORT Modify_mode);

#endif

