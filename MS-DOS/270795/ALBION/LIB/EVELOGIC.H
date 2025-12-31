/************
 * NAME     : EVELOGIC.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 27-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef EVENTLOGIC_H
#define EVENTLOGIC_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define EVENT_CONTEXTS_MAX		(40)
#define EVENT_ACTIONS_MAX		(8)

#define ACTION_ANY_VALUE_W		(32000)
#define ACTION_ANY_VALUE_B		(255)

/* Actor types */
#define NO_ACTOR_TYPE			(0)
#define PARTY_ACTOR_TYPE		(1)
#define MONSTER_ACTOR_TYPE		(2)
#define DIALOGUE_ACTOR_TYPE	(3)

/* Actor flags */
#define ME_ACTOR_FLAG			(0)
#define FRIEND_ACTOR_FLAG		(1)
#define ENEMY_ACTOR_FLAG		(2)

/* Event sources */
#define MAP_EVENT_SOURCE		(0)
#define SET_EVENT_SOURCE		(1)
#define INLINE_EVENT_SOURCE	(2)

/* Event set types */
#define DEFAULT_SET					(0)
#define MAP_SET						(1)
#define ITEM_SET						(2)
#define MAGIC_SET						(3)
#define DIALOGUE_SET	  				(4)
#define ACTING_MEMBER_SET			(5)
#define NON_ACTING_MEMBER_SET		(6)
#define ACTING_PART_SET				(7)
#define FRIEND_PART_SET				(8)
#define ENEMY_PART_SET				(9)

/* Action types */
#define ASK_ABOUT_ACTION						(0)
#define GIVE_ITEM_ACTION						(1)
#define ANY_WORD_ACTION							(2)
#define ASK_TO_JOIN_ACTION						(4)
#define ASK_TO_LEAVE_ACTION					(5)
#define DIALOGUE_START_ACTION					(6)
#define DIALOGUE_END_ACTION					(7)
#define SELECT_ANSWER_ACTION					(8)
#define UNKNOWN_WORD_ACTION					(9)
#define COMBAT_START_ACTION					(10)
#define COMBAT_END_ACTION						(11)
#define EVERY_ROUND_ACTION						(12)
#define PART_IS_HIT_ACTION						(13)
#define PART_DIES_ACTION						(14)
#define PART_HITS_ACTION						(15)
#define PART_KILLS_ACTION						(16)
#define PART_IS_ASKED_TO_SURRENDER_ACTION	(17)
#define PART_FLEES_ACTION						(18)
#define PART_ATTACKS_CLOSE_ACTION			(19)
#define PART_IS_ATTACKED_CLOSE_ACTION		(20)
#define PART_ATTACKS_LONG_ACTION				(21)
#define PART_IS_ATTACKED_LONG_ACTION		(22)
#define PART_ATTACKS_MAGIC_ACTION			(23)
#define PART_IS_ATTACKED_MAGIC_ACTION		(24)
#define PART_DEFLECTS_ATTACK_ACTION			(25)
#define PART_ATTACK_IS_DEFLECTED_ACTION	(26)
#define PART_DEFLECTS_MAGIC_ATTACK_ACTION	(27)
#define PART_MAGIC_ATTACK_IS_DEFLECTED_ACTION	(28)
#define PART_MAKES_CRITICAL_HIT_ACTION		(29)
#define TRAIN_A_SKILL_ACTION					(30)

#define HEAL_LP_ACTION							(31)
#define HEAL_A_CONDITION_ACTION				(32)
#define REMOVE_CURSED_ITEMS_ACTION			(33)
#define SCRUTINIZE_ITEM_ACTION				(34)
#define RECHARGE_ITEM_ACTION					(35)
#define RENT_A_ROOM_ACTION						(36)
#define BUY_ITEM_ACTION							(37)
#define SELL_ITEM_ACTION						(38)
#define VIEW_MERCHANT_ITEM_ACTION			(39)
#define BUY_FOOD_ACTION							(40)
#define BUY_SPELL_SCROLL_ACTION				(41)
#define BUY_TRANSPORT_ACTION					(42)
#define LEARN_SPELL_ACTION						(43)
#define REPAIR_ITEM_ACTION						(44)
#define CAST_SPELL_ACTION						(45)
#define USE_ITEM_ACTION							(46)
#define EQUIP_ITEM_ACTION						(47)
#define UNEQUIP_ITEM_ACTION					(48)
#define DROP_ITEM_ACTION						(49)
#define TAKE_ITEM_FROM_CHEST_ACTION			(50)
#define TAKE_ITEM_FROM_JUNKPILE_ACTION		(51)
#define STORE_ITEM_IN_CHEST_ACTION			(52)

