/************
 * NAME     : XLOAD.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-7-1994
 * PROJECT  : eXtended LOADing system
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : XLOAD.H
 ************/

/* includes */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include	<sys\types.h>
#include	<sys\stat.h>
#include	<fcntl.h>
#include <io.h>
#include <dos.h>
#include <direct.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBDOS.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBERROR.H>

#include <XLOAD.H>
#include <SORT.H>

#include <CONTROL.H>
#include <GRAPHICS.H>

/* defines */

#define SUBFILES_MAX				(100)

/* XLD library types */
#define XLD_NORMAL				(0)
#define XLD_PACKED				(1)
#define XLD_ENCRYPTED			(2)
#define XLD_PACKED_ENCRYPTED	(3)

/* XLD error codes */
#define XLDERR_GET_FILE_LENGTH_FAILED	(1)
#define XLDERR_NO_MEMORY_FOR_FILE		(2)
#define XLDERR_READ_FAILED					(3)
#define XLDERR_SUBFILE_ZERO				(4)
#define XLDERR_SUBFILE_TOO_HIGH			(5)
#define XLDERR_ILLEGAL_LIBRARY_TYPE		(6)
#define XLDERR_LIBRARY_CORRUPT			(7)
#define XLDERR_NOT_A_LIBRARY				(8)
#define XLDERR_OPEN_FAILED					(9)
#define XLDERR_OPEN_LIBRARY_FAILED		(10)
#define XLDERR_WRITE_FAILED				(11)
#define XLDERR_CANNOT_SAVE_PACKED		(12)
#define XLDERR_SUBFILE_LENGTH_ZERO		(13)
#define XLDERR_FILE_DOES_NOT_EXIST		(14)

#define XLDERR_MAX							(14)

/* structure definitions */

/* XLD library header */
struct XLD_Library {
	UNCHAR ID[4];					/* Library ID ('XLD0') */
	UNCHAR Format;					/* 'I' for Intel, 'M' for Motorola */
	UNBYTE Library_type;			/* See above */
	UNSHORT Nr_subfiles;			/* Number of subfiles in this library */
};
/* This library header will be followed by UNLONGs containing the lengths
 of the subfiles. */

/* XLOAD error data */
struct XLD_error {
	UNSHORT Code;
	struct XFile_type *Xftype;
	UNSHORT Subfile_nr;
};

/* Batch load data */
struct Batch_load_data {
	UNSHORT Subfile_nr;
	UNSHORT Desort_index;
};

/* prototypes */

static BOOLEAN XLD_Do_load_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);

static void XLD_Swap(UNLONG A, UNLONG B, UNBYTE *Data);
static BOOLEAN XLD_Compare(UNLONG A, UNLONG B, UNBYTE *Data);

static MEM_HANDLE XLD_Do_load_subfile(struct XFile_type *Xftype,
 UNSHORT Subfile_nr);
static MEM_HANDLE XLD_Load_unpacked_subfile(struct XFile_type *Xftype,
 UNSHORT Subfile_nr);

static BOOLEAN XLD_Open_library(struct XFile_type *Xftype, UNSHORT Subfile_nr);
static void XLD_Close_library(void);

static void XLD_Error(UNSHORT Error_code, struct XFile_type *Xftype,
 UNSHORT Subfile_nr);
static void XLD_Print_error(UNCHAR *buffer, UNBYTE *data);

/* global variables */

XLD_ERROR_T XLD_Last_error;

static const UNCHAR *Library_ID = "XLD0";

static BOOLEAN XLD_Library_open = FALSE;
static BOOLEAN XLD_Batch_mode = FALSE;
static BOOLEAN XLD_Full_batch_mode = FALSE;

static SISHORT XLD_Library_ID;

static struct XLD_Library XLD_Library;

static UNLONG XLD_Subfile_lengths[SUBFILES_MAX];
static UNLONG XLD_Subfile_length;
static UNLONG XLD_Subfile_offset;
static UNLONG XLD_Batch_offset;

static UNCHAR XLD_Library_name[] = "Xload";

