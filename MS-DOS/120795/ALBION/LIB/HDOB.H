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
 ** Type definitions *******************************************************
 */

typedef void (*HDOB_handler) (struct HDOB *HDOB);

/*
 ** Structure definitions **************************************************
 */

/* HDOB data */
struct HDOB {
	UNSHORT Draw_mode;  				/* Draw mode (see below) */
	UNSHORT Flags;						/* Flags (see below) */

	SISHORT X;							/* Coordinates (relative when attached) */
	SISHORT Y;
	UNSHORT Width;						/* Size of HDOB */
	UNSHORT Height;

	UNSHORT Nr_frames;				/* Number of animation frames */

	UNLONG Draw_data;					/* Special data for certain draw modes */

	MEM_HANDLE Graphics_handle;	/* Handle of graphics memory block / NULL */
	UNLONG Graphics_offset;			/* Offset to graphics / absolute pointer */

	HDOB_handler Handler; 			/* Pointer to handler function */
	void *Handler_data; 				/* Pointer to handler data */

	/* Set by HDOB functions : */
	UNSHORT Number;					/* HDOB number */
	UNSHORT Current_frame;			/* Current animation frame */

	MEM_HANDLE Background_handle;	/* Handle of background memory block */
};

/* Values for HDOB.Draw_mode */
#define HDOB_MASK			(0)
#define HDOB_BLOCK		(1)
#define HDOB_SILHOUETTE	(2)
#define HDOB_BOX			(3)
#define HDOB_RECOLOUR	(4)
#define HDOB_RECOLBOX	(5)

/* Bit masks for HDOB.Flags */
#define HDOB_ON			(1<<0)	/* HDOB is in use (set by HDOB functions) */
#define HDOB_ATTACHED	(1<<1)	/* Attached to the mouse pointer */
#define HDOB_WAVE_ANIM	(1<<2)	/* Default is circle animation */
#define HDOB_WAVE_DIR	(1<<3)	/* Wave animation direction (set by HDOB functions) */
#define HDOB_READY		(1<<4)	/* HDOB is ready (set by HDOB functions) */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

void Reset_HDOBs(void);
void Show_HDOBs(void);
void Hide_HDOBs(void);

UNSHORT Add_HDOB(struct HDOB *New);
void Change_HDOB(UNSHORT HDOB_nr, struct HDOB *New);
void Delete_HDOB(UNSHORT HDOB_Nr);

void Set_HDOB_position(UNSHORT HDOB_Nr, SISHORT X, SISHORT Y);

void Draw_HDOBs(struct BBPOINT *Mouse);
void Erase_HDOBs(struct BBPOINT *Mouse);
void Update_HDOBs(void);

void Drop_HDOB(UNSHORT HDOB_nr);
void Drop_HDOB_handler(struct HDOB *HDOB);

void Vibrate_HDOB(UNSHORT HDOB_nr, UNSHORT Nr_vibrations);

void Move_HDOB(UNSHORT HDOB_nr, SISHORT Target_X, SISHORT Target_Y,
 UNSHORT Speed);
void Move_HDOB_handler(struct HDOB *HDOB);

#endif

