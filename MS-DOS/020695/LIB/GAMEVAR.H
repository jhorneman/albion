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

#ifdef NDEBUG
#define Error(a) Report_error(a, __LINE__, __FILE__)
#else
#define Error(a) Report_error(a, 0, NULL)
#endif

#define MAX_SYSTEM_TEXTS		(500)

/* Standard colours */
#define BLACK			(192)
#define BLINK			(193)
#define WHITE			(194)
#define BRIGHTEST		(195)
#define BRIGHTER		(196)
#define BRIGHT			(197)
#define NORMAL			(198)
#define DARK			(199)
#define DARKER			(200)
#define DARKEST		(201)
#define INDIGO			(202)
#define PURPLE			(205)
#define RED				(208)
#define GREEN			(213)
#define GOLD			(219)
#define TURQUOISE		(224)
#define BLUE			(228)
#define SKIN			(232)
#define MARMOR			(240)

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

/* Bitmasks for Music_flags */
#define SOUNDSYSTEM_ON	(1<<0)
#define MUSIC_ON			(1<<1)
#define AMBIENT_ON		(1<<2)
#define SOUND_FX_ON		(1<<3)

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

extern struct OPM Main_OPM, Slab_OPM;
extern struct SCREENPORT Screen;
extern struct BBPALETTE Palette;

extern MEM_HANDLE Party_char_handles[6];
extern MEM_HANDLE Small_portrait_handles[6];

extern MEM_HANDLE Active_char_handle;

extern MEM_HANDLE Party_event_set_handles[6][2];
extern MEM_HANDLE Party_event_text_handles[6][2];

extern MEM_HANDLE Default_event_set_handle;
extern MEM_HANDLE Default_event_text_handle;

extern MEM_HANDLE Item_event_set_handle;
extern MEM_HANDLE Item_event_text_handle;

extern MEM_HANDLE Magic_event_set_handle;
extern MEM_HANDLE Magic_event_text_handle;

extern MEM_HANDLE Item_graphics_handle;

extern MEM_HANDLE Spell_data_handle;

extern BOOLEAN Quit_program;
extern BOOLEAN Terminal_condition;
extern BOOLEAN Cheat_mode;
extern BOOLEAN Diagnostic_mode;
extern BOOLEAN Super_VGA;
extern BOOLEAN Bumped;
extern BOOLEAN Moved;

extern UNLONG Music_flags;

extern UNSHORT Nr_auto_notes;

extern UNSHORT Language;

extern struct Party_data PARTY_DATA;

extern UNSHORT Screen_width, Screen_height, Panel_Y;
extern UNSHORT Forbidden_X, Forbidden_Y;

extern UNBYTE Recolour_tables[10][256];
extern UNBYTE Red_table[256];

extern UNSHORT Earth_object;

extern UNLONG Body_items_offset;
extern UNLONG Backpack_items_offset;

extern UNSHORT Compass_bit_coordinates[60][2];

extern const double PI;

/*
 ** Prototypes *************************************************************
 */

void BBMAIN(UNSHORT argc, const UNCHAR **argv);
void Exit_program(void);

void Evaluate_game_arguments(UNSHORT argc, UNCHAR **argv);

BOOLEAN Init_screen(void);
void Exit_screen(void);

BOOLEAN Init_game_data(void);
void Exit_game_data(void);

BOOLEAN Calculate_game_tables(void);

BOOLEAN Init_party_data(void);
void Exit_party_data(void);

void Save_game_state(UNSHORT Number, UNCHAR *Name);
void Load_game_state(UNSHORT Number);

void Report_error(UNSHORT Error_code, int Line_nr, char *Filename);
void Print_error(UNCHAR *buffer, UNBYTE *data);

/* Palette management functions */
void Init_palette(void);
BOOLEAN Load_palette(UNSHORT Palette_nr);
void Update_palette_part(UNSHORT Start, UNSHORT Size);

/* Palette support functions */
void Calculate_recolouring_table(UNBYTE *Table, UNSHORT Start, UNSHORT Size,
 UNSHORT Target_R, UNSHORT Target_G, UNSHORT Target_B, UNSHORT Percentage);

