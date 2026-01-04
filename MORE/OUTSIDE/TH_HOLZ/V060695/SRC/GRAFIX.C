//------------------------------------------------------------------------------
//grafix.c - Routinen fÅr die grafik eben:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Ein Rechteck mit den 3d-ecken (Hell/Dunkel) zeichnen:
//------------------------------------------------------------------------------
void Rectangle3d (struct OPM *Opm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c1, SILONG c2, SILONG c3)
{
	SILONG y;

	//Kastenhintergrund:
	for (y=y1; y<y2-y1+1; y++)
	{
		//Aus hlines zusammensetzten:
		OPM_HorLine (Opm, x1, y, x2-x1+1, c2);
	}

	//Helleren Bereich oben und links malen:
	OPM_HorLine (Opm, x1, y1, x2-x1, c1);
	OPM_VerLine (Opm, x1, y1, y2-y1, c1);

	//Dunkleren Bereich rechs und unten malen:
	OPM_HorLine (Opm, x1+1, y2, x2-x1, c3);
	OPM_VerLine (Opm, x2, y1+1, y2-y1, c3);
}
