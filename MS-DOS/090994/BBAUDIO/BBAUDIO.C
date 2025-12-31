/************
 * NAME     :  BBAUDIO.C	        AUTOR :  BE , BlueByte
 * PROJECT  :  PCLIB
 * DATE     :  20.01.94 12:19     START :  20.01.94 12:19
 *
 * FUNCTION :  Port for all programs, Audio Standard functions.
 * SYNOPSIS :
 * NOTES    :
 * BUGS     :
 * SEE ALSO :
 * UPDATE   :
 ************/

/*
 ** defines *******************************************************************
 */

/*
#define INITSNDDRIVER_DEBUG
#define SETEFFECT_DEBUG
#define SOUNDMANAGER_DEBUG
#define STOPALLEFFECTS_DEBUG
#define GETFREEAILCHANNEL_DEBUG
#define STARTMUSIC_DEBUG
#define STOPMUSIC_DEBUG
#define SEARCHTIMBRE_DEBUG
#define MUSICSTATUS_DEBUG
#define STOPALLMUSIC_DEBUG
*/

/*
 ** #includes *****************************************************************
 */

#include <stdio.h>			/* BC includes */
#include <dos.h>

#include <bbdef.h>			/* BB includes */
#include <bbmath.h>
#include <bbvga.h>

#include "bbaudio.h"

#include <ail.h>				/* other */

/*
 ** global var and data *******************************************************
 */

/* Treiber Strukturen:
 */
struct bbsnddrv bbsnddrv[ BBSNDDRV_MAX ];

/* Allgemeine error variable:
 */
short	bbsnd_error = 0;

/* Steuerstruktur:
 */
