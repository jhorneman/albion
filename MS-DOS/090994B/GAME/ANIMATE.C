/************
 * NAME     : ANIMATE.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28.07.94 12:07
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */
#include <BBDEF.H>
#include <MAP.H>
#include <ANIMATE.H>
#include <math.h>

/* global variables */
UNSHORT Anim_update;
struct Anim_entry Circle_anim[7*8];
struct Anim_entry Wave_anim[6*8];

/* prototypes */
void Reset_random_animation(struct Anim_entry *Ptr);

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
	struct Anim_entry *Ptr;
	UNSHORT i, j, Frame;
	SIBYTE Dir;

	/* Initialize circle animation table */
	Ptr = &Circle_anim[0];

	/* Start at length 2 (!) */
	for (i=2;i<=8;i++)
	{
		/* Start with frame 0 */
		Frame = 0;

		/* Initialize random data */
		Reset_random_animation(Ptr);

		for (j=0;j<8;j++)
		{
			/* Store frame */
			Ptr->Frame = Frame;

			/* Next frame */
			Frame++;

			/* Last frame ? */
			if (Frame == i)
			{
				/* Yes -> Reset random data */
				Reset_random_animation(Ptr);
				/* Back to the first frame */
				Frame = 0;
			}

			/* Next */
			Ptr++;
		}
	}

	/* Initialize wave animation table */
	Ptr = &Wave_anim[0];

	/* Start at length 3 (!) */
	for (i=3;i<=8;i++)
	{
		/* Start with frame 0, going forward */
		Frame = 0;
		Dir = 1;

		/* Initialize random data */
		Reset_random_animation(Ptr);

		for (j=0;j<8;j++)
		{
			/* Store frame */
			Ptr->Frame = Frame;

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
				if (!Frame)
				{
					/* Yes -> Reset random data */
					Reset_random_animation(Ptr);
					/* Reverse direction */
					Dir = 1;
				}
			}

			/* Next */
			Ptr++;
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
	struct Anim_entry *Ptr;
	UNSHORT i, j, Frame;
	SIBYTE Dir;

	/* Update circle animation table */
	Ptr = &Circle_anim[0];

	/* Start at length 2 (!) */
	for (i=2;i<=8;i++)
	{
		for (j=0;j<8;j++)
		{
			/* Get current frame */
			Frame = Ptr->Frame;

			/* Next frame */
			Frame++;

			/* Last frame ? */
			if (Frame == i)
			{
				/* Yes -> Reset random data */
				Reset_random_animation(Ptr);
				/* Back to the first frame */
				Frame = 0;
			}

			/* Store frame */
			Ptr->Frame = Frame;

			/* Next */
			Ptr++;
		}
	}

	/* Initialize wave animation table */
	Ptr = &Wave_anim[0];

	/* Start at length 3 (!) */
	for (i=3;i<=8;i++)
	{
		for (j=0;j<8;j++)
		{
			/* Get current frame and direction */
			Frame = Ptr->Frame;
			Dir = Ptr->Direction;

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
				if (!Frame)
				{
					/* Yes -> Reset random data */
					Reset_random_animation(Ptr);
					/* Reverse direction */
					Dir = 1;
				}
			}

			/* Store frame and direction */
			Ptr->Frame = Frame;
			Ptr->Direction = Dir;

			/* Next */
			Ptr++;
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
 * LAST      : 28.07.94 12:42
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
	UNSHORT i;

	/* Exit if animation length is one frame */
	if ((Nr_frames < 2) || (Nr_frames > 8))
		return(0);

	/* Wave animation with more than two frames ? */
	if ((Flags & WAVE_ANIM) && (Nr_frames > 2))
		/* Yes -> Use wave animation table */
		Ptr = &Wave_anim[ (Nr_frames-3) * 8];
	else
		/* No -> Use circle animation table */
		Ptr = &Circle_anim[ (Nr_frames-2) * 8 ];

	/* Asynchronous animation ? */
	if (Flags & ASYNCH_ANIM)
		/* Yes -> Select asynch group */
		Ptr += ((Hash_nr & 0xF0) >> 4);

	/* Random animation ? */
	if (Flags & RANDOM_ANIM)
	{
		/* Yes -> Select random group */
		i = (Hash_nr + (UNSHORT) Ptr->Random_offset) & 0x000F;

		/* Animate ? */
		if (!(Ptr->Random_bitlist & (1<<i)))
			return (0);
	}

	/* Get current frame */
	return (Ptr->Frame);
}

