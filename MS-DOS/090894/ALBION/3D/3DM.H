

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
	#define	ERR_3DM_WINDOW_SIZE	0xF003
	#define	ERR_3DM_ANGLE		0xF004
	#define	ERR_3DM_POSITION	0xF005
	#define	ERR_3DM_HEIGHT		0xF006
	#define	ERR_3DM_MAP		0xF007
	#define	ERR_3DM_MAP_SIZE	0xF008
	#define	ERR_3DM_WALL_HEIGHT	0xF009
	#define	ERR_3DM_DATA		0xF00A


 /* 3DM: DEFAULT-WERTE DER 3DM-PARAMETER */
 /* ------------------------------------ */

	#define	DEFAULT_ANGLE		  15
	#define	DEFAULT_SQUARE_SIZE	 512
	#define	DEFAULT_WINDOW_WIDTH	 112
	#define	DEFAULT_WINDOW_HEIGHT	  60
	#define	DEFAULT_MAP_WIDTH	  20
	#define	DEFAULT_MAP_HEIGHT	  20
	#define	DEFAULT_WALL_HEIGHT	 100


 /* 3DM: INTERVAL-GRENZEN DER ZULéSSIGEN PARAMETER */
 /* ---------------------------------------------- */

	#define	MIN_ANGLE		  10
	#define	MAX_ANGLE		  20
	#define	MIN_SQUARE_SIZE		 256
	#define	MAX_SQUARE_SIZE		1024
	#define	MIN_WINDOW_WIDTH	  50
	#define	MAX_WINDOW_WIDTH	 640
	#define	MIN_WINDOW_HEIGHT	  40
	#define	MAX_WINDOW_HEIGHT	 480
	#define	MIN_MAP_WIDTH		   4
	#define	MAX_MAP_WIDTH		 100
	#define	MIN_MAP_HEIGHT		   4
	#define	MAX_MAP_HEIGHT		 100
	#define	MIN_WALL_HEIGHT		  10
	#define	MAX_WALL_HEIGHT		 200


 /* 3DM: DEKLARATION DER MODUL-INTERNEN VARIABLEN. */
 /*	(...AUF DIESE VARIABLEN SOLLTE EXTERN NICHT ZUGEGRIFFEN WERDEN...) */
 /* ---------------------------------------------------------------------- */

	extern	UNBYTE	_3DM_initialized;

	extern	UNBYTE	*TextureLayer,		/* Zeiger auf die aktuelle Texture-Map */
			*MapData;		/* Zeiger auf die eigentliche Map, 3 Bytes pro Eintrag, 1. Eintrag ganz links oben... */

	extern	UNSHORT	MapWidth,		/* Breite der "Dungeon"-Map in Quadranten */
			MapHeight,		/* Hîhe der "Dungeon"-Map in Quadranten */
			WallHeight,		/* Hîhe der WÑnde im aktuellen Dungeon */
			BlockSizeInUnits,	/* Grî·e eines Blocks in 3D-Units */
			WindowWidth,		/* Breite des 3DM-Fensters */
			WindowHeight;		/* Hîhe des 3DM-Fensters */
			


 /* 3DM: PROTOTYP-DEKLARATION DER MODUL-FUNKTIONEN */
 /* ---------------------------------------------- */

	UNSHORT	Init_3DM( void);			/* 3DM-Initialisierung. RÅckgabewert 0 wenn alles OK, sonst Fehlercode. */
	UNSHORT	Set_3DM_Angle( UNSHORT angle);		/* Neuen Blickwinkel setzen */
	UNSHORT	Set_3DM_SquareSize( UNSHORT units);	/* Grî·e eines Blocks in 3D-Units festsetzen. WERT MU· 2ER-POTENZ SEIN ! */
	UNSHORT	Set_3DM_WindowSize( UNSHORT x,
				    UNSHORT y);		/* Neue Bildgrî·e festlegen. */
	UNSHORT	Set_3DM_Map( UNBYTE  *map,
			     UNSHORT  mapX,
			     UNSHORT  mapY,
			     UNSHORT  wallHeight);	/* Neue Map und die zugehîrige Grî·en installieren */
	UNSHORT	RenderView( void);			/* Erzeugt 3D-Ansicht der Map im aktuellen Blickpunkt und Blickrivhtung. */
	UNSHORT	Close_3DM( void);			/* 3DM-Deinstallierung und Freigabe. RÅckgabewert 0, wenn alles OK... */


	#define	_3DM_
#endif
