/************
 * NAME     : XLOAD.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-7-1994
 * PROJECT  : eXtended LOADing system
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : XLOAD.H
 ************/

/* includes */

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBDOS.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBERROR.H>

#include <XLOAD.H>
#include <SORT.H>

/* global variables */

BOOLEAN XLD_Library_open = FALSE;
BOOLEAN XLD_Batch_mode = FALSE;
SISHORT XLD_Library_ID;
struct XLD_Library XLD_Library;
UNLONG XLD_Subfile_lengths[SUBFILES_MAX];
UNLONG XLD_Subfile_length;
UNLONG XLD_Subfile_offset;
UNLONG XLD_Batch_offset;

UNCHAR XLD_Library_name[] = "Xload";

UNCHAR *XLD_Error_strings[] = {
	"Illegal error code.",
	"File length could not be determined.",
	"No memory could be allocated.",
	"Read failed.",
	"Subfile number is zero.",
	"Subfile number is too high.",
	"Library has an illegal type.",
	"Library header is corrupt.",
	"This is not a library.",
	"Open failed.",
	"Library could not be opened."
};

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Error
 * FUNCTION  : Report an XLOAD error.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.94 14:17
 * LAST      : 08.08.94 14:17
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

	Error.Code = Error_code;			/* Initialize XLOAD error structure */
	Error.Xftype = Xftype;
	Error.Subfile_nr = Subfile_nr;

	/* Push error on the error stack */
	ERROR_PushError(XLD_Print_error,XLD_Library_name,sizeof(struct XLD_error),(UNBYTE *) &Error);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Print_error
 * FUNCTION  : Print an XLOAD error.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.94 14:21
 * LAST      : 08.08.94 14:21
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
	UNSHORT i;

	Error = (struct XLD_error *) data;

	/* Get error code */
	i = Error->Code;

	/* Catch illegal errors */
	if (i>XLDERR_MAX)
		i = 0;

 	/* Print error */
	sprintf((char *)buffer,"%s Filename : %s, Subfile number : %u.",
	 XLD_Error_strings[i], Error->Xftype->Filename, Error->Subfile_nr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_file
 * FUNCTION  : Load a file.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 13:30
 * LAST      : 22.07.94 13:30
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
	SILONG Length, Read;
	SISHORT ID;
	MEM_HANDLE Handle;
	UNBYTE *Pointer;

	/* Get file length */
	Length = DOS_GetFileLength(Xftype->Filename);
	if (Length < 0)
	{
		XLD_Error(XLDERR_GET_FILE_LENGTH_FAILED, Xftype, 0);
		return(NULL);
	}

	/* Allocate memory for file */
	Handle = MEM_Do_allocate((UNLONG) Length, 0L, (struct File_type *) Xftype);
	if (!Handle)
	{
		XLD_Error(XLDERR_NO_MEMORY_FOR_FILE, Xftype, 0);
		return(NULL);
	}

	/* Open the file */
 	ID = DOS_Open(Xftype->Filename,BBDOSFILESTAT_READ);
	/* Succesful ? */
	if (ID>=0)
	{
		/* Yes -> Read file into memory */
		Pointer = MEM_Claim_pointer(Handle);
		Read = DOS_Read(ID, Pointer, (UNLONG) Length);

		/* Close the file */
		DOS_Close(ID);

		/* Succesful ? */
		if (Read != Length)
		{
			/* No -> Free memory */
			MEM_Free_pointer(Handle);
			MEM_Free_memory(Handle);
			Handle = NULL;
			XLD_Error(XLDERR_READ_FAILED, Xftype, 0);
		}
		else
		{
			/* Yes -> Call convertor function (if any) */
			if (Xftype->Convertor)
				(Xftype->Convertor)(Pointer, Handle);
			MEM_Free_pointer(Handle);
		}
	}

	return(Handle);
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
	memcpy(&Standard,&MEM_Default_file_type,sizeof(struct File_type));
	Standard.Filename = Filename;

	/* Load it */
	return(XLD_Load_file(&Standard));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_subfile
 * FUNCTION  : Load a subfile from a library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:01
 * LAST      : 22.07.94 14:01
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
	UNSHORT i;
	MEM_HANDLE Handle;

	/* Exit if subfile number is 0 */
	if (!Subfile_nr)
	{
		XLD_Error(XLDERR_SUBFILE_ZERO, Xftype, Subfile_nr);
		return(NULL);
	}

	/* Try to re-allocate */
	Handle = MEM_Reallocate_memory((UNLONG) Subfile_nr,
	 (struct File_type *) Xftype);
	if (Handle)
		return(Handle);

	/* Open library */
	if (!XLD_Open_library(Xftype))
	{
		XLD_Error(XLDERR_OPEN_LIBRARY_FAILED, Xftype, Subfile_nr);
		return(NULL);
	}

	/* Exit if subfile number is too high */
	if (Subfile_nr > XLD_Library.Nr_subfiles)
	{
		XLD_Close_library();
		XLD_Error(XLDERR_SUBFILE_TOO_HIGH, Xftype, Subfile_nr);
		return(NULL);
	}

	/* Get subfile length */
	XLD_Subfile_length = XLD_Subfile_lengths[Subfile_nr-1];

	/* Calculate seek offset to subfile */
	XLD_Subfile_offset = 0;
	for (i=0;i<Subfile_nr-1;i++)
		XLD_Subfile_offset += XLD_Subfile_lengths[i];

	/* Load the subfile */
	Handle = XLD_Do_load_subfile(Xftype, Subfile_nr);
	XLD_Close_library();

	return(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_unique_subfile
 * FUNCTION  : Load a unique subfile from a library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:03
 * LAST      : 23.07.94 16:03
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
	UNSHORT i;
	MEM_HANDLE Handle;

	/* Exit if subfile number is 0 */
	if (!Subfile_nr)
	{
		XLD_Error(XLDERR_SUBFILE_ZERO, Xftype, Subfile_nr);
		return(NULL);
	}

	/* Try to duplicate */
	Handle = MEM_Duplicate_memory((UNLONG) Subfile_nr,
	 (struct File_type *) Xftype);
	if (Handle)
		return(Handle);

	/* Open library */
	if (!XLD_Open_library(Xftype))
	{
		XLD_Error(XLDERR_OPEN_LIBRARY_FAILED, Xftype, Subfile_nr);
		return(NULL);
	}

	/* Exit if subfile number is too high */
	if (Subfile_nr > XLD_Library.Nr_subfiles)
	{
		XLD_Close_library();
		XLD_Error(XLDERR_SUBFILE_TOO_HIGH, Xftype, Subfile_nr);
		return(NULL);
	}

	/* Get subfile length */
	XLD_Subfile_length = XLD_Subfile_lengths[Subfile_nr-1];

	/* Calculate seek offset to subfile */
	XLD_Subfile_offset = 0;
	for (i=0;i<Subfile_nr-1;i++)
		XLD_Subfile_offset += XLD_Subfile_lengths[i];

	/* Load the subfile */
	Handle = XLD_Do_load_subfile(Xftype, Subfile_nr);
	XLD_Close_library();

	return(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_batch
 * FUNCTION  : Load a batch of subfiles from a library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:05
 * LAST      : 23.07.94 16:05
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Number - Number of subfiles in batch.
 *             UNSHORT *Subfile_list - Pointer to list of subfile numbers (1...).
 *             MEM_HANDLE *Handle_list - Pointer to list of memory handles.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the subfile number has the flag XLD_UNIQUE set, a unique
 *              copy will be loaded.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XLD_Load_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list)
{
	MEM_HANDLE Buffer_handle;
	UNSHORT *Buffer, Subfile_nr;

	/* Exit if no subfiles must be loaded (not an error) */
	if (!Number)
		return;

	/* Only one subfile to be loaded ? */
	if (Number == 1)
	{
		/* Yes -> Get subfile number */
		Subfile_nr = *Subfile_list;
		/* Unique ? */
		if (Subfile_nr & XLD_UNIQUE)
		{
			/* Yes */
			Subfile_nr &= ~XLD_UNIQUE;
			*Handle_list = XLD_Load_subfile(Xftype, *Subfile_list);
		}
		else
		/* No */
			*Handle_list = XLD_Load_subfile(Xftype, *Subfile_list);

		/* Exit */
		return;
	}

	/* Switch to batch mode */
	XLD_Batch_mode = TRUE;

	/* Clear all handles */
	{
		UNSHORT i;
		MEM_HANDLE *Ptr;

		Ptr = Handle_list;
		for (i=0;i<Number;i++)
			*Ptr++ = NULL;
	}

	/* Create sort buffer */
	Buffer_handle = MEM_Allocate_memory(Number * 4);

	Buffer = (UNSHORT *) MEM_Claim_pointer(Buffer_handle);

	/* Fill sort buffer */
	{
		UNSHORT i, *Ptr1, *Ptr2;

		Ptr1 = Buffer;
		Ptr2 = Subfile_list;

		for (i=0;i<Number;i++)
		{
			/* Insert subfile number */
			*Ptr1++ = *Ptr2++;

			/* Insert index for desorting */
			*Ptr1++ = i;
		}
	}

	/* Sort subfiles */
	Shellsort(XLD_Swap, XLD_Compare, (UNLONG) Number,
	 (UNBYTE *) Buffer);

	/* Try to re-allocate all subfiles and filter out zeros */
	{
		UNSHORT i,j,x;
		UNSHORT *Ptr;
		MEM_HANDLE h;

		j = Number;
		Ptr = Buffer;
		for (i=0;i<Number;i++)
		{
			h = NULL;

			/* Get subfile number */
			x = *Ptr;

			/* Zero ? */
			if (x)
			{
				/* No -> Unique ? */
				if (x & XLD_UNIQUE)
				{
					/* Yes -> Try to duplicate */
					x &= ~XLD_UNIQUE;
					if (x)
						h = MEM_Duplicate_memory((UNLONG) x, (struct File_type *) Xftype);
				}
				else
					/* No -> Try to re-allocate */
					h = MEM_Reallocate_memory((UNLONG) x, (struct File_type *) Xftype);

				/* Succesful ? */
				if (h)
					/* Yes -> Desort and insert handle */
					Handle_list[*(Ptr+1)] = h;
			}

			/* Subfile number zero or succesful re-allocation ? */
			if (!x || h)
			{
				/* Yes -> Count down */
				j--;
				/* Mark */
				*(Ptr+1) = 0xFFFF;
			}

			/* Next subfile */
			Ptr += 2;
		}

		/* Any subfiles left ? */
		if (!j)
			return;
	}

	/* Open library */
	if (!XLD_Open_library(Xftype))
	{
		XLD_Error(XLDERR_OPEN_LIBRARY_FAILED, Xftype, 0);
		return;
	}

	/* Clear */
	XLD_Batch_offset = 0;
	XLD_Subfile_length = 0;

	/* Load all subfiles */
	{
		UNSHORT i, j, Old_subfile_nr, *Ptr;
		UNLONG t;
		MEM_HANDLE h, *Ptr2;

		Old_subfile_nr = 0;
		Ptr = Buffer;
		Ptr2 = Handle_list;

		for (i=0;i<Number;i++)
		{
			/* Is this subfile marked ? */
			if (*(Ptr+1) == 0xFFFF)
			{
				/* Yes -> Skip */
				Ptr += 2;
				continue;
			}

			/* Get subfile number */
			Subfile_nr = *Ptr;

			/* Unique ? */
			if (Subfile_nr & XLD_UNIQUE)
				/* Yes */
				Subfile_nr &= ~XLD_UNIQUE;
			else
			{
				/* No -> Same subfile as previous ? */
				if (Subfile_nr == Old_subfile_nr)
				{
					/* Yes -> Desort and insert handle */
					Handle_list[*(Ptr+1)] = h;
					/* Next subfile */
					Ptr+=2;
					continue;
				}
			}

			/* Skip if subfile number is too high */
			if (Subfile_nr > XLD_Library.Nr_subfiles)
				continue;

			/* Save subfile number */
			Old_subfile_nr = Subfile_nr;

			/* Calculate seek offset to subfile */
			XLD_Subfile_offset = 0;
			for (j=0;j<Subfile_nr-1;j++)
				XLD_Subfile_offset += XLD_Subfile_lengths[i];

			/* Relative to the previous subfile */
			t = XLD_Subfile_offset;
			XLD_Subfile_offset -= (XLD_Batch_offset + XLD_Subfile_length);
			XLD_Batch_offset = t;

			/* Get subfile length */
			XLD_Subfile_length = XLD_Subfile_lengths[Subfile_nr-1];

			/* Load the subfile */
			h = XLD_Do_load_subfile(Xftype, Subfile_nr);

			/* Desort and insert handle */
			Handle_list[*(Ptr+1)] = h;

			/* Next subfile */
			Ptr+=2;
		}
	}
	/* Close library */
	XLD_Close_library();

	/* Batch mode off */
	XLD_Batch_mode = FALSE;

	/* Destroy sort buffer */
	MEM_Free_pointer(Buffer_handle);
	MEM_Free_memory(Buffer_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Swap
 * FUNCTION  : Swap two subfiles and indices (for sorting).
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:25
 * LAST      : 23.07.94 16:25
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Although Data points to an UNSHORT array, this function works
 *              with UNLONGs because the array's second dimension is 2.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XLD_Swap(UNLONG A, UNLONG B, UNBYTE *Data)
{
	UNLONG T, *P;

	P = (UNLONG *) Data;

	T = P[A];
	P[A] = P[B];
	P[B] = T;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Compare
 * FUNCTION  : Compare two subfiles (for sorting).
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.07.94 16:25
 * LAST      : 23.07.94 16:25
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
	UNSHORT *P;

	P = (UNSHORT *) Data;

	return(P[A+A] > P[B+B]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Do_load_subfile
 * FUNCTION  : Select and execute a subfile loader.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:46
 * LAST      : 22.07.94 14:46
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * NOTES     : - By this point, the library has been opened and various
 *              global variables have been set.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
XLD_Do_load_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr)
{
	MEM_HANDLE Handle;

	switch (XLD_Library.Library_type)
	{
		case XLD_NORMAL:
			Handle = XLD_Load_unpacked_subfile(Xftype,Subfile_nr);
			break;
		case XLD_PACKED:
//			Handle = XLD_Load_packed_subfile(Xftype,Subfile_nr);
			break;
		case XLD_ENCRYPTED:
//			Handle = XLD_Load_encrypted_subfile(Xftype,Subfile_nr);
			break;
		case XLD_PACKED_ENCRYPTED:
//			Handle = XLD_Load_packed_encrypted_subfile(Xftype,Subfile_nr);
			break;
		default:
			XLD_Error(XLDERR_ILLEGAL_LIBRARY_TYPE, Xftype, Subfile_nr);
			Handle = NULL;
	}

	return(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Load_unpacked_subfile
 * FUNCTION  : Load an unpacked subfile.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:58
 * LAST      : 22.07.94 14:58
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 *             UNSHORT Subfile_nr - Subfile number (1...).
 * RESULT    : MEM_HANDLE : Memory handle of file / NULL = error.
 * BUGS      : No known.
 * NOTES     : - By this point, the library has been opened and various
 *              global variables have been set.
 *             - Seek pointer must be on the start of the first subfile.
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
		XLD_Error(XLDERR_NO_MEMORY_FOR_FILE, Xftype, Subfile_nr);
		return(NULL);
	}

	/* Is the length zero ? */
	if (!XLD_Subfile_length)
	{
		/* Yes -> Are we in batch mode ? */
		if (XLD_Batch_mode)
			/* Yes -> Seek anyway! */
			DOS_Seek(XLD_Library_ID, BBDOS_SEEK_CURRENTPOS,
			 (SILONG) XLD_Subfile_offset);

		/* Clear the file */
		MEM_Clear_memory(Handle);

		return(Handle);
	}

	/* Seek to the start of the subfile */
	DOS_Seek(XLD_Library_ID, BBDOS_SEEK_CURRENTPOS, (SILONG) XLD_Subfile_offset);

	/* Read subfile into memory */
	Pointer = MEM_Claim_pointer(Handle);
	Read = DOS_Read(XLD_Library_ID, Pointer, (UNLONG) XLD_Subfile_length);

	/* Succesful ? */
	if (Read != XLD_Subfile_length)
	{
		/* No -> Free memory */
		MEM_Free_pointer(Handle);
		MEM_Free_memory(Handle);
		XLD_Error(XLDERR_READ_FAILED, Xftype, 0);
		Handle = NULL;
	}
	else
	{
		/* Yes -> Call convertor function (if any) */
		if (Xftype->Convertor)
			(Xftype->Convertor)(Pointer, Handle);
		MEM_Free_pointer(Handle);
	}

	return(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XLD_Open_library
 * FUNCTION  : Open an XLD library.
 * FILE      : XLOAD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 14:28
 * LAST      : 22.07.94 14:28
 * INPUTS    : struct XFile_type *Xftype - Pointer to XLD file type.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  : XLOAD.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
XLD_Open_library(struct XFile_type *Xftype)
{
	SILONG Read;

	/* Already open ? */
	if (XLD_Library_open)
		return(FALSE);

	/* No */
	XLD_Library_open = FALSE;

	/* Open the library file */
 	XLD_Library_ID = DOS_Open(Xftype->Filename,BBDOSFILESTAT_READ);
	if (XLD_Library_ID < 0)
	{
		XLD_Error(XLDERR_OPEN_FAILED, Xftype, 0);
		return(FALSE);
	}

	/* Read header into memory */
	Read = DOS_Read(XLD_Library_ID, (UNBYTE *) &XLD_Library,
	 sizeof(struct XLD_Library));
	if (Read != sizeof(struct XLD_Library))
	{
		DOS_Close(XLD_Library_ID);
		XLD_Error(XLDERR_LIBRARY_CORRUPT, Xftype, 0);
		return(FALSE);
	}

	/* Check library ID */
	if (*((UNLONG *) &XLD_Library.ID) != LIBRARY_ID)
	{
		DOS_Close(XLD_Library_ID);
		XLD_Error(XLDERR_NOT_A_LIBRARY, Xftype, 0);
		return(FALSE);
  	}

	/* Read subfile lengths */
	Read = DOS_Read(XLD_Library_ID, (UNBYTE *) &XLD_Subfile_lengths[0],
	 4 * XLD_Library.Nr_subfiles);
	if (Read != 4 * XLD_Library.Nr_subfiles)
	{
		DOS_Close(XLD_Library_ID);
		XLD_Error(XLDERR_LIBRARY_CORRUPT, Xftype, 0);
		return(FALSE);
	}

	XLD_Library_open = TRUE;

	return(TRUE);
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

