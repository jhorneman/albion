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
#include <NPCS.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_PARTY_CHARS			(12)

#define ITEMS_ON_BODY			(9)
#define ITEMS_PER_CHAR			(24)
#define ITEMS_PER_CHEST			(24)

#define ITEMS_PER_PACKET		(99)

#define MAX_ITEMS					(1024)
#define MAX_ITEM_GFX				(468)

#define MAX_SPELL_CLASSES		(7)
#define SPELLS_PER_CLASS		(30)

#define EVENTS_PER_3D_MAP		(64)
#define EVENTS_PER_MAP			(250)

#define GOTO_POINTS_PER_MAP	(16)

#define MAX_CLASSES				(32)
#define MAX_RACES					(16)
#define MAX_MAPS					(512L)
#define MAX_TRANSPORTS			(32)

#define MAX_TRAVELMODES			(7)

#define CAMP_INTERVAL			(8)
#define CAMP_DURATION			(8)

#define EVENING_TIME				(19)
#define MORNING_TIME				(4)
#define DAWN_TIME					(7)

#define MAX_LEVEL					(50)
#define MAX_GAME_LANGUAGES		(3)

#define MONSTERS_PER_GROUP		(18)
#define ANIMS_PER_MONSTER		(8)
#define MONSTER_ANIM_LENGTH	(32)

#define ITEM_NAME_LENGTH		(20)		/* Including EOL */
#define CHAR_NAME_LENGTH		(16)		/* Including EOL */

#define PORTRAIT_WIDTH			(34)
#define PORTRAIT_HEIGHT			(37)

/* Special case */
#define MELLTHAS_CHAR			(5)

/* Bit array sizes */
#define MAX_QUEST					(1024)
#define MAX_EVENT					(MAX_MAPS * EVENTS_PER_MAP)
#define MAX_CD						(MAX_MAPS * NPCS_PER_MAP)
#define MAX_WORDS					(3 * 500)
#define MAX_GOTO_POINTS			(256)
#define MAX_CHESTS				(999)
#define MAX_DOORS					(999)
#define MAX_BYTE_COUNTERS		(300)

#define SAVE_NAME_LENGTH		(38)

#define GOLD_WEIGHT				(5)		/* Weight of 1 gold coin */
#define FOOD_WEIGHT				(250)		/* Weight of 1 food ration */

/* Directions (4) */
#define NORTH						(0)
#define EAST						(1)
#define SOUTH						(2)
#define WEST						(3)

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
#define MAX_LANGUAGES		(3)

/* Standard event sets */
#define DEFAULT_EVENT_SET	(1)
#define ITEM_EVENT_SET		(2)
#define MAGIC_EVENT_SET		(3)

/* Travelmodes */
#define ON_FOOT				(0)
#define RIDING_1				(1)

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
#define LOCKPICK_IT			(21)
#define LIGHT_IT 				(22)

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
#define ISKAI_RACE			(1)

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

#define MAX_ATTRS	 			(9)

/* Skills */
#define CLOSE_RANGE_ATTACK	(0)
#define LONG_RANGE_ATTACK	(1)
#define CRITICAL_HIT			(2)
#define LOCK_PICKING			(3)

#define MAX_SKILLS			(4)

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
#define INCAPABLE_MASK		((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << FLEEING) | (1 << ASLEEP))
#define FIGHTING_MASK		(1 << UNCONSCIOUS)
#define CONTROL_MASK			((1 << PANICKED) | (1 << INSANE))
#define DAMAGE_MASK			(0)
#define DEAD_MASK				(1 << UNCONSCIOUS)
#define MAGIC_MASK			((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << IRRITATED) | (1 << INSANE) | (1 << PANICKED) | (1 << FLEEING) | (1 << ASLEEP))
#define ATTACK_MASK			((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << PANICKED) | (1 << FLEEING) | (1 << ASLEEP))
#define MOVE_MASK				((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << FLEEING) | (1 << ASLEEP))
#define FLEE_MASK				((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << ASLEEP))
#define BLINK_MASK			((1 << UNCONSCIOUS) | (1 << LAMED))
#define TALK_MASK				((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << INSANE) | (1 << ASLEEP))
#define ACTIVE_MASK			((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << INSANE) | (1 << ASLEEP) | (1 << PANICKED) | (1 << FLEEING))
#define DEFEND_MASK			((1 << UNCONSCIOUS) | (1 << LAMED) | (1 << PANICKED) | (1 << FLEEING) | (1 << ASLEEP))
#define INVENTORY_MASK		(1 << FLEEING)

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
#define PARTY_CHAR_TYPE		(0)
#define NPC_CHAR_TYPE		(1)
#define MONSTER_CHAR_TYPE	(2)

/*
 ** Enumerations ***********************************************************
 */

/* Temporary spells */
enum eTemp_spell_type {
	ATTACK_TEMP_SPELL,
	DEFENCE_TEMP_SPELL,
	ANTI_MAGIC_TEMP_SPELL,
	MAX_TEMP_SPELLS
};

/*
 ** Type definitions *******************************************************
 */

typedef enum eTemp_spell_type TEMP_SPELL_TYPE_T;

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

/* Item packets */
struct Item_packet {
	UNBYTE Quantity;
	UNBYTE Charges;
	UNBYTE Nr_enchantments;
	UNBYTE Flags;
	UNSHORT Index;
};

/* Bitmasks for Item_packet.Flags */
#define MAGIC_CHECK			(1 << 0)
#define BROKEN_ITEM			(1 << 1)
#define CURSED					(1 << 2)

