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
#define XLD_UNIQUE				(1<<15)

/*
 ** Structure definitions **************************************************
 */

struct XFile_type {
	struct File_type Ftype;		/* File type for BBMEM */
	UNCHAR *Filename;				/* Pointer to filename */
	void (*Convertor)(UNBYTE *, MEM_HANDLE);	/* Pointer to convertor function */
};

/*
 ** Prototypes *************************************************************
 */

extern MEM_HANDLE XLD_Load_file(struct XFile_type *Xftype);
extern void XLD_Save_file(MEM_HANDLE Handle, struct XFile_type *Xftype);
extern MEM_HANDLE XLD_Load_standard_file(UNCHAR *Filename);

extern MEM_HANDLE XLD_Load_subfile(struct XFile_type *Xftype,
 UNSHORT Subfile_nr);
extern void XLD_Save_subfile(MEM_HANDLE Handle, struct XFile_type *Xftype,
 UNSHORT Subfile_nr);
extern MEM_HANDLE XLD_Load_unique_subfile(struct XFile_type *Xftype,
 UNSHORT Subfile_nr);

extern BOOLEAN XLD_Load_full_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);
extern BOOLEAN XLD_Load_partial_batch(struct XFile_type *Xftype, UNSHORT Number,
 UNSHORT *Subfile_list, MEM_HANDLE *Handle_list);

#endif

