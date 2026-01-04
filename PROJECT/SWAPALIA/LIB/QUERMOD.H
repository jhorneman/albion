/************
 * NAME     : QUERMOD.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef QUERMOD_H
#define QUERMOD_H

#include <BBDEF.H>
#include <EVELOGIC.H>

/*
 ** Defines ****************************************************************
 */

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
#define INVENTORY_MEMBER_DC		(6)
#define SUBJECT_DC					(7)

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
#define VISIBLE_QM_TYPE						(33)
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

extern MEM_HANDLE Used_item_char_handle;
extern UNSHORT Used_item_slot_index;

/*
 ** Prototypes *************************************************************
 */

void Change_Used_Item_event(void);

void Datachange_event(void);

void Query_event(void);

void Modify_event(void);

#endif

