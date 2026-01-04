/*
 ** FLI / FLC stuff ************************************************************
 */

#include "BBDEFxx.H"

#include "FLC.H"
#include "FLI_FAST.H"

#include <dos.h>
#include <mem.h>




/* do chunk 12 */

UNBYTE far *
DOCHUNK12( register UNBYTE far * fliworkptr )
{

	/* SIBYTE aligned delta compression chunk */


	/* locals */


	/* work pointer for video ram */

	register UNBYTE far * vw;


	/* lines to go in chunk */

	UNSHORT linestogo;

	/* packets in line */

	UNBYTE packets;

	/* pointer to base line in video ram */

	UNBYTE far * vb;



	/* get video ram base pointer */

	vb = ( ( UNBYTE far * ) 0xa0000000 ) + *( ( UNSHORT * ) fliworkptr ) * 320;
	fliworkptr += 2;


	/* get lines to go */

	linestogo = *( ( UNSHORT * ) fliworkptr );
	fliworkptr += 2;



	/* any line to go ? */

	if ( ! linestogo )
		return( fliworkptr );



	/* yes ! */

	/* work on all lines */

	do
	{
		/* get number of packets */

		packets = ( *fliworkptr++ );


		/* any packet for this line ? */

		if ( packets )
		{
			/* yes ! */


			/* set video work pointer */

			vw = vb;


			/* startup skip */

			vw += ( UNSHORT ) ( *fliworkptr++ );



			/* for on all packets */

			do
			{
				/* local vars */

				register SIBYTE count;


				/* load counter */

				count = *fliworkptr++;


				/* counter positive ? */

				if ( count >= 0 )
				{
					/* yes: copy string */

					// while ( count-- )
					//  	*vw++ = *fliworkptr++;

					memcpy( vw, fliworkptr, count );
					vw += count;
					fliworkptr += count;

				}
				else
				{
					/* no: fill with byte */

					register UNBYTE color;

					color = *fliworkptr++;

					// while ( count++ )
					// 	*vw++ = color;

					setmem( vw, -count, color );
					vw -= count;
				}


				/* dec number of packets */

				packets--;


				/* add skip byte */

				if ( packets )
					vw += ( UNSHORT ) ( *fliworkptr++ );


			}
			while ( packets );
		}


		/* next line in video ram */

		vb += 320;


		/* dec lines to go */

		linestogo--;




		/* setup ems and anim pointer */

		/* EMS offset > 48K ? */

		if ( FP_OFF( fliworkptr ) >= 0xc000 )
		{
			/* yes ! */

			/* skip 3 EMS pages */

			SKIP3EMSPAGES();

			/* recalc pointer */

			fliworkptr -= 0xc000;
		}



	}
	while ( linestogo );



	/* return pointer */

	return( fliworkptr );

}



