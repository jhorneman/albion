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
#define EVENT_SET				(20)
#define EVENT_TEXT			(21)
#define MAP_TEXT				(22)
#define ITEM_LIST				(23)
#define ITEM_NAMES			(24)
#define ITEM_GFX				(25)
#define FBODY_PICS			(26)
#define AUTOMAP 				(27)
#define AUTOMAP_GFX			(28)
#define SONG					(29)
#define DSAMPLE				(30)
#define WAVELIB				(31)
#define SAVED_GAME			(32)
#define CHEST_DATA			(33)
#define MERCHANT_DATA		(34)
#define NPC_CHAR				(35)
#define MONSTER_GROUP		(36)
#define MONSTER_CHAR			(37)
#define MONSTER_GFX			(38)
#define COMBAT_BACKGROUND	(39)
#define COMBAT_GFX			(40)
#define TACTICAL_ICONS		(41)
#define SPELL_DATA			(42)
#define NPCKL_GFX				(43)
#define FLIC					(44)
#define DICTIONARY			(45)
#define SCRIPT					(46)
#define PICTURE				(47)
#define TRANS_TABLE			(48)

#define MAX_XFTYPES			(49)

/* Flags for XFile_type.Flags */
#define LANGUAGE_DEPENDENT		(1 << 0)
#define SAVE_FILE					(1 << 1)

/*
 ** Global variables *******************************************************
 */

extern struct XFile_type Xftypes[MAX_XFTYPES];

/*
 ** Prototypes *************************************************************
 */

void Init_data_paths(void);
void Add_data_path(UNCHAR *Data_path_ptr);

void Select_data_path(UNSHORT Data_path_index);

void Search_second_data_path(void);

struct XFile_type *Prepare_file_type(UNSHORT File_type_index);

MEM_HANDLE Load_file(UNSHORT File_type_index);
void Save_file(MEM_HANDLE Handle, UNSHORT File_type_index);

UNLONG Get_subfile_length(UNSHORT File_type_index, UNSHORT Subfile_nr);

BOOLEAN Open_subfile(UNSHORT File_type_index, UNSHORT Subfile_nr,
 SISHORT *File_handle_ptr, UNLONG *File_base_offset_ptr);
void Close_subfile(SISHORT File_handle);

MEM_HANDLE Load_subfile(UNSHORT File_type_index, UNSHORT Subfile_nr);
void Save_subfile(MEM_HANDLE Handle, UNSHORT File_type_index,
 UNSHORT Subfile_nr);
MEM_HANDLE Load_unique_subfile(UNSHORT File_type_index, UNSHORT Subfile_nr);

BOOLEAN Load_full_batch(UNSHORT File_type_index, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);
BOOLEAN Load_partial_batch(UNSHORT File_type_index, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);

#endif

