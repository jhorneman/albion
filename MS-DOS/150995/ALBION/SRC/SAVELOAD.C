/************
 * NAME     : SAVELOAD.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-9-1995
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

#include <SAVELOAD.H>
#include <XFTYPES.H>
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

#define SAVE_GAME_MAGIC			(0x25051971)
#define SAVE_GAME_VERSION_NR	(2)

/* prototypes */

void Select_saved_game(UNSHORT Number);

/* global variables */

UNCHAR Save_game_fname[20];

UNCHAR Current_save_dir[] = "CURRENT";
UNCHAR Initial_save_dir[] = "INITIAL";
UNCHAR *Save_dir_ptr = Current_save_dir;

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_saved_game
 * FUNCTION  : Select a saved game.
 * FILE      : SAVELOAD.C
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
	sprintf(Save_game_fname, "SAVES\\SAVE.%03u", Number);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_initial_game_state
 * FUNCTION  : Load initial game state.
 * FILE      : SAVELOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.95 12:04
 * LAST      : 03.09.95 19:19
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will ONLY look in the first data path!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Load_initial_game_state(void)
{
	struct XFile_type *XFT;
	MEM_HANDLE Handle;
	BOOLEAN Failure = FALSE;
	BOOLEAN Result;
	UNSHORT Zero = 0;
	UNSHORT File_type_index;
	UNSHORT Group_nr;

	/* Initialize permanent modifications */
	Nr_permanent_modifications = Init_modifications((UNBYTE *) &Zero,
	 &Permanent_modification_list);

	/* Initialize dialogue log */
	Init_dialogue_log((UNBYTE *) &Zero);

	/* Select first data path */
	Select_data_path(0);

	/* Check all eXtended file types */
	for (File_type_index=0;File_type_index<MAX_XFTYPES;File_type_index++)
	{
		/* Is this a file that should be saved ? */
		if (Xftypes[File_type_index].Flags & SAVE_FILE)
		{
			/* Yes -> Try all subgroups */
			for (Group_nr=0;Group_nr<=9;Group_nr++)
			{
				/* Prepare file type for reading the current file */
				Save_dir_ptr = Initial_save_dir;
				XFT = Prepare_file_type(File_type_index);

				/* Try to load this XLD-library */
				Handle = XLD_Load_XLD_library(XFT, Group_nr);

				/* Success ? */
				if (Handle)
				{
					/* Yes -> Prepare file type for writing the current file */
					Save_dir_ptr = Current_save_dir;
					XFT = Prepare_file_type(File_type_index);

					/* Save this XLD-library */
					Result = XLD_Save_XLD_library(Handle, XFT, Group_nr);

					/* Destroy XLD-library */
					MEM_Kill_memory(Handle);

					/* Exit if an error occurred during saving */
					if (!Result)
					{
						Failure = TRUE;
						break;
					}
				}
				else
				{
					/* No -> File not found ? */
					if (XLD_Last_error == XLD_FILE_NOT_FOUND)
					{
		 				/* Yes -> Clear error stack */
						ERROR_ClearStack();
					}
					else
					{
						/* No -> Exit (real error) */
						Failure = TRUE;
						break;
					}
				}
			}
		}

		/* Exit if an error occurred */
		if (Failure)
			break;
	}

	/* Reset save directory pointer */
	Save_dir_ptr = Current_save_dir;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_game_state
 * FUNCTION  : Load game state.
 * FILE      : SAVELOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 13:41
 * LAST      : 03.09.95 20:52
 * INPUTS    : UNSHORT Number - Number of saved game (1...).
 *             BOOLEAN No_load_flag - Set if file parts should not be changed.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function will ONLY look in the first data path!
 *             - When the No load flag is set, the saved game number should
 *              still have a meaningful value.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_game_state(UNSHORT Number, BOOLEAN No_load_flag)
{
	struct XFile_type *XFT;
	MEM_HANDLE Handle;
	BOOLEAN Success = TRUE;
	BOOLEAN Result;
	UNLONG Magic;
	UNLONG Version_nr;
	UNLONG Length;
	SISHORT File_handle;
	UNSHORT File_type_index;
	UNSHORT Group_nr;
	UNBYTE *Ptr;

	/* Make sure files are saved to current save directory */
	Save_dir_ptr = Current_save_dir;

	/* Select saved game */
	Select_saved_game(Number);

	/* Select first data path */
	Select_data_path(0);

	/* Exit if this saved game file does not exist */
	if (!File_exists(Save_game_fname))
		return FALSE;

	/* Open saved game file */
	File_handle = DOS_Open(Save_game_fname, BBDOSFILESTAT_READ);

	/* Skip saved game name */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Seek(File_handle, BBDOS_SEEK_CURRENTPOS, Length);

	/* Read magic longword and save game version number */
	DOS_Read(File_handle, (UNBYTE *) &Magic, sizeof(UNLONG));
	DOS_Read(File_handle, (UNBYTE *) &Version_nr, sizeof(UNLONG));

	/* Correct ? */
	if (Magic != SAVE_GAME_MAGIC)
	{
		/* No -> Close file */
		DOS_Close(File_handle);

		/* Report error */
		Error(ERROR_ILLEGAL_SAVE_GAME);

		return FALSE;
	}

	/* Version number OK ? */
	if (Version_nr != SAVE_GAME_VERSION_NR)
	{
		/* No -> Close file */
		DOS_Close(File_handle);

		/* Report error */
		Error(ERROR_ILLEGAL_SAVE_GAME_VERSION);

		return FALSE;
	}

	/* Read party data */
	DOS_Read(File_handle, (UNBYTE *) &PARTY_DATA, sizeof(struct Party_data));

	/* Read NPC data */
	DOS_Read(File_handle, (UNBYTE *) &(VNPCs[0]), NPCS_PER_MAP *
	 sizeof(struct VNPC_data));

	/* Read permanent modification list */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Nr_permanent_modifications = Init_modifications(Ptr,
	 &Permanent_modification_list);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Read temporary modification list */
	DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	Handle = MEM_Allocate_memory(Length);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Read(File_handle, Ptr, Length);

	Nr_temporary_modifications = Init_modifications(Ptr,
	 &Temporary_modification_list);

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

	/* Change file parts ? */
	if (!No_load_flag)
	{
		/* Yes -> Extract file parts from saved game */
		for (;;)
		{
			/* Read length of next file part */
			DOS_Read(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

			/* Exit if length is zero */
			if (!Length)
				break;

			/* Read file type index and group number */
			DOS_Read(File_handle, (UNBYTE *) &File_type_index, sizeof(UNSHORT));
			DOS_Read(File_handle, (UNBYTE *) &Group_nr, sizeof(UNSHORT));

			/* Prepare file type */
			XFT = Prepare_file_type(File_type_index);

			/* Allocate memory for file part */
			Handle = MEM_Allocate_memory(Length);

			/* Read file part */
			Ptr = MEM_Claim_pointer(Handle);
			DOS_Read(File_handle, Ptr, Length);
			MEM_Free_pointer(Handle);

			/* Save file part */
			Result = XLD_Save_XLD_library(Handle, XFT, Group_nr);

			/* Free memory */
			MEM_Free_memory(Handle);

			/* Exit if saving failed */
			if (!Result)
			{
				Success = FALSE;
				break;
			}
		}
	}

	/* Close saved game file */
	DOS_Close(File_handle);

	/* Set flag */
	Loading_game = Success;

	return Success;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_game_state
 * FUNCTION  : Save game state.
 * FILE      : SAVELOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 13:41
 * LAST      : 03.09.95 20:52
 * INPUTS    : UNSHORT Number - Number of saved game (1...).
 *             UNCHAR *Name - Saved game name.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will ONLY look in the first data path!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_game_state(UNSHORT Number, UNCHAR *Name)
{
	struct XFile_type *XFT;
	MEM_HANDLE Handle;
	BOOLEAN Failure = FALSE;
	UNLONG Magic = SAVE_GAME_MAGIC;
	UNLONG Version_nr = SAVE_GAME_VERSION_NR;
	UNLONG Length;
	SISHORT File_handle;
	UNSHORT File_type_index;
	UNSHORT Group_nr;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Save character data of all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			Save_subfile(Party_char_handles[i], PARTY_CHAR,
			 PARTY_DATA.Member_nrs[i]);
		}
	}

	/* 3D map ? */
	if (_3D_map)
	{
		/* Yes -> Save the automap */
		Save_automap();

		/* Copy extra information to party data */
		for (i=0;i<3;i++)
		{
			PARTY_DATA.Player_X[i] = I3DM.Player_X[i];
			PARTY_DATA.Player_Z[i] = I3DM.Player_Z[i];
		}

		PARTY_DATA.Camera_angle			= I3DM.Camera_angle;
		PARTY_DATA.Camera_height		= I3DM.Camera_height;
		PARTY_DATA.Horizon_Y				= I3DM.Horizon_Y;
		PARTY_DATA.Window_3D_width		= I3DM.Window_3D_width;
		PARTY_DATA.Window_3D_height	= I3DM.Window_3D_height;
	}

	/* Make sure files are loaded from current save directory */
	Save_dir_ptr = Current_save_dir;

	/* Select saved game */
	Select_saved_game(Number);

	/* Select first data path */
	Select_data_path(0);

	/* Open saved game file */
	File_handle = DOS_Open(Save_game_fname, BBDOSFILESTAT_WRITE);

	/* Write saved game name */
	Length = strlen(Name);
	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Name, Length);

	/* Write magic longword and save game version number */
	DOS_Write(File_handle, (UNBYTE *) &Magic, sizeof(UNLONG));
	DOS_Write(File_handle, (UNBYTE *) &Version_nr, sizeof(UNLONG));

	/* Write party data */
	DOS_Write(File_handle, (UNBYTE *) &PARTY_DATA, sizeof(struct Party_data));

	/* Write NPC data */
	DOS_Write(File_handle, (UNBYTE *) &(VNPCs[0]), NPCS_PER_MAP *
	 sizeof(struct VNPC_data));

	/* Write permanent modification list */
	Handle = Prepare_modifications_for_saving(Permanent_modification_list,
	 Nr_permanent_modifications);

	Length = MEM_Get_block_size(Handle);
	Ptr = MEM_Claim_pointer(Handle);

	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));
	DOS_Write(File_handle, Ptr, Length);

	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Write temporary modification list */
	Handle = Prepare_modifications_for_saving(Temporary_modification_list,
	 Nr_temporary_modifications);

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

	/* Check all eXtended file types */
	for (File_type_index=0;File_type_index<MAX_XFTYPES;File_type_index++)
	{
		/* Is this a file that should be saved ? */
		if (Xftypes[File_type_index].Flags & SAVE_FILE)
		{
			/* Yes -> Try all subgroups */
			for (Group_nr=0;Group_nr<=9;Group_nr++)
			{
				/* Prepare file type */
				XFT = Prepare_file_type(File_type_index);

				/* Try to load this XLD-library */
				Handle = XLD_Load_XLD_library(XFT, Group_nr);

				/* Success ? */
				if (Handle)
				{
					/* Yes -> Get length of library */
					Length = MEM_Get_block_size(Handle);

					/* Write length */
					DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

					/* Write file type index and group number */
					DOS_Write(File_handle, (UNBYTE *) &File_type_index, sizeof(UNSHORT));
					DOS_Write(File_handle, (UNBYTE *) &Group_nr, sizeof(UNSHORT));

					/* Write library */
					Ptr = MEM_Claim_pointer(Handle);
					DOS_Write(File_handle, Ptr, Length);
					MEM_Free_pointer(Handle);

					/* Free memory */
					MEM_Kill_memory(Handle);
				}
				else
				{
					/* No -> File not found ? */
					if (XLD_Last_error == XLD_FILE_NOT_FOUND)
					{
		 				/* Yes -> Clear error stack */
						ERROR_ClearStack();
					}
					else
					{
						/* No -> Exit (real error) */
						Failure = TRUE;
						break;
					}
				}
			}
		}

		/* Exit if an error occurred */
		if (Failure)
			break;
	}

	/* Write 0 to indicate end of saved game file */
	Length = 0;
	DOS_Write(File_handle, (UNBYTE *) &Length, sizeof(UNLONG));

	/* Close saved game file */
	DOS_Close(File_handle);
}

