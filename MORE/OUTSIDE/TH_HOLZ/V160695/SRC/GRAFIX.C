//------------------------------------------------------------------------------
//grafix.c - Routinen fÅr die grafik eben:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Ein gefÅlltes Rechteck mit den 3d-ecken (Hell/Dunkel) zeichnen:
//------------------------------------------------------------------------------
void Rectangle3d (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c1, SILONG c2, SILONG c3)
{
	SILONG y;

	//Kastenhintergrund:
	for (y=y1; y<=y2; y++)
	{
		//Aus hlines zusammensetzten:
		OPM_HorLine (pOpm, x1, y, x2-x1+1, c2);
	}

	//Helleren Bereich oben und links malen:
	OPM_HorLine (pOpm, x1, y1, x2-x1, c1);
	OPM_VerLine (pOpm, x1, y1, y2-y1, c1);

	//Dunkleren Bereich rechs und unten malen:
	OPM_HorLine (pOpm, x1+1, y2, x2-x1, c3);
	OPM_VerLine (pOpm, x2, y1+1, y2-y1, c3);
}

//------------------------------------------------------------------------------
//Ein Fenster mit den 3d-ecken (Hell/Dunkel) zeichnen:
//------------------------------------------------------------------------------
void Window3d (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c1, SILONG c2, SILONG c3, SILONG c4)
{
	//Ñu·erer Rahmen:
	Rectangle3d (pOpm, x1, y1, x2, y2, c1, c2, c3);

	//inerer Rahmen:
	Rectangle3d (pOpm, x1+MENU_BORDER_WIDTH,
							 y1+MENU_BORDER_WIDTH,
							 x2-MENU_BORDER_WIDTH,
							 y2-MENU_BORDER_WIDTH, c3, c2, c1);

	//inneres Papier:
	Rectangle3d (pOpm, x1+MENU_BORDER_WIDTH+1,
							 y1+MENU_BORDER_WIDTH+1,
							 x2-MENU_BORDER_WIDTH-1,
							 y2-MENU_BORDER_WIDTH-1, c4, c4, c4);
}
