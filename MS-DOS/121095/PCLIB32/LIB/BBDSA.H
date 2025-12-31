#ifndef BBDSA_H
#define BBDSA_H

/************
 * NAME     :  BBDSA.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/


/* Includes */

#include <BBDEF.H>
#include <BBOPM.H>

/* Defines */

#ifndef BYTEPIXEL
	#define BYTEPIXEL
	#define ANZBYTEPROPIXEL 1
#endif

/* Number of colors in a SCREENPORT palette */
#define SCREEN_PALETTE_ENTRIES	256

/* SCREENPORT.screenstatus */
#define SCREENPORTSTAT_NULL		0L
#define SCREENPORTSTAT_INUSE	( 1L << 0 )

/* SCREENPORT.screentype */
#define SCREENTYPE_NULL		0L
#define SCREENTYPE_FORCE_VESA	( 1L << 2 )
#define SCREENTYPE_FORCE_CHAIN4	( 1L << 3 )
#define SCREENTYPE_FORCE	( SCREENTYPE_FORCE_VESA |SCREENTYPE_FORCE_CHAIN4 )

/* spezielle Definitonen fr Vesa bzw wenn Doublebuffer m”glich */
#define SCREENTYPE_DOUBLEBUFFER ( 1L << 16)

/* DSA_CopyMainOPMToScreen, DSA_CopyOPMToScreen copyflag values */
#define	DSA_CMOPMTS_CHANGED		0	/* Copy if OPM changed */
#define DSA_CMOPMTS_ALWAYS		1	/* Copy always */

/* neue Copyflags einfacher als alte */
#define	DSA_CHANGED		DSA_CMOPMTS_CHANGED	/* Copy if OPM changed */
#define DSA_ALWAYS		DSA_CMOPMTS_ALWAYS	/* Copy always */

/* Fehlermeldungen werden in DSA_Openscreen_error bergeben bei falscher Initialisiereung */
#define DSA_OPENSCREEN_ERROR_VESA_NODRIVER 1
#define DSA_OPENSCREEN_ERROR_VESA_COULDNOTACTVATEMODE 2
#define DSA_OPENSCREEN_ERROR_VESA_COULDNOTGETSIZEOFVIDEORAM 4
#define DSA_OPENSCREEN_ERROR_VESA_NOTENOUGHVIDEORAM 8
#define DSA_OPENSCREEN_ERROR_VESA_SETTINGPOSITION 16

/* Typedefs */

/* Structures */


/* BBCOLOR structure */

struct BBCOLOR
{
	UNBYTE red;			/* Red of color */
	UNBYTE green;		/* Green of color */
	UNBYTE blue;			/* Blue of color */
	UNBYTE alpha;		/* Alpha of color */
};


struct BBPALETTE
{
	UNSHORT			entries;						/* Number of entries in palette */
	UNSHORT			version;						/* Version : always 0 in V1.0 */
	struct BBCOLOR	color[SCREEN_PALETTE_ENTRIES];	/*Color entries of palette */
};



/* SCREENPORT structure */

struct SCREENPORT
{
	/* Machine indipendent elements of SCREENPORT */

	UNLONG				screenstatus;	/* Status flag set */
	struct OPM *		screenopmptr;	/* Pointer to base OPM */
	struct BBPALETTE *	screenpalptr;	/* Pointer to palette */

	UNLONG				screentype;		/* Screen type */

	/* Machine depending elements */

};

/* FehlerMeldung von DSA_OpenScreen */
extern SILONG DSA_OpenScreen_error;

/* Globale Palette */
extern struct SCREENPORT global_screenport;

/* Globale Palette */
extern struct BBPALETTE global_palette;

/* Prototypes */
extern BOOLEAN DSA_Init( void );
extern void DSA_Exit( void );

//extern SISHORT DSA_OpenScreen( struct SCREENPORT * scrnptr, struct OPM * opmptr, struct BBPALETTE * palettenptr, char * titlestrptr, SISHORT xpos, SISHORT ypos,UNLONG type );
extern BOOLEAN DSA_OpenScr(struct OPM * opmptr, UNLONG type);
#define DSA_OpenScreen(a,b,c,d,e,f,g) DSA_OpenScr(b,g)

extern void DSA_CloseScr( void );
#define DSA_CloseScreen(a) DSA_CloseScr()

extern void DSA_DoubleBuffer( void );
extern void DSA_WaitTOF( void );
extern void DSA_WaitBOF( void );

extern void DSA_CopyMainOPMToScr( UNSHORT copyflag );
#define DSA_CopyMainOPMToScreen(a,b) DSA_CopyMainOPMToScr(b)

extern void DSA_CopyOPMToScr( struct OPM * opmptr, SISHORT x, SISHORT y, UNSHORT copyflag );
#define DSA_CopyOPMToScreen(a,b,c,d,e) DSA_CopyOPMToScr(b,c,d,e)

extern void DSA_CopyScrToOPM( struct OPM * opmptr, SISHORT x, SISHORT y );
#define DSA_CopyScreenToOPM(a,b,c,d) DSA_CopyScrToOPM(a,c,d)

extern void DSA_SPal( struct BBPALETTE * srcpalptr, UNSHORT srcstartcolor, UNSHORT colorstocopy, UNSHORT deststartcolor );
#define DSA_SetPal(a,b,c,d,e) DSA_SPal(b,c,d,e)

extern void DSA_SCol( UNSHORT colorid, UNBYTE red, UNBYTE green, UNBYTE blue );
#define DSA_SetColor(a,b,c,d,e,f) DSA_SCol(b,c,d,e)

extern void DSA_APal( void );
#define DSA_ActivatePal(a) DSA_APal()

#endif
