/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_second_data_path
 * FUNCTION  : Look for a second data path.
 * FILE      : XFTYPES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.08.95 10:45
 * LAST      : 25.09.95 19:02
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
	static UNCHAR Source_path_command[] = "SOURCE_PATH=";

	MEM_HANDLE Handle;
	SILONG File_length;
	UNLONG Read;
	UNLONG i;
	SISHORT ID;
	UNBYTE *Pointer;
	UNCHAR New_data_path[MAX_DATA_PATH_LENGTH];
	UNCHAR *Char_ptr;

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
						for (i=0;i<File_length - strlen(Source_path_command) + 1;i++)
						{
							/* Is this the source path entry ? */
							if (!strnicmp((UNCHAR *) (Pointer + i),
							 Source_path_command, strlen(Source_path_command)))
							{
								/* Yes -> Extract source path */
								strncpy
								(
									New_data_path,
									(UNCHAR *) (Pointer + i + strlen(Source_path_command)),
									MAX_DATA_PATH_LENGTH - 1
								);

								/* Insert EOL just to be very, very sure */
								New_data_path[MAX_DATA_PATH_LENGTH - 1] = '\0';

								/* Look for NL and replace it by EOL */
								Char_ptr = strchr(New_data_path, '\n');
								if (Char_ptr)
								{
									*Char_ptr = '\0';
								}

								/* Look for CR and replace it by EOL */
								Char_ptr = strchr(New_data_path, '\r');
								if (Char_ptr)
								{
									*Char_ptr = '\0';
								}

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

