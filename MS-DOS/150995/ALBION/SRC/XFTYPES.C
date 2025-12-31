/************
 * NAME     : XFTYPES.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 27-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <stdio.h>
#include <string.h>
#include <direct.h>

#include <BBDEF.H>
#include <BBDOS.H>
#include <BBMEM.H>
#include <BBERROR.H>

#include <XLOAD.H>

#include <XFTYPES.H>
#include <SAVELOAD.H>
#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <MAP.H>
#include <DIALOGUE.H>
#include <3DCOMMON.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <GAMETEXT.H>
#include <AUTOMAP.H>
#include <ICCHANGE.H>
#include <NPCS.H>

/* defines */

#define MAX_DATA_PATHS			(2)
#define MAX_DATA_PATH_LENGTH	(100)

/* prototypes */

/* global variables */

static UNCHAR Data_paths[MAX_DATA_PATHS][MAX_DATA_PATH_LENGTH];
static UNSHORT Nr_data_paths = 0;

static UNCHAR Data_dir[] = "XLDLIBS";

static UNCHAR *Language_dirs[] = {
	"GERMAN",
	"ENGLISH",
	"FRENCH"
};

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
	"SYSTEXTS",
	"EVNTSET0.XLD",
	"EVNTTXT0.XLD",
	"MAPTEXT0.XLD",
	"ITEMLIST.DAT",
	"ITEMNAME.DAT",
	"ITEMGFX",
	"FBODPIX0.XLD",
	"AUTOMAP0.XLD",
	"AUTOGFX0.XLD",
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
	"TACTICO0.XLD",
	"SPELLDAT.DAT",
	"NPCKL0.XLD",
	"FLICS0.XLD",
	"WORDLIS0.XLD",
	"SCRIPT0.XLD",
	"PICTURE0.XLD",
	"TRANSTB0.XLD"
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
	"System texts",
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
	"Tactical icon",
	"Spell data",
	"Small NPC graphics",
	"Flic",
	"Dictionary",
	"Script",
	"Picture",
	"Transparency tables"
};

struct XFile_type Xftypes[MAX_XFTYPES] = {
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