static UNCHAR *XLD_Error_strings[XLDERR_MAX + 1] = {
	"Illegal error code.",
	"File length could not be determined.",
	"No memory could be allocated.",
	"Read failed.",
	"Subfile 0 not allowed.",
	"Subfile number is too high.",
	"Library has an illegal type.",
	"Library header is corrupt.",
	"This is not a library.",
	"Open failed.",
	"Library could not be opened.",
	"Write failed.",
	"Cannot save packed sub-files.",
	"Subfile length is 0.",
	"File does not exist."
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_file
 * FUNCTION  : Load a file.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 13:30
 * LAST      : 27.07.95 16:24
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
XLD_Load_file(struct XFile_type *Xftype)
{
	SILONG File_length;
	SILONG Read;
	SISHORT ID;
	MEM_HANDLE Handle = NULL;
	UNBYTE *Pointer;

	/* No errors until now */
	XLD_Last_error = XLD_NO_ERROR;

	/* Install loading mouse pointer */
	Push_mouse(&(Mouse_pointers[LOADING_MPTR]));

	/* Does this file exist ? */
	if (!File_exists(Xftype->Filename))
	{
		/* No -> Report error */
		XLD_Error(XLDERR_FILE_DOES_NOT_EXIST, Xftype, 0);

		XLD_Last_error = XLD_FILE_NOT_FOUND;

		goto EXIT_XLD_LOAD_FILE;
	}

	/* Get file length */
	File_length = DOS_GetFileLength(Xftype->Filename);
	if (File_length < 0)
	{
		/* Error -> Report */
		XLD_Error(XLDERR_GET_FILE_LENGTH_FAILED, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		goto EXIT_XLD_LOAD_FILE;
	}

	/* Allocate memory for file */
	Handle = MEM_Do_allocate((UNLONG) File_length, 0L,
	 (struct File_type *) Xftype);
	if (!Handle)
	{
		/* Error -> Report */
		XLD_Error(XLDERR_NO_MEMORY_FOR_FILE, Xftype, 0);

		XLD_Last_error = XLD_OUT_OF_MEMORY;

		goto EXIT_XLD_LOAD_FILE;
	}

	/* Open the file */
 	ID = DOS_Open(Xftype->Filename,BBDOSFILESTAT_READ);
	if (ID < 0)
	{
		/* Error -> Report */
		XLD_Error(XLDERR_OPEN_FAILED, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		goto EXIT_XLD_LOAD_FILE;
	}

	/* Read file into memory */
	Pointer = MEM_Claim_pointer(Handle);
	Read = DOS_Read(ID, Pointer, (UNLONG) File_length);

	/* Close the file */
	DOS_Close(ID);

	/* Succesful ? */
	if (Read != File_length)
	{
		/* No -> Free memory */
		MEM_Free_pointer(Handle);
		MEM_Free_memory(Handle);

		/* Report error */
		XLD_Error(XLDERR_READ_FAILED, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		goto EXIT_XLD_LOAD_FILE;
	}
	else
	{
		/* Yes -> Call convertor function (if any) */
		if (Xftype->Converter)
			(Xftype->Converter)(Pointer, Handle);

		MEM_Free_pointer(Handle);
	}

EXIT_XLD_LOAD_FILE:
	/* Remove mouse pointer */
	Pop_mouse();

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Save_file
 * FUNCTION  : Save a file.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:52
 * LAST      : 28.07.95 10:17
 * INPUTS    : MEM_HANDLE Handle - Memory handle of file.
 *             struct XFile_type *Xftype - Pointer to XLD file type.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Save_file(MEM_HANDLE Handle, struct XFile_type *Xftype)
{
	SILONG File_length;
	SILONG Write;
	SISHORT ID;
	UNBYTE *Pointer;

	/* No errors until now */
	XLD_Last_error = XLD_NO_ERROR;

	/* Install loading mouse pointer */
	Push_mouse(&(Mouse_pointers[LOADING_MPTR]));

	/* Get file length */
	File_length = MEM_Get_block_size(Handle);

	/* Get file address */
	Pointer = MEM_Claim_pointer(Handle);

	/* Open the file */
 	ID = DOS_Open(Xftype->Filename,BBDOSFILESTAT_WRITE);

	/* Succesful ? */
	if (ID>=0)
	{
		/* Yes -> Write file */
		Write = DOS_Write(ID, Pointer, (UNLONG) File_length);

		/* Close the file */
		DOS_Close(ID);

		/* Succesful ? */
		if (Write != File_length)
		{
			/* No -> Error */
			XLD_Error(XLDERR_WRITE_FAILED, Xftype, 0);

			XLD_Last_error = XLD_FILE_ERROR;
		}
	}
	else
	{
		/* No -> Error */
		XLD_Error(XLDERR_OPEN_FAILED, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;
	}

	MEM_Free_pointer(Handle);

	/* Remove mouse pointer */
	Pop_mouse();

	/* Any errors ? */
	if (XLD_Last_error == XLD_NO_ERROR)
		return TRUE;
	else
		return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_standard_file
 * FUNCTION  : Load a file, using just the filename.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 13:50
 * LAST      : 22.07.94 13:50
 * INPUTS    : UNCHAR *Filename - Pointer to filename.
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
XLD_Load_standard_file(UNCHAR *Filename)
{
	struct XFile_type Standard;

	/* Create eXtended File Type */
	memcpy(&Standard, &MEM_Default_file_type, sizeof(struct File_type));
	Standard.Filename = Filename;

	/* Load it */
	return(XLD_Load_file(&Standard));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_XLD_library
 * FUNCTION  : Load an entire XLD-library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.95 15:46
 * LAST      : 27.07.95 16:34
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Group_nr - Group number (0...9).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
XLD_Load_XLD_library(struct XFile_type *Xftype, UNSHORT Group_nr)
{
	MEM_HANDLE Handle = NULL;
	SILONG File_length;
	SILONG Read;
	SISHORT ID;
	UNBYTE *Pointer;
	UNCHAR *Number = "0";

	/* No errors until now */
	XLD_Last_error = XLD_NO_ERROR;

	/* Install loading mouse pointer */
	Push_mouse(&(Mouse_pointers[LOADING_MPTR]));

	/* Select subfile group */
	sprintf(Number, "%u", Group_nr);
	*(Xftype->Filename + strlen(Xftype->Filename) - 5) = *Number;

	/* Does this file exist ? */
	if (!File_exists(Xftype->Filename))
	{
		/* No -> Report error */
		XLD_Error(XLDERR_FILE_DOES_NOT_EXIST, Xftype, 0);

		XLD_Last_error = XLD_FILE_NOT_FOUND;

		goto EXIT_XLD_LOAD_LIBRARY;
	}

	/* Get file length */
	File_length = DOS_GetFileLength(Xftype->Filename);
	if (File_length < 0)
	{
		/* Error -> Report */
		XLD_Error(XLDERR_GET_FILE_LENGTH_FAILED, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		goto EXIT_XLD_LOAD_LIBRARY;
	}

	/* Allocate memory for file */
	Handle = MEM_Do_allocate((UNLONG) File_length, 0L,
	 (struct File_type *) Xftype);
	if (!Handle)
	{
		/* Error -> Report */
		XLD_Error(XLDERR_NO_MEMORY_FOR_FILE, Xftype, 0);

		XLD_Last_error = XLD_OUT_OF_MEMORY;

		goto EXIT_XLD_LOAD_LIBRARY;
	}

	/* Open the file */
 	ID = DOS_Open(Xftype->Filename,BBDOSFILESTAT_READ);
	if (ID < 0)
	{
		/* Error -> Report */
		XLD_Error(XLDERR_OPEN_FAILED, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		goto EXIT_XLD_LOAD_LIBRARY;
	}

	/* Read file into memory */
	Pointer = MEM_Claim_pointer(Handle);
	Read = DOS_Read(ID, Pointer, (UNLONG) File_length);

	/* Close the file */
	DOS_Close(ID);

	MEM_Free_pointer(Handle);

	/* Succesful ? */
	if (Read != File_length)
	{
		/* No -> Free memory */
		MEM_Free_memory(Handle);

		/* Report error */
		XLD_Error(XLDERR_READ_FAILED, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;
	}

EXIT_XLD_LOAD_LIBRARY:
	/* Remove mouse pointer */
	Pop_mouse();

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Save_XLD_library
 * FUNCTION  : Save an entire XLD-library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.95 15:58
 * LAST      : 28.07.95 10:20
 * INPUTS    : MEM_HANDLE Handle - Memory handle of file.
 *             struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Group_nr - Group number (0...9).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Save_XLD_library(MEM_HANDLE Handle, struct XFile_type *Xftype,
 UNSHORT Group_nr)
{
	UNCHAR *Number = "0";

	/* Select subfile group */
	sprintf(Number, "%u", Group_nr);
	*(Xftype->Filename + strlen(Xftype->Filename) - 5) = *Number;

	/* Save file */
	return (XLD_Save_file(Handle, Xftype));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_subfile
 * FUNCTION  : Load a subfile from a library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:01
 * LAST      : 27.07.95 16:35
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
XLD_Load_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr)
{
	MEM_HANDLE Handle = NULL;
	UNSHORT Subsubfile_nr;
	UNSHORT i;

	/* No errors until now */
	XLD_Last_error = XLD_NO_ERROR;

	/* Is subfile number 0 ? */
	if (!Subfile_nr)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_ZERO, Xftype, Subfile_nr);

		XLD_Last_error = XLD_ILLEGAL_PARAMETERS;

		return NULL;
	}

	/* Try to re-allocate */
	Handle = MEM_Reallocate_memory((UNLONG) Subfile_nr,
	 (struct File_type *) Xftype);
	if (Handle)
		return Handle;

	/* Install loading mouse pointer */
	Push_mouse(&(Mouse_pointers[LOADING_MPTR]));

	/* Open library */
	if (!XLD_Open_library(Xftype, Subfile_nr))
	{
		/* Error -> Report */
		XLD_Error(XLDERR_OPEN_LIBRARY_FAILED, Xftype, Subfile_nr);

		goto EXIT_XLD_LOAD_SUBFILE;
	}

	/* Calculate subfile number within group */
	if (Subfile_nr < 100)
	{
		Subsubfile_nr = Subfile_nr - 1;
	}
	else
	{
		Subsubfile_nr = Subfile_nr % 100;
	}

	/* Is the subfile number too high ? */
	if (Subsubfile_nr >= XLD_Library.Nr_subfiles)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_TOO_HIGH, Xftype, Subfile_nr);

		XLD_Last_error = XLD_ILLEGAL_PARAMETERS;

		goto EXIT_XLD_LOAD_SUBFILE;
	}

	/* Get subfile length */
	XLD_Subfile_length = XLD_Subfile_lengths[Subsubfile_nr];

	/* Is the subfile length zero ? */
	if (!XLD_Subfile_length)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_LENGTH_ZERO, Xftype, Subfile_nr);

		XLD_Last_error = XLD_FILE_ERROR;

		goto EXIT_XLD_LOAD_SUBFILE;
	}

	/* Calculate seek offset to subfile */
	XLD_Subfile_offset = 0;
	for (i=0;i<Subsubfile_nr;i++)
	{
		XLD_Subfile_offset += XLD_Subfile_lengths[i];
	}

	/* Load the subfile */
	Handle = XLD_Do_load_subfile(Xftype, Subfile_nr);

EXIT_XLD_LOAD_SUBFILE:
	/* Close the library */
	/* (The Close function will check if the library was actually opened.) */
	XLD_Close_library();

	/* Remove mouse pointer */
	Pop_mouse();

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Save_subfile
 * FUNCTION  : Save a subfile into a library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 15:59
 * LAST      : 28.07.95 10:20
 * INPUTS    : MEM_HANDLE Handle - Memory handle of file.
 *             struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Save_subfile(MEM_HANDLE Handle, struct XFile_type *Xftype,
 UNSHORT Subfile_nr)
{
	SILONG Write;
	UNSHORT Subsubfile_nr;
	UNSHORT i;
	UNBYTE *Pointer;

	/* No errors until now */
	XLD_Last_error = XLD_NO_ERROR;

	/* Is subfile number 0 ? */
	if (!Subfile_nr)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_ZERO, Xftype, Subfile_nr);

		XLD_Last_error = XLD_ILLEGAL_PARAMETERS;

		return FALSE;
	}

	/* Install loading mouse pointer */
	Push_mouse(&(Mouse_pointers[LOADING_MPTR]));

	/* Open library */
	if (!XLD_Open_library(Xftype, Subfile_nr))
	{
		/* Error -> Report */
		XLD_Error(XLDERR_OPEN_LIBRARY_FAILED, Xftype, Subfile_nr);

		goto EXIT_XLD_SAVE_SUBFILE;
	}

	/* Calculate subfile number within group */
	if (Subfile_nr < 100)
	{
		Subsubfile_nr = Subfile_nr - 1;
	}
	else
	{
		Subsubfile_nr = Subfile_nr % 100;
	}

	/* Is the subfile number too high ? */
	if (Subsubfile_nr >= XLD_Library.Nr_subfiles)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_TOO_HIGH, Xftype, Subfile_nr);

		XLD_Last_error = XLD_ILLEGAL_PARAMETERS;

		goto EXIT_XLD_SAVE_SUBFILE;
	}

	/* Get subfile length */
	XLD_Subfile_length = XLD_Subfile_lengths[Subsubfile_nr];

	/* Is the subfile length zero ? */
	if (!XLD_Subfile_length)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_LENGTH_ZERO, Xftype, Subfile_nr);

		XLD_Last_error = XLD_FILE_ERROR;

		goto EXIT_XLD_SAVE_SUBFILE;
	}

	/* Calculate seek offset to subfile */
	XLD_Subfile_offset = 0;
	for (i=0;i<Subsubfile_nr;i++)
	{
		XLD_Subfile_offset += XLD_Subfile_lengths[i];
	}

	/* Save the subfile */
	switch (XLD_Library.Library_type)
	{
		case XLD_NORMAL:
		case XLD_ENCRYPTED:
		{
			/* Seek to the start of the subfile */
			DOS_Seek(XLD_Library_ID, BBDOS_SEEK_CURRENTPOS,
			 (SILONG) XLD_Subfile_offset);

			/* Get subfile address */
			Pointer = MEM_Claim_pointer(Handle);

			/* Write subfile */
			Write = DOS_Write(XLD_Library_ID, Pointer,
			 (UNLONG) XLD_Subfile_length);

			/* Succesful ? */
			if (Write != XLD_Subfile_length)
			{
				/* No -> Error */
				XLD_Error(XLDERR_WRITE_FAILED, Xftype, Subfile_nr);

				XLD_Last_error = XLD_FILE_ERROR;
			}

			MEM_Free_pointer(Handle);

			break;
		}
		case XLD_PACKED:
		case XLD_PACKED_ENCRYPTED:
		{
			XLD_Error(XLDERR_CANNOT_SAVE_PACKED, Xftype, Subfile_nr);

			XLD_Last_error = XLD_ILLEGAL_PARAMETERS;
			break;
		}
		default:
		{
			XLD_Error(XLDERR_ILLEGAL_LIBRARY_TYPE, Xftype, Subfile_nr);

			XLD_Last_error = XLD_ILLEGAL_PARAMETERS;
			break;
		}
	}

EXIT_XLD_SAVE_SUBFILE:
	/* Close the library */
	/* (The Close function will check if the library was actually opened.) */
	XLD_Close_library();

	/* Remove mouse pointer */
	Pop_mouse();

	/* Any errors ? */
	if (XLD_Last_error == XLD_NO_ERROR)
		return TRUE;
	else
		return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_unique_subfile
 * FUNCTION  : Load a unique subfile from a library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:03
 * LAST      : 27.07.95 16:52
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
XLD_Load_unique_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr)
{
	MEM_HANDLE Handle = NULL;
	UNSHORT Subsubfile_nr;
	UNSHORT i;

	/* No errors until now */
	XLD_Last_error = XLD_NO_ERROR;

	/* Is subfile number 0 ? */
	if (!Subfile_nr)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_ZERO, Xftype, Subfile_nr);

		XLD_Last_error = XLD_ILLEGAL_PARAMETERS;

		return NULL;
	}

	/* Try to duplicate */
	Handle = MEM_Duplicate_memory((UNLONG) Subfile_nr,
	 (struct File_type *) Xftype);
	if (Handle)
		return Handle;

	/* Install loading mouse pointer */
	Push_mouse(&(Mouse_pointers[LOADING_MPTR]));

	/* Open library */
	if (!XLD_Open_library(Xftype, Subfile_nr))
	{
		/* Error -> Report */
		XLD_Error(XLDERR_OPEN_LIBRARY_FAILED, Xftype, Subfile_nr);

		goto EXIT_XLD_LOAD_UNIQUE_SUBFILE;
	}

	/* Calculate subfile number within group */
	if (Subfile_nr < 100)
	{
		Subsubfile_nr = Subfile_nr - 1;
	}
	else
	{
		Subsubfile_nr = Subfile_nr % 100;
	}

	/* Is the subfile number too high ? */
	if (Subsubfile_nr >= XLD_Library.Nr_subfiles)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_TOO_HIGH, Xftype, Subfile_nr);

		XLD_Last_error = XLD_ILLEGAL_PARAMETERS;

		goto EXIT_XLD_LOAD_UNIQUE_SUBFILE;
	}

	/* Get subfile length */
	XLD_Subfile_length = XLD_Subfile_lengths[Subsubfile_nr];

	/* Is the subfile length zero ? */
	if (!XLD_Subfile_length)
	{
		/* Yes -> Report error */
		XLD_Error(XLDERR_SUBFILE_LENGTH_ZERO, Xftype, Subfile_nr);

		XLD_Last_error = XLD_FILE_ERROR;

		goto EXIT_XLD_LOAD_UNIQUE_SUBFILE;
	}

	/* Calculate seek offset to subfile */
	XLD_Subfile_offset = 0;
	for (i=0;i<Subsubfile_nr;i++)
	{
		XLD_Subfile_offset += XLD_Subfile_lengths[i];
	}

	/* Load the subfile */
	Handle = XLD_Do_load_subfile(Xftype, Subfile_nr);