struct bbsoundctrl bbsndctrl =
{
BBSNDSTAT_NULL,
BBSND_NOT_INITIALIZED,
0,
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InitSndDriver
 * FUNCTION  : Installiert einen Sound Treiber im System.
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 20.01.94 13:01:29
 * LAST      : 20.01.94 13:01:29
 * INPUTS    : struct bbsnddrv *reqdrv : ptr to tabelle, io,irq,dma,address
 *                                       werte.
 *             ubyte huge *memptr : ptr to memory, alloc space for driver
 *                                  internal structures.
 * RESULT    : struct bbsnddrv *bbsnd : ptr to sound struct, handle for this drv
 *                                      NULL = no free handle or error. See error variable
 *                                      .length gives used memory.
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct bbsnddrv *
InitSndDriver( struct bbsnddrv *reqdrv, ubyte huge *memptr )
{
struct bbsnddrv *bbsnd = NULL;

drvr_desc *desc;

register short i;

short isdsp = FALSE;
short isxmi = FALSE;

long total = 0L;
long length;

/* Suche freien Treiber:
 */
for(i=0; i<BBSNDDRV_MAX; i++)
	{
	if (bbsnddrv[i].status & BBSNDDRV_IS_FREE)
		{	bbsnd = &bbsnddrv[i];	break;	}
	}

if (bbsnd == NULL)
	{
	bbsnd_error = BBSNDERR_NOFREEBBDRV;
	return( NULL );
	}

/* Setze Adresse von Treiber. MUSS auf offset 0 liegen !
 */
reqdrv->drvadr = NORMPTR( reqdrv->drvadr );

if (FP_OFF(reqdrv->drvadr) != 0)
	{
	bbsnd_error = BBSNDERR_DRVNOTONOFFSETNULL;
	return( NULL );
	}

/* Initialisiere Treiber:
 */
if ((bbsnd->aildrv=AIL_register_driver( reqdrv->drvadr )) == -1)
	{
	bbsnd_error = BBSNDERR_DRVNOTCOMPATIBLE;
	return( NULL );
	}

/*
 ** Beschreibe Treiber, hole werte ********************************************
 */

desc = AIL_describe_driver( bbsnd->aildrv );

if (desc->drvr_type == DSP_DRVR)
	{
	isdsp = TRUE;

	bbsnd->status |= BBSNDDRV_DSPDRV;

	#ifdef INITSNDDRIVER_DEBUG
		printf("BBAUDIO: INIT DRV: IS DSP DRIVER\n");
	#endif
	}
if (desc->drvr_type == XMIDI_DRVR)
	{
	isxmi = TRUE;

	#ifdef INITSNDDRIVER_DEBUG
		printf("BBAUDIO: INIT DRV: IS XMIDI DRIVER\n");
	#endif
	}

#ifdef INITSNDDRIVER_DEBUG
	printf("BBAUDIO: INIT DRV: DEFS : IO %d IRQ %d DMA %d DRQ %d\n",desc->default_IO,desc->default_IRQ,desc->default_DMA,desc->default_DRQ );
	printf("BBAUDIO: INIT DRV: DEFS : EXT %s\n",desc->data_suffix );
#endif

/* Ist der Treiber ok mit diesen Werten ?
 */
if (AIL_detect_device( bbsnd->aildrv, reqdrv->io, reqdrv->irq, reqdrv->dma, reqdrv->drq ) == 0)
	{
	#ifdef INITSNDDRIVER_DEBUG
		printf("BBAUDIO: INIT DRV FAILED !! : io %u irq %u dma %u drq %u\n", reqdrv->io, reqdrv->irq, reqdrv->dma, reqdrv->drq );
	#endif

	bbsnd_error = BBSNDERR_DRVNOTDETECTED;
	return( NULL );
	}

/*
 ** INIT AIL DRV **************************************************************
 */

AIL_init_driver( bbsnd->aildrv, reqdrv->io, reqdrv->irq, reqdrv->dma, reqdrv->drq );

/*
 ** BUFFER FUER MIDI **********************************************************
 */

if (isxmi)
	{
	/* Alloziere Tabelle fuer songs, interne Daten des Midi Treibers
	 */
	length = (long) AIL_state_table_size( bbsnd->aildrv );

	bbsnd->statetable = memptr;
	total  += length;
	memptr += length;

	#ifdef INITSNDDRIVER_DEBUG
		printf("BBAUDIO: INIT DRV: STATE TABLE %ld\n",length );
	#endif

	/* Alloziere platz fuer instrument Daten falls nicht auf der Karte
	 */
	length = (long) AIL_default_timbre_cache_size( bbsnd->aildrv );

	/* Falls timbre gebraucht werden alloziere sie und sage es dem Treiber
	 */
	if (length > 0)
		{
		AIL_define_timbre_cache( bbsnd->aildrv, memptr, (short) length );

		bbsnd->timbrecache = memptr;
		total  += length;
		memptr += length;

		#ifdef INITSNDDRIVER_DEBUG
			printf("BBAUDIO: INIT DRV: TIMBRE CACHE %ld\n",length );
		#endif
		}
	else
		{
		bbsnd->timbrecache = NULL;

		#ifdef INITSNDDRIVER_DEBUG
			printf("BBAUDIO: INIT DRV: NO TIMBRE CACHE NEEDED !\n");
		#endif
		}

	bbsnd->length = total;
	}

/*
 ** INIT BBAUDIO **************************************************************
 */

/* Clear status. Store all values
 */
bbsnd->status = BBSNDDRV_INIT;
bbsnd->drvadr = reqdrv->drvadr;
bbsnd->io			= reqdrv->io;
bbsnd->irq		= reqdrv->irq;
bbsnd->dma		= reqdrv->dma;
bbsnd->drq		= reqdrv->drq;

/* Setze flags hier nachj  set
 */
if (isdsp)
	bbsnd->status |= BBSNDDRV_DSPDRV;
if (isxmi)
	bbsnd->status |= BBSNDDRV_XMIDRV;

return( bbsnd );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ClearSndDriver
 * FUNCTION  : Entfernt einen Treiber
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 20.01.94 13:01:42
 * LAST      : 20.01.94 13:01:42
 * INPUTS    : none
 * RESULT    : none
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
ClearSndDriver( struct bbsnddrv *snddrv )
{
/* Nicht geladen ? Nicht entfernen !
 */
if ( !(snddrv->status & BBSNDDRV_LOADED))
	return;

/* Stoppe sofort diesen Treiber:
 */
AIL_shutdown_driver( snddrv->aildrv, NULL );

/* Gebe handle frei:
 */
AIL_release_driver_handle( snddrv->aildrv );

snddrv->status = BBSNDDRV_IS_FREE;
snddrv->drvadr = NULL;
snddrv->aildrv = -1;
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : InstallSndSystem
 * FUNCTION  : Initialisiert die Sound Routine
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 20.01.94 13:01:29
 * LAST      : 20.01.94 13:01:29
 * INPUTS    : none
 * RESULT    : 0=ok, -1 = error
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

short
InstallSndSystem( void )
{
AIL_startup();

return( 0 );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : RemoveSndSystem
 * FUNCTION  : Schliesst die Sound Routine.
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 20.01.94 13:01:42
 * LAST      : 20.01.94 13:01:42
 * INPUTS    : none
 * RESULT    : none
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
RemoveSndSystem( void )
{
AIL_shutdown( NULL );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SoundManager
 * FUNCTION  : Steuerroutine fuer alle Effekte und Musik.
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 24.01.94 11:36:09
 * LAST      : 24.01.94 11:36:09
 * INPUTS    : none
 * RESULT    : short wert : 0
 * BUGS      :
 * NOTES     : Sorgt fuer Loop der Effekte.
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

short
SoundManager( void )
{
struct bbsnddrv 		*snddrv;
struct bbsndeffect	*fx;
struct bbsndmusic	  *music;

register short i;

short digiplay	= FALSE;
short back			= 0;

short j;
short	channel;

/*
 **  Ist der Manager schon initialisiert ? ************************************
 */

if (bbsndctrl.initialized == BBSND_NOT_INITIALIZED)
	{
	for(i=0; i<BBSNDDRV_MAX; i++)
		{
		/* Hole Basis Adresse von diesem Treiber:
		 */
		snddrv = &bbsnddrv[ i ];

		snddrv->drvadr = NULL;
		snddrv->aildrv = -1;
		snddrv->status = BBSNDDRV_IS_FREE;

		/* Alle Effekte auf null:
		 */
		for(j=0; j<BBSNDEFFECT_MAX; j++)
			{
			fx = &(snddrv->bbsndeffect[j]);

			fx->status = BBSNDDRV_FXSTAT_IS_FREE;
			}

		/* Alle musiken auf null:
		 */
		for(j=0; j<BBSNDMUSIC_MAX; j++)
			{
			music = &(snddrv->bbsndmusic[j]);

			music->status = BBSNDDRV_MUSICSTAT_IS_FREE;
			}
		}

	bbsndctrl.initialized = BBSND_INITIALIZED;
	}

/*
 ** EFFEKTE *******************************************************************
 */

/* Gehe Alle Treiber durch, schaue ob es fuer Sie Aufgaben gibt:
 */
for(i=0; i<BBSNDDRV_MAX; i++)
	{
	/* Hole Basis Adresse von diesem Treiber:
	 */
	snddrv = &bbsnddrv[ i ];

	/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
	 */
	if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
		{
		/*
		#ifdef SOUNDMANAGER_DEBUG
			printf("BBAUDIO: MANAGER : DRV %d STOPPED OR FREE !\n",i );
		#endif
		*/

		continue;
		}

	/* Ist dieser Treiber kein DSP driver ? Keine Effekte !!
	 */
	if (! (snddrv->status & BBSNDDRV_DSPDRV))
		continue;

	/* Gehe die Effektliste des Treibers durch:
	 */
	for(j=0; j<BBSNDEFFECT_MAX; j++)
		{
		fx = &(snddrv->bbsndeffect[j]);

		/* Freier Slot ?
		 */
		if (fx->status & BBSNDDRV_FXSTAT_IS_FREE)
			continue;

		/* Spielt ? Ist zuende ?
		 */
		if (fx->status & BBSNDDRV_FXSTAT_PLAYING)
			{
			if (AIL_sound_buffer_status( snddrv->aildrv, fx->channel ) == DAC_DONE)
				{
				#ifdef SOUNDMANAGER_DEBUG
					printf("BBAUDIO: FX %d PLAY ENDED !\n",j );
				#endif

				fx->status &= ~BBSNDDRV_FXSTAT_PLAYING;
				fx->count  -= 1;
				}
			}

		/* Dieser Effect spielt nicht ? Versuche ihn zu initialisieren,
		 * oder loesche ihn, oder setze ihn auf die Warteliste.
		 */
		if (! (fx->status & BBSNDDRV_FXSTAT_PLAYING))
			{
			/* Nochmal Starten ? Oder Loeschen ?
			 */
			if (fx->count <= 0)
				{
				fx->status = BBSNDDRV_FXSTAT_IS_FREE;

				#ifdef SOUNDMANAGER_DEBUG
					printf("BBAUDIO: FX %d ENDED, DELETED !\n",j );
				#endif
				}
			else
				{
				/* Suche freien Kanal fuer diesen Effect, CRASH effect:
				 */
				if (fx->status & BBSND_FXCRASH)
					{
					#ifdef SOUNDMANAGER_DEBUG
						printf("BBAUDIO: STOP EFFECTS FOR DRV %d (FX %d)\n",i,j );
					#endif
					Stopalleffects( snddrv );

					if ((channel=Getfreeailfxchannel( snddrv )) == -1)
						{
						fx->status = BBSNDDRV_FXSTAT_IS_FREE;

						#ifdef SOUNDMANAGER_DEBUG
							printf("BBAUDIO: NO CHANNEL FOR CRASH FX %d, FORGET IT !\n",j );
						#endif

						continue;
						}
					}
				else
					{
					if ((channel=Getfreeailfxchannel( snddrv )) == -1)
						{
						#ifdef SOUNDMANAGER_DEBUG
							printf("BBAUDIO: NO CHANNEL FOR FX %d !\n",j );
						#endif

						/* Falls kein Kanal da: Soll der Effekt warten ?
						 */
						if ( !(fx->status & BBSND_FXWAIT))
							{
							fx->status = BBSNDDRV_FXSTAT_IS_FREE;

							#ifdef SOUNDMANAGER_DEBUG
								printf("BBAUDIO: FX %d DELETED !\n",j );
							#endif
							}

						continue;
						}
					}

				/* Setze diesen Effect:
				 */
				fx->channel = channel;

				#ifdef SOUNDMANAGER_DEBUG
					if ((channel > 1) || (channel < 0))
						{
						printf("BBAUDIO: WRONG FX CHANNEL ! (%d)\n",channel );
						continue;
						}
				#endif

				AIL_set_digital_playback_volume( snddrv->aildrv, fx->volume );
				AIL_index_VOC_block( snddrv->aildrv, fx->memptr, -1, &fx->ailbuff );
				AIL_register_sound_buffer( snddrv->aildrv, fx->channel, &fx->ailbuff );
				AIL_format_sound_buffer( snddrv->aildrv, &fx->ailbuff );

				/*
				AIL_play_VOC_file( snddrv->aildrv, fx->memptr, -1 );
				*/
				fx->status |= BBSNDDRV_FXSTAT_PLAYING;

				digiplay = TRUE;

				#ifdef SOUNDMANAGER_DEBUG
					printf("BBAUDIO: START FX NR %d CHN %d DRV %d\n",j,channel,i );
				#endif

				break;
				}
			}
		}	/* for j */

	/* Starte Sound Effekte:
	 */
	if (digiplay)
		{
		AIL_start_digital_playback( snddrv->aildrv );
		digiplay = FALSE;
		}
	}	/* for i*/


/*
 ******************************************************************************
 */

/*
SETCOLOR( 0, 0x0,0x0,0x0 );
*/

return( back );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Seteffect
 * FUNCTION  : Sets a effect for playing in queue
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 22.01.94 12:35:31
 * LAST      : 22.01.94 12:35:31
 * INPUTS    : struct bbsnddrv *snddrv : ptr to snddrviver for output
 *             ubyte huge *memptr : Speicher adresse des effekts
 *             short volume : 0-127 Lautstaerke
 *             short count : Anzahl Wiederholungen
 *             ushort status : Spezielle Anforderungen an Effekt:
 * RESULT    : short fxnum : Effekt Nummer zur Referenz, oder -1 = wird nicht
 *                           gespielt.
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

short
Seteffect( struct bbsnddrv *snddrv, ubyte huge *memptr, short volume, short count, ushort status )
{
struct bbsndeffect huge *fx;

register short j;

/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
 */
if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
	{
	#ifdef SETEFFECT_DEBUG
		printf("BBAUDIO: SETEFFECT - DRV FREE OR STOPPED !\n");
	#endif

	return( -1 );
	}

/* Gehe die Effektliste des Treibers durch:
 */
for(j=0; j<BBSNDEFFECT_MAX; j++)
	{
	fx = &(snddrv->bbsndeffect[j]);

	/* Effekt bereits gesetzt ?
 	 */
	if ( !(fx->status & BBSNDDRV_FXSTAT_IS_FREE))
		continue;

	fx->status	= status;
	fx->count		= count;
	fx->volume  = volume;
	fx->memptr  = memptr;

	#ifdef SETEFFECT_DEBUG
		printf("BBAUDIO: SET FX STAT %d CNT %d VOL %d\n",status,count,volume );
	#endif

	return( j );
	}	/* for j */

#ifdef SETEFFECT_DEBUG
	printf("BBAUDIO: NO FREE FX QUEUE !\n");
#endif

return( -1 );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Getfreeailfxchannel
 * FUNCTION  : Sucht fuer diesen Treiber den naechsten freien fx Kanal.
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 24.01.94 13:28:50
 * LAST      : 24.01.94 13:28:50
 * INPUTS    : struct bbsnddrv *snddrv : ptr to snddrviver.
 * RESULT    : short channel : kanal nummer oder -1 = kein Kanal frei.
 * BUGS      :
 * NOTES     : Interne Funktion
 * SEE ALSO  : SoundManager
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

short
Getfreeailfxchannel( struct bbsnddrv *snddrv )
{
register short i;

/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
 */
if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
	{
	#ifdef GETFREEAILCHANNEL_DEBUG
		printf("BBAUDIO: GET FREE AIL - DRV FREE OR STOPPED !\n");
	#endif

	return( -1 );
	}

/* Suche freien Kanal:
 */
for(i=0; i<2; i++)
	{
	if (AIL_sound_buffer_status( snddrv->aildrv, i ) == DAC_DONE)
		return( i );
	}

#ifdef GETFREEAILCHANNEL_DEBUG
	printf("BBAUDIO: GET FREE AIL - REALLY NO CHANNEL !\n");
#endif

return( -1 );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Stopalleffects
 * FUNCTION  : Haelt sofort alle Effects an die im Moment spielen:
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 24.01.94 16:01:35
 * LAST      : 24.01.94 16:01:35
 * INPUTS    : struct bbsnddrv *snddrv : ptr to snddrviver.
 * RESULT    : none
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Stopalleffects( struct bbsnddrv *snddrv )
{
struct bbsndeffect *fx;

register short i;

short	breakflag = FALSE;

/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
 */
if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
	{
	#ifdef STOPALLEFFECTS_DEBUG
		printf("BBAUDIO: STOP FX - DRV FREE OR STOPPED !\n");
	#endif

	return;
	}

for(i=0; i<2; i++)
	{
	if (AIL_sound_buffer_status( snddrv->aildrv, i ) == DAC_PLAYING)
		breakflag = TRUE;
	}

if (breakflag)
	{
	/*
	SETCOLOR( 0, 0xff,0x0,0x0 );
	 */

	AIL_stop_digital_playback( snddrv->aildrv );

	/*
	SETCOLOR( 0, 0x0,0xff,0x0 );
	*/
	}

for(i=0; i<BBSNDEFFECT_MAX; i++)
	{
	fx = &snddrv->bbsndeffect[i];

	/* Effekt bereits gesetzt ?
 	 */
	if ( !(fx->status & BBSNDDRV_FXSTAT_PLAYING))
		continue;

	fx->status = BBSNDDRV_FXSTAT_IS_FREE;
	fx->count  = 0;

	#ifdef STOPALLEFFECTS_DEBUG
		printf("BBAUDIO: FX %d STOPPED AND DELETED !\n",i);
	#endif
	}	/* for i */
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Stopmusic
 * FUNCTION  : Haelt eine musik sofort an und gibt handle frei.
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 27.01.94 15:38:18
 * LAST      : 27.01.94 15:38:18
 * INPUTS    : struct bbsnddrv *snddrv : ptr to snddrviver.
 *             short musicnum : nummer der musik die angehalten werden soll
 * RESULT    : none
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Stopmusic( struct bbsnddrv *snddrv, short musicnum )
{
/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
 */
if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
	{
	#ifdef STOPMUSIC_DEBUG
		printf("BBAUDIO: STOPMUSIC - DRV FREE OR STOPPED !\n");
	#endif

	return;
	}

AIL_stop_sequence( snddrv->aildrv, snddrv->bbsndmusic[ musicnum ].ailseq );
AIL_release_sequence_handle( snddrv->aildrv, snddrv->bbsndmusic[ musicnum ].ailseq );

snddrv->bbsndmusic[ musicnum ].status = BBSNDDRV_MUSICSTAT_IS_FREE;
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Startmusic
 * FUNCTION  : Startet eine musik
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 27.01.94 16:01:24
 * LAST      : 27.01.94 16:01:24
 * INPUTS    : struct bbsnddrv *snddrv : ptr to snddrviver.
 * RESULT    : short musicnum : nummer der musik, -1 fuer error
 * BUGS      :
 * NOTES     : Benoetigt in snddrv .timbrelib ptr gesetzt !!
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

short
Startmusic( struct bbsnddrv *snddrv, ubyte huge *musicdata )
{
struct bbsndmusic	*music;

ushort treq;
ushort patch;
ushort bank;
ushort channel;

ubyte  huge  *timptr;

long	 count = 0;

register short j;

/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
 */
if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
	{
	#ifdef STARTMUSIC_DEBUG
		printf("BBAUDIO: STARTMUSIC - DRV FREE OR STOPPED !\n");
	#endif

	return( -1 );
	}

/* Gehe die Effektliste des Treibers durch:
 */
for(j=0; j<BBSNDMUSIC_MAX; j++)
	{
	music = &(snddrv->bbsndmusic[j]);

	/* Musik frei ?
 	 */
	if ( !(music->status & BBSNDDRV_FXSTAT_IS_FREE))
		continue;

	/*
	SETCOLOR( 0, 0xff,0x0,0x0 );
 	 */

	/* Setze ail sequence:
	 */
	if ((music->ailseq=AIL_register_sequence( snddrv->aildrv, musicdata, 0, snddrv->statetable, &(snddrv->ctrltable[0]) )) == -1)
		{
		#ifdef STARTMUSIC_DEBUG
			printf("BBAUDIO: MUSIC !!NOT!! REGISTERED !\n",j );
		#endif

		return( -1 );
		}

	#ifdef STARTMUSIC_DEBUG
		printf("BBAUDIO: MUSIC AT %d REGISTERED !\n",j );
	#endif

	/*
	SETCOLOR( 0, 0x0,0xff,0x0 );
   */

	/* Suche alle Instrumente, falls welche angefordert werden:
	 */
	while( (treq=AIL_timbre_request( snddrv->aildrv, music->ailseq)) != 0xFFFF)
		{
		/* bank liegt im high byte, patch im low byte. Beide werden zum suchen in
		 * der timbre library gebraucht:
		 */
		bank  = treq / 256;
		patch = treq % 256;

		#ifdef STARTMUSIC_DEBUG
			printf("BBAUDIO: REQUEST TIMBRE: (B %d : P %d)!\n", bank,patch );
		#endif

		#ifdef STARTMUSIC_DEBUG
		count++;

		if (count == 10000)
			{
			printf("BBAUDIO: TIMBRE NOT FOUND (MAIN LOOP)\n", bank,patch );
			break;
			}
		#endif

		/* Braucht diese Karte diesen Timbre aus der lib ?
		 */
		if (AIL_timbre_status( snddrv->aildrv, bank, patch ) != 0)
			{
			#ifdef STARTMUSIC_DEBUG
				printf("BBAUDIO: TIMBRE IS ON BOARD (B %d : P %d)!\n", bank,patch );
			#endif

			break;
			}

		if ((timptr=Searchtimbre( snddrv->timbrelib, bank, patch )) != NULL)
			{
			AIL_install_timbre( snddrv->aildrv, bank, patch, timptr );

			#ifdef STARTMUSIC_DEBUG
				printf("BBAUDIO: TIMBRE FOUND AND INSTALLED: B %d P %d\n",bank,patch );
			#endif
			}
		else
			{
			#ifdef STARTMUSIC_DEBUG
				printf("BBAUDIO: TIMBRE NOT FOUND !\n");
			#endif

			/* Ausstieg !!
			 */
			break;
			}
		}

	if ((channel=AIL_lock_channel( snddrv->aildrv )) != 0)
		{
		AIL_send_channel_voice_message( snddrv->aildrv, 121, 0,0 );
		AIL_release_channel( snddrv->aildrv, channel );
		}

	AIL_start_sequence( snddrv->aildrv, music->ailseq );

	#ifdef STARTMUSIC_DEBUG
		printf("BBAUDIO: MUSIC AT %d STARTED !\n",j );
	#endif

	music->status = BBSNDDRV_MUSICSTAT_PLAYING;

	return( j );
	}	/* for j */

return( -1 );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Searchtimbre
 * FUNCTION  : Sucht im Speicher nach einem TIMBRE.
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 27.01.94 16:34:50
 * LAST      : 27.01.94 16:34:50
 * INPUTS    : ubyte huge *memptr : ptr auf Speicher
 *             ushort bank : midi bank fuer instrument
 *             ushort patch : patch fuer midi bank
 * RESULT    : ubyte huge *memptr : ptr to timbre or null (not found)
 * BUGS      :
 * NOTES     : Interne Funktion
 * SEE ALSO  : Startmusic
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

ubyte huge *
Searchtimbre( ubyte huge *memptr, ushort bank, ushort patch )
{
ubyte	huge *orgmemptr = memptr;

short count = 0;

struct bbsndgtlhead *gtl = (struct bbsndgtlhead *) memptr;

while((gtl->bank != bank) || (gtl->patch != patch))
	{
	if (gtl->bank == (ubyte) -1)
		return( NULL );

	#ifdef SEARCHTIMBRE_DEBUG
		count++;
		if (count > 1000)
			{
			printf("BBAUDIO: TIMBRE NOT FOUND !\n");
			break;
			}
	#endif

	gtl++;
	}

orgmemptr += gtl->offset;

#ifdef SEARCHTIMBRE_DEBUG
	printf("BBAUDIO: FOUND TIMBRE at %ld %d:%d\n",gtl->offset,bank,patch);
#endif

return( orgmemptr );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Musicstatus
 * FUNCTION  : Liefere Status dieser Musik
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 28.01.94 11:36:40
 * LAST      : 28.01.94 11:36:40
 * INPUTS    : struct bbsnddrv *snddrv : ptr to snddrviver
 *             short musicnum : nummer der Musik
 * RESULT    : ushort status : status flag fuer musik:
 *                             BBSNDMUSIC_STATUS_STOPPED
 *                             BBSNDMUSIC_STATUS_PLAYING
 *                             BBSNDMUSIC_STATUS_DONE
 *
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

ushort
Musicstatus( struct bbsnddrv *snddrv, short musicnum )
{
struct bbsndmusic	*music;

ushort ailstat;

/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
 */
if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
	{
	#ifdef MUSICSTATUS_DEBUG
		printf("BBAUDIO: MUSICSTATUS - DRV FREE OR STOPPED !\n");
	#endif

	return( 0 );
	}

music = &(snddrv->bbsndmusic[ musicnum ]);

/* Musik frei ?
 */
if (music->status & BBSNDDRV_MUSICSTAT_IS_FREE)
	return( 0 );

ailstat = AIL_sequence_status( snddrv->aildrv, music->ailseq );

return( ailstat );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Musicvolume
 * FUNCTION  : Musik Lautstaerke setzen und ein/ausblenden.
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 31.01.94 15:37:45
 * LAST      : 31.01.94 15:37:45
 * INPUTS    : struct bbsnddrv *snddrv : ptr to snddrviver
 *             short musicnum : nummer der Musik
 *             short volume : 0-100 Prozent Lautstaerke
 *             short blend : Zeit in millisekunden, ein ausblenden
 * RESULT    : none
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Musicvolume( struct bbsnddrv *snddrv, short musicnum, short volume, short blend )
{
struct bbsndmusic	*music;

/* Nur bis 90 prozent erlauben !
 */
if (volume > 100)
	volume = 100;

/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
 */
if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
	{
	#ifdef MUSICSTATUS_DEBUG
		printf("BBAUDIO: MUSICSTATUS - DRV FREE OR STOPPED !\n");
	#endif

	return;
	}

music = &(snddrv->bbsndmusic[ musicnum ]);

AIL_set_relative_volume( snddrv->aildrv, music->ailseq, volume, blend );
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Stopallmusic
 * FUNCTION  : Haelt sofort alle Musiken an
 * FILE      : BBAUDIO.C
 * AUTHOR    : ICEMAN
 * FIRST     : 07.02.94 18:56
 * LAST      : 07.02.94 18:56
 * INPUTS    : struct bbsnddrv *snddrv : ptr to snddrviver.
 * RESULT    : none
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Stopallmusic( struct bbsnddrv *snddrv )
{
struct bbsndmusic *music;

register short i;

short	breakflag = FALSE;

/* Ist dieser Treiber initialisiert ? Oder inaktiv ?
 */
if ((snddrv->status & BBSNDDRV_IS_FREE) || (snddrv->status & BBSNDDRV_STOPPED))
	{
	#ifdef STOPALLMUSIC_DEBUG
		printf("BBAUDIO: STOP MUSIC - DRV FREE OR STOPPED !\n");
	#endif

	return;
	}

for(i=0; i<BBSNDMUSIC_MAX; i++)
	{
	music = &snddrv->bbsndmusic[i];

	AIL_stop_sequence( snddrv->aildrv, music->ailseq );
	AIL_release_sequence_handle( snddrv->aildrv, music->ailseq );

	music->status = BBSNDDRV_MUSICSTAT_IS_FREE;

	#ifdef STOPALLMUSIC_DEBUG
		printf("BBAUDIO: MUSIC %d STOPPED AND DELETED !\n",i);
	#endif
	}	/* for i */
}

/* #FUNCTION END# */

