; Game constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 10-3-1994

	incdir	DDT:
	include	Constants/Global.i

Items_per_char	EQU 24
Items_per_chest	EQU 24
Events_per_map	EQU 64
NPCs_per_map	EQU 32

Max_classes	EQU 32
Max_races	EQU 15
Max_maps	EQU 1023
Max_transports	EQU 32
Max_mods	EQU 750
Max_skills	EQU 6
Max_attrs	EQU 7
Max_level	EQU 50
Max_languages	EQU 8
Max_spell_classes	EQU 7
Max_spells	EQU 30
Max_monsters	EQU 18

Max_quest	EQU 1024				; Bit array sizes
Max_event	EQU Max_maps*Events_per_map
Max_CD	EQU Max_maps*NPCs_per_map
Max_words	EQU 1024
Max_Goto_points	EQU 256
Max_chests	EQU 255
Max_doors	EQU 255
Max_byte_counters	EQU 32

Save_name_length	EQU 38

Gold_weight	EQU 5			; Weight of 1 gold coin
Food_weight	EQU 250			; Weight of 1 food unit

;*****************************************************************************
; These are directions
	rsreset
	rsreset
North:	rs.b 1
East:	rs.b 1
South:	rs.b 1
West:	rs.b 1

	rsreset
North8:	rs.b 1
North_east:	rs.b 1
East8:	rs.b 1
South_east:	rs.b 1
South8:	rs.b 1
South_west:	rs.b 1
West8:	rs.b 1
North_west:	rs.b 1

;*****************************************************************************
; These are the supported languages
	rsreset
German:	rs.b 1
English:	rs.b 1
French:	rs.b 1

;*****************************************************************************
; These are the constant world time values

Months_per_year	EQU 12
Days_per_month	EQU 30
Hours_per_day	EQU 24
Minutes_per_hour	EQU 60
Minutes_per_step	EQU 5
VBLs_per_step	EQU 500			; (10 seconds on 50	Hz)
Moves_per_step	EQU 4

Max_steps	= Hours_per_day*(Minutes_per_hour/Minutes_per_step)

;*****************************************************************************
; These are the states
	rsreset
Standing:	rs.b 1
Sitting_N_left:	rs.b 1
Sitting_N_right:	rs.b 1
Sitting_E:	rs.b 1
Sitting_S_left:	rs.b 1
Sitting_S_right:	rs.b 1
Sitting_W:	rs.b 1
Sleeping:	rs.b 1
Swimming:	rs.b 1
Climbing:	rs.b 1
Wading:	rs.b 1
Walking_on_grass:	rs.b 1

;*****************************************************************************
; These are the travelmodes
	rsreset
On_foot:	rs.b 1
Riding_1:	rs.b 1
Riding_2:	rs.b 1
Raft:	rs.b 1
Rowing_boat:	rs.b 1
Max_travelmodes:	rs.b 0

;*****************************************************************************
; These are the offsets for the transportations
	rsreset
Trans_type:	rs.b 1
Trans_X:	rs.b 1
Trans_Y:	rs.b 1
	rseven
Trans_map_nr:	rs.w 1
Trans_data_size:	rs.b 0

;*****************************************************************************
; These are the offsets for the modifications
	rsreset
Modif_map_nr:	rs.w 1
Modif_X:	rs.b 1
Modif_Y:	rs.b 1
Modif_mode:	rs.b 1			; See Change Icon event
	rseven
Modif_icon:	rs.w 1
Modif_data_size:	rs.b 0

;*****************************************************************************
; These are the offsets for the automapper notes
	rsreset
Autonote_map_nr:	rs.b 1
Autonote_X:	rs.b 1			; Screen coordinates
Autonote_Y:	rs.b 1
Autonote_string:	rs.b 0
	; EVEN

;*****************************************************************************
; These are the offsets for the object packets
	rsreset
Quantity:	rs.b 1				; Quantity
Charges:	rs.b 1				; Magical	charges  (-1 = infinite)
Enchantment_normal:	rs.b 1
Item_bits_DYNAMIC:	rs.b 1
Object_index:	rs.w 1
Object_packet_size:	rs.b 0

Copy_packet	macro
	move.l	(\1),(\2)			; Assumes size is 6 !!!
	move.w	4(\1),4(\2)
	endm

;*****************************************************************************
; These are the offsets for the item data
	rsreset
