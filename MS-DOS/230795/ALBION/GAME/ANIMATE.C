/************
 * NAME     : ANIMATE.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28.07.94 12:07
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */
#include <stdlib.h>

#include <BBDEF.H>

#include <MAP.H>
#include <ANIMATE.H>

/* global variables */
static struct Anim_entry Circle_anim[7][8];
static struct Anim_entry Wave_anim[6][8];

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_animation
 * FUNCTION  : Initialize animation tables.
 * FILE      : ANIMATE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:07
 * LAST      : 28.07.94 12:07
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_animation(void)
{
	UNSHORT i, j;
	SISHORT Frame, Dir;

	/* Initialize circle animation table */
	/* Start at length 2 (!) */
	for (i=2;i<=8;i++)
	{
		/* Start with frame 0 */
		Frame = 0;

		for (j=0;j<8;j++)
		{
			/* Reset random data */
			Reset_random_animation(&Circle_anim[i-2][j]);

			/* Store frame */
			Circle_anim[i-2][j].Frame = Frame;

			/* Next frame */
			Frame++;

			/* Last frame ? */
			if (Frame == i)
			{
				/* Yes -> Back to the first frame */
				Frame = 0;
			}
		}
	}

	/* Initialize wave animation table */
	/* Start at length 3 (!) */
	for (i=3;i<=8;i++)
	{
		/* Start with frame 0, going forward */
		Frame = 0;
		Dir = 1;

		for (j=0;j<8;j++)
		{
			/* Reset random data */
			Reset_random_animation(&Wave_anim[i-3][j]);

			/* Store frame and direction */
			Wave_anim[i-3][j].Frame = Frame;
			Wave_anim[i-3][j].Direction = Dir;

			/* Going forward or backward ? */
			if (Dir > 0)
			{
				/* Forward -> Next frame */
				Frame++;

				/* Last frame ? */
				if (Frame == i)
				{
					/* Yes -> Reverse direction */
					Dir = -1;
					Frame -= 2;
				}
			}
			else
			{
				/* Backward -> Previous frame */
				Frame--;

				/* Back at the first frame ? */
				if (Frame <= 0)
				{
					/* Yes -> Reverse direction */
					Frame = 0;
					Dir = 1;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_random_animation
 * FUNCTION  : Reset random animation data.
 * FILE      : ANIMATE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:33
 * LAST      : 28.07.94 12:33
 * INPUTS    : struct Anim_entry *Ptr - Pointer to animation table entry.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_random_animation(struct Anim_entry *Ptr)
{
	UNSHORT i, Bitlist = 0;

	for (i=0;i<RANDOM_ANIM_BIAS;i++)
		Bitlist |= (1 << (rand() & 0x000F));

	Ptr->Random_bitlist = Bitlist;
	Ptr->Random_offset = rand() & 0xFF;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_animation
 * FUNCTION  : Update animation arrays.
 * FILE      : ANIMATE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:38
 * LAST      : 28.07.94 12:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Length 1 (circle) and lengths 1 & 2 (wave) are missing.
 *              This should be considered by the actual animation routine.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_animation(void)
{
	UNSHORT i, j;
	SISHORT Frame, Dir;

	/* Update circle animation table */
	/* Start at length 2 (!) */
	for (i=2;i<=8;i++)
	{
		for (j=0;j<8;j++)
		{
			/* Get current frame */
			Frame = Circle_anim[i-2][j].Frame;

			/* Next frame */
			Frame++;

			/* Last frame ? */
			if (Frame == i)
			{
				/* Yes -> Reset random data */
				Reset_random_animation(&Circle_anim[i-2][j]);
				/* Back to the first frame */
				Frame = 0;
			}

			/* Store frame */
			Circle_anim[i-2][j].Frame = Frame;
		}
	}

	/* Initialize wave animation table */
	/* Start at length 3 (!) */
	for (i=3;i<=8;i++)
	{
		for (j=0;j<8;j++)
		{
			/* Get current frame and direction */
			Frame = Wave_anim[i-3][j].Frame;
			Dir = Wave_anim[i-3][j].Direction;

			/* Going forward or backward ? */
			if (Dir > 0)
			{
				/* Forward -> Next frame */
				Frame++;

				/* Last frame ? */
				if (Frame == i)
				{
					/* Yes -> Reverse direction */
					Dir = -1;
					Frame -= 2;
				}
			}
			else
			{
				/* Backward -> Previous frame */
				Frame--;

				/* Back at the first frame ? */
				if (Frame <= 0)
				{
					/* Yes -> Reset random data */
					Reset_random_animation(&Wave_anim[i-3][j]);
					/* Reverse direction */
					Frame = 0;
					Dir = 1;
				}
			}

			/* Store frame and direction */
			Wave_anim[i-3][j].Frame = Frame;
			Wave_anim[i-3][j].Direction = Dir;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_animation_frame
 * FUNCTION  : Get current animation frame.
 * FILE      : ANIMATE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:42
 * LAST      : 22.07.95 18:01
 * INPUTS    : UNLONG Flags - Flags determining animation type.
 *             UNSHORT Nr_frames - Number of animation frames (1...8).
 *             UNSHORT Hash_nr - Hash number.
 * RESULT    : UNSHORT : Current animation frame (0...7).
 * BUGS      : No known.
 * SEE ALSO  : MAP.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_animation_frame(UNLONG Flags, UNSHORT Nr_frames, UNSHORT Hash_nr)
{
	struct Anim_entry *Ptr;
	UNSHORT Group;
	UNSHORT j = 0;

	/* Exit if animation length is one frame */
	if ((Nr_frames < 2) || (Nr_frames > 8))
		return 0;

	/* Wave animation with more than two frames ? */
	if ((Flags & WAVE_ANIM) && (Nr_frames > 2))
	{
		/* Yes -> Use wave animation table */
		Ptr = &Wave_anim[Nr_frames-3][0];
	}
	else
	{
		/* No -> Use circle animation table */
		Ptr = &Circle_anim[Nr_frames-2][0];
	}

	/* Asynchronous animation ? */
	if (Flags & ASYNCH_ANIM)
	{
		/* Yes -> Select asynch group */
		/* (the bottom 3 bits must be used as these are the only difference
			between objects in a 3D object group) */
		j = Hash_nr & 0x0007;
	}

	/* Random animation ? */
	if (Flags & RANDOM_ANIM)
	{
		/* Yes -> Select random group */
		Group = (Hash_nr + (UNSHORT) Ptr[j].Random_offset) & 0x000F;

		/* Animate ? */
		if (!(Ptr[j].Random_bitlist & (1 << Group)))
			return (0);
	}

	/* Get current frame */
	return (Ptr[j].Frame & 0x0007);
}