	/* System texts */
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
			0, //MEM_LOCK,
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
			0, //MEM_LOCK,
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
			0, //MEM_LOCK,
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
	},

	/* Scripts */
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

	/* Pictures */
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

	/* Transparency tables */
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
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_data_paths
 * FUNCTION  : Initialize data paths.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 09:50
 * LAST      : 28.07.95 09:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will install the current directory as the
 *              first data path.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_data_paths(void)
{
	char *cwd;

	/* No data paths */
	Nr_data_paths = 0;

	/* Get current working directory */
	cwd = getcwd(&(Data_paths[0][0]), MAX_DATA_PATH_LENGTH);

	/* Success ? */
	if (cwd != NULL)
	{
		/* Yes -> Count up */
		Nr_data_paths++;
	}

	/* Select first data path */
	Select_data_path(0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_data_path
 * FUNCTION  : Add a new data path.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 09:53
 * LAST      : 28.07.95 09:53
 * INPUTS    : UNCHAR *Data_path_ptr - Pointer to data path.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_data_path(UNCHAR *Data_path_ptr)
{
	/* Any data paths free ? */
	if (Nr_data_paths < MAX_DATA_PATHS)
	{
		/* Yes -> Copy data path */
		strcpy(&(Data_paths[Nr_data_paths][0]), Data_path_ptr);

		/* Count up */
		Nr_data_paths++;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_data_path
 * FUNCTION  : Select a data path.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 10:08
 * LAST      : 28.07.95 10:08
 * INPUTS    : UNSHORT Data_path_index - Data path index (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_data_path(UNSHORT Data_path_index)
{
	/* Is this a legal data path index ? */
	if (Data_path_index < Nr_data_paths)
	{
		/* Yes -> Select it */
		Change_directory(&(Data_paths[Data_path_index][0]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_second_data_path
 * FUNCTION  : Look for a second data path.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.08.95 10:45
 * LAST      : 14.08.95 11:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function must be called before any other data paths
 *              are selected.
 *             - This function uses a goto to handle errors.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Search_second_data_path(void)
{
	static UNCHAR Setup_ini_filename[] = "SETUP.INI";
	static UNCHAR CD_ROM_command[] = "CD_ROM_DRIVE=";

	MEM_HANDLE Handle;
	SILONG File_length;
	UNLONG Read;
	UNLONG i;
	SISHORT ID;
	UNBYTE *Pointer;
	UNCHAR *New_data_path = " :\\ALBION";

	/* Does the setup file exist ? */
	if (File_exists(Setup_ini_filename))
	{
		/* Yes -> Get file length */
		File_length = DOS_GetFileLength(Setup_ini_filename);
		if (File_length > 0)
		{
			/* Allocate memory for file */
			Handle = MEM_Do_allocate((UNLONG) File_length, 0L, NULL);
			if (Handle)
			{
				/* Open the file */
			 	ID = DOS_Open(Setup_ini_filename, BBDOSFILESTAT_READ);
				if (ID >= 0)
				{
					/* Read file into memory */
					Pointer = MEM_Claim_pointer(Handle);
					Read = DOS_Read(ID, Pointer, (UNLONG) File_length);

					/* Close the file */
					DOS_Close(ID);

					/* Succesful ? */
					if (Read == File_length)
					{
						/* Yes -> Scan INI file */
						for (i=0;i<File_length - strlen(CD_ROM_command) + 1;i++)
						{
							/* Is this the CD-ROM drive entry ? */
							if (!strnicmp((UNCHAR *) (Pointer + i),
							 CD_ROM_command, strlen(CD_ROM_command)))
							{
								/* Yes -> Extract CD-ROM drive character */
								New_data_path[0] = *(Pointer + i + strlen(CD_ROM_command));

								/* Add data path */
								Add_data_path(New_data_path);

								break;
							}
						}
					}

					/* Free memory */
					MEM_Free_pointer(Handle);
					MEM_Free_memory(Handle);
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_file_type
 * FUNCTION  : Prepare an entry from the eXtended file type array.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.95 12:18
 * LAST      : 28.07.95 10:10
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 * RESULT    : struct XFile_type * - Pointer to eXtended file type.
 * BUGS      : No known.
 * NOTES     : - This function will build a path which may depend on the
 *              current language.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct XFile_type *
Prepare_file_type(UNSHORT File_type_index)
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
		sprintf(Path, "%s\\%s\\%s", Data_dir, Language_dirs[Language],
		 Filenames[File_type_index]);
	}
	else
	{
		/* Is this a save file ? */
		if (XFT->Flags & SAVE_FILE)
		{
			/* Yes -> Build path */
			sprintf(Path, "%s\\%s\\%s", Data_dir, Save_dir_ptr,
			 Filenames[File_type_index]);
		}
		else
		{
			/* No -> Build path */
			sprintf(Path, "%s\\%s", Data_dir, Filenames[File_type_index]);
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
 * LAST      : 10.08.95 17:55
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
	MEM_HANDLE Handle = NULL;
	UNSHORT i;

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Check each data path */
	for (i=0;i<Nr_data_paths;i++)
	{
		/* Select current data path */
		Select_data_path(i);

		/* Try to load the file */
		Handle = XLD_Load_file(XFT);

		/* Success ? */
		if (Handle)
		{
			/* Yes -> Exit */
			break;
		}
		else
		{
			/* No -> File not found ? */
			if (XLD_Last_error == XLD_FILE_NOT_FOUND)
			{
				/* Yes -> Last data path ? */
				if (i == Nr_data_paths - 1)
				{
					/* Yes -> Exit (file not found) */
					break;
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
			else
			{
				/* No -> Exit (real error) */
				break;
			}
		}
	}

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_file
 * FUNCTION  : Save a file.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:52
 * LAST      : 28.07.95 10:14
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

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Select first data path */
	Select_data_path(0);

	/* Save file */
	XLD_Save_file(Handle, XFT);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_subfile_length
 * FUNCTION  : Get the length of a subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.95 15:30
 * LAST      : 15.09.95 14:08
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : UNLONG : File length / 0xFFFFFFFF = error.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_subfile_length(UNSHORT File_type_index, UNSHORT Subfile_nr)
{
	struct XFile_type *XFT;
	UNLONG Subfile_length = 0xFFFFFFFF;
	UNSHORT i;

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Check each data path */
	for (i=0;i<Nr_data_paths;i++)
	{
		/* Select current data path */
		Select_data_path(i);

		/* Try to get the subfile's length */
		Subfile_length = XLD_Get_subfile_length(XFT, Subfile_nr);

		/* Success ? */
		if (Subfile_length != 0xFFFFFFFF)
		{
			/* Yes -> Exit */
			break;
		}
		else
		{
			/* No -> File not found ? */
			if (XLD_Last_error == XLD_FILE_NOT_FOUND)
			{
				/* Yes -> Last data path ? */
				if (i == Nr_data_paths - 1)
				{
					/* Yes -> Exit (file not found) */
					break;
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
			else
			{
				/* No -> Exit (real error) */
				break;
			}
		}
	}

	return Subfile_length;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Open_subfile
 * FUNCTION  : Open a subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.95 11:00
 * LAST      : 12.09.95 11:00
 * INPUTS    : UNSHORT File_type_index - Index of XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 *             SISHORT *File_handle_ptr - Pointer to BBDOS file handle.
 *             UNLONG *File_base_offset_ptr - Pointer to file base offset.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Open_subfile(UNSHORT File_type_index, UNSHORT Subfile_nr,
 SISHORT *File_handle_ptr, UNLONG *File_base_offset_ptr)
{
	struct XFile_type *XFT;
	BOOLEAN Success = FALSE;
	BOOLEAN Result;
	UNSHORT i;

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Check each data path */
	for (i=0;i<Nr_data_paths;i++)
	{
		/* Select current data path */
		Select_data_path(i);

		/* Try to open the subfile */
		Result = XLD_Open_subfile(XFT, Subfile_nr, File_handle_ptr,
		 File_base_offset_ptr);

		/* Success ? */
		if (Result)
		{
			/* Yes -> Exit */
			Success = TRUE;
			break;
		}
		else
		{
			/* No -> File not found ? */
			if (XLD_Last_error == XLD_FILE_NOT_FOUND)
			{
				/* Yes -> Last data path ? */
				if (i == Nr_data_paths - 1)
				{
					/* Yes -> Exit (file not found) */
					break;
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
			else
			{
				/* No -> Exit (real error) */
				break;
			}
		}
	}

	return Success;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Close_subfile
 * FUNCTION  : Close a subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.95 11:04
 * LAST      : 12.09.95 11:04
 * INPUTS    : SISHORT File_handle - BBDOS file handle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XFTYPES.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Close_subfile(SISHORT File_handle)
{
	/* Just do it
	  (this works because all XLD_Close_subfile does is close the file). */
	XLD_Close_subfile(File_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_subfile
 * FUNCTION  : Load a subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:01
 * LAST      : 10.08.95 17:55
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
	MEM_HANDLE Handle = NULL;
	UNSHORT i;

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Check each data path */
	for (i=0;i<Nr_data_paths;i++)
	{
		/* Select current data path */
		Select_data_path(i);

		/* Try to load the subfile */
		Handle = XLD_Load_subfile(XFT, Subfile_nr);

		/* Success ? */
		if (Handle)
		{
			/* Yes -> Exit */
			break;
		}
		else
		{
			/* No -> File not found ? */
			if (XLD_Last_error == XLD_FILE_NOT_FOUND)
			{
				/* Yes -> Last data path ? */
				if (i == Nr_data_paths - 1)
				{
					/* Yes -> Exit (file not found) */
					break;
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
			else
			{
				/* No -> Exit (real error) */
				break;
			}
		}
	}

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_subfile
 * FUNCTION  : Save a subfile into a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:59
 * LAST      : 28.07.95 10:15
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
	BOOLEAN Result;
	UNSHORT i;

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Check each data path */
	for (i=0;i<Nr_data_paths;i++)
	{
		/* Select current data path */
		Select_data_path(i);

		/* Try to save the subfile */
		Result = XLD_Save_subfile(Handle, XFT, Subfile_nr);

		/* Success ? */
		if (Result)
		{
			/* Yes -> Exit */
			break;
		}
		else
		{
			/* No -> File not found ? */
			if (XLD_Last_error == XLD_FILE_NOT_FOUND)
			{
 				/* Yes -> Last data path ? */
				if (i == Nr_data_paths - 1)
				{
					/* Yes -> Exit (file not found) */
					break;
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
			else
			{
				/* No -> Exit (real error) */
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_unique_subfile
 * FUNCTION  : Load a unique subfile from a library.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:03
 * LAST      : 10.08.95 17:56
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
	MEM_HANDLE Handle = NULL;
	UNSHORT i;

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Check each data path */
	for (i=0;i<Nr_data_paths;i++)
	{
		/* Select current data path */
		Select_data_path(i);

		/* Try to load the subfile */
		Handle = XLD_Load_unique_subfile(XFT, Subfile_nr);

		/* Success ? */
		if (Handle)
		{
			/* Yes -> Exit */
			break;
		}
		else
		{
			/* No -> File not found ? */
			if (XLD_Last_error == XLD_FILE_NOT_FOUND)
			{
				/* Yes -> Last data path ? */
				if (i == Nr_data_paths - 1)
				{
					/* Yes -> Exit (file not found) */
					break;
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
			else
			{
				/* No -> Exit (real error) */
				break;
			}
		}
	}

	return Handle;
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
 * LAST      : 10.08.95 17:56
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
	BOOLEAN Result = FALSE;
	UNSHORT i;

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Check each data path */
	for (i=0;i<Nr_data_paths;i++)
	{
		/* Select current data path */
		Select_data_path(i);

		/* Try to load the batch */
		Result = XLD_Load_full_batch(XFT, Number, Subfile_list, Handle_list);

		/* Success ? */
		if (Result)
		{
			/* Yes -> Exit */
			break;
		}
		else
		{
			/* No -> File not found ? */
			if (XLD_Last_error == XLD_FILE_NOT_FOUND)
			{
				/* Yes -> Last data path ? */
				if (i == Nr_data_paths - 1)
				{
					/* Yes -> Exit (file not found) */
					break;
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
			else
			{
				/* No -> Exit (real error) */
				break;
			}
		}
	}

	return Result;
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
 * LAST      : 10.08.95 17:56
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
	BOOLEAN Result = FALSE;
	UNSHORT i;

	/* Prepare file type */
	XFT = Prepare_file_type(File_type_index);

	/* Check each data path */
	for (i=0;i<Nr_data_paths;i++)
	{
		/* Select current data path */
		Select_data_path(i);

		/* Try to load the batch */
		Result = XLD_Load_partial_batch(XFT, Number, Subfile_list, Handle_list);

		/* Success ? */
		if (Result)
		{
			/* Yes -> Exit */
			break;
		}
		else
		{
			/* No -> File not found ? */
			if (XLD_Last_error == XLD_FILE_NOT_FOUND)
			{
 				/* Yes -> Last data path ? */
				if (i == Nr_data_paths - 1)
				{
					/* Yes -> Exit (file not found) */
					break;
				}
				else
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();
				}
			}
			else
			{
				/* No -> Exit (real error) */
				break;
			}
		}
	}

	return Result;
}