#define TAKE_ITEM_FROM_CHAR_ACTION			(54)
#define GAME_BEGINS_ACTION						(55)
#define GAME_OVER_ACTION						(56)
#define SIGNAL_ACTION							(57)
#define LEVEL_MADE_ACTION						(58)
#define ENTER_AUTOMAPPER_ACTION				(59)
#define GOTO_POINT_USED_ACTION				(60)
#define MAKE_CAMP_ACTION						(61)
#define APRES_COMBAT_ACTION					(62)
#define CONDITION_BEGINS_ACTION				(63)
#define CONDITION_EFFECT_ACTION				(64)
#define PUT_ITEM_IN_CHAR_ACTION				(65)
#define SWITCH_TO_INVENTORY_ACTION			(66)
#define MAKE_ACTIVE_ACTION						(67)

/* Trigger modes */
#define NORMAL_TRIGGER			(0)
#define EXAMINE_TRIGGER			(1)
#define TOUCH_TRIGGER			(2)
#define SPEAK_TRIGGER			(3)
#define USE_ITEM_TRIGGER		(4)
#define MAP_INIT_TRIGGER		(5)
#define EVERY_STEP_TRIGGER		(6)
#define EVERY_HOUR_TRIGGER		(7)
#define EVERY_DAY_TRIGGER		(8)
#define DEFAULT_TRIGGER			(9)
#define ACTION_TRIGGER			(10)
#define NPC_TRIGGER				(11)
#define TAKE_TRIGGER				(12)

#define MAX_TRIGGER_MODES		(13)

/* Event error codes */
#define ERROR_ILLEGAL_EVENT_BLOCK	(1)
#define ERROR_ENDLESS_LOOP				(2)
#define ERROR_ILLEGAL_EVENT_TYPE		(3)

/*
 ** Type definitions *******************************************************
 */

typedef void (*Set_handler) (UNSHORT);
typedef void (*Event_handler) (void);
typedef BOOLEAN (*Context_validator) (struct Event_context *);
typedef BOOLEAN (*Action_handler) (struct Event_action *);

/*
 ** Structure definitions **************************************************
 */

/* Map event entry */
struct Map_event_entry {
	UNSHORT X;
	UNSHORT Trigger_modes;
	UNSHORT First_block_nr;		/* 0... */
};

/* Event block */
struct Event_block {
	UNBYTE Type;
	UNBYTE Byte_1;
	UNBYTE Byte_2;
	UNBYTE Byte_3;
	UNBYTE Byte_4;
	UNBYTE Byte_5;
	UNSHORT Word_6;
	UNSHORT Word_8;
	UNSHORT Next_event_nr;		/* 0... */
};

/* Event context */
struct Event_context {
	UNSHORT Source;
	UNSHORT Flags;
	MEM_HANDLE Set_handle;		/* Memory block containing event data */
	MEM_HANDLE Text_handle;		/* Memory block containing texts */
	UNSHORT Chain_nr;				/* Current chain number */
	UNSHORT Chain_block_nr;		/* Absolute block number of chain start */
	UNSHORT Block_nr;				/* Current block number */
	UNSHORT Max_blocks;			/* Maximum number of event blocks */
	UNLONG Base;					/* Offset to event blocks */
	struct Event_block Data;	/* Current event block */
	Context_validator Validator;
	union {
		struct {
			UNSHORT Map_nr;
			UNSHORT Trigger;
			UNSHORT Value;			/* e.g. used item index or spoken word index */
			UNSHORT X, Y;
		} Map_source;
		struct {
			UNSHORT Char_type;	/* 0xFFFF is no character */
			UNSHORT Char_index;
		} Set_source;
	} Source_data;
};

/* Bitmasks for Event_context.Flags */
#define BREAK_EVENT_CHAIN	(1<<0)
#define SUCCESS_FLAG			(1<<1)

/* Standard event set */
struct Standard_event_set {
	UNSHORT Nr_chains, Nr_blocks;
};

/* Event set list entry */
struct Event_set_list_entry {
	UNSHORT Type;
	UNSHORT Value;
};

/* Event action data */
struct Event_action {
	UNSHORT Flags;
	UNSHORT Actor_type;				/* Actor data */
	UNSHORT Actor_index;
	UNSHORT Action_type;				/* Action data */
	SISHORT Action_value;
	SISHORT Action_extra;
	Action_handler Action_ptr;		/* Action handler */
	void *Action_data;				/* Data for action handler */
	struct Event_set_list_entry *Event_set_ptr;
};

/* Bitmasks for Event_action.Flags */
#define HAVE_EXECUTED		(1 << 0)
#define FORBIDDEN				(1 << 1)
#define FOUND_CHAIN			(1 << 2)
#define ACTION_SUCCESSFUL	(1 << 3)

