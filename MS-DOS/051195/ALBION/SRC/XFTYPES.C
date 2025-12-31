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
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <dos.h>
#include <sys\types.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>

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

static UNCHAR INI_filename[] = "SETUP.INI";

static UNCHAR Evil_characters[] = " \r\n\t";

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
	"",
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
		NO_DATA_PATH_0
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
		NO_DATA_PATH_0
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
			NULL,
			0xFF,
			128,
			MEM_KILL_ALWAYS,
			NULL
		},
		NULL,
		NULL,
		LANGUAGE_DEPENDENT | NO_DATA_PATH_0
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
		NO_DATA_PATH_0
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

	/* UNUSED */
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
		NO_DATA_PATH_0
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
		NO_DATA_PATH_0
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
		NO_DATA_PATH_0
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
		NO_DATA_PATH_0
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
 * LAST      : 25.10.95 22:53
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function will install the current directory as the
 *              first data path.
 *             - This function will be called before PCLIB32 is initialised!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_data_paths(void)
{
	static UNCHAR Program_name[] = "ALBION.EXE";
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

	/* Can the program be found here ? */
	if (File_exists(Program_name))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
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
 * NOTES     : - This function will be called before PCLIB32 is initialised!
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
 * NOTES     : - This function will be called before PCLIB32 is initialised!
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
 * LAST      : 30.10.95 02:39
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Second data path was found.
 * BUGS      : No known.
 * NOTES     : - This function will be called before PCLIB32 is initialised!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Search_second_data_path(void)
{
	BOOLEAN Data_path_was_found = FALSE;
	BOOLEAN Result;
	int File_handle;
	UNSHORT Length;
	UNCHAR New_data_path[MAX_DATA_PATH_LENGTH];

	/* Read source path variable */
	Result = Read_INI_variable
	(
		"SYSTEM",
		"SOURCE_PATH",
		New_data_path,
		MAX_DATA_PATH_LENGTH
	);

	/* Found ? */
	if (Result)
	{
		/* Yes -> Remove backslash at the end (if any) */
		Length = strlen(New_data_path);

		if (New_data_path[Length - 1] == '\\')
		{
			New_data_path[Length - 1] = '\0';
		}

		/* Select this data path */
		Change_directory(New_data_path);

		/* Does a data directory exist ? */
		if (Directory_exists(Data_dir))
		{
			/* Yes -> Can an arbitrary file be opened for writing ? */
			File_handle = open
			(
				"ALBION.EX_",
				O_WRONLY | O_BINARY
			);
			if (File_handle != -1)
			{
				/* Yes -> Close the file */
				close(File_handle);
			}
			else
			{
				/* No -> Access denied ? */
				if (errno == EACCES)
				{
					/* Yes -> Add as second data path */
					Add_data_path(New_data_path);

					/* Data path was found */
					Data_path_was_found = TRUE;
				}
			}
		}
	}

	return Data_path_was_found;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_CD
 * FUNCTION  : Check if the second data path is the Albion CD.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.95 15:48
 * LAST      : 28.10.95 15:48
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Second data path is the Albion CD.
 * BUGS      : No known.
 * NOTES     : - This function will be called before PCLIB32 is initialised!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_CD(void)
{
	static const UNCHAR Volume_name[] = "ALBION";

	struct find_t File_info;
	unsigned Return_code;

	/* Select second data path */
	Select_data_path(1);

	/* Find volume */
	Return_code = _dos_findfirst
	(
		Volume_name,
		_A_VOLID,
		&File_info
	);

	/* Found it ? */
	if (Return_code)
		return FALSE;
	else
		return TRUE;
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
		_bprintf
		(
			Path,
			100,
			"%s\\%s\\%s",
			Data_dir,
			Language_dirs[Language],
			Filenames[File_type_index]
		);
	}
	else
	{
		/* Is this a save file ? */
		if (XFT->Flags & SAVE_FILE)
		{
			/* Yes -> Build path */
			_bprintf
			(
				Path,
				100,
				"%s\\%s\\%s",
				Data_dir,
				Save_dir_ptr,
				Filenames[File_type_index]
			);
		}
		else
		{
			/* No -> Build path */
			_bprintf
			(
				Path,
				100,
				"%s\\%s",
				Data_dir,
				Filenames[File_type_index]
			);
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
		/* Can this file be loaded from data path 0 ? */
		if ((XFT->Flags & NO_DATA_PATH_0) && (i == 0))
		{
			/* No -> Skip */
			continue;
		}

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
		/* Can this file be loaded from data path 0 ? */
		if ((XFT->Flags & NO_DATA_PATH_0) && (i == 0))
		{
			/* No -> Skip */
			continue;
		}

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
		/* Can this file be loaded from data path 0 ? */
		if ((XFT->Flags & NO_DATA_PATH_0) && (i == 0))
		{
			/* No -> Skip */
			continue;
		}

		/* Select current data path */
		Select_data_path(i);

		/* Try to open the subfile */
		Result = XLD_Open_subfile
		(
			XFT,
			Subfile_nr,
			File_handle_ptr,
			File_base_offset_ptr
		);

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
		/* Can this file be loaded from data path 0 ? */
		if ((XFT->Flags & NO_DATA_PATH_0) && (i == 0))
		{
			/* No -> Skip */
			continue;
		}

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
		/* Can this file be loaded from data path 0 ? */
		if ((XFT->Flags & NO_DATA_PATH_0) && (i == 0))
		{
			/* No -> Skip */
			continue;
		}

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
		/* Can this file be loaded from data path 0 ? */
		if ((XFT->Flags & NO_DATA_PATH_0) && (i == 0))
		{
			/* No -> Skip */
			continue;
		}

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
		/* Can this file be loaded from data path 0 ? */
		if ((XFT->Flags & NO_DATA_PATH_0) && (i == 0))
		{
			/* No -> Skip */
			continue;
		}

		/* Select current data path */
		Select_data_path(i);

		/* Try to load the batch */
		Result = XLD_Load_full_batch
		(
			XFT,
			Number,
			Subfile_list,
			Handle_list
		);

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
		/* Can this file be loaded from data path 0 ? */
		if ((XFT->Flags & NO_DATA_PATH_0) && (i == 0))
		{
			/* No -> Skip */
			continue;
		}

		/* Select current data path */
		Select_data_path(i);

		/* Try to load the batch */
		Result = XLD_Load_partial_batch
		(
			XFT,
			Number,
			Subfile_list,
			Handle_list
		);

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
 * NAME      : Read_INI_variable
 * FUNCTION  : Read a variable from the INI file.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 20:23
 * LAST      : 27.09.95 21:37
 * INPUTS    : UNCHAR *Section_name - Pointer to section name.
 *             UNCHAR *Variable_name - Pointer to variable name.
 *             UNCHAR *Value_buffer - Pointer to output buffer of variable.
 *             UNSHORT Max_variable_length - Length of output buffer.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function can be called before PCLIB32 is initialised!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Read_INI_variable(UNCHAR *Section_name, UNCHAR *Variable_name,
 UNCHAR *Value_buffer, UNSHORT Max_variable_length)
{
	FILE *INI_file_handle;

	BOOLEAN Section_found;
	BOOLEAN Variable_found;
	UNSHORT Length;
	UNCHAR Line_buffer[200];
	UNCHAR *Ptr;

	/* Clear value buffer */
	memset(Value_buffer, '\0', Max_variable_length);

	/* Look in first data path */
	Select_data_path(0);

	/* Try to open INI file */
	INI_file_handle = fopen
	(
		INI_filename,
		"rt"
	);

	/* Success ? */
	if (!INI_file_handle)
	{
		/* No -> Error */
		Error(ERROR_SETUP_INI_ACCESS_ERROR);
		return FALSE;
	}

	/* Scan INI file */
	Section_found = FALSE;
	Variable_found = FALSE;
	for (;;)
	{
		/* Read next line */
		Ptr = fgets
		(
			Line_buffer,
			199,
			INI_file_handle
		);

		/* Exit if no line could be read */
		if (!Ptr)
			break;

		/* Strip offending characters from the start of the string */
		Length = strspn(Line_buffer, Evil_characters);
		if (Length)
		{
			memmove
			(
				Line_buffer,
				Line_buffer + Length,
				strlen(Line_buffer - Length + 1)
			);
		}

		/* Strip offending characters from the end of the string */
		Length = strcspn(Line_buffer,	Evil_characters);
		Line_buffer[Length] = '\0';

		/* Continue if the string is empty */
		if (!strlen(Line_buffer))
			continue;

		/* Is this a section header ? */
		if ((Line_buffer[0] == '[') &&
		 (Line_buffer[strlen(Line_buffer) - 1] == ']'))
		{
			/* Yes -> Was the section found already ? */
			if (Section_found)
			{
				/* Yes -> The variable isn't in this section */
				break;
			}
			else
			{
				/* No -> Is it the section we're looking for ? */
				if (!strnicmp
				(
					Line_buffer + 1,
					Section_name,
					strlen(Line_buffer) - 2
				))
				{
					/* Yes -> Yay! */
					Section_found = TRUE;
				}
			}
		}
		else
		{
			/* No -> Is this variable we're looking for ? */
			if (!strnicmp
			(
				Line_buffer,
				Variable_name,
				min(strlen(Line_buffer), strlen(Variable_name))
			))
			{
				/* Yes -> Look for = */
				Ptr = strchr(Line_buffer,(int) '=');

				/* Found ? */
				if (Ptr)
				{
					/* Yes -> Copy variable to output buffer */
					strncpy
					(
						Value_buffer,
						Ptr + 1,
						Max_variable_length - 1
					);

					/* Insert EOL */
					Value_buffer[Max_variable_length - 1] = '\0';
				}
				else
				{
					/* No -> Clear output buffer */
					Value_buffer[0] = '\0';
				}

				/* Yay! */
				Variable_found = TRUE;
				break;
			}
		}
	}

	/* Close file */
	fclose(INI_file_handle);

	return Variable_found;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_INI_variable
 * FUNCTION  : Write a variable in the INI file.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 20:24
 * LAST      : 10.10.95 19:09
 * INPUTS    : UNCHAR *Section_name - Pointer to section name.
 *             UNCHAR *Variable_name - Pointer to variable name.
 *             UNCHAR *Value - Pointer to new variable value.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function can be called before PCLIB32 is initialised!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Write_INI_variable(UNCHAR *Section_name, UNCHAR *Variable_name,
 UNCHAR *Value_buffer)
{
	FILE *INI_file_handle;
	FILE *Temp_file_handle;
	int Return;

	BOOLEAN Section_found;
	BOOLEAN Variable_found;
	UNSHORT Length;
	UNCHAR Line_buffer[200];
	UNCHAR Line_buffer2[200];
	UNCHAR *Ptr;

	UNCHAR Temp_filename[_MAX_PATH];
	UNCHAR Drive[_MAX_DRIVE];
	UNCHAR Dir[_MAX_DIR];
	UNCHAR Fname[_MAX_FNAME];
	UNCHAR Ext[_MAX_EXT];

	/* Look in first data path */
	Select_data_path(0);

	/* Deconstruct INI filename */
	_splitpath
	(
		INI_filename,
		Drive,
		Dir,
		Fname,
		Ext
	);

	/* Build temporary filename */
	_makepath
	(
		Temp_filename,
		Drive,
		Dir,
		Fname,
		"TMP"
	);

	/* Try to open INI file */
	INI_file_handle = fopen
	(
		INI_filename,
		"rt"
	);

	/* Success ? */
	if (!INI_file_handle)
	{
		/* No -> Error */
		Error(ERROR_SETUP_INI_ACCESS_ERROR);
		return FALSE;
	}

	/* Try to open temporary file */
	Temp_file_handle = fopen
	(
		Temp_filename,
		"wt"
	);

	/* Success ? */
	if (!Temp_file_handle)
	{
		/* No -> Close INI file */
		fclose(INI_file_handle);

		/* Error */
		Error(ERROR_SETUP_INI_ACCESS_ERROR);
		return FALSE;
	}

	/* Scan INI file */
	Section_found = FALSE;
	Variable_found = FALSE;
	for (;;)
	{
		/* Read next line */
		Ptr = fgets
		(
			Line_buffer,
			199,
			INI_file_handle
		);

		/* Exit if no line could be read */
		if (!Ptr)
			break;

		/* Strip offending characters from the start of the string */
		Length = strspn(Line_buffer, Evil_characters);
		if (Length)
		{
			memmove
			(
				Line_buffer,
				Line_buffer + Length,
				strlen(Line_buffer - Length + 1)
			);
		}

		/* Strip offending characters from the end of the string */
		Length = strcspn(Line_buffer,	Evil_characters);
		Line_buffer[Length] = '\0';

		/* Is the string empty ? */
		if (!strlen(Line_buffer))
		{
			/* Yes -> Write empty string to output file */
			Return = fputs
			(
				"\n",
				Temp_file_handle
			);

			/* Success ? */
			if (Return <= 0)
			{
				/* No -> Error */
				Error(ERROR_SETUP_INI_ACCESS_ERROR);
				break;
			}

			/* Continue with next line */
			continue;
		}

		/* Already found the variable ? */
		if (!Variable_found)
		{
			/* No -> Is this a section header ? */
			if ((Line_buffer[0] == '[') &&
			 (Line_buffer[strlen(Line_buffer) - 1] == ']'))
			{
				/* Yes -> Was the section found already ? */
				if (Section_found)
				{
					/* Yes -> The variable isn't in this section, so we'll just
					  have to insert it here */
					_bprintf
					(
						Line_buffer2,
						200,
						"%s=%s\n",
						Variable_name,
						Value_buffer
					);

					/* Write line to output file */
					Return = fputs
					(
						Line_buffer2,
						Temp_file_handle
					);

					/* Success ? */
					if (Return <= 0)
					{
						/* No -> Error */
						Error(ERROR_SETUP_INI_ACCESS_ERROR);
						break;
					}

					/* Yay! */
					Variable_found = TRUE;
				}
				else
				{
					/* No -> Is it the section we're looking for ? */
					if (!strnicmp
					(
						Line_buffer + 1,
						Section_name,
						strlen(Line_buffer) - 2
					))
					{
						/* Yes -> Yay! */
						Section_found = TRUE;
					}
				}
			}
			else
			{
				/* No -> Is this variable we're looking for ? */
				if (!strnicmp
				(
					Line_buffer,
					Variable_name,
					min(strlen(Line_buffer), strlen(Variable_name))
				))
				{
					/* Yes -> Re-write it
					 (this line will be written to the output file later on) */
					_bprintf
					(
						Line_buffer,
						200,
						"%s=%s",
						Variable_name,
						Value_buffer
					);

					/* Yay! */
					Variable_found = TRUE;
				}
			}
		}

		/* Write line to output file */
		Return = fputs
		(
			Line_buffer,
			Temp_file_handle
		);

		/* Success ? */
		if (Return <= 0)
		{
			/* No -> Error */
			Error(ERROR_SETUP_INI_ACCESS_ERROR);
			break;
		}

		/* Write end-of-line to output file */
		Return = fputs
		(
			"\n",
			Temp_file_handle
		);

		/* Success ? */
		if (Return <= 0)
		{
			/* No -> Error */
			Error(ERROR_SETUP_INI_ACCESS_ERROR);
			break;
		}
	}

	/* Found the section ? */
	if (!Section_found)
	{
		/* No -> Then we must create it */
		_bprintf
		(
			Line_buffer,
			200,
			"[%s]\n",
			Section_name
		);

		/* Write line to output file */
		Return = fputs
		(
			Line_buffer,
			Temp_file_handle
		);

		/* Success ? */
		if (Return <= 0)
		{
			/* No -> Error */
			Error(ERROR_SETUP_INI_ACCESS_ERROR);
		}
		else
		{
			/* Yes -> Then we must create the variable */
			_bprintf
			(
				Line_buffer,
				200,
				"%s=%s\n",
				Variable_name,
				Value_buffer
			);

			/* Write line to output file */
			Return = fputs
			(
				Line_buffer,
				Temp_file_handle
			);

			/* Success ? */
			if (Return <= 0)
			{
				/* No -> Error */
				Error(ERROR_SETUP_INI_ACCESS_ERROR);
			}
			else
			{
				/* Yes -> Yay! */
				Variable_found = TRUE;
			}
		}
	}
	else
	{
		/* Yes -> Found the variable ? */
		if (!Variable_found)
		{
			/* Yes -> We'll just have to insert it here */
			_bprintf
			(
				Line_buffer,
				200,
				"%s=%s\n",
				Variable_name,
				Value_buffer
			);

			/* Write line to output file */
			Return = fputs
			(
				Line_buffer,
				Temp_file_handle
			);

			/* Success ? */
			if (Return <= 0)
			{
				/* No -> Error */
				Error(ERROR_SETUP_INI_ACCESS_ERROR);
			}
			else
			{
				/* Yes -> Yay! */
				Variable_found = TRUE;
			}
		}
	}

	/* Close files */
	fclose(INI_file_handle);
	fclose(Temp_file_handle);

	/* Were we successful ? */
	if (Variable_found)
	{
		/* Yes -> Delete the original INI file */
		remove(INI_filename);

		/* Rename the temporary file, making it the INI file */
		rename(Temp_filename, INI_filename);
	}
	else
	{
		/* No -> Delete the temporary file */
		remove(Temp_filename);
	}

	return Variable_found;
}

