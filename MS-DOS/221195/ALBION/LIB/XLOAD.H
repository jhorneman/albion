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

/* Batch subfile number flag */
#define XLD_UNIQUE				(1 << 15)

/*
 ** Enumerations ***********************************************************
 */

/* XLD error types */
enum eXLD_error {
	XLD_NO_ERROR,
	XLD_FILE_ERROR,
	XLD_FILE_NOT_FOUND,
	XLD_OUT_OF_MEMORY,
	XLD_ILLEGAL_PARAMETERS
};

/*
 ** Type definitions *******************************************************
 */

typedef enum eXLD_error XLD_ERROR_T;

typedef void (*File_converter) (UNBYTE *Ptr, MEM_HANDLE Handle);

/*
 ** Structure definitions **************************************************
 */

struct XFile_type {
	struct File_type Ftype;		/* File type for BBMEM */
	UNCHAR *Filename;				/* Pointer to filename */
	File_converter Converter;	/* Pointer to converter function */
	UNCHAR Flags;					/* Flags (not used by XLOAD) */
};

/*
 ** Global variables *******************************************************
 */

extern XLD_ERROR_T XLD_Last_error;

/*
 ** Prototypes *************************************************************
 */

extern MEM_HANDLE XLD_Load_file(struct XFile_type *Xftype);
extern BOOLEAN XLD_Save_file(MEM_HANDLE Handle, struct XFile_type *Xftype);
extern MEM_HANDLE XLD_Load_standard_file(UNCHAR *Filename);

extern MEM_HANDLE XLD_Load_XLD_library(struct XFile_type *Xftype,
 UNSHORT Group_nr);
extern BOOLEAN XLD_Save_XLD_library(MEM_HANDLE Handle,
 struct XFile_type *Xftype, UNSHORT Group_nr);

extern UNLONG XLD_Get_subfile_length(struct XFile_type *Xftype, UNSHORT Subfile_nr);

extern BOOLEAN XLD_Open_subfile(struct XFile_type *Xftype, UNSHORT Subfile_nr,
 SISHORT *File_handle_ptr, UNLONG *File_base_offset_ptr);
extern void XLD_Close_subfile(SISHORT File_handle);

extern MEM_HANDLE XLD_Load_subfile(struct XFile_type *Xftype,
 UNSHORT Subfile_nr);
extern BOOLEAN XLD_Save_subfile(MEM_HANDLE Handle, struct XFile_type *Xftype,
 UNSHORT Subfile_nr);
extern MEM_HANDLE XLD_Load_unique_subfile(struct XFile_type *Xftype,
 UNSHORT Subfile_nr);

extern BOOLEAN XLD_Load_full_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);
extern BOOLEAN XLD_Load_partial_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);

extern BOOLEAN File_exists(UNCHAR *Filename);
extern BOOLEAN Directory_exists(UNCHAR *Directory_name);
extern void Change_directory(UNCHAR *Pathname);

#endif

