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

#include <BBDEF.H>
#include <MAP.H>

/*
 ** Defines ****************************************************************
 */

#define PARTY_CHARS_MAX			(12)

#define ITEMS_ON_BODY			(9)
#define ITEMS_PER_CHAR			(24)
#define ITEMS_PER_CHEST			(24)

#define ITEMS_PER_PACKET		(99)

#define ITEMS_MAX					(1024)
#define ITEM_GFX_MAX				(468)

#define SPELL_CLASSES_MAX		(7)
#define SPELLS_PER_CLASS		(30)

#define EVENTS_PER_3D_MAP		(64)
#define EVENTS_PER_MAP			(250)

#define GOTO_POINTS_PER_MAP	(16)

#define CLASSES_MAX				(32)
#define RACES_MAX					(16)
#define MAPS_MAX					(512L)
#define TRANSPORTS_MAX			(32)

#define TRAVELMODES_MAX			(7)

#define LEVEL_MAX					(50)
#define GAME_LANGUAGES_MAX		(8)

#define MONSTERS_PER_GROUP		(18)
#define ANIMS_PER_MONSTER		(8)
#define MONSTER_ANIM_LENGTH	(32)

#define ITEM_NAME_LENGTH		(20)		/* Including EOL */
#define CHAR_NAME_LENGTH		(16)		/* Including EOL */

/* Bit array sizes */
#define QUEST_MAX				(1024)
#define EVENT_MAX				(MAPS_MAX * EVENTS_PER_MAP)
#define CD_MAX					(MAPS_MAX * NPCS_PER_MAP)
#define WORDS_MAX				(1024)
#define GOTO_POINTS_MAX		(256)
#define CHESTS_MAX			(255)
#define DOORS_MAX				(255)
#define BYTE_COUNTERS_MAX	(300)

#define SAVE_NAME_LENGTH	(38)

#define GOLD_WEIGHT			(5)		/* Weight of 1 gold coin */
#define FOOD_WEIGHT			(250)		/* Weight of 1 food ration */

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
#define LANGUAGES_MAX		(3)

/* Standard event sets */
#define DEFAULT_EVENT_SET	(1)
#define ITEM_EVENT_SET		(2)
#define MAGIC_EVENT_SET		(3)

/* Travelmodes */
#define ON_FOOT				(0)

/* Item types */
#define ARMOUR_IT				(1)
#define HELMET_IT				(2)
#define SHOES_IT				(3)
#define SHIELD_IT				(4)
#define CLOSE_RANGE_IT		(5)
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

/* Body inventory slot numbers (as found in item data !) */
#define NO_BODY_PLACE		(0)
#define NECK					(1)
#define HEAD					(2)
#define TAIL					(3)
#define RIGHT_HAND			(4)
#define BODY					(5)
#define LEFT_HAND				(6)
#define RIGHT_FINGER			(7)
#define FEET					(8)
#define LEFT_FINGER			(9)
#define HAND_OR_TAIL			(10)

/* Races */
#define ISKAI_RACE			(2)

/* Sexes */
#define MALE					(0)
#define FEMALE					(1)
#define NEUTRAL				(2)

/* Attributes */
#define STRENGTH				(1)
#define INTELLIGENCE			(2)
#define DEXTERITY				(3)
#define SPEED					(4)
#define STAMINA				(5)
#define LUCK					(6)
#define MAGIC_RESISTANCE	(7)
#define MAGIC_TALENT			(8)
#define AGE						(9)

#define ATTRS_MAX	 			(9)

/* Skills */
#define CLOSE_RANGE_ATTACK	(1)
#define LONG_RANGE_ATTACK	(2)
#define CRITICAL_HIT			(3)
#define LOCK_PICKING			(4)

#define SKILLS_MAX			(4)

/* Body conditions */
#define UNCONSCIOUS			(0)
#define POISONED				(1)
#define DISEASED				(2)
#define EXHAUSTED				(3)
#define LAMED					(4)
#define FLEEING				(5)
#define STONED					(6)
#define BLIND					(7)
#define PANICKED				(8)
#define ASLEEP					(9)
#define INSANE					(10)
#define IRRITATED				(11)

#define MAX_CONDITIONS		(12)

