/************
 * NAME     : HDOB.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 20-7-1994
 * PROJECT  : HDOB system
 * NOTES    : - It is VITAL that [ MEM_Claim_pointer ] return zero when
 *             handle NULL is claimed.
 *          : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : HDOB.H
 ************/

/* includes */

#include <stdlib.h>
#include <math.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <GFXFUNC.H>
#include <HDOB.H>

#include <CONTROL.H>

/* global variables */

static UNSHORT HDOB_Hide_counter;

static struct HDOB HDOBs[HDOBS_MAX];

/* external variables */
extern struct OPM Main_OPM;

/* structure definitions */

struct Drop_HDOB_data {
	SILONG X;
	SILONG Y;
	SILONG dX;
	SILONG dY;
	SILONG Gravity;
};

struct Move_HDOB_data {
	SISHORT Target_X;
	SISHORT Target_Y;
	UNSHORT Speed;
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_HDOBs
 * FUNCTION  : Delete all current HDOBs and switch visibility on.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 16:20
 * LAST      : 20.07.94 16:20
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_HDOBs(void)
{
	struct HDOB *Current;
	UNSHORT i;

	/* Delete all HDOBs */
	for (i=0;i<HDOBS_MAX;i++)
	{
		Current = &HDOBs[i];

		/* Is the current HDOB active ? */
		if (Current->Flags & HDOB_ON)
		{
			/* Yes -> Delete */
			Current->Flags &= ~HDOB_ON;

			/* Destroy background buffer */
			MEM_Free_memory(Current->Background_handle);
			Current->Background_handle = NULL;
		}
	}

	/* Switch visibility on */
	HDOB_Hide_counter = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_HDOBs
 * FUNCTION  : Show HDOBs.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 16:21
 * LAST      : 20.07.94 16:21
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - You must call this function once for each Hide_HDOBs()-call.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_HDOBs(void)
{
	if (HDOB_Hide_counter)
		HDOB_Hide_counter--;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Hide_HDOBs
 * FUNCTION  : Hide HDOBs.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 16:22
 * LAST      : 20.07.94 16:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - You must call this function once for each Show_HDOBs()-call.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Hide_HDOBs(void)
{
	if (HDOB_Hide_counter < 0xFFFF)
		HDOB_Hide_counter++;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_HDOB
 * FUNCTION  : Add a HDOB to the current list.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 13:06
 * LAST      : 20.07.94 13:06
 * INPUTS    : struct HDOB *New - Pointer to new HDOB.
 * RESULT    : UNSHORT : HDOB number / 0xFFFF = error.
 * BUGS      : No known.
 * NOTES     : - The HDOB data will be copied to an internal array.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Add_HDOB(struct HDOB *New)
{
	struct HDOB *Free = NULL;
	UNSHORT i, Output = 0xFFFF;

	/* Find a free HDOB */
	for (i=0;i<HDOBS_MAX;i++)
	{
		/* Is this HDOB free ? */
		if (!(HDOBs[i].Flags & HDOB_ON))
		{
			/* Yes */
			Free = &HDOBs[i];

			/* Copy HDOB data */
			BASEMEM_CopyMem((UNBYTE *) New, (UNBYTE *) Free, sizeof(struct HDOB));

			/* Indicate HDOB is on */
			Free->Flags |= HDOB_ON;

			/* Insert HDOB numer */
			Free->Number = i;

			/* Reset animation frame */
			Free->Current_frame = 0;

			/* Allocate background buffer */
			Free->Background_handle = MEM_Allocate_memory(New->Width * New->Height);

			/* Exit */
			Output = i;
			break;
		}
	}
	return (Output);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_HDOB
 * FUNCTION  : Change a HDOB in the current list.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.03.95 10:35
 * LAST      : 02.03.95 10:35
 * INPUTS    : UNSHORT HDOB_nr - Number of HDOB that must be deleted.
 *             struct HDOB *New - Pointer to new HDOB.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The HDOB data will be copied to an internal array.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_HDOB(UNSHORT HDOB_nr, struct HDOB *New)
{
	struct HDOB *HDOB;
	struct HDOB Temp;

	/* Get HDOB data */
	HDOB = &HDOBs[HDOB_nr];

	/* Is this HDOB in use ? */
	if (HDOB->Flags & HDOB_ON)
	{
		/* Yes -> Copy old HDOB data into temporary buffer */
		BASEMEM_CopyMem((UNBYTE *) HDOB, (UNBYTE *) &Temp, sizeof(struct HDOB));

		/* Copy new HDOB data over old */
		BASEMEM_CopyMem((UNBYTE *) New, (UNBYTE *) HDOB, sizeof(struct HDOB));

		/* Keep some of the old data */
		HDOB->Number = Temp.Number;
		HDOB->Current_frame = Temp.Current_frame;
		HDOB->Background_handle = Temp.Background_handle;

		/* Indicate HDOB is on */
		HDOB->Flags |= HDOB_ON;

		/* Is the current animation frame outside of the new
		 animation length ? */
		if (Temp.Current_frame >= HDOB->Nr_frames)
		{
			/* Yes -> Reset current frame */
			HDOB->Current_frame = 0;
		}

		/* Did the size change ? */
		if ((Temp.Width != HDOB->Width) || (Temp.Height != HDOB->Height))
		{
			/* Yes -> Resize background buffer */
			MEM_Resize_memory(HDOB->Background_handle, HDOB->Width *
			 HDOB->Height);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_HDOB
 * FUNCTION  : Delete a HDOB from the current list.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 13:13
 * LAST      : 20.07.94 13:13
 * INPUTS    : UNSHORT HDOB_nr - Number of HDOB that must be deleted.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may be called from HDOB handlers.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Delete_HDOB(UNSHORT HDOB_Nr)
{
	struct HDOB *Delete;

	if (HDOB_Nr != 0xFFFF)
	{
		Delete = &HDOBs[HDOB_Nr];

		/* Switch HDOB off */
		Delete->Flags &= ~HDOB_ON;

		/* Destroy background buffer */
		MEM_Free_memory(Delete->Background_handle);
		Delete->Background_handle = NULL;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_HDOB_position
 * FUNCTION  : Set the position of a HDOB.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 15:03
 * LAST      : 22.07.94 15:03
 * INPUTS    : UNSHORT HDOB_Nr - Number of HDOB that must be moved.
 *             SISHORT X - New X-coordinate.
 *             SISHORT Y - New Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_HDOB_position(UNSHORT HDOB_Nr, SISHORT X, SISHORT Y)
{
	if (HDOB_Nr != 0xFFFF)
	{
		HDOBs[HDOB_Nr].X = X;
		HDOBs[HDOB_Nr].Y = Y;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_HDOBs
 * FUNCTION  : Display HDOBs on the screen.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 15:01
 * LAST      : 20.07.94 15:01
 * INPUTS    : struct BBPOINT *Mouse - Pointer to point containing the
 *              current mouse coordinates (for attached HDOBs).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_HDOBs(struct BBPOINT *Mouse)
{
	struct HDOB *Current;
	UNBYTE *Graphics_ptr, *Background_ptr;
	UNSHORT i;
	SISHORT X, Y;

	/* Show at all ? */
	if (HDOB_Hide_counter)
		return;

	/* Draw all HDOBs (forward) */
	for (i=0;i<HDOBS_MAX;i++)
	{
		Current = &HDOBs[i];

		/* Is the current HDOB active ? */
		if (Current->Flags & HDOB_ON)
		{
			/* Yes -> Is the current HDOB attached to the mouse ? */
			if (Current->Flags & HDOB_ATTACHED)
			{
				/* Yes -> Add HDOB coordinates to mouse coordinates */
				X = Mouse->x + Current->X;
				Y = Mouse->y + Current->Y;
			}
			else
			{
				/* No -> Use the HDOB coordinates */
				X = Current->X;
				Y = Current->Y;
			}

			/* Save background */
			Background_ptr = MEM_Claim_pointer(Current->Background_handle);
			Get_block(&Main_OPM, X, Y, Current->Width, Current->Height, Background_ptr);
			MEM_Free_pointer(Current->Background_handle);

			/* Get graphics address */
			Graphics_ptr = MEM_Claim_pointer(Current->Graphics_handle) +
			 Current->Graphics_offset + (Current->Current_frame * Current->Width *
			 Current->Height);

			/* Draw HDOB */
			switch (Current->Draw_mode)
			{
				case HDOB_MASK:
				{
					Put_masked_block(&Main_OPM, X, Y, Current->Width,
					 Current->Height, Graphics_ptr);
					break;
				}
				case HDOB_BLOCK:
				{
					Put_unmasked_block(&Main_OPM, X, Y, Current->Width,
					 Current->Height, Graphics_ptr);
					break;
				}
				case HDOB_SILHOUETTE:
				{
					Put_silhouette(&Main_OPM, X, Y, Current->Width,
					 Current->Height, Graphics_ptr, (UNBYTE) Current->Draw_data);
					break;
				}
				case HDOB_BOX:
				{
					OPM_FillBox(&Main_OPM, X, Y, Current->Width,
					 Current->Height, (UNBYTE) Current->Draw_data);
					break;
				}
				case HDOB_RECOLOUR:
				{
					Put_recoloured_block(&Main_OPM, X, Y, Current->Width,
					 Current->Height, Graphics_ptr, (UNBYTE *) Current->Draw_data);
					break;
				}
				case HDOB_RECOLBOX:
				{
					Put_recoloured_box(&Main_OPM, X, Y, Current->Width,
					 Current->Height, (UNBYTE *) Current->Draw_data);
					break;
				}
			}

			MEM_Free_pointer(Current->Background_handle);

			/* Is this HDOB animated ? */
			if (Current->Nr_frames > 1)
			{
				/* Yes -> Wave or circle animation ? */
				if ((Current->Flags & HDOB_WAVE_ANIM) && (Current->Nr_frames > 2))
				{
					/* Wave -> Back or forth ? */
					if (Current->Flags & HDOB_WAVE_DIR)
					{
						/* Back -> Reached first frame ? */
						if (!Current->Current_frame)
						{
							/* Yes -> Reverse direction */
							Current->Current_frame = 1;
							Current->Flags &= ~HDOB_WAVE_DIR;
						}
						else
						{
							/* No -> Go back one frame */
							Current->Current_frame--;
						}
					}
					else
					{
						/* Forth -> Reached last frame ? */
						if (Current->Current_frame == Current->Nr_frames - 1)
						{
							/* Yes -> Reverse direction */
							Current->Current_frame = Current->Nr_frames - 2;
							Current->Flags |= HDOB_WAVE_DIR;
						}
						else
						{
							/* No -> Go forth one frame */
							Current->Current_frame++;
						}
					}
				}
				else
				{
					/* Circle ->  Reached last frame ? */
					if (Current->Current_frame == Current->Nr_frames - 1)
					{
						/* Yes -> Back to the first frame */
						Current->Current_frame = 0;
					}
					else
					{
						/* No -> Go forth one frame */
						Current->Current_frame++;
					}
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Erase_HDOBs
 * FUNCTION  : Remove HDOBs from the screen.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 15:03
 * LAST      : 20.07.94 15:03
 * INPUTS    : struct BBPOINT *Mouse - Pointer to point containing the
 *              current mouse coordinates (for attached HDOBs).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Erase_HDOBs(struct BBPOINT *Mouse)
{
	struct HDOB *Current;
	UNBYTE *Background_ptr;
	SISHORT i;
	SISHORT X, Y;

	/* Show at all ? */
	if (HDOB_Hide_counter)
		return;

	/* Erase all HDOBs (backward) */
	for (i=HDOBS_MAX-1;i>=0;i--)
	{
		Current = &HDOBs[i];

		/* Is the current HDOB active ? */
		if (Current->Flags & HDOB_ON)
		{
			/* Yes -> Is the current HDOB attached to the mouse ? */
			if (Current->Flags & HDOB_ATTACHED)
			{
				/* Yes -> Add HDOB coordinates to mouse coordinates */
				X = Mouse->x + Current->X;
				Y = Mouse->y + Current->Y;
			}
			else
			{
				/* No -> Use the HDOB coordinates */
				X = Current->X;
				Y = Current->Y;
			}

			/* Restore background */
			Background_ptr = MEM_Claim_pointer(Current->Background_handle);
			Put_unmasked_block(&Main_OPM, X, Y, Current->Width, Current->Height,
			 Background_ptr);
			MEM_Free_pointer(Current->Background_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_HDOBs
 * FUNCTION  : Update all HDOBs.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.02.95 12:48
 * LAST      : 28.02.95 12:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_HDOBs(void)
{
	struct HDOB *Current;
	UNSHORT i;

	/* Update all HDOBs */
	for (i=0;i<HDOBS_MAX;i++)
	{
		Current = &HDOBs[i];

		/* Is the current HDOB active ? */
		if (Current->Flags & HDOB_ON)
		{
			/* Yes -> Call HDOB handler (if any) */
			if (Current->Handler)
				(Current->Handler)(Current);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_HDOB
 * FUNCTION  : Drop a HDOB.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.02.95 13:41
 * LAST      : 24.02.95 13:41
 * INPUTS    : UNSHORT HDOB_nr - HDOB index.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The HDOB must have been initialized and added.
 *             - The HDOB's current position is used as the source position.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_HDOB(UNSHORT HDOB_nr)
{
	struct Drop_HDOB_data Data;
	struct HDOB *HDOB;
	SILONG dX, dY;

	/* Get HDOB data */
	HDOB = &(HDOBs[HDOB_nr]);

	/* Anything there ? */
	if (HDOB->Flags & HDOB_ON)
	{
		/* Yes -> Calculate drop parameters */
		do
		{
			/* Get random vector */
			dX = (rand() & 0x00000fff) - 7 * 256 + 128;
			dY = (rand() & 0x00000fff) - 14 * 256;
		}
		/* Until it is not the null vector */
		while (!dX && !dY);

		/* Initialize drop data */
		Data.X = HDOB->X * 65536;
		Data.Y = HDOB->Y * 65536;
		Data.dX = dX * 128;
		Data.dY = dY * 128;
		Data.Gravity = (rand() & 0x00000fff) + 64000/2 - 2048;

		/* Set HDOB handler and handler data */
		HDOB->Handler = Drop_HDOB_handler;
		HDOB->Handler_data = &Data;

		/* HDOB is not ready */
		HDOB->Flags &= ~HDOB_READY;

		do
		{
			/* Update display */
			Update_display();
			Update_input();
			Switch_screens();
		}
		/* Until the HDOB is ready */
		while (!(HDOB->Flags & HDOB_READY));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_HDOB_handler
 * FUNCTION  : HDOB handler which drops a HDOB.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.02.95 13:49
 * LAST      : 28.02.95 13:49
 * INPUTS    : struct HDOB *HDOB - Pointer to HDOB data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_HDOB_handler(struct HDOB *HDOB)
{
	struct Drop_HDOB_data *Drop_data;
	SISHORT wX, wY;

	/* Get drop data */
	Drop_data = (struct Drop_HDOB_data *) HDOB->Handler_data;

	/* Move along vector */
	Drop_data->X += Drop_data->dX;
	Drop_data->Y += Drop_data->dY;

	/* Calculate real coordinates */
	wX = Drop_data->X / 65536;
	wY = Drop_data->Y / 65536;

	/* Off screen ? */
	if ((wX > (0 - HDOB->Width)) && (wX < Main_OPM.width) &&
	 (wY < Main_OPM.height))
	{
		/* No -> Set new coordinates */
		Set_HDOB_position(HDOB->Number, wX, wY);

		/* Gravitate */
		Drop_data->dY += Drop_data->Gravity;
	}
	else
	{
		/* Yes -> HDOB is ready */
		HDOB->Flags |= HDOB_READY;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Vibrate_HDOB
 * FUNCTION  : Vibrate a HDOB.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.02.95 13:55
 * LAST      : 24.02.95 13:55
 * INPUTS    : UNSHORT HDOB_nr - HDOB index.
 *             UNSHORT Nr_vibrations - Number of vibrations.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The HDOB must have been initialized and added.
 *             - The HDOB's current position is used as the source position.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Vibrate_HDOB(UNSHORT HDOB_nr, UNSHORT Nr_vibrations)
{
	struct HDOB *HDOB;
	SISHORT X, Y;
	UNSHORT i;

	/* Get HDOB data */
	HDOB = &HDOBs[HDOB_nr];

	/* Get current coordinates */
	X = HDOB->X;
	Y = HDOB->Y;

	/* Vibrate HDOB */
	for (i=0;i<Nr_vibrations;i++)
	{
		/* A little left */
		Set_HDOB_position(HDOB_nr, X-1, Y);

		/* Update display */
		Update_display();
		Update_input();
		Switch_screens();

		/* A little right */
		Set_HDOB_position(HDOB_nr, X, Y);

		/* Update display */
		Update_display();
		Update_input();
		Switch_screens();

		/* A little right */
		Set_HDOB_position(HDOB_nr, X+1, Y);

		/* Update display */
		Update_display();
		Update_input();
		Switch_screens();

 		/* A little left */
		Set_HDOB_position(HDOB_nr, X, Y);

		/* Update display */
		Update_display();
		Update_input();
		Switch_screens();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_HDOB
 * FUNCTION  : Move a HDOB towards a target.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.02.95 14:01
 * LAST      : 24.02.95 14:01
 * INPUTS    : UNSHORT HDOB_nr - HDOB index.
 *             SISHORT Target_X - Target X-coordinate.
 *             SISHORT Target_Y - Target Y-coordinate.
 *             UNSHORT Speed - Movement speed.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The HDOB must have been initialized and added.
 *             - The HDOB's current position is used as the source position.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_HDOB(UNSHORT HDOB_nr, SISHORT Target_X, SISHORT Target_Y, UNSHORT Speed)
{
	struct Move_HDOB_data Data;
	struct HDOB *HDOB;

	/* Get HDOB data */
	HDOB = &(HDOBs[HDOB_nr]);

	/* Anything there ? */
	if (HDOB->Flags & HDOB_ON)
	{
		/* Yes -> Initialize movement data */
		Data.Target_X = Target_X;
		Data.Target_Y = Target_Y;
		Data.Speed = Speed;

		/* Set HDOB handler and handler data */
		HDOB->Handler = Move_HDOB_handler;
		HDOB->Handler_data = &Data;

		/* HDOB is not ready */
		HDOB->Flags &= ~HDOB_READY;

		do
		{
			/* Update display */
			Update_display();
			Update_input();
			Switch_screens();
		}
		/* Until the HDOB is ready */
		while (!(HDOB->Flags & HDOB_READY));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_HDOB_handler
 * FUNCTION  : HDOB handler which moves a HDOB to a target.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.02.95 13:46
 * LAST      : 28.02.95 13:46
 * INPUTS    : struct HDOB *HDOB - Pointer to HDOB data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_HDOB_handler(struct HDOB *HDOB)
{
	struct Move_HDOB_data *Move_data;
	SILONG dX;
	SILONG dY;
	SILONG Length;
	SISHORT Source_X;
	SISHORT Source_Y;

	/* Get movement data */
	Move_data = (struct Move_HDOB_data *) HDOB->Handler_data;

	/* Get source coordinates */
	Source_X = HDOB->X;
	Source_Y = HDOB->Y;

	/* Calculate vector */
	dX = (SILONG)(Move_data->Target_X - Source_X);
	dY = (SILONG)(Move_data->Target_Y - Source_Y);

	/* Null vector ? */
	if (dX || dY)
	{
		/* No -> Calculate vector length */
		Length = (SILONG) sqrt((double) (dX * dX + dY * dY));

		/* Almost there ? */
		if (Length > (SILONG) Move_data->Speed)
		{
			/* No -> Calculate movement vector */
			dX = (dX * (SILONG) Move_data->Speed) / Length;
			dY = (dY * (SILONG) Move_data->Speed) / Length;
		}

		/* Move along vector */
		Source_X += dX;
		Source_Y += dY;

		/* Set new coordinates */
		Set_HDOB_position(HDOB->Number, Source_X, Source_Y);
	}
	else
	{
		/* Yes -> HDOB is ready */
		HDOB->Flags |= HDOB_READY;
	}
}

