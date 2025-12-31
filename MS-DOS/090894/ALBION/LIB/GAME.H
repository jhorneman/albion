/************
 * NAME     : GAME.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GAME_H
#define GAME_H

/*
 ** Defines ****************************************************************
 */

#define ITEMS_ON_BODY		(9)
#define ITEMS_PER_CHAR		(24)
#define ITEMS_PER_CHEST		(24)
#define EVENTS_PER_MAP		(64)
#define NPCS_PER_MAP			(32)

#define CLASSES_MAX			(32)
#define RACES_MAX				(15)
#define MAPS_MAX				(1023)
#define TRANSPORTS_MAX		(32)

#define ATTRS_MAX				(9)
#define SKILLS_MAX			(5)
#define LEVEL_MAX				(50)
#define LANGUAGES_MAX		(8)
#define SPELL_CLASSES_MAX	(7)
#define SPELLS_PER_CLASS	(30)

#define MONSTERS_PER_GROUP	(18)
#define ANIMS_PER_MONSTER		(8)
#define MONSTER_ANIM_LENGTH	(32)

/* Bit array sizes */
#define QUEST_MAX				(1024)
#define EVENT_MAX				(MAPS_MAX * EVENTS_PER_MAP)
#define CD_MAX					(MAPS_MAX * NPCS_PER_MAP)
#define WORDS_MAX				(1024)
#define GOTO_POINTS_MAX		(256)
#define CHESTS_MAX			(255)
#define DOORS_MAX				(255)
#define BYTE_COUNTERS_MAX	(32)

#define SAVE_NAME_LENGTH	(38)

#define GOLD_WEIGHT			(5)	/* Weight of 1 gold coin */
#define FOOD_WEIGHT			(250)	/* Weight of 1 food ration */

/* Directions (4) */
#define NORTH					(0)
#define EAST					(1)
#define SOUTH					(2)
#define WEST					(3)

/* Directions (8) */
#define NORTH8					(0)
#define NORTH_EAST			(1)
#define EAST8					(2)
#define SOUTH_EAST			(3)
#define SOUTH8					(4)
#define SOUTH_WEST			(5)
#define WEST8					(6)
#define NORTH_WEST			(7)

/* Supported languages */
#define GERMAN					(0)
#define ENGLISH				(1)
#define FRENCH					(2)

/* Constant world time values */
#define MONTHS_PER_YEAR		(12)
#define DAYS_PER_MONTH     (40)
#define HOURS_PER_DAY		(18)
#define MINUTES_PER_HOUR	(60)
#define MINUTES_PER_STEP	(5)

#define STEPS_MAX				(HOURS_PER_DAY * (MINUTES_PER_HOUR / MINUTES_PER_STEP))

#define ITEM_NAME_LENGTH	(20)		/* Including EOL */
#define CHAR_NAME_LENGTH	(16)		/* Including EOL */

/* Item types */
#define ARMOUR_IT				(1)
#define HELMET_IT				(2)
#define SHOES_IT				(3)
#define SHIELD_IT				(4)
#define SHORT_RANGE_IT		(5)
#define LONG_RANGE_IT		(6)
#define AMMO_IT				(7)
#define TEXT_SCROLL_IT		(8)
#define SPELL_SCROLL_IT		(9)
#define DRINK_IT				(10)
#define AMULET_IT				(11)
#define BROOCH_IT				(12)
#define RING_IT				(13)
#define VALUABLE_IT			(14)
#define TOOL_IT				(15)
#define KEY_IT					(16)
#define NORMAL_IT				(17)
#define MAGIC_IT				(18)
#define SPECIAL_IT			(19)
#define TRANSPORT_IT			(20)

/* Body inventory slot numbers */
#define NECK					(0)
#define HEAD					(1)
#define TAIL					(2)
#define RIGHT_HAND			(3)
#define BODY					(4)
#define LEFT_HAND				(5)
#define RIGHT_FINGER			(6)
#define FEET					(7)
#define LEFT_FINGER			(8)

/* Sexes */
#define MALE					(0)
#define FEMALE					(1)
#define NEUTRAL				(2)

/* Attributes */
#define STRENGTH				(0)
#define INTELLIGENCE			(1)
#define DEXTERITY				(2)
#define SPEED					(3)
#define STAMINA				(4)
#define LUCK					(5)
#define MAGIC_RESISTANCE	(6)
#define MAGIC_TALENT			(7)
#define AGE						(8)

