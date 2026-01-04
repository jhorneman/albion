/*
 ** FLI / FLC stuff ************************************************************
 */

#include "BBDEFxx.H"

#include "CDFLI.H"
#include "FLC.H"

#include <stdio.h>
#include <dos.h>

#include "BB.h"


/* extern */

extern	short 	CDFLI_EMS_HANDLE;
extern	ubyte far * CDFLI_EMS_FRAMEADDR;
extern	short	CDFLI_EMS_OFFSET;

extern 	void 	SETVMODE13PAL( UNBYTE * palptr );





/* set pixel */

void
flc_setpixel( UNSHORT x, UNSHORT y, UNBYTE color )
{
	*( ( ( UNBYTE * ) 0xa0000000 ) + x + y * 320 ) = color;
}



/* first frame */

BOOLEAN
flc_init_anim( struct flc_struct * flc_struct )
{

	/* locals */

	UNBYTE * animptr;


	/* get anim ptr */

	animptr = flc_struct->animptr;


	/* is it a flc file ? */

	{
		UNSHORT magic = *( ( UNSHORT * ) ( animptr + 4L ) );

		if ( ( magic != 0xaf11 ) && ( magic != 0xaf12 ) )

			/* No ! */
			return( FALSE );
	}

	/* get number of frames */

	flc_struct->frames = *( ( UNSHORT * ) ( animptr +  6L ) );
	flc_struct->actframe = 0;


	/* init ptr */

	flc_struct->workos = 0L;


	/* get width and height */

	flc_struct->width  = *( ( UNSHORT * ) ( animptr +  8L ) );
	flc_struct->height = *( ( UNSHORT * ) ( animptr + 10L ) );



	/* everything is ok */

	return( TRUE );
}








