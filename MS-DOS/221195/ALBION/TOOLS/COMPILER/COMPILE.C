/************
 * NAME     : COMPILE.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-7-1994
 * PROJECT  : XLD Library compiler
 * NOTES    :
 * SEE ALSO : XLOAD.C
 ************/

/* includes */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <errno.h>
#include <string.h>

#include <BBDEF.H>

/* defines */

/* XLD library types */
#define XLD_NORMAL				(0)
#define XLD_PACKED				(1)
#define XLD_ENCRYPTED			(2)
#define XLD_PACKED_ENCRYPTED	(3)

/* Maximum number of subfiles */
#define MAX_SUBFILES				(100)

/* Maxmimum length of path */
#define PATH_LENGTH				(100)

/* Compile block size */
#define COMPILE_BLOCK_SIZE		(100 * 1024)

/* structure definitions */

/* XLD library header */
struct XLD_Library {
	UNCHAR ID[4];					/* Library ID ('XLD0') */
	UNCHAR Format;					/* 'I' for Intel, 'M' for Motorola */
	UNBYTE Library_type;			/* See above */
	UNSHORT Nr_subfiles;			/* Number of subfiles in this library */
};

/* global variables */

UNBYTE Compile_buffer[COMPILE_BLOCK_SIZE];

UNSHORT Total_nr_subfiles;
UNSHORT Highest_subfile_index;

UNCHAR *Subfile_path_ptrs[MAX_SUBFILES + 1];
UNCHAR Path_buffer[(MAX_SUBFILES + 1) * PATH_LENGTH];

int Return_value = 0;

/* prototypes */

void main(int argc, char** argv);

void Get_subfile_data(UNSHORT First_subfile_nr, UNSHORT Nr_subfiles,
 UNCHAR *Source_path);

UNLONG Compile_XLD_library(UNSHORT First_subfile_nr, UNSHORT Nr_subfiles,
 UNCHAR *Library_path);

UNLONG Get_file_length(FILE *File_handle);

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : main
 * FUNCTION  : Main function of XLD compiler.
 * FILE      : COMPILE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.95 22:18
 * LAST      : 13.09.95 11:34
 * INPUTS    : int argc - Number of arguments.
 *             char **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
