//------------------------------------------------------------------------------
//Basisfunktionen fÅr die Window.c Routinen:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Zeichnet einen Pfeil an (x|y) mit Richtung oben oder unten
//------------------------------------------------------------------------------
void DrawArrow (struct OPM *pOpm, SILONG x, SILONG y, SILONG Color, SILONG Orientation)
{
	OPM_HorLine (pOpm, x-1, y-Orientation*4, 3, Color);
	OPM_HorLine (pOpm, x-1, y-Orientation*3, 3, Color);
	OPM_HorLine (pOpm, x-1, y-Orientation*2, 3, Color);
	OPM_HorLine (pOpm, x-1, y-Orientation*1, 3, Color);
	OPM_HorLine (pOpm, x-4, y+Orientation*0, 9, Color);
	OPM_HorLine (pOpm, x-3, y+Orientation*1, 7, Color);
	OPM_HorLine (pOpm, x-2, y+Orientation*2, 5, Color);
	OPM_HorLine (pOpm, x-1, y+Orientation*3, 3, Color);
	OPM_HorLine (pOpm, x  , y+Orientation*4, 1, Color);
}

//------------------------------------------------------------------------------
//Ein ungefÅlltes Rechteck zeichnen:
//------------------------------------------------------------------------------
void Rectangle (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c)
{
	if (x2<x1 || y2<y1) return;

	OPM_HorLine (pOpm, x1, y1, x2-x1+1, c);
	OPM_VerLine (pOpm, x1, y1, y2-y1+1, c);
	OPM_HorLine (pOpm, x1, y2, x2-x1+1, c);
	OPM_VerLine (pOpm, x2, y1, y2-y1+1, c);
}

