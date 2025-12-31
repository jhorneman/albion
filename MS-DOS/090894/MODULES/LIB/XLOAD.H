/************
 * NAME     : XLOAD.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-7-1994
 * PROJECT  : eXtended LOADing system
 * NOTES    :
 * SEE ALSO : XLOAD.C
 ************/

#ifndef XLOAD_H
#define XLOAD_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** Defines ****************************************************************
 */

#define SUBFILES_MAX				(600)
#define LIBRARY_ID				((UNLONG) (('X'<<24)|('L'<<16)|('D'<<8)|('0')))

/* Batch subfile number flag */
#define XLD_UNIQUE				(1<<15)

/* XLD library types */
#define XLD_NORMAL				(0)
#define XLD_PACKED				(1)
#define XLD_ENCRYPTED			(2)
#define XLD_PACKED_ENCRYPTED	(3)

/* XLD error codes */
#define XLDERR_GET_FILE_LENGTH_FAILED	(0)
#define XLDERR_NO_MEMORY_FOR_FILE		(1)
#define XLDERR_READ_FAILED					(2)
#define XLDERR_SUBFILE_ZERO				(3)
#define XLDERR_SUBFILE_TOO_HIGH			(4)
#define XLDERR_ILLEGAL_LIBRARY_TYPE		(5)
#define XLDERR_LIBRARY_CORRUPT			(6)
#define XLDERR_NOT_A_LIBRARY				(7)
#define XLDERR_OPEN_FAILED					(8)
#define XLDERR_OPEN_LIBRARY_FAILED		(9)

#define XLDERR_MAX							(9)

/*
 ** Structure definitions **************************************************
 */

struct XFile_type {
	struct File_type Ftype;		/* File type for BBMEM */
	UNCHAR *Filename;				/* Pointer to filename */
	void (*Convertor)(UNBYTE *, MEM_HANDLE);	/* Pointer to convertor function */
};

struct XLD_Library {
	UNCHAR ID[4];					/* Library ID ('XLD0') */
	UNCHAR Format;					/* 'I' for Intel, 'M' for Motorola */
	UNBYTE Library_type;			/* See above */
	UNSHORT Nr_subfiles;			/* Number of subfiles in this library */
};
/* This library-header will be followed by UNLONGs containing the lengths
 of the subfiles. */

struct XLD_error {
	UNSHORT Code;
	struct XFile_type *Xftype;
	UNSHORT Subfile_nr;
};

/*
 ** Prototypes *************************************************************
 */

void XLD_Error(UNSHORT Error_code, struct XFile_type *Xftype,
 UNSHORT Subfile_nr);
void XLD_Print_error(UNCHAR *buffer, UNBYTE *data);

MEM_HANDLE XLD_Load_file(struct XFile_type *Xftype);
MEM_HANDLE XLD_Load_standard_file(UNCHAR *Filename);

MEM_HANDLE XLD_Load_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr);
MEM_HANDLE XLD_Load_unique_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr);

void XLD_Load_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);
void XLD_Load_unique_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);

void XLD_Swap(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN XLD_Compare(UNLONG A, UNLONG B, UNBYTE *Data);

MEM_HANDLE XLD_Do_load_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr);
MEM_HANDLE XLD_Load_unpacked_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr);

BOOLEAN XLD_Open_library(struct XFile_type *Xftype);
void XLD_Close_library(void);

#endif