/* Item data */
struct Item_data {
	UNBYTE _pad01;
	UNBYTE Type;
	UNBYTE Body_place;
	UNBYTE Break_chance;
	UNBYTE Sex_use;
	UNBYTE Hand_use;
	UNBYTE LP_max;
	UNBYTE SP_max;
	UNBYTE Attribute;
	UNBYTE Attribute_normal;
	UNBYTE Skill;
	UNBYTE Skill_normal;
	UNBYTE Protection_pts;
	UNBYTE Damage_pts;
	UNBYTE Ammo_ID;
	UNBYTE Item_skills[2];
	UNBYTE Malus[2];
	UNBYTE Misc[2];
	UNBYTE Spell_class;
	UNBYTE Spell_nr;
	UNBYTE Init_charges;
	UNBYTE Init_enchantment_normal;
	UNBYTE Max_enchantments;
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
#define UNDROPPABLE 			(1 << 1)
#define MULTIPLE				(1 << 2)
#define COMBAT_EQUIP			(1 << 3)
#define DESTROY_AFTER_USE	(1 << 4)

/* Attributes */
struct Attribute {
	SISHORT Normal;
	SISHORT Maximum;
	SISHORT _pad;
	SISHORT Backup;
};

/* Skills */
struct Skill {
 	SISHORT Normal;
	SISHORT Maximum;
	SISHORT _pad;
	SISHORT Backup;
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
	UNBYTE _pad11;
	UNBYTE _pad00;
	UNBYTE Known_languages;
	UNBYTE Graphics_2D_nr;
	UNBYTE Portrait_nr;
	UNBYTE Full_body_pic_nr;
	UNBYTE Tactic_index;
	UNBYTE Show_set_index;
	UNBYTE Flags;
	UNBYTE Courage;
	UNBYTE _pad01;
	UNBYTE Attacks_per_round;

	UNSHORT xEvent_set_nrs[2];
	UNSHORT xTraining_points;
	UNSHORT Char_gold;
	UNSHORT Char_food;
	UNSHORT Destination_CD_bit;
	UNSHORT xBody_conditions;
	UNSHORT Experience_bonus;

	UNSHORT _pad02[4];

	struct Attribute xAttributes[MAX_ATTRS];
	struct Attribute _pad03[10 - MAX_ATTRS];
	struct Skill xSkills[MAX_SKILLS];
	struct Skill _pad04[10 - MAX_SKILLS];

	SISHORT xLife_points;
	SISHORT xLife_points_maximum;
	UNSHORT _pad07;
	SISHORT xSpell_points;
	SISHORT xSpell_points_maximum;
	UNSHORT _pad08;
	SISHORT xProtection;
	UNSHORT _pad09;
	SISHORT xDamage;
	UNSHORT _pad10;

	UNSHORT _pad05[2];

	SISHORT Special[5];
	UNSHORT Description_text_nr;

	SILONG xExperience_points;
	UNLONG xKnown_spells[MAX_SPELL_CLASSES];
	UNLONG _pad12;

	UNCHAR xName[MAX_LANGUAGES][CHAR_NAME_LENGTH];

	SISHORT xSpell_capabilities[MAX_SPELL_CLASSES][SPELLS_PER_CLASS];

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

/* Chest data */
struct Chest_data {
	struct Item_packet Items[ITEMS_PER_CHEST];
	SISHORT Chest_gold;
	SISHORT Chest_food;
};

/* Temporary spell data */
struct Temporary_spell_data {
	SISHORT Duration;				/* In steps */
	SISHORT Strength;				/* In % */
};

/* Party data */
struct Party_data {
	UNSHORT Year;					/* Time information */
	UNSHORT Month;
	UNSHORT Day;
	UNSHORT Hour;
	UNSHORT Minute;

	UNSHORT Map_nr;				/* Location information */
	SISHORT X;
	SISHORT Y;
	UNSHORT View_direction;

	struct Temporary_spell_data Light_spell;
	struct Temporary_spell_data Temporary_spells[MAX_PARTY_CHARS][MAX_TEMP_SPELLS];

	UNBYTE Walking_order[6];

	/* 3D information */
	UNSHORT Player_X[3];
	UNSHORT Player_Z[3];
	UNLONG Camera_angle;
	UNSHORT Camera_height;
	SISHORT Horizon_Y;

	UNSHORT Monster_move_delay;
	UNSHORT _pad01;

	UNSHORT Light_state;
	UNSHORT Map_music_nr;

	UNSHORT Current_step;
	UNSHORT Current_substep;

	UNBYTE _more_data[200];

	UNSHORT Active_member;

	UNSHORT Member_nrs[6];

	UNSHORT Relative_year;

	UNSHORT Travel_mode;

	UNSHORT Special_item_flags;
	UNSHORT Internal_flags;

	UNSHORT Camp_counter;

	UNBYTE Battle_order[6];

	struct Trans_data Transportations[MAX_TRANSPORTS];

	UNBYTE Quest[(MAX_QUEST + 7) / 8];
	UNBYTE Event[(MAX_EVENT + 7) / 8];
	UNBYTE CD[(MAX_CD + 7) / 8];
	UNBYTE Goto_points[(MAX_GOTO_POINTS + 7) / 8];
	UNBYTE Chest_open[(MAX_CHESTS + 7) / 8];
	UNBYTE Door_open[(MAX_DOORS + 7) / 8];

	UNBYTE Byte_counters[MAX_BYTE_COUNTERS];
};

/* Bitmasks for Party_data.Special_item_flags */
#define COMPASS_ITEM_FLAG		(1 << 0)
#define MONSTER_EYE_ITEM_FLAG	(1 << 1)
#define CLOCK_ITEM_FLAG			(1 << 3)

#endif