main(int argc, char** argv)
{
	UNLONG Library_file_length;
	UNSHORT First_subfile_index;
	UNSHORT Nr_subfiles_in_sublib;
	UNSHORT Sublib_nr;
	UNCHAR Source_path[200];
	UNCHAR Library_path[200];

	/* Exit if number of arguments is illegal */
	if (argc != 4)
	{
		printf("Illegal number of arguments.\n");
		return;
	}

	/* Get source and library path beginnings from environment variables */
	strcpy(Source_path, getenv("XLD_SRC_PATH"));
	strcpy(Library_path, getenv("XLD_LIB_PATH"));

	strcat(Source_path,"\\");
	strcat(Library_path,"\\");

	/* Append rests of paths from arguments */
	strcat(Source_path, argv[1]);
	strcat(Library_path, argv[2]);

	/* Get other arguments */
	Sublib_nr = atoi(argv[3]);

	/* Tell the user what's going on */
	printf("\n");
	printf("************************************************************\n");
	printf("Compiling XLD-library.\n");
	printf("Source : %s.\n", Source_path);
	printf("Target : %s.\n", Library_path);
	printf("************************************************************\n");
	printf("\n");

	/* Is this the first sub-library ? */
	if (Sublib_nr)
	{
		/* No -> Read paths of subfiles */
		Get_subfile_data
		(
			0,
			100,
			Source_path
		);

		/* Calculate first subfile index and number of subfiles in this
		  sub-library */
		First_subfile_index = Sublib_nr * 100;

		/* Is this the last sub-library ? */
		if (Sublib_nr == Highest_subfile_index / 100)
		{
			Nr_subfiles_in_sublib = Highest_subfile_index % 100 + 1;
		}
		else
		{
			Nr_subfiles_in_sublib = 100;
		}
	}
	else
	{
		/* Yes -> Read paths of subfiles */
		Get_subfile_data
		(
			1,
			99,
			Source_path
		);

		/* Calculate first subfile index and number of subfiles in this
		  sub-library */
		First_subfile_index = 1;

		/* Is this the last sub-library ? */
		if (Sublib_nr == Highest_subfile_index / 100)
		{
			Nr_subfiles_in_sublib = Highest_subfile_index;
		}
		else
		{
			Nr_subfiles_in_sublib = 99;
		}
	}

	/* Insert sub-library code into library path */
	*(Library_path + strlen(Library_path) - 5) = '0' + Sublib_nr;

	/* Compile library */
	Library_file_length = Compile_XLD_library
	(
		First_subfile_index,
		Nr_subfiles_in_sublib,
		Library_path
	);

	/* Status report */
	printf("XLD-library %u contains ", Sublib_nr);

	if (Nr_subfiles_in_sublib == 1)
		printf("1 file.\n");
	else
		printf("%u files.\n", Nr_subfiles_in_sublib);

	printf("Library length : %u.\n", Library_file_length);

	exit(Return_value);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_subfile_data
 * FUNCTION  : Get the paths and lengths of all subfiles.
 * FILE      : COMPILE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.95 22:19
 * LAST      : 13.09.95 11:42
 * INPUTS    : UNSHORT First_subfile_nr - Number of first subfile (1...).
 *             UNSHORT Nr_subfiles - Number of subfiles to compile (1...100).
 *             UNCHAR *Source_path - Path where the subfiles can be found,
 *              including ?? wildcard.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will set the global variable
 *              Highest_subfile_index, and it will fill the Subfile_path_ptrs
 *              array. If a subfile does not exist, it's entry in this array
 *              will be NULL.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_subfile_data(UNSHORT First_subfile_nr, UNSHORT Nr_subfiles,
 UNCHAR *Source_path)
{
	struct find_t File_info;
	unsigned Return_code;

	UNSHORT Wildcard_offset;
	UNSHORT Offset;
	UNSHORT Subfile_index;
	UNSHORT i;

	UNCHAR *Seek_ptr;
	UNCHAR *Path_buffer_ptr;
	UNCHAR *Subfile_name;
	UNCHAR Path[PATH_LENGTH];
	UNCHAR Nr[3];

	/* Reset variables */
	Highest_subfile_index	= 0;
	Path_buffer_ptr			= Path_buffer;

	/* Clear subfile filename array */
	for (i=0;i<MAX_SUBFILES;i++)
	{
		Subfile_path_ptrs[i] = NULL;
	}

	/* Find wildcard in source path */
	Seek_ptr = (UNCHAR *) strstr(Source_path, "??");

	/* Found ? */
	if (!Seek_ptr)
	{
		printf("Source path does not contain ??.\n");
		Return_value = -1;
		return;
	}

	/* Calculate offset to wildcard from the end of the filename */
	Wildcard_offset = strlen(Source_path) - (Seek_ptr - Source_path);

	/* Find start of filename in source path */
	Seek_ptr = (UNCHAR *) strrchr(Source_path, '\\');

	/* Found ? */
	if (!Seek_ptr)
	{
		printf("Illegal source path.\n");
		Return_value = -1;
		return;
	}

	/* Copy path */
	strncpy(Path, Source_path, (Seek_ptr - Source_path + 1));
	Path[Seek_ptr - Source_path + 1] = '\0';

	/* Find first file */
	Return_code = _dos_findfirst
	(
		Source_path,
		_A_NORMAL,
		&File_info
	);

	/* Found any ? */
	while (!Return_code)
	{
		/* Yes -> Get subfile name */
		Subfile_name = (UNCHAR *) File_info.name;

		/* Extract subfile index from path */
		Offset = strlen(Subfile_name) - Wildcard_offset;

		Nr[0] = Subfile_name[Offset];
		Nr[1] = Subfile_name[Offset + 1];
		Nr[2] = '\0';

		Subfile_index = atoi(Nr);

		/* Legal subfile index ? */
		if ((Subfile_index >= First_subfile_nr) &&
		 (Subfile_index < First_subfile_nr + Nr_subfiles))
		{
			/* Yes -> Higher than the current highest index ? */
			if (Subfile_index > Highest_subfile_index)
			{
				/* Yes -> Set new highest index */
				Highest_subfile_index = Subfile_index;
			}

			/* Store path of subfile */
			Subfile_path_ptrs[Subfile_index] = Path_buffer_ptr;

			/* Copy path into buffer */
			strcpy(Path_buffer_ptr, Path);
			Path_buffer_ptr += strlen(Path);

			strcpy(Path_buffer_ptr, Subfile_name);
			Path_buffer_ptr += strlen(Subfile_name);

			/* Insert EOL */
			*Path_buffer_ptr++ = '\0';
		}

		/* Find next file */
		Return_code = _dos_findnext(&File_info);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compile_XLD_library
 * FUNCTION  : Compile an XLD-library.
 * FILE      : COMPILE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.95 21:52
 * LAST      : 13.09.95 12:37
 * INPUTS    : UNSHORT First_subfile_nr - Number of first subfile (1...).
 *             UNSHORT Nr_subfiles - Number of subfiles to compile (1...100).
 *             UNCHAR *Library_path - Path of XLD-library.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Compile_XLD_library(UNSHORT First_subfile_nr, UNSHORT Nr_subfiles,
 UNCHAR *Library_path)
{
	static UNCHAR XLD_ID[4] = {'X','L','D','0'};
	static UNCHAR Format = 'I';

	static UNBYTE Library_types[4] = {
		XLD_NORMAL,
		XLD_PACKED,
		XLD_ENCRYPTED,
		XLD_PACKED_ENCRYPTED
	};

/*	int Library_file_handle;
	int Subfile_handle; */
	FILE *Library_file_handle;
	FILE *Subfile_handle;

	UNLONG Lengths[100];
	UNLONG Library_file_length;
	UNLONG Subfile_length;
	UNSHORT i, j;

	/* Clip first subfile number */
	First_subfile_nr %= MAX_SUBFILES;

	/* Too many subfiles ? */
	if (Nr_subfiles > MAX_SUBFILES)
	{
		/* Yes -> Report error and exit */
		printf("Too many subfiles in library.\n");

		Return_value = -1;
		return 0;
	}

	/* Clear lengths array */
	for (i=0;i<MAX_SUBFILES;i++)
	{
		Lengths[i] = 0;
	}

	/* Try to open library */
/*	Library_file_handle = open
	(
		Library_path,
		O_WRONLY | O_CREAT | O_BINARY,
		S_IRWXU | S_IRWXG | S_IRWXO
	); */

	Library_file_handle = fopen
	(
		Library_path,
		"wb"
	);

	/* Success ? */
//	if (Library_file_handle == -1)
	if (Library_file_handle == NULL)
	{
		/* No -> Report error and exit */
		printf("Library couldn't be opened.\n");

		Return_value = -1;
		return 0;
	}

	/* Write library header */
/*	write
	(
		Library_file_handle,
		&XLD_ID[0],
		4
	);
	write
	(
		Library_file_handle,
		&Format,
		1
	);
	write
	(
		Library_file_handle,
		&Library_types[XLD_NORMAL],
		1
	); */
	fwrite
	(
		&XLD_ID[0],
		1,
		4,
		Library_file_handle
	);
	fwrite
	(
		&Format,
		1,
		1,
		Library_file_handle
	);
	fwrite
	(
		&Library_types[XLD_NORMAL],
		1,
		1,
		Library_file_handle
	);

	/* Write number of subfiles */
/*	write
	(
		Library_file_handle,
		&Nr_subfiles,
		2
	); */
	fwrite
	(
		&Nr_subfiles,
		sizeof(UNSHORT),
		1,
		Library_file_handle
	);

	/* Write dummy subfile lengths */
/*	write
	(
		Library_file_handle,
		&Lengths[0],
		Nr_subfiles * 4
	); */
	fwrite
	(
		&Lengths[0],
		sizeof(UNLONG),
		Nr_subfiles,
		Library_file_handle
	);

	/* Compile library */
	Library_file_length = 8 + (Nr_subfiles * sizeof(UNLONG));
	for (i=0;i<Nr_subfiles;i++)
	{
		/* Does this subfile exist ? */
		if (Subfile_path_ptrs[First_subfile_nr + i])
		{
			/* Yes -> Open subfile */
/*			Subfile_handle = open
			(
				Subfile_path_ptrs[First_subfile_nr + i],
				O_RDONLY | O_BINARY
			); */
			Subfile_handle = fopen
			(
				Subfile_path_ptrs[First_subfile_nr + i],
				"rb"
			);

			/* Success ? */
//			if (Subfile_handle == -1)
			if (Subfile_handle == NULL)
			{
				/* No -> Close library */
//				close(Library_file_handle);
				fclose(Library_file_handle);

				/* Report error and exit */
				printf("Subfile %u couldn't be opened. OS error: %u\n",
				 First_subfile_nr + i, errno);

				Return_value = -1;
				break;
			}

			/* Get subfile length */
//			Subfile_length = filelength(Subfile_handle);
			Subfile_length = Get_file_length(Subfile_handle);

			/* Store for later */
			Lengths[i] = Subfile_length;

			/* Read whole blocks */
			for (j=0;j<(Subfile_length / COMPILE_BLOCK_SIZE);j++)
			{
				/* Read a block from the subfile */
/*				read
				(
					Subfile_handle,
					Compile_buffer,
					COMPILE_BLOCK_SIZE
				); */
				fread
				(
					Compile_buffer,
					1,
					COMPILE_BLOCK_SIZE,
					Subfile_handle
				);

				/* Write the block to the library */
/*				write
				(
					Library_file_handle,
					Compile_buffer,
					COMPILE_BLOCK_SIZE
				); */
				fwrite
				(
					Compile_buffer,
					1,
					COMPILE_BLOCK_SIZE,
					Library_file_handle
				);
			}

			/* Anything left ? */
			if (Subfile_length % COMPILE_BLOCK_SIZE)
			{
				/* Yes -> Read the rest from the subfile */
/*				read
				(
					Subfile_handle,
					Compile_buffer,
					Subfile_length % COMPILE_BLOCK_SIZE
				); */
				fread
				(
					Compile_buffer,
					1,
					Subfile_length % COMPILE_BLOCK_SIZE,
					Subfile_handle
				);

				/* Write the rest to the library */
/*				write
				(
					Library_file_handle,
					Compile_buffer,
					Subfile_length % COMPILE_BLOCK_SIZE
				); */
				fwrite
				(
					Compile_buffer,
					1,
					Subfile_length % COMPILE_BLOCK_SIZE,
					Library_file_handle
				);
			}

			/* Close subfile */
//			close(Subfile_handle);
			fclose(Subfile_handle);

			/* Add up */
			Library_file_length += Subfile_length;
		}
	}

	/* Seek back to the start of the file */
/*	lseek
	(
		Library_file_handle,
		sizeof(struct XLD_Library),
		SEEK_SET
	); */
	fseek
	(
		Library_file_handle,
		sizeof(struct XLD_Library),
		SEEK_SET
	);

	/* Write subfile lengths */
/*	write
	(
		Library_file_handle,
		&Lengths[0],
		Nr_subfiles * 4
	); */
	fwrite
	(
		&Lengths[0],
		sizeof(UNLONG),
		Nr_subfiles,
		Library_file_handle
	);

	/* Close library */
//	close(Library_file_handle);
	fclose(Library_file_handle);

	return Library_file_length;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_file_length
 * FUNCTION  : Get the length of an opened file.
 * FILE      : COMPILE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.09.95 15:49
 * LAST      : 17.09.95 15:49
 * INPUTS    : FILE *File_handle - Handle of opened file.
 * RESULT    : UNLONG : File length.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_file_length(FILE *File_handle)
{
	UNLONG Current_position;
	UNLONG File_length;

	/* Get current seek position within file */
	Current_position = ftell(File_handle);

	/* Seek to the end of the file */
	fseek
	(
		File_handle,
		0,
		SEEK_END
	);

	/* Get file position (= file length) */
	File_length = ftell(File_handle);

	/* Seek back to the original position */
	fseek
	(
		File_handle,
		Current_position,
		SEEK_SET
	);

	return File_length;
}

