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
#include <BBDOS.H>
#include <BBMEM.H>

#include <XLOAD.H>

#include <XFTYPES.H>
#include <GAMEVAR.H>
#include <MAP.H>
#include <DIALOGUE.H>
#include <3D_MAP.H>
#include <GAMETEXT.H>

/* defines */

#define MAX_XFTYPES		(48)

/* Flags for XFile_type.Flags */
#define LANGUAGE_DEPENDENT		(1 << 0)
#define SAVE_FILE					(1 << 1)

/* prototypes */

struct XFile_type *Prepare_XFile_type(UNSHORT File_type_index);

/* global variables */

UNCHAR Save_game_fname[20];

static UNCHAR Data_path[] = "XLDLIBS";
static UNCHAR *Language_paths[] = {
	"GERMAN",
	"ENGLISH",
	"FRENCH"
};

static UNCHAR Current_save_path[] = "CURRENT";
static UNCHAR Initial_save_path[] = "INITIAL";
static UNCHAR *Save_path_ptr = Current_save_path;

static UNCHAR Filenames[MAX_XFTYPES][13] = {
	"MAPDATA0.XLD",
	"ICONDAT0.XLD",
	"ICONGFX0.XLD",
	"PALETTE0.XLD",
	"PALETTE.000",
	"SLAB",
	"PARTGR0.XLD",
	"PARTKL0.XLD",
	"LABDATA0.XLD",
	"3DWALLS0.XLD",
	"3DOBJEC0.XLD",
	"3DOVERL0.XLD",
	"3DFLOOR0.XLD",
	"NPCGR0.XLD",
	"3DBCKGR0.XLD",
	"FONTS0.XLD",
	"BLKLIST0.XLD",
	"PRTCHAR0.XLD",
	"SMLPORT0.XLD",
	"SYSTEXT.GER",
	"SYSTEXT.ENG",
	"SYSTEXT.FRE",
	"EVNTSET0.XLD",
	"EVNTTXT0.XLD",
	"MAPTEXT0.XLD",
	"ITEMLIST.DAT",
	"ITEMNAME.DAT",
	"ITEMGFX",
	"FBODPIX0.XLD",
	"AUTOMAP0.XLD",
	"AUTOMGFX",
	"SONGS0.XLD",
	"SAMPLES0.XLD",
	"WAVELIB0.XLD",
	"",
	"CHESTDT0.XLD",
	"MERCHDT0.XLD",
	"NPCCHAR0.XLD",
	"MONGRP0.XLD",
	"MONCHAR0.XLD",
	"MONGFX0.XLD",
	"COMBACK0.XLD",
	"COMGFX0.XLD",
	"TACTICON",
	"SPELLDAT.DAT",
	"NPCKL0.XLD",
	"FLICS0.XLD",
	"WORDLIS0.XLD"
};

static UNCHAR Ftypenames[MAX_XFTYPES][32] = {
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
	"Flic",
	"Dictionary"
};

static struct XFile_type Xftypes[MAX_XFTYPES] = {
	/* Map data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* 2D icon data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* 2D icon graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Palettes */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Base palette */
	{
		{
			MEM_Relocate,
			0xFF,
			0,
			MEM_KILL_ALWAYS,
			NULL
		},
		NULL,
		NULL,
		0
 	},

