/************
 * NAME     : XOPM.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 5-8-1994
 * PROJECT  : eXtended OPM functions
 * NOTES    :
 * SEE ALSO : XOPM.C
 ************/

#ifndef XOPM_H
#define XOPM_H

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* eXtended OPMs */
struct XOPM {
	UNSHORT Flags;
	SISHORT X,Y;
	MEM_HANDLE Handle;
	struct XOPM *Parent;
	struct XOPM *Child;
	struct XOPM *Next;
	struct OPM OPM;
};

/* Bitmasks for XOPM.Flags */
#define XOPM_UPDATE		(1<<0)	/* XOPM was changed and must be updated */
#define XOPM_DOUBLE		(1<<1)	/* XOPM was updated previous frame */

/*
 ** Prototypes *************************************************************
 */

void XOPM_Create_base(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height,
 struct XOPM *XOPM);
void XOPM_Delete_base(struct XOPM *XOPM);
void XOPM_Add_child(struct XOPM *Base, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height, struct XOPM *Child);
void XOPM_Remove_child(struct XOPM *Child);

#endif

