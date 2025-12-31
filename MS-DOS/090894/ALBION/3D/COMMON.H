


	/* GEMEINSAMER DATENBEREICH - SCHNITTSTELLE ZU "ALBION"... */
	/* ------------------------------------------------------- */



extern	UNBYTE	*BackgroundMap,			/* Zeiger auf die Background-Grafik */
		*ShadeTable,			/* Zeiger auf die Shading-Tabellen (je 256 EintrÑge) */
		*ViewBuffer;			/* Zeiger auf den Speicherblock, in dem 3DM das Bild erzeugt */

extern	UNSHORT	CameraHeight,			/* Hîhe des Blickpunktes */
		BlockSizeInCM,			/* Grî·e eines Blocks in Zentimetern */
		NumOfNPCs,			/* Anzahl aller NPCs */
		TextureWidth,			/* Breite der aktuellen Texture-Map */
		TextureHeight,			/* Hîhe der aktuellen Texture-Map */
		ShadeFactor,			/* ?? */
		BackgroundWidth,		/* Breite der Background-Grafik */
		BackgroundHeight;		/* Hîhe der Background-Grafik */

extern	SISHORT	HorizonVertOffset;		/* Vertikale Horizont-Verschiebung */

extern	UNLONG	Flags;				/* ?? */

extern	SILONG	CameraX,			/* X-Position des Blickpunktes (Player-Position) */
		CameraZ,			/* Z-Position des Blickpunktes (Player-Position) */
		CameraAngle;			/* Blick-Winkel (Richtung) */



extern	Object_group_3D	*	GroupList;	/* Liste aller verfÅgbaren Objektgruppen */

extern	       Floor_3D *	FloorList;	/* Liste aller verfÅgbaren Floor-Elemente */

extern		Wall_3D **	WallList;	/* Liste aller verfÅgbaren Wand-Elemente */

extern	      Object_3D	*	ObjectList;	/* Liste aller vorhandenen Objekte */

extern		 NPC_3D	*	NPCList;	/* Liste aller vorhandenen NPCs */


