/************
 * NAME     : HDOB.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 20-7-1994
 * PROJECT  : HDOB system
 * NOTES    :
 * SEE ALSO : HDOB.C
 ************/

#ifndef HDOB_H
#define HDOB_H

/*
 ** Defines ****************************************************************
 */

#define HDOBS_MAX		(8)

/*
 ** Structure definitions **************************************************
 */

struct HDOB {
	SISHORT X,Y;						/* Coordinates (relative when attached) */
	UNSHORT Width,Height;			/* Size of HDOB */
	UNBYTE Draw_mode;					/* Draw mode (see below) */
	UNBYTE Flags;						/* Flags (see below) */
	UNLONG Data;						/* Special data for certain draw modes */
	MEM_HANDLE Graphics_handle;	/* Handle of graphics memory block / NULL */
	UNLONG Graphics_offset;			/* Offset to graphics / absolute pointer */
	MEM_HANDLE Background_handle;	/* Handle of background memory block
											    (set by Add_HDOB) */
};

/* Values for HDOB.Draw_mode */
#define HDOB_MASK			(0)
#define HDOB_BLOCK		(1)
#define HDOB_SILHOUETTE	(2)
#define HDOB_BOX			(3)
#define HDOB_RECOLOUR	(4)
#define HDOB_RECOLBOX	(5)

/* Bit masks for HDOB.Flags */
#define HDOB_ON			(1<<0)
#define HDOB_ATTACHED	(1<<1)

/*
 ** Prototypes *************************************************************
 */

void Reset_HDOBs(void);
void Show_HDOBs(void);
void Hide_HDOBs(void);

UNSHORT Add_HDOB(struct HDOB *New);
void Delete_HDOB(UNSHORT HDOB_Nr);

void Set_HDOB_position(UNSHORT HDOB_Nr, SISHORT X, SISHORT Y);

void Draw_HDOBs(struct BBPOINT *Mouse);
void Erase_HDOBs(struct BBPOINT *Mouse);

#endif