/* Condition combinations */
#define INCAPABLE_MASK		((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << FLEEING) | (1 << PANICKED) | (1 << ASLEEP))
#define FIGHTING_MASK		(1 << UNCONSCIOUS)
#define CONTROL_MASK			((1 << PANICKED) | (1 << INSANE))
#define DAMAGE_MASK			(0)
#define DEAD_MASK				(1 << UNCONSCIOUS)
#define MAGIC_MASK			(0)
#define ATTACK_MASK			(0)
#define MOVE_MASK				(0)
#define FLEE_MASK				(0)

/* Monster animations */
#define WALK_ANIM					(0)
#define CLOSE_RANGE_ANIM		(1)
#define LONG_RANGE_ANIM			(2)
#define MAGIC_ATTACK_ANIM		(3)
#define HIT_ANIM					(4)
#define DIE_ANIM					(5)
#define APPEAR_ANIM				(6)
#define SPECIAL_ANIM				(7)

/* Character types */
#define PARTY_CHAR_TYPE			(0)
#define NPC_CHAR_TYPE			(1)
#define MONSTER_CHAR_TYPE		(2)

/*
 ** Structure definitions **************************************************
 */

/* Transportation data */
struct Trans_data {
	UNBYTE Type;
	UNBYTE X, Y;
	UNBYTE _pad01;
	UNSHORT Map_nr;
};

/* Modification data */
struct Modif_data {
	UNSHORT Map_nr;
	UNBYTE X, Y;
	UNBYTE Mode;
	UNBYTE _pad01;
	UNSHORT Icon_nr;
};

/* Automapper note data */
struct Autonote_data {
	UNSHORT Length;			/* Including this structure */
	UNSHORT Map_nr;
	UNSHORT X, Y;
	/* String follows, + EOL and EVEN */
};

/* Item packets */
struct Item_packet {
	UNBYTE Quantity;
	UNBYTE Charges;
	UNBYTE Enchantment_normal;
	UNBYTE Flags;
	UNSHORT Index;
};

/* Bitmasks for Item_packet.Flags */
#define MAGIC_CHECK			(1<<0)
#define BROKEN_ITEM			(1<<1)
#define CURSED					(1<<2)
#define LIGHTER				(1<<4)
#define DOUBLED				(1<<3)

/* Item data */
struct Item_data {
	UNBYTE _pad01;
	UNBYTE Type;
	UNBYTE Body_place;
	UNBYTE Break_chance;
	UNBYTE Sex_use;
	UNBYTE Hand_use;
	UNBYTE LP_max, SP_max;
	UNBYTE Attribute, Attribute_normal;
	UNBYTE Skill, Skill_normal;
	UNBYTE Protection_pts, Damage_pts;
	UNBYTE Ammo_ID;
	UNBYTE Item_skills[2];
	UNBYTE Malus[2];
	UNBYTE Misc[2];
	UNBYTE Spell_class, Spell_nr;
	UNBYTE Init_charges;
	UNBYTE Init_enchantment_normal;
	UNBYTE Max_enchantment;
	UNBYTE Max_charges;
	UNBYTE Flags;
	UNBYTE Init_packet_flags;
	UNBYTE Nr_frames;
	UNSHORT Weight;
	UNSHORT Price;
	UNSHORT Pic_nr;
	UNLONG Class_use;
};

/* Bitmasks for Item_data.Flags */
#define METALLIC				(1<<0)
#define UNDROPPABLE 			(1<<1)
#define MULTIPLE				(1<<2)
#define COMBAT_EQUIP			(1<<3)
#define DESTROY_AFTER_USE	(1<<4)
#define ORGANIC				(1<<5)

/* Attributes */
struct Attribute {
	UNSHORT Normal;
	UNSHORT Maximum;
	SISHORT Magic;			/* Added to normal */
	UNSHORT Backup;
};

/* Skills */
struct Skill {
 	UNSHORT Normal;
	UNSHORT Maximum;
	SISHORT Magic;			/* Added to normal */
	UNSHORT Backup;
};

/* Character data */
struct Character_data {
	/* NPC, party and monster */
	UNBYTE Type;
	UNBYTE xSex;
	UNBYTE xRace;
	UNBYTE xClass;
	UNBYTE xKnown_spell_classes;
	UNBYTE xLevel;
	UNBYTE xHands_occupied;
	UNBYTE _pad00; 		//Ringfingers_occupied;
	UNBYTE Known_languages;
	UNBYTE Graphics_2D_nr;
	UNBYTE Portrait_nr;
	UNBYTE Full_body_pic_nr;
	UNBYTE Tactic_type;
	UNBYTE Tactic_icon_nr;
	UNBYTE Flags;
	UNBYTE Courage;
	UNBYTE _pad01;
	UNBYTE Attacks_per_round;