UNSHORT
flc_next_frame( struct flc_struct * flc_struct )
{

	/* locals */

	UNSHORT chunks;

	UNBYTE far * animworkptr;

	UNSHORT width;
	UNSHORT height;

	UNBYTE * palptr;

	SISHORT returnflag;


	UNLONG	workos;
	UNSHORT workemsos;



	/* get pal ptr */

	palptr = flc_struct->palptr;




	/* init animworkptr */

	workos = flc_struct->workos;
	workemsos = ( workos >> 14 );


	SETEMS( CDFLI_EMS_HANDLE, 0, ( ( workemsos + 0 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 1, ( ( workemsos + 1 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 2, ( ( workemsos + 2 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 3, ( ( workemsos + 3 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );


	animworkptr = CDFLI_EMS_FRAMEADDR + ( workos & 0x00003fffL );




	/* is it a frame chunk ? */

	if ( *( ( UNSHORT * ) ( animworkptr + 4L ) ) != 0xf1fa )
		return( FLC_ANIM_ERROR );


	/* Init return value */

	returnflag = 0;


	/* get width and height */

	width  = flc_struct->width;
	height = flc_struct->height;


	/* get pointer to next frame chunk */

	flc_struct->workos = flc_struct->workos + *( ( UNLONG * ) ( animworkptr ) );



	/* get number of chunks in frame */

	chunks = *( ( UNSHORT * ) ( animworkptr + 6L ) );


	/* pointer to first chunk */

	animworkptr += 16L;


	/* work on chunk of frame */

	if ( chunks )
	{

		do
		{

			/* locals */

			UNBYTE * nextchunkptr;
			UNSHORT typeofchunk;


			/* calc pointer to next chunk */

			nextchunkptr = animworkptr + *( ( UNLONG * ) animworkptr );

			/* get type of chunk */

			typeofchunk = *( ( UNSHORT * ) ( animworkptr + 4L ) );

			/* pointer to chunk */

			animworkptr += 6L;



			/* work on chunk */

			switch ( typeofchunk )
			{

				/* 256 level color chunk */

				case 4:
				{
					UNSHORT packets;
					UNBYTE color = 0;
					UNSHORT tocopy;
					UNBYTE r;
					UNBYTE g;
					UNBYTE b;

					packets = *( ( UNSHORT * ) animworkptr );
					animworkptr += 2L;

					while ( packets-- )
					{
						color += *animworkptr++;
						tocopy = ( UNSHORT ) *animworkptr++;

						if ( tocopy == 0 )
							tocopy = 0x100;

						while ( tocopy-- )
						{
							r = *animworkptr++;
							g = *animworkptr++;
							b = *animworkptr++;

							palptr[ color * 3 + 0 ]	= r;
							palptr[ color * 3 + 1 ]	= g;
							palptr[ color * 3 + 2 ] = b;

							color++;
						}

						SETVMODE13PAL( palptr );
					}

					break;
				}


				/* 64 level color chunk */

				case 11:
				{
					UNSHORT packets;
					UNBYTE color = 0;
					UNSHORT tocopy;
					UNBYTE r;
					UNBYTE g;
					UNBYTE b;

					packets = *( ( UNSHORT * ) animworkptr );
					animworkptr += 2L;

					while ( packets-- )
					{
						color += *animworkptr++;
						tocopy = ( UNSHORT ) *animworkptr++;

						if ( tocopy == 0 )
							tocopy = 0x100;

						while ( tocopy-- )
						{
							r = *animworkptr++;
							g = *animworkptr++;
							b = *animworkptr++;

							palptr[ color * 3 + 0 ]	= r << 2;
							palptr[ color * 3 + 1 ]	= g << 2;
							palptr[ color * 3 + 2 ] = b << 2;

							color++;
						}


						SETVMODE13PAL( palptr );

					}

					break;
				}


				/* word aligned delta compression chunk */

				case 7:
				{

					/* locals */

					UNSHORT x;
					UNSHORT y;
					UNSHORT linestogo;

					UNSHORT packetword;

					UNSHORT packets;

					SIBYTE count;
					UNBYTE color1;
					UNBYTE color2;


					/* get starting y pos */

					y = 0;


					/* get lines to go */

					linestogo = *( ( UNSHORT * ) animworkptr );
					animworkptr += 2L;



					/* work on lines */

					if ( linestogo )
					{
						do
						{

							packets = 0xffff;

							do
							{
								packetword = *( ( UNSHORT * ) animworkptr );
								animworkptr += 2L;

								switch ( ( packetword & 0xc000 ) >> 14 )
								{
									case 0:
									{
										packets = packetword;

										break;
									}

									case 2:
									{
										flc_setpixel( width-1, y, packetword & 0xff );

										packets = *( ( UNSHORT * ) animworkptr );

										animworkptr += 2L;

										break;
									}

									case 3:
									{
										y -= packetword;

										break;
									}
								}
							}
							while ( packets == 0xffff );


							x = 0;


							if ( packets == 0 )
							{
								y++;
							}
							else
							{


								x += ( UNSHORT ) ( *animworkptr++ );


								do
								{

									count = *animworkptr++;


									if ( count >= 0 )
									{
										while ( count-- )
										{
											flc_setpixel( x++, y, *animworkptr++ );
											flc_setpixel( x++, y, *animworkptr++ );

										}
									}
									else
									{
										color1 = *animworkptr++;
										color2 = *animworkptr++;

										count = -count;

										while ( count-- )
										{
											flc_setpixel( x++, y, color1 );
											flc_setpixel( x++, y, color2 );
										}
									}


									packets--;


									if ( packets )
										x += ( UNSHORT ) ( *animworkptr++ );


								}
								while ( packets );


								y++;

							}



							linestogo--;


						} while ( linestogo != 0 );

					}

					break;
				}



				/* SIBYTE aligned delta compression chunk */

				case 12:
				{

					#ifdef LANGSAM



						/* locals */

						UNSHORT x;
						UNSHORT y;
						UNSHORT linestogo;
						UNBYTE packets;


						/* get starting y pos */

						y = *( ( UNSHORT * ) animworkptr );
						animworkptr += 2L;


						/* get lines to go */

						linestogo = *( ( UNSHORT * ) animworkptr );
						animworkptr += 2L;




						if ( linestogo )
						{
							do
							{

								packets = ( *animworkptr++ );


								x = 0;



								if ( packets != 0 )
								{


									x += ( UNSHORT ) ( *animworkptr++ );


									do
									{
										register SIBYTE count;


										count = *animworkptr++;


										if ( count >= 0 )
										{
											while ( count-- )
												flc_setpixel( x++, y, *animworkptr++ );
										}
										else
										{
											register UNBYTE color;

											color = *animworkptr++;

											count = -count;

											while ( count-- )
												flc_setpixel( x++, y, color );
										}


										packets--;


										if ( packets )
											x += ( UNSHORT ) ( *animworkptr++ );


									} while ( packets );
								}


								y++;


								linestogo--;


							} while ( linestogo != 0 );
						}



					#else


						/* locals */

						/* lines to go in chunk */

						UNSHORT linestogo;

						/* packets in line */

						UNBYTE packets;

						/* pointer to base line in video ram */

						UNBYTE far * vb;

						/* work pointer for video ram */

						register UNBYTE far * vw;



						/* get video ram base pointer */

						vb = ( ( UNBYTE far * ) 0xa0000000 ) + *( ( UNSHORT * ) animworkptr ) * 320;
						animworkptr += 2;


						/* get lines to go */

						linestogo = *( ( UNSHORT * ) animworkptr );
						animworkptr += 2;



						/* any line to go ? */

						if ( linestogo )
						{
							/* yes ! */

							/* work on all lines */

							do
							{
								/* get number of packets */

								packets = ( *animworkptr++ );


								/* any packet for this line ? */

								if ( packets )
								{
									/* yes ! */


									/* set video work pointer */

									vw = vb;


									/* startup skip */

									vw += ( UNSHORT ) ( *animworkptr++ );



									/* for on all packets */

									do
									{
										/* local vars */

										register SIBYTE count;


										/* load counter */

										count = *animworkptr++;


										/* counter positive ? */

										if ( count >= 0 )
										{
											/* yes: copy string */

											while ( count-- )
									  			*vw++ = *animworkptr++;
										}
										else
										{
											/* no: fill with byte */

											register UNBYTE color;

											color = *animworkptr++;

									  		count = -count;

									  		while ( count-- )
									  			*vw++ = color;
										}


										/* dec number of packets */

										packets--;


										/* add skip byte */

										if ( packets )
											vw += ( UNSHORT ) ( *animworkptr++ );


									}
									while ( packets );
								}


								/* next line in video ram */

								vb += 320;


								/* dec lines to go */

								linestogo--;


							}
							while ( linestogo );

						}


					#endif


					break;
				}


				/* black chunk */

				case 13:
				{
					return( FLC_ANIM_UNKNOWNCHUNK );
					// break;
				}


				/* byte run length compression chunk */

				case 15:
				{
					UNSHORT x;
					UNSHORT y;
					register SIBYTE count;

					for ( y=0; y<height; y++ )
					{
						x=0;
						animworkptr++;

						do
						{
							count = *animworkptr++;

							if ( count < 0 )
							{
								count =- count;

								#ifdef LANGSAM

							 		while ( count-- )
										flc_setpixel( x++, y, *animworkptr++ );

								#else

									{
										register UNBYTE * v = ( ( UNBYTE * ) 0xa0000000 ) + x + y * 320;

										x += count;

										while ( count-- )
											*v++ = *animworkptr++;
									}

								#endif
							}
							else
							{
								register UNBYTE color;

								color = *animworkptr++;

								#ifdef LANGSAM

									while ( count-- )
										flc_setpixel( x++, y, color );

								#else

									{
										register UNBYTE * v = ( ( UNBYTE * ) 0xa0000000 ) + x + y * 320;

										x += count;

										while ( count-- )
											*v++ = color;
									}

								#endif

							}


						} while ( x<width );
					}

					break;
				}


				/* copy chunk */

				case 16:
				{
					return( FLC_ANIM_UNKNOWNCHUNK );
					// break;
				}



				/* stamp chunk */

				case 18:
				{
					// return( FLC_ANIM_UNKNOWNCHUNK );
					break;
				}



				/* unknown flc chunk */

				default:
				{
					break;
				}

			}


			/* set pointer to next chunk */

			animworkptr = nextchunkptr;



			/* dec chunks */

			chunks--;



		} while ( chunks );

	}



	/* inc actframe */

	flc_struct->actframe++;

	if ( flc_struct->actframe == flc_struct->frames )
	{
		flc_struct->actframe = 0;

		returnflag |= FLC_ANIM_DONE;
	}


	/* return code */

	return( returnflag );
}






