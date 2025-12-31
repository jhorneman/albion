/************
 * NAME     : GAME.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 26-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GAME_H
#define GAME_H

/*
 ** Defines ****************************************************************
 */

#define MAX_ATTRIBUTES (10)
#define MAX_SKILLS (10)
#define MAX_SPELL_CLASSES (7)
#define SPELLS_PER_CLASS (30)
#define ITEMS_ON_BODY (9)
#define ITEMS_PER_CHAR (24)

/*
 ** Structure definitions **************************************************
 */

struct Item_packet {
	UNBYTE Quantity;
	UNBYTE Spell_charges;
	UNBYTE Enchantments;
	UNBYTE Dynamic_item_bits;
	UNSHORT Object_index;
};

struct Item_data {
	UNBYTE Item_graphics_nr;
	UNBYTE Item_type;
	UNBYTE Body_slot_nr;
	UNBYTE Break_chance;
	UNBYTE Sex_use;
	UNBYTE Hands_needed;
	SIBYTE LP_max_bonus;
	SIBYTE SP_max_bonus;
	UNBYTE Attribute_nr;
	SIBYTE Attribute_bonus;
	UNBYTE Skill_nr;
	SIBYTE Skill_bonus;
	SIBYTE Protection_bonus;
	SIBYTE Damage_bonus;
	UNBYTE Ammo_ID;
	UNBYTE Taxed_skill_nr[2];
	SIBYTE Skill_tax[2];
	UNBYTE Misc[2];
	UNBYTE Spell_class;
	UNBYTE Spell_nr;
	UNBYTE Initial_charges;
	UNBYTE Initial_enchantments;
	UNBYTE Max_enchantments;
	UNBYTE Max_charges;
	UNBYTE Static_item_bits;
	UNBYTE Initial_dynamic_item_bits;
	UNBYTE _pad01;
	UNSHORT Weight;
	UNSHORT Price;
	UNLONG Class_use;
};

struct Attribute {
	UNSHORT Normal;
	UNSHORT Maximum;
	SISHORT Magic;
	UNSHORT Backup;
};

struct Skill {
	UNSHORT Normal;
	UNSHORT Maximum;
	SISHORT Magic;
	UNSHORT Backup;
};

struct Character_data {
	UNBYTE Character_type;
	UNBYTE Character_sex;
	UNBYTE Character_race;
	UNBYTE Character_class;
	UNBYTE Known_spell_classes;
	UNBYTE Character_level;
	UNBYTE Hands_occupied;
	UNBYTE _pad04;
	UNBYTE Known_languages;
	UNBYTE _2D_graphics_nr;
	UNBYTE Small_portrait_nr;
	UNBYTE Full_body_pic_nr;
	UNBYTE Tactic_type;
	UNBYTE Tactical_icon_nr;
	UNBYTE Character_type_bits;
	UNBYTE Courage;
	UNBYTE _pad00;
	UNBYTE Attacks_per_round;
	UNSHORT Event_set_1_nr;
	UNSHORT Event_set_2_nr;
	UNSHORT Training_points;
	UNSHORT Gold_coins;
	UNSHORT Food_rations;
	UNSHORT Destination_CD_nr;
	UNSHORT Body_conditions;
	UNSHORT EP_bonus;
	UNSHORT _pad01[4];
	struct Attribute Attributes[MAX_ATTRIBUTES];
	struct Skill Skills[MAX_SKILLS];
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
	UNSHORT _pad02[2];
	UNSHORT Special[5];
	UNSHORT Description_text_nr;
	UNLONG Experience_points;
	UNLONG Known_spells[MAX_SPELL_CLASSES];
	UNLONG Carried_weight;
	UNCHAR German_name[16];
	UNCHAR English_name[16];
	UNCHAR French_name[16];
	UNBYTE Spell_capabilities[MAX_SPELL_CLASSES][SPELLS_PER_CLASS];

	/* Inventory data : Party & Monster characters only */
	struct Inventory[ITEMS_ON_BODY+ITEMS_PER_CHAR];

	/* Combat data : Monster characters only */
	UNBYTE Monster_graphics_nr;
	UNBYTE Animation_types;
	UNBYTE Recolour_palette_nr;
	UNBYTE _pad03;
	UNBYTE Animations[8][32];
	UNBYTE Animation_lengths[8];
	SISHORT Baseline;
	UNSHORT Horizontal_zoom_factor;
	UNSHORT Vertical_zoom_factor;
	};

#endif

