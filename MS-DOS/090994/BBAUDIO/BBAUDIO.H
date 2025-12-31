/************
 * NAME     :  BBAUDIO.H          AUTOR :  BE , BlueByte
 * PROJECT  :
 * DATE     :  20.01.94 12:33     START :  20.01.94 12:33
 * NOTES    :
 * BUGS     :
 * SEE ALSO :
 * UPDATE   :
 ************/

#ifndef BBAUDIO_H
 #define BBAUDIO_H

/*
 ** #includes *****************************************************************
 */

#include <bbdef.h>			/* BB includes */

#include <ail.h>

/*
 ** error meldungen ***********************************************************
 */

enum BBSNDERROR
{
/* Kein Fehler:
 */
BBSNDERR_NULL=0,

/* Kein Treiber mehr im System frei:
 */
BBSNDERR_NOFREEBBDRV,

/* Treiber fuer AIL liegt nicht auf offset 0:
 */
BBSNDERR_DRVNOTONOFFSETNULL,

/* Treiber fuer AIL liegt nicht auf offset 0:
 */
BBSNDERR_DRVNOTCOMPATIBLE,

/* Treiber fuer AIL nicht gefunden:
 */
BBSNDERR_DRVNOTDETECTED,
};

/*
 ** Effetcs *******************************************************************
 */

/* Maximale Anzahl wartender effekte:
 */
#define BBSNDEFFECT_MAX		4

struct bbsndeffect
{
ushort     status;

sound_buff ailbuff;

ubyte	huge *memptr;

short 		 volume;
short			 count;

short			 channel;
};

/*
 ** music *********************************************************************
 */

/* Maximal 8 Musiken gleichzeitig
 */
#define BBSNDMUSIC_MAX		8

/* Status der aktuellen Musik:
 */
#define BBSNDMUSIC_STATUS_STOPPED		0
#define BBSNDMUSIC_STATUS_PLAYING   1
#define BBSNDMUSIC_STATUS_DONE      2

struct bbsndmusic
{
ushort     	status;

HSEQUENCE		ailseq;
};

/* Header fuer Global Timbre Library
 */
struct bbsndgtlhead
{
ubyte patch;
ubyte bank;
ulong offset;
};

/*
 ** snddrv ********************************************************************
 */

/* Maximal 4 Sound Treiber gleichzeitig.
 */
#define BBSNDDRV_MAX		4

/* Status flags fuer Treiber:
 */
#define BBSNDDRV_INIT				0					/* Ist initialisiert		*/
#define BBSNDDRV_IS_FREE		(1<<0)    /* Ist frei 						*/
#define BBSNDDRV_STOPPED		(1<<1)    /* Momentan inaktiv			*/
#define BBSNDDRV_LOADED 		(1<<2)    /* Treiber geladen 			*/
#define BBSNDDRV_DSPDRV	 		(1<<3)    /* DSP Treiber 	 				*/
#define BBSNDDRV_XMIDRV	 		(1<<4)    /* XMIDI Treiber 	 		 	*/

/* Status flag fuer Effekte
 */
#define BBSNDDRV_FXSTAT_NULL		0
#define BBSNDDRV_FXSTAT_IS_FREE	(1<<0)		/* freie struktur */
#define BBSNDDRV_FXSTAT_PLAYING	(1<<1)		/* Spielt gerade 	*/
#define BBSNDDRV_FXSTAT_WAIT		(1<<2)		/* Wartet        	*/
#define BBSNDDRV_FXSTAT_CRASH		(1<<3)		/* Crash effekt  	*/

/* Status flag fuer Musik
 */
#define BBSNDDRV_MUSICSTAT_NULL		0
#define BBSNDDRV_MUSICSTAT_IS_FREE	(1<<0)		/* freie struktur */
#define BBSNDDRV_MUSICSTAT_PLAYING	(1<<1)		/* Spielt gerade 	*/
#define BBSNDDRV_MUSICSTAT_WAIT			(1<<2)		/* wartet 				*/

/* Beschreibt einen Sound Treiber, der ptr wird als Referenz benutzt.
 */
struct bbsnddrv
{
ubyte huge *drvadr;			/* address im memory fuer Treiber */

HDRIVER			aildrv;			/* AIL handle for driver */

ushort			status;			/* status von treiber */

unsigned short	io;			/* port adr von Treiber */
unsigned short  irq;
unsigned short  dma;
unsigned short  drq;

/* Sound buffer struct fuer spielende und wartende effekte:
 */
struct bbsndeffect bbsndeffect[BBSNDEFFECT_MAX];

/* Laenge des allozierten Speichers fuer diesen Treiber !
 */
long		length;

/* Ptr auf stabe table fuer XMI Treiber:
 */
ubyte huge *statetable;

/* Cache fuer TIMBRE:
 */
ubyte	huge *timbrecache;

/* Ptr auf Controller tabelle fuer erweiterte midi commands:
 */
ubyte ctrltable[ 256 ];

/* Music buffer struct fuer spielende und wartende Musiken:
 */
struct bbsndmusic bbsndmusic[BBSNDMUSIC_MAX];

/* ptr fuer TIMBRE Library:
 */
ubyte	huge *timbrelib;
};

/*
 ** Sound ctrl ****************************************************************
 */

#define BBSND_INITIALIZED				0
#define BBSND_NOT_INITIALIZED   1

/* Status flags fuer Manager
 */
#define BBSNDSTAT_NULL			0L			/* tot */

/* Steuerstruktur fuer alle Routinen.
 */
struct bbsoundctrl
{
ulong		status;

/* Ist das System schon initialisiert ?
 */
byte		initialized;
byte		pad;
};

/*
 ** Generelle werte : *********************************************************
 */

#define BBSND_NULL		  0 				/*  */
#define BBSND_FXWAIT		BBSNDDRV_FXSTAT_WAIT
#define BBSND_FXCRASH		BBSNDDRV_FXSTAT_CRASH

/*
 ** prototypes ****************************************************************
 */

short	InstallSndSystem( void );
void	RemoveSndSystem( void );

struct bbsnddrv  *InitSndDriver( struct bbsnddrv *reqdrv, ubyte huge *memptr );
void							ClearSndDriver( struct bbsnddrv *snddrv );

short	SoundManager( void );

short		Seteffect( struct bbsnddrv *snddrv, ubyte huge *memptr, short volume, short count, ushort status );
short		Getfreeailfxchannel( struct bbsnddrv *snddrv );
void		Stopalleffects( struct bbsnddrv *snddrv );

void		Stopmusic( struct bbsnddrv *snddrv, short musicnum );
short		Startmusic( struct bbsnddrv *snddrv, ubyte huge *musicdata );
ushort	Musicstatus( struct bbsnddrv *snddrv, short musicnum );
void		Musicvolume( struct bbsnddrv *snddrv, short musicnum, short volume, short blend );
void		Stopallmusic( struct bbsnddrv *snddrv );

ubyte huge *Searchtimbre( ubyte huge *memptr, ushort bank, ushort patch );

#endif

