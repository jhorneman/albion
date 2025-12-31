

/*

----------------------------------------------------------------------

 MODUL................	3DM.h
 PROJEKT..............	3DM - TEXTURE-MAP-MODUL

 BESCHREIBUNG..........	Deklarations- und Header-Modul fÅr das 3DM-System

 ERSTELLUNGSDATUM.....	15. Juli 1994
 LETZTE éNDERUNG......	15. Juli 1994
 AUTOR................	JÅrgen Friedrich
 PROGRAMMIERSPRACHE...	Turbo C++ 1.0, Watcom C/C++ 10.0

----------------------------------------------------------------------

*/



#ifndef	_3DM_


 /* AN DIESER STELLE IST DIE DEKLARATION DER		*/
 /* BBDEF.H - DATENTYPEN IMPLIZIT VORAUSGESETZT !!	*/
 /* ----------------------------------------------	*/

 /* DEKLARATION DER EINFACHEN DATENTYPEN... */

	#ifndef _SIMPLE_TYPES_

	typedef	unsigned char	uchar;
	typedef	unsigned int	uint;
	typedef	unsigned short	ushort;
	typedef	unsigned long	ulong;

	#define	_SIMPLE_TYPES_

	#endif


 /* 3DM: FEHLER-CODES */
 /* ----------------- */

	#define	ERR_3DM_OK		0x0000
	#define	ERR_3DM_NOT_INITIALIZED	0xF001
	#define	ERR_3DM_SQUARE_SIZE	0xF002
	#define	ERR_3DM_WINDOW_SIZE	0xF004
	#define	ERR_3DM_HEIGHT		0xF008
	#define	ERR_3DM_MAP		0xF009
	#define	ERR_3DM_MAP_SIZE	0xF00A
	#define	ERR_3DM_WALL_HEIGHT	0xF00B
	#define	ERR_3DM_DATA		0xF00C
	#define	ERR_3DM_SHADE_TABLE	0xF00D
	#define	ERR_3DM_RATIO		0xF00F
	#define	ERR_VIEW_DEPTH		0xF010


 /* 3DM: DEFAULT-WERTE DER 3DM-PARAMETER */
 /* ------------------------------------ */

	#define	DEFAULT_ANGLE		  150
	#define	DEFAULT_CM_TO_UNITS_LOG	    1
	#define	DEFAULT_SQUARE_SIZE	 (512 << DEFAULT_CM_TO_UNITS_LOG)
	#define	DEFAULT_SQUARE_LOG	 (9 + DEFAULT_CM_TO_UNITS_LOG)
	#define	DEFAULT_DARKEN		  750
	#define	DEFAULT_VIEWANGLE	  218
	#define	DEFAULT_WINDOW_WIDTH	  208
	#define	DEFAULT_WINDOW_HEIGHT	  112
	#define	DEFAULT_MAP_WIDTH	   20
	#define	DEFAULT_MAP_HEIGHT	   20
	#define	DEFAULT_WALL_HEIGHT	  408	// (in cm)
	#define	DEFAULT_TILE_LOG	    6
	#define	DEFAULT_CLIP_DISTANCE	   40
	#define	DEFAULT_RATIO		  100


 /* 3DM: INTERVAL-GRENZEN DER ZULéSSIGEN PARAMETER */
 /* ---------------------------------------------- */

	#define	MIN_SQUARE_SIZE		  256
	#define	MAX_SQUARE_SIZE		 1024
	#define	MIN_SQUARE_CM		  128
	#define	MAX_SQUARE_CM		 1024
	#define	MIN_HORIZON_Y		 - 50
	#define	MAX_HORIZON_Y		   50
	#define	MIN_WINDOW_WIDTH	   50
	#define	MAX_WINDOW_WIDTH	  640	// VALUE MUST NOT CHANGE ! (mirrored in tpoly32.asm)
	#define	MIN_WINDOW_HEIGHT	   40
	#define	MAX_WINDOW_HEIGHT	  400
	#define	MIN_MAP_WIDTH		    4
	#define	MAX_MAP_WIDTH		  100
	#define	MIN_MAP_HEIGHT		    4
	#define	MAX_MAP_HEIGHT		  100
	#define	MIN_WALL_HEIGHT		   50	// (in cm)
	#define	MAX_WALL_HEIGHT		 9999
	#define	MIN_RATIO		   30
	#define	MAX_RATIO		  300
	#define	MIN_VIEW_DEPTH		    5
	#define	MAX_VIEW_DEPTH		   30


 /* 3DM: PROTOTYP-DEKLARATION DER MODUL-FUNKTIONEN */
 /* ---------------------------------------------- */

	UNSHORT	Init_3DM( void);			/* 3DM-Initialisierung. RÅckgabewert 0 wenn alles OK, sonst Fehlercode. */
	UNSHORT	Set_3DM_AspectRatio( UNSHORT ratio);	/* Zur Anpassung an verschiedenen Monitor-Geometrien */
	UNSHORT	Set_3DM_ViewDepth(  UNSHORT depth);	/* Blick-Weite in KÑstchen festlegen. */
	UNSHORT	Set_3DM_SquareSize( UNSHORT cm);	/* Grî·e der Felder in Zentimetern festsetzen. */
	UNSHORT	Set_3DM_WindowSize( UNSHORT x,
				    UNSHORT y);		/* Neue Bildgrî·e festlegen. */
	UNSHORT Set_3DM_ShadeTable( UNBYTE *table);	/* Neue Shading-Tabelle festlegen */
	UNSHORT	Set_3DM_Map( UNSHORT mapX,
			     UNSHORT mapY,
			     UNSHORT wallHeight);	/* Neue Map und die zugehîrigen Grî·en installieren */
	UNSHORT	Render_3DM_View( void);			/* Erzeugt 3D-Ansicht der Map im aktuellen Blickpunkt und Blickrichtung. */
	UNSHORT	Exit_3DM( void);			/* 3DM-Deinstallierung und Freigabe. RÅckgabewert 0, wenn alles OK... */


	#define	_3DM_
#endif