//------------------------------------------------------------------------------
//Ein gefÅlltes Rechteck mit den 3d-ecken (Hell/Dunkel) zeichnen:
//------------------------------------------------------------------------------
void Rectangle3d (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c1, SILONG c2, SILONG c3)
{
	SILONG y;

	if (x2<=x1 || y2<=y1) return;

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
	if (x2<=x1 || y2<=y1) return;

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

//------------------------------------------------------------------------------
//Zeichnet einen Standart-Button mit diesem Schatten-Rahmen
//------------------------------------------------------------------------------
void DrawButton (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG FrameColor)
{
	Rectangle3d (pOpm,
					 x1,
					 y1,
					 x2,
					 y2,
					 BUTTON_COLOR_LIGHT,
					 BUTTON_COLOR,
					 BUTTON_COLOR_DARK);

	Rectangle (pOpm,
					x1-1,
					y1-1,
					x2+1,
					y2+1,
					FrameColor);
}

//------------------------------------------------------------------------------
//Zeichnet hinter das anzgegebene Fenster einen Schatten:
//------------------------------------------------------------------------------
void DrawWinShadow (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2)
{
	SILONG x,y;

	if (x2<=x1 || y2<=y1) return;

	for (y=y1; y<=y2; y++)
		for (x=x1; x<=x2; x++)
			if (y+SHADOW_WIDTH>y2 || x+SHADOW_WIDTH>x2)
				if (x+SHADOW_WIDTH<RESOLUTION_X && y+SHADOW_WIDTH<RESOLUTION_Y)
				{
					//Schwarzen Pixel setzen:
					OPM_SetPixel (pOpm, x+SHADOW_WIDTH, y+SHADOW_WIDTH, 0);
				}
}

//------------------------------------------------------------------------------
//DrÅckt einen Knopf rein:
//------------------------------------------------------------------------------
void PushButton (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2)
{
	SILONG x,y;

	for (y=y2; y>=y1; y--)
		for (x=x2; x>=x1; x--)
		{
			if (x==x1 || y==y1 || x==x1+1 || y==y1+1)
				OPM_SetPixel (pOpm, x, y, BUTTON_COLOR_DARK);
			else if (x==x1+2 || y==y1+2)
				OPM_SetPixel (pOpm, x, y, BUTTON_COLOR);
			else
				OPM_SetPixel (pOpm, x, y, OPM_GetPixel (pOpm, x-2, y-2));
		}
}

//------------------------------------------------------------------------------
//Ziegt einen Knopf wieder raus:
//------------------------------------------------------------------------------
void UnPushButton (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2)
{
	SILONG x,y;

	for (y=y1; y<=y2; y++)
		for (x=x1; x<=x2; x++)
		{
			if (x==x2 || y==y2 || x==x2-1 || y==y2-1)
				OPM_SetPixel (pOpm, x, y, BUTTON_COLOR);
			else
				OPM_SetPixel (pOpm, x, y, OPM_GetPixel (pOpm, x+2, y+2));
		}

	//Helleren Bereich oben und links malen:
	OPM_HorLine (pOpm, x1, y1, x2-x1, BUTTON_COLOR_LIGHT);
	OPM_VerLine (pOpm, x1, y1, y2-y1, BUTTON_COLOR_LIGHT);

	//Dunkleren Bereich rechs und unten malen:
	OPM_HorLine (pOpm, x1+1, y2, x2-x1, BUTTON_COLOR_DARK);
	OPM_VerLine (pOpm, x2, y1+1, y2-y1, BUTTON_COLOR_DARK);
}

//------------------------------------------------------------------------------
//Refresht den fixen Teil der Palette (Die Systemfarben)
//------------------------------------------------------------------------------
void RefreshFixedPalette (void)
{
	SILONG c;

	//Change all wanted colors:
	for (c=0; DefaultPal[c].Color!=-1; c++)
	{
		DSA_SCol (DefaultPal[c].Color,
				    DefaultPal[c].r*255/100,
					 DefaultPal[c].g*255/100,
					 DefaultPal[c].b*255/100);
	}

	//Activate changes:
	DSA_APal();
}

//------------------------------------------------------------------------------
//versucht einen Text in einen Rahmen zu drucken. TRUE wenn er reinpasst.
//------------------------------------------------------------------------------
BOOL PrintTextInFrame (UNCHAR *Text, SILONG FontColor, struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2)
{
	SILONG x,y;
	SILONG c,d,l;
	UNCHAR	 Buffer[256];
	BOOL	 IsWithinWord, WasWithinWord;

	//Sind Ma·e illusorisch ?
	if (x1>=x2 || y1+8>=y2) return(FALSE);

	//Startkoordinaten:
	x=x1;
	y=y1;

	//Init:
	l = strlen (Text);
	Buffer[0]  	  = 0;
	IsWithinWord  = FALSE;
	WasWithinWord = FALSE;

	for (c=0; c<l+1; c++)
	{
		//String anhand von Tilden und Leerzeichen zerhacken:
		if (Text[c]!=32 && Text[c]!='~' && Text[c]!=0 && Text[c]!='\n')
		{
			//Buffer um ein Zeichen erweitern:
			Buffer[strlen(Buffer)+1]=0;
			Buffer[strlen(Buffer)]=Text[c];
		}
		else
		{
			//Trennzeichenflags verwalten:
			WasWithinWord = IsWithinWord;
			IsWithinWord  = (Text[c]=='~');

			//Ist Wort(-stÅck) lÑnger als Fenster ?
			if (PixelStringLength (Buffer) + IsWithinWord * PixelCharLength('-') > x2-x1-1)
			{
				//Dieses Wort(-stÅck) ist zu lang um ins Fenster zu passen:
				return (FALSE);
			}

			//Passt das Wort noch in diese Zeile ?
			if (PixelStringLength (Buffer)+PixelCharLength('-')*(x != x1 && WasWithinWord==FALSE) > x2-x-1)
			{
				//Nein, deshalb in die neue Zeile:
				if (WasWithinWord)
				{
					OPM_Write (pOpm, "-", x, y, FontColor);
				}

				y += FONT_Y_SIZE;
				x =  x1;

				//Passt die neue Zeile noch ins Fenster ?
				if (y>=y2-8) return (FALSE);
			}

			if (x != x1 && WasWithinWord==FALSE)
			{
				OPM_Write (pOpm, " ", x, y, FontColor);
				x += PixelCharLength (' ');
			}
			for (d=0; d<strlen(Buffer); d++)
				if (Buffer[d]=='_') Buffer[d]=' ';

			OPM_Write (pOpm, Buffer, x, y, FontColor);

			x += PixelStringLength (Buffer);

			//WÅnscht sich der Benutzer einen Zeilenumbruch ?
			if (Text[c]=='\n')
			{
				y += FONT_Y_SIZE;
				x =  x1;

				//Passt die neue Zeile noch ins Fenster ?
				if (y>=y2-8) return (FALSE);
			}

			//NÑchstes Wort(-teil) vorbereiten:
			Buffer[0]=0;
		}
	}

	//Text konnte erfolgreich geschrieben werden:
	return (TRUE);
}

//------------------------------------------------------------------------------
//versucht eine Textzeile zentriert in einen Rahmen zu drucken. rc siehe oben.
//------------------------------------------------------------------------------
BOOL PrintCenteredTextInFrame (UNCHAR *Text, SILONG FontColor, struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2)
{
	return (PrintTextInFrame (Text,
									  FontColor,
									  pOpm,
									  (x1+x2)/2 - PixelStringLength(Text)/2,
									  y1,
									  x2,
									  y2));
}

//------------------------------------------------------------------------------
//Berechnet die StringlÑnge in Pixeln eines Strings:
//------------------------------------------------------------------------------
SILONG PixelStringLength (UNCHAR *Text)
{
	SILONG c,l,rc;

	l=strlen (Text);

	rc=0;
	for (c=0; c<l; c++)
	{
		//einzelne ZeichenlÑngen zusammenrechnen:
		rc+=PixelCharLength (Text[c]);
	}

	return (rc);
}

//------------------------------------------------------------------------------
//Berechnet die Stringlînge in Pixeln eines Zeichens:
//------------------------------------------------------------------------------
SILONG PixelCharLength (UNCHAR Zeichen)
{
	//Hier kann man einschreiten falls man einen neuen Font einbaut:
	if (Zeichen)
		return (8); //If ist nur um "unreferenced"-Warnung zu unterdrÅcken.
	else
		return (0); //Wird nie erreicht.
}

//------------------------------------------------------------------------------
//Kopiert Source in Target auf je volle Grî·e
//------------------------------------------------------------------------------
void ScaleCopyOpm (struct OPM *pSourceOpm, struct OPM *pTargetOpm)
{
	SILONG sx,sy; //Source-Position
	SILONG tx,ty; //Target-Position
	SILONG addSX; //FestPunktStepper
	UNCHAR *pSource, *pTarget;

	for (ty=0; ty<pTargetOpm->height; ty++)
	{
		//Wozu die Ñu·ere loop optimieren..
		sy=ty*pSourceOpm->height/pTargetOpm->height;
		pSource = pSourceOpm->data+sy*pSourceOpm->width;
		pTarget = pTargetOpm->data+ty*pTargetOpm->width;

		//standart Festkomma-Algorythmus zum skalieren:
		addSX=1024*pSourceOpm->width/pTargetOpm->width;
		sx=0;

		for (tx=0; tx<pTargetOpm->width; tx++)
		{
			pTarget[tx]=pSource[sx>>10];
			sx+=addSX;
		}
	}
}