/* Skills */
#define SHORT_RANGE_ATTACK	(0)
#define LONG_RANGE_ATTACK	(1)
#define SWIMMING				(2)
#define CRITICAL_HIT			(3)
#define LOCK_PICKING			(4)

/* Monster animations */
#define WALK_ANIM					(0)
#define SHORT_RANGE_ANIM		(1)
#define LONG_RANGE_ANIM			(2)
#define MAGIC_ATTACK_ANIM		(3)
#define HIT_ANIM					(4)
#define DIE_ANIM					(5)
#define APPEAR_ANIM				(6)
#define SPECIAL_ANIM				(7)

/*
 ** Structure definitions **************************************************
 */

/* Transportation data */
typedef struct {
	UNBYTE Type;
	UNBYTE X,Y;
	UNBYTE _pad01;
	UNSHORT Map_nr;
} Trans_data;

/* Modification data */
typedef struct {
	UNSHORT Map_nr;
	UNBYTE X,Y;
	UNBYTE Mode;
	UNBYTE _pad01;
	UNSHORT Icon_nr;
} Modif_data;

/* Automapper note data */
typedef struct {
	UNSHORT Length;
	UNSHORT Map_nr;
	UNSHORT X,Y;
} Autonote_data;

/* Item packets */
typedef struct {
	UNBYTE Quantity;
	UNBYTE Charges;
	UNBYTE Enchantment_normal;
	UNBYTE Flags;
	UNSHORT Index;
} Item_packet;

/* Bitmasks for Item_packet.Flags */
#define MAGIC_CHECK			(1<<0)
#define BROKEN_ITEM			(1<<1)
#define CURSED					(1<<2)
#define AUGMENTED				(1<<3)
#define LIGHTER				(1<<4)

/* Item data */
typedef struct {
	UNBYTE Pic_nr;
	UNBYTE Type;
	UNBYTE Body_place;
	UNBYTE Break_chance;
	UNBYTE Sex_use;
	UNBYTE Hand_use;
	UNBYTE Ring_use;
	UNBYTE LP_max, SP_max;
	UNBYTE Attribute, Attribute_normal;
	UNBYTE Skill, Skill_normal;
	UNBYTE Protection_pts, Damage_pts;
	UNBYTE Ammo_ID;
	UNBYTE Item_skills[2];
	UNBYTE Malus[2];
	UNBYTE Misc[2];
	UNBYTE Spell_class, Spell_nr;
	UNBYTE Init_charges, Init_enchantment_normal;
	UNBYTE Enchantment_max, Charges_max;
	UNBYTE Flags;
	UNBYTE Init_packet_flags;
	UNBYTE _pad01;
	UNSHORT Weight, Price, Class_use;
} Item_data;

/* Bitmasks for Item_data.Flags */
#define METALLIC				(1<<0)
#define DROPPABLE				(1<<1)
#define MULTIPLE				(1<<2)
#define COMBAT_EQUIP			(1<<3)
#define DESTROY_AFTER_USE	(1<<4)

/* Attributes */
typedef struct {
	UNSHORT Normal;
	UNSHORT Maximum;
	SISHORT Magic;
	UNSHORT Backup;
} Attribute;

/* Skills */
typedef struct {
	UNSHORT Normal;
	UNSHORT Maximum;
	SISHORT Magic;
	UNSHORT Backup;
} Skill;

