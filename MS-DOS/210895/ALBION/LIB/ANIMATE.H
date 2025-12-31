/************
 * NAME     : ANIMATE.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef ANIMATE_H
#define ANIMATE_H

/*
 ** Defines ****************************************************************
 */

#define RANDOM_ANIM_BIAS	(10)

/*
 ** Structure definitions **************************************************
 */

struct Anim_entry {
	UNSHORT Frame;
	SIBYTE Direction;				/* Wave animation direction */
	UNBYTE Random_offset;		/* To create different groups every time */
	UNSHORT Random_bitlist;
};

/*
 ** Prototypes *************************************************************
 */

void Init_animation(void);
void Update_animation(void);
void Reset_random_animation(struct Anim_entry *Ptr);
UNSHORT Get_animation_frame(UNLONG Flags, UNSHORT Nr_frames, UNSHORT Hash_nr);

#endif

