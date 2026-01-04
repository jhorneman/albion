#ifndef FLC_H
#define FLC_H


/* Includes */

#include "BBDEFxx.H"



/* Defines */

#define	FLC_ANIM_DONE	 		( 1 << 0 )
#define	FLC_ANIM_ERROR	 		( 1 << 1 )
#define	FLC_ANIM_UNKNOWNCHUNK 	( 1 << 2 )




/* Structures */

struct flc_struct
{
	UNBYTE * animptr;

	UNLONG	workos;

	UNBYTE * palptr;

	UNSHORT width;
	UNSHORT height;

	UNSHORT frames;
	UNSHORT actframe;
};



/* Prototypes */

extern void flcerrormessage( UNCHAR * text );

extern BOOLEAN flc_init_anim( struct flc_struct * flc_struct );
extern UNSHORT flc_next_frame( struct flc_struct * flc_struct );

extern UNSHORT flc_fast_next_frame( struct flc_struct * flc_struct );


#endif
