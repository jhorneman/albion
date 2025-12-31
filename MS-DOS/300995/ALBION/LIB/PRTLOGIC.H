/************
 * NAME     : PRTLOGIC.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef PRTLOGIC_H
#define PRTLOGIC_H

#include <BBDEF.H>
#include <GAME.H>

/*
 ** Macro definitions ******************************************************
 */

#define Member_present(Member_index) (PARTY_DATA.Member_nrs[Member_index - 1])

/*
 ** Defines ****************************************************************
 */

/* Bit array types */
#define QUEST_BIT_ARRAY			(0)
#define CHEST_BIT_ARRAY			(1)
#define DOOR_BIT_ARRAY			(2)
#define CD_BIT_ARRAY				(3)
#define EVENT_SAVE_BIT_ARRAY	(4)
#define KNOWN_WORDS_BIT_ARRAY	(5)
#define NEW_WORDS_BIT_ARRAY	(6)
#define GOTO_POINTS_BIT_ARRAY	(7)

/* Bit array write modes (MUST be the same as first three modify modes !) */
#define CLEAR_BIT_ARRAY		(0)
#define SET_BIT_ARRAY		(1)
#define TOGGLE_BIT_ARRAY	(2)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern MEM_HANDLE Party_char_handles[6];
extern MEM_HANDLE Small_portrait_handles[6];

extern MEM_HANDLE Active_char_handle;

extern MEM_HANDLE Party_event_set_handles[6][2];
extern MEM_HANDLE Party_event_text_handles[6][2];

extern MEM_HANDLE Default_event_set_handle;
extern MEM_HANDLE Default_event_text_handle;

extern struct Party_data PARTY_DATA;

extern UNLONG Body_items_offset;
extern UNLONG Backpack_items_offset;

extern UNSHORT Compass_bit_coordinates[60][2];

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_game_data(void);
void Exit_game_data(void);

BOOLEAN Calculate_game_tables(void);

BOOLEAN Init_party_data(UNSHORT Load_game_nr, BOOLEAN No_load_flag);
void Exit_party_data(void);

/* Party data bit array access functions */
void Clear_bit_array(UNSHORT Array_type);
BOOLEAN Read_bit_array(UNSHORT Array_type, UNLONG Bit_number);
void Write_bit_array(UNSHORT Array_type, UNLONG Bit_number,
 UNSHORT Write_mode);

void Select_new_active_member(void);
void Game_over(void);

BOOLEAN Activate_party_member(UNSHORT Member_nr);
BOOLEAN Do_activate_party_member(struct Event_action *Action);

/* Party / character status functions */
UNSHORT Find_character_in_party(UNSHORT Char_index);
UNSHORT Find_char_data_in_party(MEM_HANDLE Char_handle);

BOOLEAN Character_alive(MEM_HANDLE Char_handle);
BOOLEAN Character_capable(MEM_HANDLE Char_handle);
BOOLEAN Party_can_see(void);
BOOLEAN Party_is_full(void);
BOOLEAN Character_has_magical_abilities(MEM_HANDLE Char_handle);
BOOLEAN Character_knows_spells(MEM_HANDLE Char_handle);
BOOLEAN Character_is_overweight(MEM_HANDLE Char_handle);

/* Probe functions */
BOOLEAN Probe(SISHORT Value, UNSHORT Range);

BOOLEAN Probe_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr);
BOOLEAN Probe_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr);

UNSHORT Get_rnd_50_100(UNSHORT Value);

/* Character data access functions */

/* Attributes */
SISHORT Get_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr);
void Set_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr, SISHORT Value);

/* Skills */
SISHORT Get_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr);
void Set_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr, SISHORT Value);

/* Body conditions */
void Set_condition(MEM_HANDLE Char_handle, UNSHORT Condition_nr);
void Clear_condition(MEM_HANDLE Char_handle, UNSHORT Condition_nr);
UNSHORT Get_conditions(MEM_HANDLE Char_handle);

void Get_char_name(MEM_HANDLE Char_handle, UNCHAR *Name);