	/* Slab */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Large 2D party graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Small 2D party graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* 3D LAByrinth data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		Convert_lab_data,
		0
	},

	/* 3D wall graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* 3D object graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* 3D overlay graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* 3D floor graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Large 2D NPC graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* 3D background graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Fonts */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* 2D icon block lists */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Party member character data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		SAVE_FILE
	},

	/* Small portraits */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* German system texts */
	{
		{
			Relocate_system_texts,
			0xFF,
			128,
			MEM_KILL_ALWAYS,
			NULL
		},
		NULL,
		NULL,
		LANGUAGE_DEPENDENT
	},

	/* English system texts */
	{
		{
			Relocate_system_texts,
			0xFF,
			128,
			MEM_KILL_ALWAYS,
			NULL
		},
		NULL,
		NULL,
		LANGUAGE_DEPENDENT
	},

	/* French system texts */
	{
		{
			Relocate_system_texts,
			0xFF,
			128,
			MEM_KILL_ALWAYS,
			NULL
		},
		NULL,
		NULL,
		LANGUAGE_DEPENDENT
	},

	/* Event sets */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Event texts */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		LANGUAGE_DEPENDENT
	},

	/* Map texts */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		LANGUAGE_DEPENDENT
	},

	/* Item list */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Item names (NOT language dependent!) */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Item graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Full-body pictures */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Automaps */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		SAVE_FILE
	},

	/* Automap graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Songs */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Samples */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Wave libraries */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Saved game */
	{
		{
			MEM_Relocate,
			0xFF,
			0,
			MEM_KILL_ALWAYS,
			NULL,
		},
		Save_game_fname,
		NULL,
		0
 	},

	/* Chest data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		SAVE_FILE
	},

	/* Merchant data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		SAVE_FILE
	},

	/* NPC character data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		SAVE_FILE
	},

	/* Monster groups */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Monster character data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Monster graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Combat screen backgrounds */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Combat effect graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Tactical icons */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Spell data */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Small 2D NPC graphics */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* FLICs */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		0
	},

	/* Dictionaries */
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			NULL
		},
		NULL,
		NULL,
		LANGUAGE_DEPENDENT
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_XFile_type
 * FUNCTION  : Prepare an entry from the eXtended file type array.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.95 12:18
 * LAST      : 12.07.95 13:32
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 * RESULT    : struct XFile_type * - Pointer to eXtended file type.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct XFile_type *
Prepare_XFile_type(UNSHORT File_type_index)
{
	static UNCHAR Path[100];

	struct XFile_type *XFT;

	/* Get eXtended file type */
	XFT = &(Xftypes[File_type_index]);

	/* Insert file type name for BBMEM */
	XFT->Ftype.Name = Ftypenames[File_type_index];

	/* Is this a language-dependent file ? */
	if (XFT->Flags & LANGUAGE_DEPENDENT)
	{
		/* Yes -> Build path */
		sprintf(Path, "%s\\%s\\%s", Data_path, Language_paths[Language],
		 Filenames[File_type_index]);
	}
	else
	{
		/* Is this a save file ? */
		if (XFT->Flags & SAVE_FILE)
		{
			/* Yes -> Build path */
			sprintf(Path, "%s\\%s\\%s", Data_path, Save_path_ptr,
			 Filenames[File_type_index]);
		}
		else
		{
			/* No -> Build path */
			sprintf(Path, "%s\\%s", Data_path, Filenames[File_type_index]);
		}
	}

	/* Insert path */
	XFT->Filename = Path;

	return XFT;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_file
 * FUNCTION  : Load a file.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.03.95 14:15
 * LAST      : 12.07.95 13:31
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
	struct XFile_type *XFT;

	XFT = Prepare_XFile_type(File_type_index);

	return(XLD_Load_file(XFT));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_file
 * FUNCTION  : Save a file.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:52
 * LAST      : 12.07.95 13:31
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
	struct XFile_type *XFT;

	XFT = Prepare_XFile_type(File_type_index);

	XLD_Save_file(Handle, XFT);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_subfile
 * FUNCTION  : Load a subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:01
 * LAST      : 12.07.95 13:31
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
	struct XFile_type *XFT;

	XFT = Prepare_XFile_type(File_type_index);

	return(XLD_Load_subfile(XFT, Subfile_nr));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_subfile
 * FUNCTION  : Save a subfile into a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:59
 * LAST      : 12.07.95 13:31
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
	struct XFile_type *XFT;

	XFT = Prepare_XFile_type(File_type_index);

	XLD_Save_subfile(Handle, XFT, Subfile_nr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_unique_subfile
 * FUNCTION  : Load a unique subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:03
 * LAST      : 12.07.95 13:31
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
	struct XFile_type *XFT;

	XFT = Prepare_XFile_type(File_type_index);

	return(XLD_Load_unique_subfile(XFT, Subfile_nr));
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
 * LAST      : 12.07.95 13:31
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
	struct XFile_type *XFT;

	XFT = Prepare_XFile_type(File_type_index);

	return(XLD_Load_full_batch(XFT, Number, Subfile_list, Handle_list));
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
 * LAST      : 12.07.95 13:31
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
	struct XFile_type *XFT;

	XFT = Prepare_XFile_type(File_type_index);

	return(XLD_Load_partial_batch(XFT, Number, Subfile_list, Handle_list));
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

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_initial_game_state
 * FUNCTION  : Load initial game state.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.95 12:04
 * LAST      : 12.07.95 12:04
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Load_initial_game_state(void)
{
	struct XFile_type *XFT;
	MEM_HANDLE Handle;
	UNSHORT Zero = 0;
	UNSHORT i, j;

	/* Initialize modifications */
	Init_modifications((UNBYTE *) &Zero);

	/* Initialize dialogue log */
	Init_dialogue_log((UNBYTE *) &Zero);

	/* Check all eXtended file types */
	for (i=0;i<MAX_XFTYPES;i++)
	{
		/* Is this a file that should be saved ? */
		if (Xftypes[i].Flags & SAVE_FILE)
		{
			/* Yes -> Try all subgroups */
			for (j=0;j<=9;j++)
			{
				/* Prepare eXtended file type for reading the current file */
				Save_path_ptr = Initial_save_path;
				XFT = Prepare_XFile_type(i);

				/* Try to load this XLD-library */
				Handle = XLD_Load_XLD_library(XFT, j);

				/* Success ? */
				if (Handle)
				{
					/* Yes -> Prepare eXtended file type for writing the current
					 file */
					Save_path_ptr = Current_save_path;
					XFT = Prepare_XFile_type(i);

					/* Save this XLD-library */
					XLD_Save_XLD_library(Handle, XFT, j);

					/* Destroy XLD-library */
					MEM_Kill_memory(Handle);
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
		}
	}

	/* Reset save path pointer */
	Save_path_ptr = Current_save_path;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_game_state
 * FUNCTION  : Load game state.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 13:41
 * LAST      : 30.06.95 11:55
 * INPUTS    : UNSHORT Number - Number of saved game (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Load_game_state(UNSHORT Number)
{
	MEM_HANDLE Handle;
	UNLONG Length;
	SISHORT File_handle;
	UNBYTE *Ptr;

	/* Select saved game */
	Select_saved_game(Number);

	/* Open saved game file */
	File_handle = DOS_Open(Save_game_fname, BBDOSFILESTAT_READ);

	/* Skip saved game name */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Seek(File_handle, BBDOS_SEEK_CURRENTPOS, Length);

	/* Read party data */
	DOS_Read(File_handle, (UNBYTE *) &PARTY_DATA, sizeof(struct Party_data));

	/* Read modification list */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Init_modifications(Ptr);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Read dialogue log */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Init_dialogue_log(Ptr);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Read automapper notes */

	#if FALSE
	/* Read party character data */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Save_file(Handle, PARTY_CHAR);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Read NPC character data */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Save_file(Handle, NPC_CHAR);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Read chest data */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Save_file(Handle, CHEST_DATA);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Read merchant data */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Save_file(Handle, MERCHANT_DATA);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Read automap data */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Save_file(Handle, AUTOMAP);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);
	#endif

	/* Close saved game file */
	DOS_Close(File_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_game_state
 * FUNCTION  : Save game state.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 13:41
 * LAST      : 30.06.95 11:55
 * INPUTS    : UNSHORT Number - Number of saved game (1...).
 *             UNCHAR *Name - Saved game name.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_game_state(UNSHORT Number, UNCHAR *Name)
{
	MEM_HANDLE Handle;
	UNLONG Length;
	SISHORT File_handle;
	UNBYTE *Ptr;

	/* Select saved game */
	Select_saved_game(Number);

	/* Open saved game file */
	File_handle = DOS_Open(Save_game_fname, BBDOSFILESTAT_WRITE);

	/* Write saved game name */
	Length = strlen(Name);
	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Name, Length);

	/* Write party data */
	DOS_Write(File_handle, (UNBYTE *) &PARTY_DATA, sizeof(struct Party_data));

	/* Write modification list */
	Handle = Prepare_modifications_for_saving();

	Length = MEM_Get_block_size(Handle);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Ptr, Length);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Write dialogue log */
	Handle = Prepare_dialogue_log_for_saving();

	Length = MEM_Get_block_size(Handle);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Ptr, Length);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Write automapper notes */

	#if FALSE
	/* Write party character data */
	Handle = Load_file(PARTY_CHAR);

	Ptr = MEM_Claim_pointer(Handle);
	Length = MEM_Get_block_size(Handle);

	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Ptr, Length);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Write NPC character data */
	Handle = Load_file(NPC_CHAR);

	Ptr = MEM_Claim_pointer(Handle);
	Length = MEM_Get_block_size(Handle);

	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Ptr, Length);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Write chest data */
	Handle = Load_file(CHEST_DATA);

	Ptr = MEM_Claim_pointer(Handle);
	Length = MEM_Get_block_size(Handle);

	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Ptr, Length);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Write merchant data */
	Handle = Load_file(MERCHANT_DATA);

	Ptr = MEM_Claim_pointer(Handle);
	Length = MEM_Get_block_size(Handle);

	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Ptr, Length);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Write automap data */
	Handle = Load_file(AUTOMAP);

	Ptr = MEM_Claim_pointer(Handle);
	Length = MEM_Get_block_size(Handle);

	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Ptr, Length);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);
	#endif

	/* Close saved game file */
	DOS_Close(File_handle);
}

