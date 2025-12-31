/************
 * NAME     : XFTYPES.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 27-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef XFTYPES_H
#define XFTYPES_H

#include <BBDEF.H>
#include <BBMEM.H>

#include <XLOAD.H>

/*
 ** Defines ****************************************************************
 */

#define MAP_DATA				(0)
#define ICON_DATA				(1)
#define ICON_GFX				(2)
#define PALETTE				(3)
#define BASEPAL				(4)
#define SLAB					(5)
#define PARTYGR_GFX			(6)
#define PARTYKL_GFX			(7)
#define LAB_DATA				(8)
#define WALL_3D				(9)
#define OBJECT_3D				(10)
#define OVERLAY_3D			(11)
#define FLOOR_3D				(12)
#define NPCGR_GFX				(13)
#define BACKGROUND_GFX		(14)
#define FONTS					(15)
#define BLOCK_LIST			(16)
#define PARTY_CHAR	  		(17)
#define SMALL_PORTRAIT		(18)
#define SYSTEM_TEXTS			(19)
#define GER_SYSTEM_TEXTS	(19)
#define ENG_SYSTEM_TEXTS	(20)
#define FRE_SYSTEM_TEXTS	(21)
#define EVENT_SET				(22)
#define EVENT_TEXT			(23)
#define MAP_TEXT				(24)
#define ITEM_LIST				(25)
#define ITEM_NAMES			(26)
#define ITEM_GFX				(27)
#define FBODY_PICS			(28)
#define AUTOMAP 				(29)
#define AUTOMAP_GFX			(30)
#define SONG					(31)
#define DSAMPLE				(32)
#define WAVELIB				(33)
#define SAVED_GAME			(34)
#define CHEST_DATA			(35)
#define MERCHANT_DATA		(36)
#define NPC_CHAR				(37)
#define MONSTER_GROUP		(38)
#define MONSTER_CHAR			(39)
#define MONSTER_GFX			(40)
#define COMBAT_BACKGROUND	(41)
#define COMBAT_GFX			(42)
#define TACTICAL_ICONS		(43)
#define SPELL_DATA			(44)
#define NPCKL_GFX				(45)
#define FLIC					(46)
#define DICTIONARY			(47)
#define SCRIPT					(48)
#define PICTURE				(49)

#define MAX_XFTYPES			(50)

/*
 ** Global variables *******************************************************
 */

extern UNCHAR Save_game_fname[20];

/*
 ** Prototypes *************************************************************
 */

void Init_data_paths(void);
void Add_data_path(UNCHAR *Data_path_ptr);

void Select_data_path(UNSHORT Data_path_index);

void Search_second_data_path(void);

MEM_HANDLE Load_file(UNSHORT File_type_index);
void Save_file(MEM_HANDLE Handle, UNSHORT File_type_index);

MEM_HANDLE Load_subfile(UNSHORT File_type_index, UNSHORT Subfile_nr);
void Save_subfile(MEM_HANDLE Handle, UNSHORT File_type_index,
 UNSHORT Subfile_nr);
MEM_HANDLE Load_unique_subfile(UNSHORT File_type_index, UNSHORT Subfile_nr);

BOOLEAN Load_full_batch(UNSHORT File_type_index, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);
BOOLEAN Load_partial_batch(UNSHORT File_type_index, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);

void Select_saved_game(UNSHORT Number);

void Load_initial_game_state(void);
void Load_game_state(UNSHORT Number, BOOLEAN No_load_flag);
void Save_game_state(UNSHORT Number, UNCHAR *Name);

#endif