/* Catalogued map chain data */
struct Catalogued_map_chain {
	UNSHORT X, Y;
	UNSHORT First_block_nr;		/* 0... */
};

/*
 ** External global variables **********************************************
 */

extern struct Event_action Event_action_stack[EVENT_ACTIONS_MAX];
extern UNSHORT Event_action_stack_index;

extern struct Event_context Event_context_stack[EVENT_CONTEXTS_MAX];
extern UNSHORT Event_context_stack_index;

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Perform_dialogue_action(struct Event_action *New);
BOOLEAN Perform_action(struct Event_action *New);
BOOLEAN Execute_event_action(void);

void Handle_default_set(UNSHORT Value);
void Handle_map_set(UNSHORT Value);
void Handle_item_set(UNSHORT Value);
void Handle_magic_set(UNSHORT Value);
void Handle_dialogue_set(UNSHORT Value);
void Handle_acting_member_set(UNSHORT Value);
void Handle_non_acting_member_set(UNSHORT Value);
void Handle_acting_part_set(UNSHORT Value);
void Handle_friend_part_set(UNSHORT Value);
void Handle_enemy_part_set(UNSHORT Value);

void Search_member_sets(UNSHORT Member_nr, UNSHORT Actor_type);
void Search_monster_sets(UNSHORT Member_nr, UNSHORT Actor_type);

void Prepare_event_set_context(MEM_HANDLE Set_handle, UNSHORT Chain_nr,
 struct Event_context *Context, Context_validator Validator);
UNSHORT Search_event_set(MEM_HANDLE Set_handle, UNSHORT Actor_type);

struct Event_action *Push_event_action(void);
void Pop_event_action(void);
void Reset_event_action(void);

void Execute_event_chain(void);
BOOLEAN Chain_to_next_event(void);

struct Event_context *Push_event_context(void);
void Pop_event_context(void);
void Reset_event_context(void);

void Break_current_context(void);
void Set_success_flag(void);
void Clear_success_flag(void);

void Save_current_event(void);
BOOLEAN Event_is_saved(UNSHORT Map_nr, UNSHORT Chain_nr);

BOOLEAN Trigger_map_event_chain(UNSHORT X, UNSHORT Y, UNSHORT NPC_index,
 UNSHORT Trigger_mode, UNSHORT Value);
BOOLEAN Trigger_NPC_event_chain(UNSHORT NPC_index, UNSHORT Trigger_mode,
 UNSHORT Value);

/* Functions searching events with a certain trigger-mode */
UNSHORT Search_triggered_event(UNSHORT X, UNSHORT Y, UNLONG NPC_list,
 UNSHORT Trigger_mode);
UNSHORT Search_triggered_NPC_event(UNSHORT X, UNSHORT Y, UNLONG NPC_list,
 UNSHORT Trigger_mode);
UNSHORT Search_triggered_map_event(UNSHORT X, UNSHORT Y, UNSHORT Trigger_mode);

/* Functions to find the triggermodes for a certain position in the map */
UNSHORT Get_event_triggers(UNSHORT X, UNSHORT Y, UNSHORT NPC_index);
UNSHORT Get_NPC_event_triggers(UNSHORT X, UNSHORT Y, UNSHORT NPC_index);
UNSHORT Get_map_event_triggers(UNSHORT X, UNSHORT Y);

void Execute_chains_in_map(UNSHORT Trigger_mode);

UNLONG Catalogue_chains_in_map(UNSHORT Trigger_mode,
 struct Catalogued_map_chain *Catalogue);

void Execute_catalogued_chains_in_map(UNSHORT Trigger_mode);

void Catalogue_event_chains(UNSHORT Trigger_mode);
void Destroy_event_chain_catalogues(void);
void Update_event_chain_catalogues(void);

/* Event context validators */
BOOLEAN Map_context_validator(struct Event_context *Context);
BOOLEAN Dialogue_context_validator(struct Event_context *Context);
BOOLEAN Character_context_validator(struct Event_context *Context);

/* Map event logic support functions */
UNSHORT Get_event_first_block_nr(UNBYTE *Map_data, UNSHORT Chain_nr);
UNSHORT Get_event_chain_nr(UNBYTE *Map_data, UNSHORT First_block_nr);
struct Map_event_entry *Get_event_entry_at_position(UNBYTE *Map_data,
 SISHORT X, SISHORT Y);
UNSHORT Get_event_chain_nr_at_position(UNBYTE *Map_data, SISHORT X,
 SISHORT Y);

/* Error report functions */
void Event_error(UNSHORT Error_code);

/* Diagnostic functions */
void Display_event_data(struct Event_context *Context);

#endif