Item_pic:	rs.b 1
Item_type:	rs.b 1
Item_body_place:	rs.b 1
Item_break:	rs.b 1
Sex_use:	rs.b 1
Hand_use:	rs.b 1
Ring_use:	rs.b 1
LP_max:	rs.b 1
SP_max:	rs.b 1
Attribute:	rs.b 1
Attribute_normal:	rs.b 1
Skill:	rs.b 1
Skill_normal:	rs.b 1
Protection_pts:	rs.b 1
Damage_pts:	rs.b 1
Ammo_ID:	rs.b 1
Item_skill_1:	rs.b 1
Item_skill_2:	rs.b 1
Malus_1:	rs.b 1
Malus_2:	rs.b 1
Misc_1:	rs.b 1
Misc_2:	rs.b 1
Spell_class:	rs.b 1
Spell_number:	rs.b 1
Init_charges:	rs.b 1			; Initial value
Init_enchantment_normal:	rs.b 1		; Initial value
Enchantment_max:	rs.b 1			; Maximum enchantments
Charges_max:	rs.b 1			; Maximum spell charges
Item_bits_STATIC:	rs.b 1
Init_item_bits_DYNAMIC:	rs.b 1		; Initial value
	rseven
Weight:	rs.w 1
Price:	rs.w 1
Class_use:	rs.l 1
Item_data_size:	rs.b 0

Item_name_length	EQU 20			; Includes EOL

;*****************************************************************************
; These are the item types
	rsreset
	rs.b 1
Armour_itemtype:	rs.b 1
Helmet_itemtype:	rs.b 1
Shoe_itemtype:	rs.b 1
Shield_itemtype:	rs.b 1

Closerange_itemtype:	rs.b 1
Longrange_itemtype:	rs.b 1
Ammo_itemtype:	rs.b 1
Textscroll_itemtype:	rs.b 1
Spellscroll_itemtype:	rs.b 1

Drink_itemtype:	rs.b 1
Amulet_itemtype:	rs.b 1
Brooch_itemtype:	rs.b 1
Ring_itemtype:	rs.b 1
Valuable_itemtype:	rs.b 1

Tool_itemtype:	rs.b 1
Key_itemtype:	rs.b 1
Normal_itemtype:	rs.b 1
Magic_itemtype:	rs.b 1
Special_itemtype:	rs.b 1

Transport_itemtype:	rs.b 1

;*****************************************************************************
; These are the STATIC item bits
	rsreset
Cursed:	rs.b 1
Droppable:	rs.b 1
Multiple:	rs.b 1
Combat_equip:	rs.b 1
Destroy_after_use:	rs.b 1 
Unbreakable:	rs.b 1

;*****************************************************************************
; These are the DYNAMIC item bits
	rsreset
Magic_check:	rs.b 1
Broken_item:	rs.b 1

;*****************************************************************************
; These are the body inventory slot numbers
	rsreset
Neck:	rs.b 1
Head:	rs.b 1
Tail:	rs.b 1
Right_hand:	rs.b 1
Body:	rs.b 1
Left_hand:	rs.b 1
Right_finger:	rs.b 1
Feet:	rs.b 1
Left_finger:	rs.b 1

;*****************************************************************************
; These are the body inventory slot offsets
	rsreset
Neck_slot:	rs.b Object_packet_size
Head_slot:	rs.b Object_packet_size
Tail_slot:	rs.b Object_packet_size
Right_hand_slot:	rs.b Object_packet_size
Body_slot:	rs.b Object_packet_size
Left_hand_slot:	rs.b Object_packet_size
Right_finger_slot:	rs.b Object_packet_size
Feet_slot:	rs.b Object_packet_size
Left_finger_slot:	rs.b Object_packet_size
Backpack_slots:	rs.b 0

;*****************************************************************************
; These are the offsets for the character data
	rsreset
Char_type:	rs.b 1			; 0 = Party, 1 = NPC, 2 = Monster
Char_sex:	rs.b 1			; 0 = Male, 1 = Female, 2 = Neutral
Char_race:	rs.b 1
Char_class:	rs.b 1
Known_spell_classes:	rs.b 1
Char_level:	rs.b 1
Hands_occupied:	rs.b 1
Ringfingers_occupied:	rs.b 1
Known_languages:	rs.b 1
Char_type_bits:	rs.b 1

Small_portrait_nr:	rs.b 1
TwoD_gfx_nr:	rs.b 1
Full_body_pic_nr:	rs.b 1
Tactic_type:	rs.b 1
Tactical_icon_nr:	rs.b 1
Courage:	rs.b 1
Attacks_per_round:	rs.b 1
	rseven
Event_set_nr_1:	rs.w 1
Event_set_nr_2:	rs.w 1
Nr_training_points:	rs.w 1
Gold_coins:	rs.w 1
Food_rations:	rs.w 1
Destination_CD_bit_nr:	rs.w 1
Body_conditions:	rs.w 1
Experience_bonus:	rs.w 1

; --------- Attributes ----------------------------
Attributes:	rs.b 0
AStrength:	rs.w 4				; Magic adds to normal
AIntelligence:	rs.w 4
ADexterity:	rs.w 4
ASpeed:	rs.w 4
AStamina:	rs.w 4
ALuck:	rs.w 4
AMagic_res:	rs.w 4
AMagic_talent:	rs.w 4
AAge:	rs.w 4

