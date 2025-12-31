/************
 * NAME     : XFTYPES.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 27.07.94 16:46
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <stdio.h>

#include <BBDEF.H>

#include <XLOAD.H>

#include <XFTYPES.H>
#include <GAMEVAR.H>
#include <3D_MAP.H>
#include <GAMETEXT.H>

/* global variables */

UNCHAR Save_game_fname[20];

static UNCHAR Filenames[][21] = {
	"XLDLIBS\\MAPDATA0.XLD",
	"XLDLIBS\\ICONDAT0.XLD",
	"XLDLIBS\\ICONGFX0.XLD",
	"XLDLIBS\\PALETTE0.XLD",
	"XLDLIBS\\PALETTE.000",
	"XLDLIBS\\SLAB",
	"XLDLIBS\\PARTGR0.XLD",
	"XLDLIBS\\PARTKL0.XLD",
	"XLDLIBS\\LABDATA0.XLD",
	"XLDLIBS\\3DWALLS0.XLD",
	"XLDLIBS\\3DOBJEC0.XLD",
	"XLDLIBS\\3DOVERL0.XLD",
	"XLDLIBS\\3DFLOOR0.XLD",
	"XLDLIBS\\NPCGR0.XLD",
	"XLDLIBS\\3DBCKGR0.XLD",
	"XLDLIBS\\FONTS0.XLD",
	"XLDLIBS\\BLKLIST0.XLD",
	"XLDLIBS\\PRTCHAR0.XLD",
	"XLDLIBS\\SMLPORT0.XLD",
	"XLDLIBS\\SYSTEXT.GER",
	"XLDLIBS\\SYSTEXT.ENG",
	"XLDLIBS\\SYSTEXT.FRE",
	"XLDLIBS\\EVNTSET0.XLD",
	"XLDLIBS\\EVNTTXT0.XLD",
	"XLDLIBS\\MAPTEXT0.XLD",
	"XLDLIBS\\ITEMLIST.DAT",
	"XLDLIBS\\ITEMNAME.DAT",
	"XLDLIBS\\ITEMGFX",
	"XLDLIBS\\FBODPIX0.XLD",
	"XLDLIBS\\AUTOMAP0.XLD",
	"XLDLIBS\\AUTOMGFX",
	"XLDLIBS\\SONGS0.XLD",
	"XLDLIBS\\SAMPLES0.XLD",
	"XLDLIBS\\WAVELIB0.XLD",
	"",
	"XLDLIBS\\CHESTDT0.XLD",
	"XLDLIBS\\MERCHDT0.XLD",
	"XLDLIBS\\NPCCHAR0.XLD",
	"XLDLIBS\\MONGRP0.XLD",
	"XLDLIBS\\MONCHAR0.XLD",
	"XLDLIBS\\MONGFX0.XLD",
	"XLDLIBS\\COMBACK0.XLD",
	"XLDLIBS\\COMGFX0.XLD",
	"XLDLIBS\\TACTICON",
	"XLDLIBS\\SPELLDAT.DAT",
	"XLDLIBS\\NPCKL0.XLD",
	"XLDLIBS\\FLICS0.XLD"
};

static UNCHAR Ftypenames[][32] = {
  	"Map data",
  	"Icon data",
  	"Icon graphics",
  	"Palette",
  	"",
  	"Slab",
  	"Big party graphics",
  	"Small party graphics",
	"Lab data",
	"3D wall",
	"3D object",
	"3D overlay",
	"3D floor",
	"Big NPC graphics",
	"Background graphics",
	"Font",
	"Block list",
	"Party character data",
	"Small portrait",
	"German system texts",
	"English system texts",
	"French system texts",
	"Event set",
	"Event texts",
	"Map texts",
	"Item list",
	"Item names",
	"Item graphics",
	"Full-body picture",
	"Automap",
	"Automap graphics",
	"Song",
	"Sample",
	"Wave library",
	"Saved game",
	"Chest data",
	"Merchant data",
	"NPC character data",
	"Monster group",
	"Monster character data",
	"Monster graphics",
	"Combat background",
	"Combat graphics",
	"Tactical icons",
	"Spell data",
	"Small NPC graphics",
	"Flic"
};