UNSHORT Get_class(MEM_HANDLE Char_handle);
SISHORT Get_level(MEM_HANDLE Char_handle);
UNSHORT Get_sex(MEM_HANDLE Char_handle);
UNSHORT Get_race(MEM_HANDLE Char_handle);

UNSHORT Get_known_languages(MEM_HANDLE Char_handle);

SISHORT Get_LP(MEM_HANDLE Char_handle);
SISHORT Get_max_LP(MEM_HANDLE Char_handle);
SISHORT Get_SP(MEM_HANDLE Char_handle);
SISHORT Get_max_SP(MEM_HANDLE Char_handle);

void Set_LP(MEM_HANDLE Char_handle, SISHORT New_LP);
void Set_max_LP(MEM_HANDLE Char_handle, SISHORT New_max_LP);
void Set_SP(MEM_HANDLE Char_handle, SISHORT New_SP);
void Set_max_SP(MEM_HANDLE Char_handle, SISHORT New_max_SP);

SILONG Get_EP(MEM_HANDLE Char_handle);
void Set_EP(MEM_HANDLE Char_handle, SILONG Value);

SISHORT Get_TP(MEM_HANDLE Char_handle);
void Set_TP(MEM_HANDLE Char_handle, SISHORT Value);

SILONG Get_carried_weight(MEM_HANDLE Char_handle);
SISHORT Get_nr_occupied_hands(MEM_HANDLE Char_handle);

UNSHORT Get_event_set_index(MEM_HANDLE Char_handle, UNSHORT Char_set_index);
void Set_event_set_index(MEM_HANDLE Char_handle, UNSHORT Char_set_index,
 UNSHORT Set_index);

/* Magical abilities */
BOOLEAN Spell_class_known(MEM_HANDLE Char_handle, UNSHORT Class_nr);
BOOLEAN Spell_known(MEM_HANDLE Char_handle, UNSHORT Class_nr,
 UNSHORT Spell_nr);

SILONG Get_spell_strength(MEM_HANDLE Char_handle, UNSHORT Class_nr,
 UNSHORT Spell_nr);
void Set_spell_strength(MEM_HANDLE Char_handle, UNSHORT Class_nr,
 UNSHORT Spell_nr, SILONG Spell_strength);

SISHORT Get_damage(MEM_HANDLE Char_handle);
SISHORT Get_protection(MEM_HANDLE Char_handle);

void Do_damage(UNSHORT Member_nr, SISHORT Damage);
void Kill_party_member(UNSHORT Member_nr);
void Show_party_member_damage(UNSHORT Member_nr, SISHORT Damage);

void Do_healing(UNSHORT Member_nr, SISHORT Healing);
void Revive_party_member(UNSHORT Member_nr);
void Show_party_member_healing(UNSHORT Member_nr, SISHORT Healing);

void De_exhaust_party_member(UNSHORT Member_nr);

/* Level advancement functions */
void Check_member_levels(void);
void Increase_character_level(MEM_HANDLE Char_handle);

/* Inventory status functions */
BOOLEAN Char_inventory_empty(MEM_HANDLE Char_handle);
BOOLEAN Char_inventory_full(MEM_HANDLE Char_handle);

SISHORT Count_items_on_body(MEM_HANDLE Char_handle, UNSHORT Item_index);
SISHORT Count_items_in_backpack(MEM_HANDLE Char_handle, UNSHORT Item_index);

UNSHORT Magical_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);
UNSHORT Vital_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);

UNSHORT Alive_member_evaluator(MEM_HANDLE Char_handle);
UNSHORT Can_be_active_member_evaluator(MEM_HANDLE Char_handle);

/* Party functions */
void Remove_party_member(UNSHORT Member_index);
BOOLEAN Add_party_member(UNSHORT Char_index, UNSHORT NPC_index);

SILONG Get_party_gold(void);
void Set_party_gold(SILONG New_party_gold);

SILONG Get_party_food(void);
void Set_party_food(SILONG New_party_food);

SISHORT Count_party_members(void);
SISHORT Count_alive_party_members(void);

SISHORT Calculate_party_light(void);
void Update_party_light(void);

#endif