; --------- Skills --------------------------------
Skills:	rs.b 0
SCloserange_attack:	rs.w 4			; Magic adds to normal
SLongrange_attack:	rs.w 4
SSwimming:	rs.w 4
SCritical_hit:	rs.w 4
SPicking_locks:	rs.w 4

Life_points:	rs.w 3			; Magic adds to maximum
Power_points:	rs.w 3
Protection:	rs.w 1
Protection_magic:	rs.w 1
Damage:	rs.w 1
Damage_magic:	rs.w 1

Special_1:	rs.w 1				; Attacks per round
Special_2:	rs.w 1				; Life-points
Special_3:	rs.w 1				; Power-points
Special_5:	rs.w 1				; Training points

Description_text_nr:	rs.w 1		; -1 = no text

Experience_points:	rs.l 1
Spells_class_0:	rs.l 1
Spells_class_1:	rs.l 1
Spells_class_2:	rs.l 1
Spells_class_3:	rs.l 1
Spells_class_4:	rs.l 1
Spells_class_5:	rs.l 1
Spells_class_6:	rs.l 1
Weight_normal:	rs.l 1

Char_name:	rs.b 15				; German
	rs.b 1
	rs.b 15				; English
	rs.b 1
	rs.b 15				; French
	rs.b 1
Spell_capabilities_0:	rs.b Max_spells
Spell_capabilities_1:	rs.b Max_spells
Spell_capabilities_2:	rs.b Max_spells
Spell_capabilities_3:	rs.b Max_spells
Spell_capabilities_4:	rs.b Max_spells
Spell_capabilities_5:	rs.b Max_spells
Spell_capabilities_6:	rs.b Max_spells
Char_data_size:	rs.b 0

;*****************************************************************************
; These are the offsets for attribute/skill values
	rsreset
	rs.w 1			; Normal
Maximum:	rs.w 1
Magic:	rs.w 1
Backup:	rs.w 1
Skill_data_size:	rs.b 0
Attr_data_size:	rs.b 0

;*****************************************************************************
; These are the offsets for the PARTY character data
	rsreset
	rs.b Char_data_size
Char_inventory:	rs.b (9+Items_per_char)*Object_packet_size

;*****************************************************************************
; These are the offsets for the NPC character data
	rsreset
	rs.b Char_data_size

;*****************************************************************************
; These are the offsets for the MONSTER character data
	rsreset
	rs.b Char_data_size
	rs.b (9+Items_per_char)*Object_packet_size
Clone_data_size:	rs.b 0
Monster_gfx_nr:	rs.b 1
Anim_motion:	rs.b 1			; Circle / wave bits
Recolour_pal_nr:	rs.b 1
	rseven
Anim_data:	rs.b 256				; Animation sequences
Anim_lengths:	rs.b 8			; Animation lengths
Monster_baseline:	rs.w 1
Monster_X_factor:	rs.w 1
Monster_Y_factor:	rs.w 1

;*****************************************************************************
; These are the monster animations
	rsreset
	rs.b 1
Walk_anim:	rs.b 1
Closerange_anim:	rs.b 1
Longrange_anim:	rs.b 1
Magic_anim:	rs.b 1
Hit_anim:	rs.b 1
Die_anim:	rs.b 1
Appear_anim:	rs.b 1
Special_anim:	rs.b 1

Set_anim	macro
	move.b	#\1,Part_anim(\2)		; Set animation type
	st	Part_anim_index(\2)
	bclr	#Part_wavedir,Part_flags(\2)	; Forth
	endm

;*****************************************************************************
; These are the character type bits
	rsreset
Animal_chartype:	rs.b 1
Intelligent_chartype:	rs.b 1
Supernatural_chartype:	rs.b 1
Person_chartype:	rs.b 1
Evil_chartype:	rs.b 1
God_chartype:	rs.b 1
Ghost_chartype:	rs.b 1
End_monster_chartype:	rs.b 1

;*****************************************************************************
; These are the offsets for the spell data
	rsreset
Spell_area_bits:	rs.b 1
Spell_point_cost:	rs.b 1
Spell_learn_point_cost:	rs.b 1
Spell_target_bits:	rs.b 1
Spell_data_size:	rs.b 0

;*****************************************************************************
; These are the spell area bits
	rsreset
City_spell_area:	rs.b 1
Dungeon_spell_area:	rs.b 1
Wilderness_spell_area:	rs.b 1
Interior_spell_area:	rs.b 1
Camp_spell_area:	rs.b 1
Combat_spell_area:	rs.b 1
Spaceship_spell_area:	rs.b 1

;*****************************************************************************
; These are the spell target bits
	rsreset
Single_target_bit:	rs.b 1
	rs.b 5
Item_target_bit:	rs.b 1
Special_target_bit:	rs.b 1

;*****************************************************************************
; These are other game constants

	include	Game/Events.i
	include	Game/Map.i
	include	Game/2D_map.i
	include	Game/3D_map.i
