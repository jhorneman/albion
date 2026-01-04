
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include	<sys\types.h>
#include	<sys\stat.h>
#include	<fcntl.h>
#include <io.h>
#include <dos.h>
#include <direct.h>

	/* Get current working directory */
	cwd = getcwd(&(Data_paths[0][0]), MAX_DATA_PATH_LENGTH);

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : File_exists
 * FUNCTION  : Check if a file exists.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.07.95 15:05
 * LAST      : 27.07.95 15:34
 * INPUTS    : UNCHAR *Filename - Pointer to filename.
 * RESULT    : BOOLEAN : TRUE (exists) or FALSE (does not exist).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
File_exists(UNCHAR *Filename)
{
	BOOLEAN Result = TRUE;
	short ID;

	/* Try to open the file */
	ID = open(Filename, O_RDONLY | O_BINARY);

	/* Failure ? */
	if (ID == -1)
	{
		/* Yes -> File not found ? */
		if (errno == ENOENT)
		{
			/* Yes -> File does not exist */
			Result = FALSE;
		}
	}
	else
	{
		/* No -> Close file */
		close(ID);
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_directory
 * FUNCTION  : Change the current directory.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.07.95 15:05
 * LAST      : 28.07.95 12:43
 * INPUTS    : UNCHAR *Pathname - Pointer to pathname.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_directory(UNCHAR *Pathname)
{
	unsigned int Dummy;
	unsigned int Drive_nr;

	/* Does the pathname contain a drive letter ? */
	if (strlen(Pathname) > 2)
	{
		if (*(Pathname + 1) == ':')
		{
			/* Yes -> Change to this drive */
			Drive_nr = (unsigned int) toupper(Pathname[0]) -
			 (unsigned int) 'A' + 1;

			_dos_setdrive(Drive_nr, &Dummy);
		}
	}

	/* Change directory */
	chdir(Pathname);
}