/* Party data bit array access functions */
BOOLEAN Read_bit_array(UNSHORT Array_type, UNLONG Bit_number);
void Write_bit_array(UNSHORT Array_type, UNLONG Bit_number,
 UNSHORT Write_mode);

BOOLEAN Activate_party_member(UNSHORT Member_nr);
BOOLEAN Do_activate_party_member(struct Event_action *Action);

/* Party / character status functions */
BOOLEAN Character_capable(MEM_HANDLE Char_handle);
BOOLEAN Party_can_see(void);
BOOLEAN Character_has_magical_abilities(MEM_HANDLE Char_handle);
BOOLEAN Character_knows_spells(MEM_HANDLE Char_handle);

/* Probe functions */
BOOLEAN Probe(SISHORT Value, UNSHORT Range);

BOOLEAN Probe_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr);
BOOLEAN Probe_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr);

UNSHORT Get_rnd_50_100(UNSHORT Value);

/* Character data access functions */
/* Attributes */
UNSHORT Get_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr);
void Set_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr, SISHORT Value);

/* Skills */
UNSHORT Get_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr);
void Set_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr, SISHORT Value);

/* Body conditions */
void Set_condition(MEM_HANDLE Char_handle, UNSHORT Condition_nr);
void Clear_condition(MEM_HANDLE Char_handle, UNSHORT Condition_nr);
UNSHORT Get_conditions(MEM_HANDLE Char_handle);

void Get_char_name(MEM_HANDLE Char_handle, UNCHAR *Name);

UNSHORT Get_class(MEM_HANDLE Char_handle);
UNSHORT Get_level(MEM_HANDLE Char_handle);
UNSHORT Get_sex(MEM_HANDLE Char_handle);
UNSHORT Get_race(MEM_HANDLE Char_handle);

UNSHORT Get_LP(MEM_HANDLE Char_handle);
UNSHORT Get_SP(MEM_HANDLE Char_handle);

UNLONG Get_EP(MEM_HANDLE Char_handle);
void Set_EP(MEM_HANDLE Char_handle, UNLONG Value);

UNLONG Get_carried_weight(MEM_HANDLE Char_handle);
UNSHORT Get_nr_occupied_hands(MEM_HANDLE Char_handle);

/* Magical abilities */
BOOLEAN Spell_class_known(MEM_HANDLE Char_handle, UNSHORT Class_nr);
BOOLEAN Spell_known(MEM_HANDLE Char_handle, UNSHORT Class_nr,
 UNSHORT Spell_nr);
UNLONG Get_spell_strength(MEM_HANDLE Char_handle, UNSHORT Class_nr,
 UNSHORT Spell_nr);

UNSHORT Get_damage(MEM_HANDLE Char_handle);
UNSHORT Get_protection(MEM_HANDLE Char_handle);

void Increase_LP(MEM_HANDLE Char_handle, UNSHORT Added_LP);
void Increase_SP(MEM_HANDLE Char_handle, UNSHORT Added_SP);

void Do_damage(UNSHORT Member_nr, UNSHORT Damage);
void Kill_party_member(UNSHORT Member_nr);

/* Level advancement functions */
void Calculate_level_factors(void);
void Check_member_levels(void);
void Increase_character_level(MEM_HANDLE Char_handle);

/* Item packet & data access functions */
BOOLEAN Packet_empty(struct Item_packet *Packet);
void Clear_packet(struct Item_packet *Packet);
struct Item_data *Get_item_data(struct Item_packet *Packet);
void Free_item_data(void);
void Get_item_name(struct Item_packet *Packet, UNCHAR *Name);

/* Inventory status functions */
BOOLEAN Char_inventory_empty(MEM_HANDLE Char_handle);
BOOLEAN Char_inventory_full(MEM_HANDLE Char_handle);
BOOLEAN Inventory_empty(struct Item_packet *Packet, UNSHORT Nr_items);
BOOLEAN Inventory_full(struct Item_packet *Packet, UNSHORT Nr_items);

UNSHORT Count_items_on_body(MEM_HANDLE Char_handle, UNSHORT Item_index);
UNSHORT Count_items_in_backpack(MEM_HANDLE Char_handle, UNSHORT Item_index);

UNSHORT Magical_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);
UNSHORT Alive_member_evaluator(MEM_HANDLE Char_handle);

#endif