/* Character data */
typedef struct {
	UNBYTE Type;
	UNBYTE Sex;
	UNBYTE Race;
	UNBYTE Class;
	UNBYTE Known_spell_classes;
	UNBYTE Level;
	UNBYTE Hands_occupied;
	UNBYTE Ringfingers_occupied;
	UNBYTE Known_languages;
	UNBYTE Flags;
	UNBYTE Portrait_nr;
	UNBYTE Graphics_2D_nr;
	UNBYTE Full_body_pic_nr;
	UNBYTE Tactic_type;
	UNBYTE Tactic_icon_nr;
	UNBYTE Courage;
	UNBYTE Attacks_per_round;
	UNBYTE _pad01;

	UNSHORT Event_set_1_nr, Event_set_2_nr;
	UNSHORT Nr_training_points;
	UNSHORT Nr_gold_coins;
	UNSHORT Nr_food_rations;
	UNSHORT Destination_CD_bit;
	UNSHORT Body_conditions;
	UNSHORT Experience_bonus;

	Attribute Attributes[ATTRS_MAX];
	Skill Skills[SKILLS_MAX];

	UNSHORT Life_points;
	UNSHORT Life_points_maximum;
	SISHORT Life_points_magic;
	UNSHORT Spell_points;
	UNSHORT Spell_points_maximum;
	SISHORT Spell_points_magic;
	UNSHORT Protection;
	SISHORT Protection_magic;
	UNSHORT Damage;
	SISHORT Damage_magic;
	UNSHORT Special[4];
	UNSHORT Description_text_nr;

	UNLONG Experience_pts;
	UNLONG Known_spells[SPELL_CLASSES_MAX];
	UNLONG Carried_weight;

	UNCHAR Name[3][CHAR_NAME_LENGTH];

	UNBYTE Spell_capabilities[SPELL_CLASSES_MAX][SPELLS_PER_CLASS];

	/* Party and monster only */
	Item_packet Body_items[ITEMS_ON_BODY];
	Item_packet Backpack_items[ITEMS_PER_CHAR];

	/* Monster only */
	UNBYTE Monster_graphics_nr;
	UNBYTE Anim_motion;
	UNBYTE Recolour_palette_nr;
	UNBYTE _pad02;
	UNBYTE Anim_sequences[ANIMS_PER_MONSTER][MONSTER_ANIM_LENGTH];
	UNBYTE Anim_lengths[ANIMS_PER_MONSTER];
	UNSHORT Baseline;
	UNSHORT X_factor, Y_factor;

} Character_data;

/* Bitmasks for Character_data.Flags */
#define ANIMAL					(1<<0)
#define INTELLIGENT			(1<<1)
#define SUPERNATURAL			(1<<2)
#define PERSON					(1<<3)
#define EVIL					(1<<4)
#define GOD						(1<<5)
#define GHOST					(1<<6)
#define END_MONSTER			(1<<7)

/* Spell data */
typedef struct {
	UNBYTE Area_bits;
	UNBYTE Point_cost;
	UNBYTE Level;
	UNBYTE Target_bits;
} Spell_data;

/* Bitmasks for Spell_data.Area_bits */
#define CITY_AREA				(1<<0)
#define DUNGEON_AREA			(1<<1)
#define WILDERNESS_AREA		(1<<2)
#define INTERIOR_AREA		(1<<3)
#define CAMP_AREA				(1<<4)
#define COMBAT_AREA			(1<<5)
#define SPACESHIP_AREA		(1<<6)

/* Bitmasks for Spell_data.Target_bits */
#define ONE_FRIEND_TARGET	(1<<0)
#define ROW_FRIENDS_TARGET	(1<<1)
#define ALL_FRIENDS_TARGET	(1<<2)
#define ONE_ENEMY_TARGET	(1<<3)
#define ROW_ENEMIES_TARGET	(1<<4)
#define ALL_ENEMIES_TARGET	(1<<5)
#define ITEM_TARGET			(1<<6)
#define SPECIAL_TARGET		(1<<7)

/* Party data */
typedef struct {
	UNSHORT Year, Month, Day, Hour, Minute;
	UNSHORT Map_nr, X, Y, View_direction;
	UNSHORT Permanent_spells[6][2];
	UNSHORT Active_member;
	UNSHORT Member_nrs[6];
	UNSHORT Relative_year;
	UNSHORT Travel_mode;
	UNSHORT Special_item_flags;
	UNSHORT Internal_flags;
	UNSHORT Camp_counter;
	UNBYTE Battle_order[6];
	Trans_data Transportations[TRANSPORTS_MAX];

	UNBYTE Quest[(QUEST_MAX+7)/8];
	UNBYTE Event[(EVENT_MAX+7)/8];
	UNBYTE CD[(CD_MAX+7)/8];
	UNBYTE WD[(WORDS_MAX+7)/8];
	UNBYTE Goto_points[(GOTO_POINTS_MAX+7)/8];
	UNBYTE Chest_open[(CHESTS_MAX+7)/8];
	UNBYTE Door_open[(DOORS_MAX+7)/8];
	UNBYTE Byte_counters[BYTE_COUNTERS_MAX];
} Party_data;

/*
 ** Global variables *******************************************************
 */

extern struct OPM Main_OPM;
extern struct SCREENPORT Screen;
extern struct BBPALETTE Palette;

extern Party_data PARTY_DATA;

extern UNSHORT Screen_width, Screen_height;

/*
 ** Prototypes *************************************************************
 */

void BBMAIN(UNSHORT argc, UNCHAR **argv);
void Switch_screens(void);

void xxInit(void);
void xxExit(void);
void Main_loop(void);

void Colour_cycle_forward(UNSHORT Start, UNSHORT Size);

#endif