static struct XFile_type Xftypes[] = {
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[MAP_DATA]
		},
		Filenames[MAP_DATA],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[ICON_DATA]
		},
		Filenames[ICON_DATA],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[ICON_GFX]
		},
		Filenames[ICON_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[PALETTE]
		},
		Filenames[PALETTE],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			0,
			MEM_KILL_ALWAYS,
			NULL
		},
		Filenames[BASEPAL],
		NULL
 	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[SLAB]
		},
		Filenames[SLAB],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[PARTYGR_GFX]
		},
		Filenames[PARTYGR_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[PARTYKL_GFX]
		},
		Filenames[PARTYKL_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[LAB_DATA]
		},
		Filenames[LAB_DATA],
		Convert_lab_data
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[WALL_3D]
		},
		Filenames[WALL_3D],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[OBJECT_3D]
		},
		Filenames[OBJECT_3D],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[OVERLAY_3D]
		},
		Filenames[OVERLAY_3D],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[FLOOR_3D]
		},
		Filenames[FLOOR_3D],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[NPCGR_GFX]
		},
		Filenames[NPCGR_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[BACKGROUND_GFX]
		},
		Filenames[BACKGROUND_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[FONTS]
		},
		Filenames[FONTS],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[BLOCK_LIST]
		},
		Filenames[BLOCK_LIST],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[PARTY_CHAR]
		},
		Filenames[PARTY_CHAR],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[SMALL_PORTRAIT]
		},
		Filenames[SMALL_PORTRAIT],
		NULL
	},

	{
		{
			Relocate_system_texts,
			0xFF,
			128,
			MEM_KILL_ALWAYS,
			Ftypenames[GER_SYSTEM_TEXTS]
		},
		Filenames[GER_SYSTEM_TEXTS],
		NULL
	},

	{
		{
			Relocate_system_texts,
			0xFF,
			128,
			MEM_KILL_ALWAYS,
			Ftypenames[ENG_SYSTEM_TEXTS]
		},
		Filenames[ENG_SYSTEM_TEXTS],
		NULL
	},

	{
		{
			Relocate_system_texts,
			0xFF,
			128,
			MEM_KILL_ALWAYS,
			Ftypenames[FRE_SYSTEM_TEXTS]
		},
		Filenames[FRE_SYSTEM_TEXTS],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[EVENT_SET]
		},
		Filenames[EVENT_SET],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[EVENT_TEXT]
		},
		Filenames[EVENT_TEXT],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[MAP_TEXT]
		},
		Filenames[MAP_TEXT],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[ITEM_LIST]
		},
		Filenames[ITEM_LIST],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[ITEM_NAMES]
		},
		Filenames[ITEM_NAMES],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[ITEM_GFX]
		},
		Filenames[ITEM_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[FBODY_PICS]
		},
		Filenames[FBODY_PICS],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[AUTOMAP]
		},
		Filenames[AUTOMAP],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[AUTOMAP_GFX]
		},
		Filenames[AUTOMAP_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[SONG]
		},
		Filenames[SONG],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[DSAMPLE]
		},
		Filenames[DSAMPLE],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[WAVELIB]
		},
		Filenames[WAVELIB],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			0,
			MEM_KILL_ALWAYS,
			NULL
		},
		Save_game_fname,
		NULL
 	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[CHEST_DATA]
		},
		Filenames[CHEST_DATA],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[MERCHANT_DATA]
		},
		Filenames[MERCHANT_DATA],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[NPC_CHAR]
		},
		Filenames[NPC_CHAR],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[MONSTER_GROUP]
		},
		Filenames[MONSTER_GROUP],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[MONSTER_CHAR]
		},
		Filenames[MONSTER_CHAR],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[MONSTER_GFX]
		},
		Filenames[MONSTER_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[COMBAT_BACKGROUND]
		},
		Filenames[COMBAT_BACKGROUND],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[COMBAT_GFX]
		},
		Filenames[COMBAT_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[TACTICAL_ICONS]
		},
		Filenames[TACTICAL_ICONS],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[SPELL_DATA]
		},
		Filenames[SPELL_DATA],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[NPCKL_GFX]
		},
		Filenames[NPCKL_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			Ftypenames[FLIC]
		},
		Filenames[FLIC],
		NULL
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_file
 * FUNCTION  : Load a file.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.03.95 14:15
 * LAST      : 30.03.95 14:15
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Load_file(UNSHORT File_type_index)
{
	return(XLD_Load_file(&Xftypes[File_type_index]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_file
 * FUNCTION  : Save a file.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:52
 * LAST      : 07.02.95 15:52
 * INPUTS    : MEM_HANDLE Handle - Memory handle of file.
 *             UNSHORT File_type_index - Index of XLD file type.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_file(MEM_HANDLE Handle, UNSHORT File_type_index)
{
	XLD_Save_file(Handle, &Xftypes[File_type_index]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_subfile
 * FUNCTION  : Load a subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:01
 * LAST      : 22.07.94 14:01
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Load_subfile(UNSHORT File_type_index, UNSHORT Subfile_nr)
{
	return(XLD_Load_subfile(&Xftypes[File_type_index], Subfile_nr));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_subfile
 * FUNCTION  : Save a subfile into a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:59
 * LAST      : 07.02.95 15:59
 * INPUTS    : MEM_HANDLE Handle - Memory handle of file.
 *             UNSHORT File_type_index - Index of XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_subfile(MEM_HANDLE Handle, UNSHORT File_type_index, UNSHORT Subfile_nr)
{
	XLD_Save_subfile(Handle, &Xftypes[File_type_index], Subfile_nr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_unique_subfile
 * FUNCTION  : Load a unique subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:03
 * LAST      : 23.07.94 16:03
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Load_unique_subfile(UNSHORT File_type_index, UNSHORT Subfile_nr)
{
	return(XLD_Load_unique_subfile(&Xftypes[File_type_index], Subfile_nr));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_full_batch
 * FUNCTION  : Load a batch of subfiles from a library. ALL files in the
 *              batch must be loaded.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.03.95 14:38
 * LAST      : 15.03.95 14:38
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 *             UNSHORT Number - Number of subfiles in batch.
 *             UNSHORT *Subfile_list - Pointer to list of subfile numbers (1...).
 *             MEM_HANDLE *Handle_list - Pointer to list of memory handles.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * NOTES     : - If the subfile number has the flag XLD_UNIQUE set, a unique
 *              copy will be loaded.
 *             - If the subfile number is zero, handle NULL will be returned.
 *             - The subfile list will remain unchanged.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_full_batch(UNSHORT File_type_index, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list)
{
/*	UNSHORT i;

	for (i=0;i<Number;i++)
	{
		Handle_list[i] = XLD_Load_subfile(&Xftypes[File_type_index],
		 Subfile_list[i]);
	}

	return(TRUE); */

	return(XLD_Load_full_batch(&Xftypes[File_type_index], Number,
	 Subfile_list, Handle_list));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_partial_batch
 * FUNCTION  : Load a batch of subfiles from a library. NOT all files in the
 *              batch must be loaded.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:05
 * LAST      : 15.03.95 14:43
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 *             UNSHORT Number - Number of subfiles in batch.
 *             UNSHORT *Subfile_list - Pointer to list of subfile numbers (1...).
 *             MEM_HANDLE *Handle_list - Pointer to list of memory handles.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * NOTES     : - If the subfile number has the flag XLD_UNIQUE set, a unique
 *              copy will be loaded.
 *             - If the subfile number is zero, handle NULL will be returned.
 *             - The subfile list will remain unchanged.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_partial_batch(UNSHORT File_type_index, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list)
{
/*	UNSHORT i;

	for (i=0;i<Number;i++)
	{
		Handle_list[i] = XLD_Load_subfile(&Xftypes[File_type_index],
		 Subfile_list[i]);
	}

	return(TRUE); */

	return(XLD_Load_partial_batch(&Xftypes[File_type_index], Number,
	 Subfile_list, Handle_list));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_saved_game
 * FUNCTION  : Select a saved game.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 13:18
 * LAST      : 21.02.95 13:18
 * INPUTS    : UNSHORT Number - Number of saved game (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Always call this function before loading or saving a
 *              saved game.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_saved_game(UNSHORT Number)
{
	/* Build complete filename */
	sprintf(Save_game_fname, "SAVES\\SAVE.%-3u", Number);
}

