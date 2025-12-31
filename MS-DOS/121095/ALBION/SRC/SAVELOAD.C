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
#include <MUSIC.H>
#include <VERSION.H>
#include <GAMETIME.H>

/* defines */

#define SAVE_GAME_MAGIC						(0x25051971)
#define MIN_SAVE_GAME_VERSION_NR			(75)

/* prototypes */

/* global variables */

UNSHORT Current_save_game_version_nr;

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
 * LAST      : 04.10.95 13:08
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
	Nr_permanent_modifications = Init_modifications
	(
		(UNBYTE *) &Zero,
		&Permanent_modification_list
	);

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
		 				/* Yes -> Pop error */
						ERROR_PopError();
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

	/* Set save game version number */
	Current_save_game_version_nr = Get_version_number();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_game_state
 * FUNCTION  : Load game state.
 * FILE      : SAVELOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 13:41
 * LAST      : 04.10.95 13:09
 * INPUTS    : UNSHORT Saved_game_nr - Saved_game_nr of saved game (1...).
 *             BOOLEAN Dont_load - Set if file parts should not be changed.
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
Load_game_state(UNSHORT Saved_game_nr, BOOLEAN Dont_load)
{
	struct XFile_type *XFT;
	MEM_HANDLE Handle;
	BOOLEAN Success = FALSE;
	BOOLEAN Result;
	UNLONG Magic;
	UNLONG Version_nr;
	UNLONG Length;
	UNLONG Read_bytes;
	SISHORT File_handle = -1;
	UNSHORT File_type_index;
	UNSHORT Group_nr;
	UNCHAR Number[] = "000";
	UNBYTE *Ptr;

	/* Make sure files are saved to current save directory */
	Save_dir_ptr = Current_save_dir;

	/* Select saved game */
	Select_saved_game(Saved_game_nr);

	/* Select first data path */
	Select_data_path(0);

	/* Does this saved game file exist ? */
	if (!File_exists(Save_game_fname))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Open saved game file */
	File_handle = DOS_Open
	(
		Save_game_fname,
		BBDOSFILESTAT_READ
	);

	/* Success ? */
	if (File_handle == -1)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Read length of saved game name */
	Read_bytes = DOS_Read
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Read everything ? */
	if (Read_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Skip saved game name */
	Result = DOS_Seek
	(
		File_handle,
		BBDOS_SEEK_CURRENTPOS,
		Length
	);

	/* Success ? */
	if (!Result)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Read magic longword */
	Read_bytes = DOS_Read
	(
		File_handle,
		(UNBYTE *) &Magic,
		sizeof(UNLONG)
	);

	/* Read everything ? */
	if (Read_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Correct ? */
	if (Magic != SAVE_GAME_MAGIC)
	{
		/* No -> Close file */
		DOS_Close(File_handle);

		/* Report error */
		Error(ERROR_ILLEGAL_SAVE_GAME);

		return FALSE;
	}

	/* Read save game version number */
	Read_bytes = DOS_Read
	(
		File_handle,
		(UNBYTE *) &Version_nr,
		sizeof(UNLONG)
	);

	/* Read everything ? */
	if (Read_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Version number OK ? */
	if ((Version_nr < MIN_SAVE_GAME_VERSION_NR) ||
	 (Version_nr > Get_version_number()))
	{
		/* No -> Error */
		Error(ERROR_ILLEGAL_SAVE_GAME_VERSION);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Store version number */
	Current_save_game_version_nr = Version_nr;

/**** Read party data ****/

	/* Read party data */
	Read_bytes = DOS_Read
	(
		File_handle,
		(UNBYTE *) &PARTY_DATA,
		sizeof(struct Party_data)
	);

	/* Read everything ? */
	if (Read_bytes != sizeof(struct Party_data))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

/**** Read NPC data ****/

	/* Read NPC data */
	Read_bytes = DOS_Read
	(
		File_handle,
		(UNBYTE *) &(VNPCs[0]),
		NPCS_PER_MAP * sizeof(struct VNPC_data)
	);

	/* Read everything ? */
	if (Read_bytes != NPCS_PER_MAP * sizeof(struct VNPC_data))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

/**** Read located sound effect table ****/

	/* Get located sound effect table length */
	Length = Get_located_effect_table_length();

	/* Allocate memory for located sound effect table */
	Handle = MEM_Allocate_memory(Length);

	/* Successful ? */
	if (!Handle)
	{
		/* No -> Error */
		Error(ERROR_OUT_OF_MEMORY);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Read located sound effect table */
	Ptr = MEM_Claim_pointer(Handle);
	Read_bytes = DOS_Read
	(
		File_handle,
		Ptr,
		Length
	);

	/* Read everything ? */
	if (Read_bytes != Length)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Initialise located sound effect table */
	Load_located_effects(Ptr);

	/* Free memory */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

/**** Read permanent modification list ****/

	/* Read length of permanent modification list */
	Read_bytes = DOS_Read
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Read everything ? */
	if (Read_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Length is zero ? */
	if (!Length)
	{
		/* Yes -> Error */
		Error(ERROR_ILLEGAL_SAVE_GAME);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Allocate memory for permanent modification list */
	Handle = MEM_Allocate_memory(Length);

	/* Successful ? */
	if (!Handle)
	{
		/* No -> Error */
		Error(ERROR_OUT_OF_MEMORY);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Read permanent modification list */
	Ptr = MEM_Claim_pointer(Handle);
	Read_bytes = DOS_Read
	(
		File_handle,
		Ptr,
		Length
	);

	/* Read everything ? */
	if (Read_bytes != Length)
	{
		/* No -> Free memory */
		MEM_Free_pointer(Handle);
		MEM_Free_memory(Handle);

		/* Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Initialise permanent modifications */
	Nr_permanent_modifications = Init_modifications
	(
		Ptr,
		&Permanent_modification_list
	);

	/* Free memory */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

/**** Read temporary modification list ****/

	/* Read length of temporary modification list */
	Read_bytes = DOS_Read
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Read everything ? */
	if (Read_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Length is zero ? */
	if (!Length)
	{
		/* Yes -> Error */
		Error(ERROR_ILLEGAL_SAVE_GAME);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Allocate memory for temporary modification list */
	Handle = MEM_Allocate_memory(Length);

	/* Successful ? */
	if (!Handle)
	{
		/* No -> Error */
		Error(ERROR_OUT_OF_MEMORY);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Read temporary modification list */
	Ptr = MEM_Claim_pointer(Handle);
	Read_bytes = DOS_Read
	(
		File_handle,
		Ptr,
		Length
	);

	/* Read everything ? */
	if (Read_bytes != Length)
	{
		/* No -> Free memory */
		MEM_Free_pointer(Handle);
		MEM_Free_memory(Handle);

		/* Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Initialise temporary modifications */
	Nr_temporary_modifications = Init_modifications
	(
		Ptr,
		&Temporary_modification_list
	);

	/* Free memory */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

/**** Read dialogue log ****/

	/* Read length of dialogue log */
	Read_bytes = DOS_Read
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Read everything ? */
	if (Read_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Length is zero ? */
	if (!Length)
	{
		/* Yes -> Error */
		Error(ERROR_ILLEGAL_SAVE_GAME);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Allocate memory for dialogue log */
	Handle = MEM_Allocate_memory(Length);

	/* Successful ? */
	if (!Handle)
	{
		/* No -> Error */
		Error(ERROR_OUT_OF_MEMORY);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Read dialogue log */
	Ptr = MEM_Claim_pointer(Handle);
	Read_bytes = DOS_Read
	(
		File_handle,
		Ptr,
		Length
	);

	/* Read everything ? */
	if (Read_bytes != Length)
	{
		/* No -> Free memory */
		MEM_Free_pointer(Handle);
		MEM_Free_memory(Handle);

		/* Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_LOAD_GAME_STATE;
	}

	/* Initialise dialogue log */
	Init_dialogue_log(Ptr);

	/* Free memory */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

/**** Read file parts ****/

	/* Change file parts ? */
	if (!Dont_load)
	{
		/* Yes -> Extract file parts from saved game */
		for (;;)
		{
			/* Read length of next file part */
			Read_bytes = DOS_Read
			(
				File_handle,
				(UNBYTE *) &Length,
				sizeof(UNLONG)
			);

			/* Read everything ? */
			if (Read_bytes != sizeof(UNLONG))
			{
				/* No -> Error */
				Error(ERROR_SAVED_GAME_ACCESS_ERROR);
				break;
			}

			/* Length is zero ? */
			if (!Length)
			{
				/* Yes -> End of saved game */
				Success = TRUE;
				break;
			}

			/* Allocate memory for file part */
			Handle = MEM_Allocate_memory(Length);

			/* Successful ? */
			if (!Handle)
			{
				/* No -> Error */
				Error(ERROR_OUT_OF_MEMORY);
				break;
			}

			/* Read file type index */
			Read_bytes = DOS_Read
			(
				File_handle,
				(UNBYTE *) &File_type_index,
				sizeof(UNSHORT)
			);

			/* Read everything ? */
			if (Read_bytes != sizeof(UNSHORT))
			{
				/* No -> Error */
				Error(ERROR_SAVED_GAME_ACCESS_ERROR);
				break;
			}

			/* Read group number */
			Read_bytes = DOS_Read
			(
				File_handle,
				(UNBYTE *) &Group_nr,
				sizeof(UNSHORT)
			);

			/* Read everything ? */
			if (Read_bytes != sizeof(UNSHORT))
			{
				/* No -> Error */
				Error(ERROR_SAVED_GAME_ACCESS_ERROR);
				break;
			}

			/* Read file part */
			Ptr = MEM_Claim_pointer(Handle);
			Read_bytes = DOS_Read
			(
				File_handle,
				Ptr,
				Length
			);
			MEM_Free_pointer(Handle);

			/* Read everything ? */
			if (Read_bytes != Length)
			{
				/* No -> Free memory */
				MEM_Free_memory(Handle);

				/* Error */
				Error(ERROR_SAVED_GAME_ACCESS_ERROR);
				break;
			}

			/* Prepare file type */
			XFT = Prepare_file_type(File_type_index);

			/* Save file part */
			Result = XLD_Save_XLD_library
			(
				Handle,
				XFT,
				Group_nr
			);

			/* Free memory */
			MEM_Free_memory(Handle);

			/* Saving failed ? */
			if (!Result)
			{
				/* Yes -> Error */
				Error(ERROR_SAVED_GAME_ACCESS_ERROR);
				break;
			}
		}
	}
	else
	{
		/* No -> Success */
		Success = TRUE;
	}

EXIT_LOAD_GAME_STATE:

	/* Has the saved game file been opened ? */
	if (File_handle != -1)
	{
		/* Yes -> Close saved game file */
		DOS_Close(File_handle);

		/* Has an error occurred ? */
		if (Success)
		{
			/* No -> Store saved game number in INI file */
			sprintf(Number, "%u", Saved_game_nr);
			Write_INI_variable
			(
				"ALBION",
				"SAVED_GAME_NR",
				Number
			);
		}
	}

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
 * LAST      : 04.10.95 13:06
 * INPUTS    : UNSHORT Saved_game_nr - Saved_game_nr of saved game (1...).
 *             UNCHAR *Saved_game_name - Saved game name.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function will ONLY look in the first data path!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Save_game_state(UNSHORT Saved_game_nr, UNCHAR *Saved_game_name)
{
	struct XFile_type *XFT;
	MEM_HANDLE Handle;
	BOOLEAN Failure;
	BOOLEAN Success = FALSE;
	UNLONG Magic = SAVE_GAME_MAGIC;
	UNLONG Version_nr;
	UNLONG Length;
	UNLONG Written_bytes;
	SISHORT File_handle;
	UNSHORT File_type_index;
	UNSHORT Group_nr;
	UNSHORT i;
	UNCHAR Number[] = "000";
	UNBYTE *Ptr;

	/* Get program version number */
	Version_nr = Get_version_number();

	/* Save character data of all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			Save_subfile
			(
				Party_char_handles[i],
				PARTY_CHAR,
				PARTY_DATA.Member_nrs[i]
			);
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
	}

	/* Save current light state */
	PARTY_DATA.Light_state = Current_map_light_state;

	/* Save current map music number */
	PARTY_DATA.Map_music_nr = Current_map_music_nr;

	/* Save current time data */
	PARTY_DATA.Current_step		= Current_step;
	PARTY_DATA.Current_substep	= Current_substep;

	/* Make sure files are loaded from current save directory */
	Save_dir_ptr = Current_save_dir;

	/* Select saved game */
	Select_saved_game(Saved_game_nr);

	/* Select first data path */
	Select_data_path(0);

	/* Open saved game file */
	File_handle = DOS_Open
	(
		Save_game_fname,
		BBDOSFILESTAT_WRITE
	);

	/* Success ? */
	if (File_handle == -1)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write length of saved game name */
	Length = strlen(Saved_game_name);
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Wrote everything ? */
	if (Written_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write saved game name */
	Written_bytes = DOS_Write
	(
		File_handle,
		Saved_game_name,
		Length
	);

	/* Wrote everything ? */
	if (Written_bytes != Length)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write magic longword */
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &Magic,
		sizeof(UNLONG)
	);

	/* Wrote everything ? */
	if (Written_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write save game version number */
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &Version_nr,
		sizeof(UNLONG)
	);

	/* Wrote everything ? */
	if (Written_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

/**** Write party data ****/

	/* Write party data */
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &PARTY_DATA,
		sizeof(struct Party_data)
	);

	/* Wrote everything ? */
	if (Written_bytes != sizeof(struct Party_data))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

/**** Write NPC data ****/

	/* Write NPC data */
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &(VNPCs[0]),
		NPCS_PER_MAP * sizeof(struct VNPC_data)
	);

	/* Wrote everything ? */
	if (Written_bytes != NPCS_PER_MAP * sizeof(struct VNPC_data))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

/**** Write located sound effect table ****/

	/* Get located sound effect table length */
	Length = Get_located_effect_table_length();

	/* Get address of located sound effect table */
	Ptr = Get_located_effect_table_address();

	/* Write located sound effect table */
	Written_bytes = DOS_Write
	(
		File_handle,
		Ptr,
		Length
	);

	/* Wrote everything ? */
	if (Written_bytes != Length)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

/**** Write permanent modification list ****/

	/* Prepare permanent modification list for saving */
	Handle = Prepare_modifications_for_saving
	(
		Permanent_modification_list,
		Nr_permanent_modifications
	);

	/* Success ? */
	if (!Handle)
	{
		/* No -> Error */
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write length of permanent modification list */
	Length = MEM_Get_block_size(Handle);
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Wrote everything ? */
	if (Written_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write permanent modification list */
	Ptr = MEM_Claim_pointer(Handle);
	Written_bytes = DOS_Write
	(
		File_handle,
		Ptr,
		Length
	);
	MEM_Free_pointer(Handle);

	/* Free memory */
	MEM_Free_memory(Handle);

	/* Wrote everything ? */
	if (Written_bytes != Length)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

/**** Write temporary modification list ****/

	/* Prepare temporary modification list for saving */
	Handle = Prepare_modifications_for_saving
	(
		Temporary_modification_list,
		Nr_temporary_modifications
	);

	/* Success ? */
	if (!Handle)
	{
		/* No -> Error */
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write length of temporary modification list */
	Length = MEM_Get_block_size(Handle);
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Wrote everything ? */
	if (Written_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write temporary modification list */
	Ptr = MEM_Claim_pointer(Handle);
	Written_bytes = DOS_Write
	(
		File_handle,
		Ptr,
		Length
	);
	MEM_Free_pointer(Handle);

	/* Free memory */
	MEM_Free_memory(Handle);

	/* Wrote everything ? */
	if (Written_bytes != Length)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

/**** Write dialogue log ****/

	/* Prepare dialogue log for saving */
	Handle = Prepare_dialogue_log_for_saving();

	/* Success ? */
	if (!Handle)
	{
		/* No -> Error */
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write length of dialogue log */
	Length = MEM_Get_block_size(Handle);
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Wrote everything ? */
	if (Written_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

	/* Write dialogue log */
	Ptr = MEM_Claim_pointer(Handle);
	Written_bytes = DOS_Write
	(
		File_handle,
		Ptr,
		Length
	);
	MEM_Free_pointer(Handle);

	/* Free memory */
	MEM_Free_memory(Handle);

	/* Wrote everything ? */
	if (Written_bytes != Length)
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

/**** Write file parts ****/

	/* Check all eXtended file types */
	Failure = FALSE;
	for (File_type_index = 0;File_type_index < MAX_XFTYPES;File_type_index++)
	{
		/* Is this a file that should be saved ? */
		if (Xftypes[File_type_index].Flags & SAVE_FILE)
		{
			/* Yes -> Try all subgroups */
			for (Group_nr = 0;Group_nr <= 9;Group_nr++)
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
					Written_bytes = DOS_Write
					(
						File_handle,
						(UNBYTE *) &Length,
						sizeof(UNLONG)
					);

					/* Wrote everything ? */
					if (Written_bytes != sizeof(UNLONG))
					{
						/* No -> Error */
						Error(ERROR_SAVED_GAME_ACCESS_ERROR);
						Failure = TRUE;
						break;
					}

					/* Write file type index */
					Written_bytes = DOS_Write
					(
						File_handle,
						(UNBYTE *) &File_type_index,
						sizeof(UNSHORT)
					);

					/* Wrote everything ? */
					if (Written_bytes != sizeof(UNSHORT))
					{
						/* No -> Error */
						Error(ERROR_SAVED_GAME_ACCESS_ERROR);
						Failure = TRUE;
						break;
					}

					/* Write group number */
					Written_bytes = DOS_Write
					(
						File_handle,
						(UNBYTE *) &Group_nr,
						sizeof(UNSHORT)
					);

					/* Wrote everything ? */
					if (Written_bytes != sizeof(UNSHORT))
					{
						/* No -> Error */
						Error(ERROR_SAVED_GAME_ACCESS_ERROR);
						Failure = TRUE;
						break;
					}

					/* Write library */
					Ptr = MEM_Claim_pointer(Handle);
					Written_bytes = DOS_Write
					(
						File_handle,
						Ptr,
						Length
					);
					MEM_Free_pointer(Handle);

					/* Kill (!) memory */
					MEM_Kill_memory(Handle);

					/* Wrote everything ? */
					if (Written_bytes != Length)
					{
						/* No -> Error */
						Error(ERROR_SAVED_GAME_ACCESS_ERROR);
						Failure = TRUE;
						break;
					}
				}
				else
				{
					/* No -> File not found ? */
					if (XLD_Last_error == XLD_FILE_NOT_FOUND)
					{
		 				/* Yes -> Pop error */
						ERROR_PopError();
					}
					else
					{
						/* No -> Exit (real error) */
						Error(ERROR_SAVED_GAME_ACCESS_ERROR);
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
	Written_bytes = DOS_Write
	(
		File_handle,
		(UNBYTE *) &Length,
		sizeof(UNLONG)
	);

	/* Wrote everything ? */
	if (Written_bytes != sizeof(UNLONG))
	{
		/* No -> Error */
		Error(ERROR_SAVED_GAME_ACCESS_ERROR);
		goto EXIT_SAVE_GAME_STATE;
	}

	/* No errors */
	Success = TRUE;

EXIT_SAVE_GAME_STATE:

	/* Has the saved game file been opened ? */
	if (File_handle != - 1)
	{
		/* Yes -> Close saved game file */
		DOS_Close(File_handle);

		/* Has an error occurred ? */
		if (!Success)
		{
			/* Yes -> Delete saved game file */
			DOS_Delete(Save_game_fname);
		}
		else
		{
			/* No -> Store saved game number in INI file */
			sprintf(Number, "%u", Saved_game_nr);
			Write_INI_variable
			(
				"ALBION",
				"SAVED_GAME_NR",
				Number
			);
		}
	}

	return Success;
}