	UNSHORT Event_set_1_nr, Event_set_2_nr;
	UNSHORT Training_points;
	UNSHORT Nr_gold_coins;
	UNSHORT Nr_food_rations;
	UNSHORT Destination_CD_bit;
	UNSHORT xBody_conditions;
	UNSHORT Experience_bonus;

	UNSHORT _pad02[4];

	struct Attribute Attributes[ATTRS_MAX];
	struct Attribute _pad03[10 - ATTRS_MAX];
	struct Skill Skills[SKILLS_MAX];
	struct Skill _pad04[10 - SKILLS_MAX];

	UNSHORT Life_points;
	UNSHORT Life_points_maximum;
	SISHORT Life_points_magic;			/* Added to maximum */
	UNSHORT Spell_points;
	UNSHORT Spell_points_maximum;
	SISHORT Spell_points_magic;		/* Added to maximum */
	UNSHORT xProtection;
	SISHORT xProtection_magic;
	UNSHORT xDamage;
	SISHORT xDamage_magic;
	UNSHORT _pad05[2];
	UNSHORT Special[5];
	UNSHORT Description_text_nr;

	UNLONG xExperience_points;
	UNLONG xKnown_spells[SPELL_CLASSES_MAX];
	UNLONG xCarried_weight;

	UNCHAR xName[LANGUAGES_MAX][CHAR_NAME_LENGTH];

	UNSHORT xSpell_capabilities[SPELL_CLASSES_MAX][SPELLS_PER_CLASS];

	/* Party and monster only */
	struct Item_packet Body_items[ITEMS_ON_BODY];
	struct Item_packet Backpack_items[ITEMS_PER_CHAR];

	/* Monster only */
	UNBYTE Monster_graphics_nr;
	UNBYTE Anim_motion;
	UNBYTE Recolour_palette_nr;
	UNBYTE _pad06;
	UNBYTE Anim_sequences[ANIMS_PER_MONSTER][MONSTER_ANIM_LENGTH];
	UNBYTE Anim_lengths[ANIMS_PER_MONSTER];
	SISHORT Baseline;
	UNSHORT X_factor, Y_factor;
};

/* Bitmasks for Character_data.Flags */
#define ANIMAL					(1<<0)
#define INTELLIGENT			(1<<1)
#define SUPERNATURAL			(1<<2)
#define PERSON					(1<<3)
#define EVIL					(1<<4)
#define GOD						(1<<5)
#define GHOST					(1<<6)
#define END_MONSTER			(1<<7)

/* Party data */
struct Party_data {
	UNSHORT Year;					/* Time information */
	UNSHORT Month;
	UNSHORT Day;
	UNSHORT Hour;
	UNSHORT Minute;
	UNSHORT Map_nr;				/* Location information */
	UNSHORT X;
	UNSHORT Y;
	UNSHORT View_direction;
	UNSHORT Permanent_spells[PARTY_CHARS_MAX][4][2];
	UNSHORT Active_member;
	UNSHORT Member_nrs[6];
	UNSHORT Relative_year;
	UNSHORT Travel_mode;
	UNSHORT Special_item_flags;
	UNSHORT Internal_flags;
	UNSHORT Camp_counter;
	UNBYTE Battle_order[6];
	struct Trans_data Transportations[TRANSPORTS_MAX];

	UNBYTE Quest[(QUEST_MAX+7)/8];
	UNBYTE Event[(EVENT_MAX+7)/8];
	UNBYTE CD[(CD_MAX+7)/8];
	UNBYTE Known_words[(WORDS_MAX+7)/8];
	UNBYTE Goto_points[(GOTO_POINTS_MAX+7)/8];
	UNBYTE Chest_open[(CHESTS_MAX+7)/8];
	UNBYTE Door_open[(DOORS_MAX+7)/8];
	UNBYTE Byte_counters[BYTE_COUNTERS_MAX];
/*
 Followed by :
    - Modification list
    - Automapper notes
*/
};

/* Bitmasks for Party_data.Special_item_flags */
#define COMPASS_ITEM_FLAG		(1<<0)
#define MONSTER_EYE_ITEM_FLAG	(1<<1)
#define CLOCK_ITEM_FLAG			(1<<3)

#endif