EXIT_XLD_LOAD_UNIQUE_SUBFILE:
	/* Close the library */
	/* (The Close function will check if the library was actually opened.) */
	XLD_Close_library();

	/* Remove mouse pointer */
	Pop_mouse();

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_full_batch
 * FUNCTION  : Load a batch of subfiles from a library. ALL files in the
 *              batch must be loaded.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.03.95 14:38
 * LAST      : 15.03.95 14:38
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Number - Number of subfiles in batch.
 *             UNSHORT *Subfile_list - Pointer to list of subfile numbers (1...).
 *             MEM_HANDLE *Handle_list - Pointer to list of memory handles.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * NOTES     : - If the subfile number has the flag XLD_UNIQUE set, a unique
 *              copy will be loaded.
 *             - If the subfile number is zero, handle NULL will be returned.
 *             - The subfile list will remain unchanged.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Load_full_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list)
{
	XLD_Full_batch_mode = TRUE;
	return(XLD_Do_load_batch(Xftype, Number, Subfile_list, Handle_list));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_partial_batch
 * FUNCTION  : Load a batch of subfiles from a library. NOT all files in the
 *              batch must be loaded.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:05
 * LAST      : 15.03.95 14:43
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Number - Number of subfiles in batch.
 *             UNSHORT *Subfile_list - Pointer to list of subfile numbers (1...).
 *             MEM_HANDLE *Handle_list - Pointer to list of memory handles.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * NOTES     : - If the subfile number has the flag XLD_UNIQUE set, a unique
 *              copy will be loaded.
 *             - If the subfile number is zero, handle NULL will be returned.
 *             - The subfile list will remain unchanged.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Load_partial_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list)
{
	XLD_Full_batch_mode = FALSE;
	return(XLD_Do_load_batch(Xftype, Number, Subfile_list, Handle_list));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Do_load_batch
 * FUNCTION  : Load a batch of subfiles from a library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:05
 * LAST      : 27.07.95 16:57
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Number - Number of subfiles in batch.
 *             UNSHORT *Subfile_list - Pointer to list of subfile numbers
 *              (1...).
 *             MEM_HANDLE *Handle_list - Pointer to list of memory handles.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * NOTES     : - If the subfile number has the flag XLD_UNIQUE set, a unique
 *              copy will be loaded.
 *             - If the subfile number is zero, handle NULL will be returned.
 *             - The subfile list will remain unchanged.
 *             - This is the central batch loading function. It uses gotos
 *              to handle errors.
 *             - If an error occurs, all loaded files will be freed.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Do_load_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list)
{
	struct Batch_load_data *Batch_data_buffer;
	MEM_HANDLE Batch_data_buffer_handle;
	MEM_HANDLE Handle;
	BOOLEAN Result = TRUE;
	UNLONG T;
	UNSHORT Subfile_nr;
	UNSHORT Subsubfile_nr;
	SISHORT Remaining_subfiles;
	UNSHORT Old_subfile_nr;
	UNSHORT Current_group_nr = 0;
	UNSHORT Start;
	UNSHORT i, j;

	/* No errors until now */
	XLD_Last_error = XLD_NO_ERROR;

	/* Exit if no subfiles must be loaded (not an error) */
	if (!Number)
		return TRUE;

	/* Only one subfile to be loaded ? */
	if (Number == 1)
	{
		/* Yes -> Get subfile number */
		Subfile_nr = *Subfile_list;

		/* Is zero ? */
		if (!Subfile_nr)
		{
			/* Yes -> Return NULL */
			*Handle_list = NULL;
		}
		else
		{
			/* No -> Unique ? */
			if (Subfile_nr & XLD_UNIQUE)
			{
				/* Yes */
				Subfile_nr &= ~XLD_UNIQUE;
				*Handle_list = XLD_Load_unique_subfile(Xftype, *Subfile_list);
			}
			else
			{
				/* No */
				*Handle_list = XLD_Load_subfile(Xftype, *Subfile_list);
			}
		}

		/* Exit */
		return TRUE;
	}

/* From this point onward, errors may not be handled by returning directly */

	/* Switch to batch mode */
	XLD_Batch_mode = TRUE;

	/* Clear all handles */
	for (i=0;i<Number;i++)
		Handle_list[i] = NULL;

	/* Create sort buffer */
	Batch_data_buffer_handle = MEM_Allocate_memory(Number *
	 sizeof(struct Batch_load_data));

	Batch_data_buffer = (struct Batch_load_data *)
	 MEM_Claim_pointer(Batch_data_buffer_handle);

	/* Fill sort buffer */
	for (i=0;i<Number;i++)
	{
		/* Insert subfile number */
		Batch_data_buffer[i].Subfile_nr = Subfile_list[i];

		/* Insert index for desorting */
		Batch_data_buffer[i].Desort_index = i;
	}

	/* Sort subfiles */
	Shellsort(XLD_Swap, XLD_Compare, (UNLONG) Number,
	 (UNBYTE *) Batch_data_buffer);

	/* Try to re-allocate all subfiles and filter out zeros */
	Remaining_subfiles = Number;
	for (i=0;i<Number;i++)
	{
		/* Clear handle */
		Handle = NULL;

		/* Get subfile number */
		Subfile_nr = Batch_data_buffer[i].Subfile_nr;

		/* Zero ? */
		if (Subfile_nr)
		{
			/* No -> Unique ? */
			if (Subfile_nr & XLD_UNIQUE)
			{
				/* Yes -> Try to duplicate */
				Subfile_nr &= ~XLD_UNIQUE;
				if (Subfile_nr)
				{
					Handle = MEM_Duplicate_memory((UNLONG) Subfile_nr,
					 (struct File_type *) Xftype);
				}
			}
			else
			{
				/* No -> Try to re-allocate */
				Handle = MEM_Reallocate_memory((UNLONG) Subfile_nr,
				 (struct File_type *) Xftype);
			}
		}
		else
		{
			/* Yes -> In full batch mode ? */
			if (XLD_Full_batch_mode)
			{
				/* Yes -> Report error */
				XLD_Error(XLDERR_SUBFILE_ZERO, Xftype, 0);

				XLD_Last_error = XLD_FILE_ERROR;

				/* Exit */
				Result = FALSE;

				goto EXIT_XLD_DO_LOAD_BATCH;
			}
		}

		/* Subfile number zero or succesful re-allocation ? */
		if (!Subfile_nr || Handle)
		{
			/* Yes -> Count down */
			Remaining_subfiles--;

			/* Desort and insert handle */
			Handle_list[Batch_data_buffer[i].Desort_index] = Handle;

			/* Mark */
			Batch_data_buffer[i].Desort_index = 0xFFFF;
		}
	}

	/* Any subfiles left ? */
	if (!Remaining_subfiles)
	{
		/* No -> Exit */
		Result = TRUE;

		goto EXIT_XLD_DO_LOAD_BATCH;
	}

/* From this point onward, errors may not be handled by jumping to
 EXIT_XLD_DO_LOAD_BATCH */

	/* Install loading mouse pointer */
	Push_mouse(&(Mouse_pointers[LOADING_MPTR]));

	/* Determine initial group number */
	for (i=0;i<Number;i++)
	{
		/* Is this subfile marked ? */
		if (Batch_data_buffer[i].Desort_index != 0xFFFF)
		{
			/* No -> Get subfile number */
			Subfile_nr = Batch_data_buffer[i].Subfile_nr;

			Subfile_nr &= ~XLD_UNIQUE;

			/* Get group number */
			Current_group_nr = Subfile_nr / 100;

			break;
		}
	}

	/* Load all subfiles */
	Start = 0;
	while (Remaining_subfiles > 0)
	{
		/* Open library */
		if (!XLD_Open_library(Xftype, Current_group_nr * 100))
		{
			/* Error -> Report */
			XLD_Error(XLDERR_OPEN_LIBRARY_FAILED, Xftype, 0);

			/* Exit */
			Result = FALSE;

			break;
		}

		/* Clear */
		XLD_Batch_offset = 0;
		XLD_Subfile_length = 0;

		/* Load all subfiles */
		Handle = NULL;
		Old_subfile_nr = 0;

		for (i=Start;i<Number;i++)
		{
			/* Is this subfile marked ? */
			if (Batch_data_buffer[i].Desort_index == 0xFFFF)
			{
				/* Yes -> Skip */
				continue;
			}

			/* Get subfile number */
			Subfile_nr = Batch_data_buffer[i].Subfile_nr;

			/* Unique ? */
			if (Subfile_nr & XLD_UNIQUE)
			{
				/* Yes */
				Subfile_nr &= ~XLD_UNIQUE;
			}
			else
			{
				/* No -> Same subfile as previous ? */
				if (Subfile_nr == Old_subfile_nr)
				{
					/* Yes -> Desort and insert handle */
					Handle_list[Batch_data_buffer[i].Desort_index] = Handle;

					/* Count down */
					Remaining_subfiles--;

					continue;
				}
			}

			/* Is this subfile in the next group ? */
			if ((Subfile_nr / 100) != Current_group_nr)
			{
				/* Yes -> Get new group number */
				Current_group_nr = Subfile_nr / 100;

				/* Make sure the next "batch" starts with the current subfile */
				Start = i;

				break;
			}

			/* Calculate subfile number within group */
			if (Subfile_nr < 100)
			{
				Subsubfile_nr = Subfile_nr - 1;
			}
			else
			{
				Subsubfile_nr = Subfile_nr % 100;
			}

			/* Subfile number too high ? */
			if (Subsubfile_nr >= XLD_Library.Nr_subfiles)
			{
				/* Yes -> Full batch mode ? */
				if (XLD_Full_batch_mode)
				{
					/* Yes -> Report error */
					XLD_Error(XLDERR_SUBFILE_TOO_HIGH, Xftype, Subfile_nr);

					XLD_Last_error = XLD_ILLEGAL_PARAMETERS;

					/* Break off */
					Result = FALSE;
					break;
				}
				else
				{
					/* No -> Count down */
					Remaining_subfiles--;
					continue;
				}
			}

			/* Save subfile number */
			Old_subfile_nr = Subfile_nr;

			/* Calculate seek offset to subfile */
			XLD_Subfile_offset = 0;
			for (j=0;j<Subsubfile_nr;j++)
			{
				XLD_Subfile_offset += XLD_Subfile_lengths[j];
			}

			/* Relative to the previous subfile */
			T = XLD_Subfile_offset;
			XLD_Subfile_offset -= (XLD_Batch_offset + XLD_Subfile_length);
			XLD_Batch_offset = T;

			/* Get subfile length */
			XLD_Subfile_length = XLD_Subfile_lengths[Subsubfile_nr];

			/* Load the subfile */
			Handle = XLD_Do_load_subfile(Xftype, Subfile_nr);

			/* Succesful ? */
			if (!Handle)
			{
				/* No -> Break off */
				Result = FALSE;
				break;
			}

			/* Desort and insert handle */
			Handle_list[Batch_data_buffer[i].Desort_index] = Handle;

			/* Count down */
			Remaining_subfiles--;
		}

		/* Close library */
		XLD_Close_library();

		/* Exit if an error occurred */
		if (!Result)
		{
			break;
		}
	}

	/* Remove mouse pointer */
	Pop_mouse();

EXIT_XLD_DO_LOAD_BATCH:

	/* Batch mode off */
	XLD_Batch_mode = FALSE;

	/* Destroy sort buffer */
	MEM_Free_pointer(Batch_data_buffer_handle);
	MEM_Free_memory(Batch_data_buffer_handle);

	/* Has an error occurred ? */
	if (!Result)
	{
		/* Yes -> Free all sub-files loaded to this point */
		for (i=0;i<Number;i++)
		{
			if (Handle_list[i])
				MEM_Free_memory(Handle_list[i]);
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Swap
 * FUNCTION  : Swap two subfiles and indices (for sorting).
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:25
 * LAST      : 18.05.95 12:13
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XLD_Swap(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Batch_load_data *Ptr;
	struct Batch_load_data T;

	Ptr = (struct Batch_load_data *) Data;

	memcpy((UNBYTE *) &T, (UNBYTE *) (Ptr + A),
	 sizeof(struct Batch_load_data));
	memcpy((UNBYTE *) (Ptr + A), (UNBYTE *) (Ptr + B),
	 sizeof(struct Batch_load_data));
	memcpy((UNBYTE *) (Ptr + B), (UNBYTE *) &T,
	 sizeof(struct Batch_load_data));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Compare
 * FUNCTION  : Compare two subfiles (for sorting).
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:25
 * LAST      : 18.05.95 12:13
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 *	RESULT    : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Compare(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Batch_load_data *Ptr;

	Ptr = (struct Batch_load_data *) Data;

	return(Ptr[A].Subfile_nr > Ptr[B].Subfile_nr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Do_load_subfile
 * FUNCTION  : Select and execute a subfile loader.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:46
 * LAST      : 27.07.95 16:57
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * NOTES     : - By this point, the library has been opened and various
 *              global variables have been set.
 *             - This function will set XLD_Last_error when an error occurs.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
XLD_Do_load_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr)
{
	MEM_HANDLE Handle = NULL;

	switch (XLD_Library.Library_type)
	{
		case XLD_NORMAL:
		{
			Handle = XLD_Load_unpacked_subfile(Xftype,Subfile_nr);
			break;
		}
		case XLD_PACKED:
		{
//			Handle = XLD_Load_packed_subfile(Xftype,Subfile_nr);
			break;
		}
		case XLD_ENCRYPTED:
		{
//			Handle = XLD_Load_encrypted_subfile(Xftype,Subfile_nr);
			break;
		}
		case XLD_PACKED_ENCRYPTED:
		{
//			Handle = XLD_Load_packed_encrypted_subfile(Xftype,Subfile_nr);
			break;
		}
		default:
		{
			XLD_Error(XLDERR_ILLEGAL_LIBRARY_TYPE, Xftype, Subfile_nr);

			XLD_Last_error = XLD_ILLEGAL_PARAMETERS;

			break;
		}
	}

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_unpacked_subfile
 * FUNCTION  : Load an unpacked subfile.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:58
 * LAST      : 27.07.95 17:01
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * NOTES     : - By this point, the library has been opened and various
 *              global variables have been set.
 *             - Seek pointer must be on the start of the first subfile.
 *             - This function will set XLD_Last_error when an error occurs.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
XLD_Load_unpacked_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr)
{
	MEM_HANDLE Handle;
	SILONG Read;
	UNBYTE *Pointer;

	/* Allocate memory for the file */
	Handle = MEM_Do_allocate((UNLONG) XLD_Subfile_length, (UNLONG) Subfile_nr,
	 (struct File_type *) Xftype);
	if (!Handle)
	{
		/* Error -> Report */
		XLD_Error(XLDERR_NO_MEMORY_FOR_FILE, Xftype, Subfile_nr);

		XLD_Last_error = XLD_OUT_OF_MEMORY;

		return NULL;
	}

	/* Is the length zero ? */
	if (!XLD_Subfile_length)
	{
		/* Yes -> Are we in batch mode ? */
		if (XLD_Batch_mode)
		{
			/* Yes -> Seek anyway! */
			DOS_Seek(XLD_Library_ID, BBDOS_SEEK_CURRENTPOS,
			 (SILONG) XLD_Subfile_offset);
		}

		/* Clear the file */
		MEM_Clear_memory(Handle);

		return Handle;
	}

	/* Seek to the start of the subfile */
	DOS_Seek(XLD_Library_ID, BBDOS_SEEK_CURRENTPOS,
	 (SILONG) XLD_Subfile_offset);

	/* Read subfile into memory */
	Pointer = MEM_Claim_pointer(Handle);
	Read = DOS_Read(XLD_Library_ID, Pointer, (UNLONG) XLD_Subfile_length);

	/* Succesful ? */
	if (Read != XLD_Subfile_length)
	{
		/* No -> Free memory */
		MEM_Free_pointer(Handle);
		MEM_Free_memory(Handle);

		/* Report error */
		XLD_Error(XLDERR_READ_FAILED, Xftype, Subfile_nr);

		Handle = NULL;
	}
	else
	{
		/* Yes -> Call convertor function (if any) */
		if (Xftype->Converter)
			(Xftype->Converter)(Pointer, Handle);

		MEM_Free_pointer(Handle);
	}

	return Handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Open_library
 * FUNCTION  : Open an XLD library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:28
 * LAST      : 27.07.95 16:39
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...MAX_SUBFILES).
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function assumes that the filename of an XLD-library
 *              ends with ".XLD" and has at least one character before that.
 *              This character will be changed depending on the subfile
 *              number.
 *             - This function will set XLD_Last_error when an error occurs.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Open_library(struct XFile_type *Xftype, UNSHORT Subfile_nr)
{
	SILONG Read;
	UNCHAR *Number = "0";

	/* Already open ? */
	if (XLD_Library_open)
	{
		/* Yes -> Exit */
		return FALSE;
	}

	/* Select subfile group */
	sprintf(Number, "%u", Subfile_nr / 100);
	*(Xftype->Filename + strlen(Xftype->Filename) - 5) = *Number;

	/* Does this file exist ? */
	if (!File_exists(Xftype->Filename))
	{
		/* No -> Report error */
		XLD_Error(XLDERR_FILE_DOES_NOT_EXIST, Xftype, 0);

		XLD_Last_error = XLD_FILE_NOT_FOUND;

		return FALSE;
	}

	/* Open the library file */
	XLD_Library_ID = DOS_Open(Xftype->Filename, BBDOSFILESTAT_READWRITE);
	if (XLD_Library_ID < 0)
	{
		XLD_Error(XLDERR_OPEN_FAILED, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		return FALSE;
	}

	/* Read header into memory */
	Read = DOS_Read(XLD_Library_ID, (UNBYTE *) &XLD_Library,
	 sizeof(struct XLD_Library));
	if (Read != sizeof(struct XLD_Library))
	{
		DOS_Close(XLD_Library_ID);

		XLD_Error(XLDERR_LIBRARY_CORRUPT, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		return FALSE;
	}

	/* Check library ID */
	if (strncmp(Library_ID, &(XLD_Library.ID[0]), 4))
	{
		DOS_Close(XLD_Library_ID);

		XLD_Error(XLDERR_NOT_A_LIBRARY, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		return FALSE;
  	}

	/* Read subfile lengths */
	Read = DOS_Read(XLD_Library_ID, (UNBYTE *) &XLD_Subfile_lengths[0],
	 4 * XLD_Library.Nr_subfiles);
	if (Read != 4 * XLD_Library.Nr_subfiles)
	{
		DOS_Close(XLD_Library_ID);

		XLD_Error(XLDERR_LIBRARY_CORRUPT, Xftype, 0);

		XLD_Last_error = XLD_FILE_ERROR;

		return FALSE;
	}

	XLD_Library_open = TRUE;

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Close_library
 * FUNCTION  : Close the currently opened XLD library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:34
 * LAST      : 22.07.94 14:34
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function can be safely called if no XLD library has
 *              been opened.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XLD_Close_library(void)
{
	/* Is a library open ? */
	if (XLD_Library_open)
	{
		/* Yes -> Close the file */
		DOS_Close(XLD_Library_ID);
		XLD_Library_open = FALSE;
	}
}

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

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Error
 * FUNCTION  : Report an XLOAD error.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.94 14:17
 * LAST      : 14.07.95 12:55
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XLD_Error(UNSHORT Error_code, struct XFile_type *Xftype, UNSHORT Subfile_nr)
{
	struct XLD_error Error;

	/* Initialize XLOAD error structure */
	Error.Code			= Error_code;
	Error.Xftype		= Xftype;
	Error.Subfile_nr	= Subfile_nr;

	/* Push error on the error stack */
	ERROR_PushError(XLD_Print_error, XLD_Library_name, sizeof(struct XLD_error),
	 (UNBYTE *) &Error);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Print_error
 * FUNCTION  : Print an XLOAD error.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.94 14:21
 * LAST      : 27.07.95 17:23
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by XLD_Error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : XLOAD.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XLD_Print_error(UNCHAR *buffer, UNBYTE *data)
{
	struct XLD_error *Error;
	UNSHORT Error_code;

	Error = (struct XLD_error *) data;

	/* Get error code */
	Error_code = Error->Code;

	/* Catch illegal errors */
	if (Error_code > XLDERR_MAX)
		Error_code = 0;

 	/* Print error */
	if (Error->Subfile_nr)
	{
		sprintf((char *)buffer,"%s\n     (Name : %s, Subfile number : %u)",
		 XLD_Error_strings[Error_code], Error->Xftype->Ftype.Name,
		 Error->Subfile_nr);
	}
	else
	{
		sprintf((char *)buffer,"%s\n     (Name : %s)",
		 XLD_Error_strings[Error_code], Error->Xftype->Ftype.Name);
	}
}

