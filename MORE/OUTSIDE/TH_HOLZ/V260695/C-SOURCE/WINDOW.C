//------------------------------------------------------------------------------
//Window.c - die Handleroutinen fÅr die einzelnen Fenstertypen:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Gibt ein rotes Fenster mit einem OK-Butten aus.
//------------------------------------------------------------------------------
void ErrorWindow (char *Text)
{
	SILONG Size;
	struct OPM BackupOpm;

	//ggf. bestehendes Werbe-Fenster schliessen:
	EasyPrintWindow (NULL);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, 320, 200, 0, 0);

	//Fenster immer weiter wachsen lassen:
	for (Size=5; Size<RESOLUTION_Y/2; Size+=WIN_EXPLODE_SPEED)
	{
		Window3d (&SetupOpm,
					 RESOLUTION_X/2 - Size*1.6,
					 RESOLUTION_Y/2 - Size,
		          RESOLUTION_X/2 + Size*1.6,
					 RESOLUTION_Y/2 + Size,
					 ERROR_BORDER_COLOR_LIGHT,
					 ERROR_BORDER_COLOR,
					 ERROR_BORDER_COLOR_DARK,
					 ERROR_BACK_COLOR);

		//OK-Button machen:
		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 + MENU_BORDER_WIDTH - Size*1.6+1,
					 	 RESOLUTION_Y/2 + Size - 6 - FONT_Y_SIZE,
		          	 RESOLUTION_X/2 - MENU_BORDER_WIDTH + Size*1.6-1,
					 	 RESOLUTION_Y/2 - MENU_BORDER_WIDTH + Size-1,
					 	 ERROR_BORDER_COLOR_LIGHT,
					 	 ERROR_BORDER_COLOR,
					 	 ERROR_BORDER_COLOR_DARK);

		//OK-Button-Text proben:
		if (PixelStringLength(Messages[Language].OK) > Size*2)
		{
			//schon das OK passte nicht rein:
			continue;
		}

		//OK-Button-Text schreiben:
		if (!PrintTextInFrame (Messages[Language].OK,
									  ERROR_FONT_COLOR,
									  &SetupOpm,
					 	 			  RESOLUTION_X/2 - PixelStringLength(Messages[Language].OK)/2,
					 	 			  RESOLUTION_Y/2 + Size - 4 - FONT_Y_SIZE,
					 	 			  RESOLUTION_X/2 - MENU_BORDER_WIDTH + Size*1.6-3,
					 	 			  RESOLUTION_Y/2 + Size - 4 + 3))
		{
			//schon das OK passte nicht rein:
			continue;
		}

		if (PrintTextInFrame (Text,
									 ERROR_FONT_COLOR,
									  &SetupOpm,
									 RESOLUTION_X/2 - Size*1.6 + MENU_BORDER_WIDTH+2,
									 RESOLUTION_Y/2 - Size     + MENU_BORDER_WIDTH+2,
									 RESOLUTION_X/2 + Size*1.6 - MENU_BORDER_WIDTH-2,
									 RESOLUTION_Y/2 + Size     - MENU_BORDER_WIDTH-1 -
									 FONT_Y_SIZE - 4))
		{
			//Wachstum abbrechen, da Text in Fenster passt:
			break;
		}
		DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
	}
	if (Size>=RESOLUTION_Y/2)
		InternalError (ERR_ERRTEXT_TOO_BIG);

	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	//Einen Knopf verwalten. Das Ergebnis interessiert uns gar nicht:
	GetButtonAnswer (1,
						  0,
					 	  (SILONG)(RESOLUTION_X/2 + MENU_BORDER_WIDTH - Size*1.6+1), //x1
					 	  (SILONG)(RESOLUTION_Y/2 + Size - 6 - FONT_Y_SIZE),         //y1
		          	  (SILONG)(RESOLUTION_X/2 - MENU_BORDER_WIDTH + Size*1.6-1), //x2
					 	  (SILONG)(RESOLUTION_Y/2 - MENU_BORDER_WIDTH + Size-1),     //y2
						  "\x1b");                                        //ShortCut

	//Restore the old screen:
	OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
	OPM_Del (&BackupOpm);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
}

//------------------------------------------------------------------------------
//Gibt ein YesNo-Fenster mit einem Text aus:
//------------------------------------------------------------------------------
BOOL AssertWindow (char *Text)
{
	SILONG rc;
	SILONG Size;
	struct OPM BackupOpm;

	//ggf. bestehendes Werbe-Fenster schliessen:
	EasyPrintWindow (NULL);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, 320, 200, 0, 0);

	//Fenster immer weiter wachsen lassen:
	for (Size=5; Size<RESOLUTION_Y/2; Size+=WIN_EXPLODE_SPEED)
	{
		Window3d (&SetupOpm,
					 RESOLUTION_X/2 - Size*1.6,
					 RESOLUTION_Y/2 - Size,
		          RESOLUTION_X/2 + Size*1.6,
					 RESOLUTION_Y/2 + Size,
					 MENU_BORDER_COLOR_LIGHT,
					 MENU_BORDER_COLOR,
					 MENU_BORDER_COLOR_DARK,
					 MENU_BACK_COLOR);

		//Yes&No-Button-Texte proben:
		if (PixelStringLength(Messages[Language].Yes)+
			 PixelStringLength(Messages[Language].No)+
			 MENU_BORDER_WIDTH*2+6 > Size*2)
		{
			//schon Ja und Nein passen nicht rein:
			continue;
		}

		//Yes-Button machen:
		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 + MENU_BORDER_WIDTH - Size*1.6+1,
					 	 RESOLUTION_Y/2 + Size - 6 - FONT_Y_SIZE,
		          	 RESOLUTION_X/2,
					 	 RESOLUTION_Y/2 - MENU_BORDER_WIDTH + Size-1,
					 	 MENU_BORDER_COLOR_LIGHT,
					 	 MENU_BORDER_COLOR,
					 	 MENU_BORDER_COLOR_DARK);

		//Yes-Button-Text schreiben:
		if (!PrintTextInFrame (Messages[Language].Yes,
									  MENU_FONT_COLOR,
									  &SetupOpm,
					 	 			  RESOLUTION_X/2 - Size*1.6/2 - PixelStringLength(Messages[Language].Yes)/2,
					 	 			  RESOLUTION_Y/2 + Size - 4 - FONT_Y_SIZE,
					 	 			  RESOLUTION_X/2 - MENU_BORDER_WIDTH + Size*1.6-3,
					 	 			  RESOLUTION_Y/2 + Size - 4 + 3))
		{
			//schon das OK passte nicht rein:
			continue;
		}

		//No-Button machen:
		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 + 1,
					 	 RESOLUTION_Y/2 + Size - 6 - FONT_Y_SIZE,
		          	 RESOLUTION_X/2 - MENU_BORDER_WIDTH + Size*1.6-1,
					 	 RESOLUTION_Y/2 - MENU_BORDER_WIDTH + Size-1,
					 	 MENU_BORDER_COLOR_LIGHT,
					 	 MENU_BORDER_COLOR,
					 	 MENU_BORDER_COLOR_DARK);

		//No-Button-Text schreiben:
		if (!PrintTextInFrame (Messages[Language].No,
									  MENU_FONT_COLOR,
									  &SetupOpm,
					 	 			  RESOLUTION_X/2 + 1 + Size*1.6/2 - PixelStringLength(Messages[Language].No)/2,
					 	 			  RESOLUTION_Y/2 + Size - 4 - FONT_Y_SIZE,
					 	 			  RESOLUTION_X/2 - MENU_BORDER_WIDTH + Size*1.6-3,
					 	 			  RESOLUTION_Y/2 + Size - 4 + 3))
		{
			//schon das OK passte nicht rein:
			continue;
		}

		//eigentlichen Text ausgeben:
		if (PrintTextInFrame (Text,
									 MENU_FONT_COLOR,
									 &SetupOpm,
									 RESOLUTION_X/2 - Size*1.6 + MENU_BORDER_WIDTH+2,
									 RESOLUTION_Y/2 - Size     + MENU_BORDER_WIDTH+2,
									 RESOLUTION_X/2 + Size*1.6 - MENU_BORDER_WIDTH-2,
									 RESOLUTION_Y/2 + Size     - MENU_BORDER_WIDTH-1 -
									 FONT_Y_SIZE - 4))
		{
			//Wachstum abbrechen, da Text in Fenster passt:
			break;
		}
		DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
	}
	if (Size>=RESOLUTION_Y/2)
		InternalError (ERR_ERRTEXT_TOO_BIG);

	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	//Einen Knopf verwalten. Das Ergebnis interessiert uns gar nicht:
	rc=GetButtonAnswer (2,
						  0,
						  //Yes-Button:
					 	  (SILONG)(RESOLUTION_X/2 + MENU_BORDER_WIDTH - Size*1.6+1),
					 	  (SILONG)(RESOLUTION_Y/2 + Size - 6 - FONT_Y_SIZE),
		          	  (SILONG)(RESOLUTION_X/2),
					 	  (SILONG)(RESOLUTION_Y/2 - MENU_BORDER_WIDTH + Size-1),
						  "jJyY",                                         //ShortCut

						  //No-Button:
					 	  (SILONG)(RESOLUTION_X/2 + 1),
					 	  (SILONG)(RESOLUTION_Y/2 + Size - 6 - FONT_Y_SIZE),
		          	  (SILONG)(RESOLUTION_X/2 - MENU_BORDER_WIDTH + Size*1.6-1),
					 	  (SILONG)(RESOLUTION_Y/2 - MENU_BORDER_WIDTH + Size-1),
						  "nN\x1b");                                      //ShortCut

	//Restore the old screen:
	OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
	OPM_Del (&BackupOpm);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	return (!rc);
}

//------------------------------------------------------------------------------
//versucht einen Text in einen Rahmen zu drucken. TRUE wenn er reinpasst.
//------------------------------------------------------------------------------
BOOL PrintTextInFrame (char *Text, SILONG FontColor, struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2)
{
	SILONG x,y;
	SILONG c,d,l;
	char	 Buffer[256];
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
//Berechnet die StringlÑnge in Pixeln eines Strings:
//------------------------------------------------------------------------------
SILONG PixelStringLength (char *Text)
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
SILONG PixelCharLength (char Zeichen)
{
	//Hier kann man einschreiten falls man einen neuen Font einbaut:
	if (Zeichen)
		return (8); //If ist nur um "unreferenced"-Warnung zu unterdrÅcken.
	else
		return (0); //Wird nie erreicht.
}

//------------------------------------------------------------------------------
//Verwaltet beliebig viele Buttons. Aufruf immer:
// - AnzButtons
// - AnzButtons * ⁄ SILONG x1, y1, x2, y2
//                ¿ char *KeyboardShortcuts
//------------------------------------------------------------------------------
SILONG GetButtonAnswer (SILONG AnzButtons, SILONG DefButton, ...)
{
	SILONG c;
	MENU	 Menu;
	va_list	Vars;

	Menu.AnzEntrys = AnzButtons;

	//Tabelle initialisieren:
	va_start (Vars, DefButton);

	for (c=0; c<AnzButtons; c++)
	{
		Menu.MenuEntry[c].x1 = va_arg (Vars, SILONG); //fetch an SILONG from Stack
		Menu.MenuEntry[c].y1 = va_arg (Vars, SILONG);
		Menu.MenuEntry[c].x2 = va_arg (Vars, SILONG);
		Menu.MenuEntry[c].y2 = va_arg (Vars, SILONG);
		Menu.MenuEntry[c].Shortcut = va_arg (Vars, char *);
	}

	//Daten bereinigen:
	va_end (Vars);

	return (GetMenuAnswer(&Menu, DefButton));
}

//------------------------------------------------------------------------------
//Verwaltet beliebig viele Buttons.
//------------------------------------------------------------------------------
SILONG GetMenuAnswer (MENU *pMenu, SILONG DefButton)
{
	SILONG c,d;
	SILONG HighlightOption;
	SILONG LastTime=0;
	BOOL	 BlinkState=0;
	char	 Key;
	struct OPM BackupOpm;

	HighlightOption = DefButton;

	//What a silly user !
	if (pMenu->AnzEntrys<=0)
		return (0);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, 320, 200, 0, 0);

	while (1)
	{
		Last_button_state=Button_state;
		Update_input();

		if ( ((SILONG*)0x46C)[0] - LastTime>10 )
		{
			LastTime=((SILONG*)0x46C)[0];
			BlinkState^=1;

			if (BlinkState)
			{
				//highlight the active option:
				OPM_Box (&SetupOpm,
							pMenu->MenuEntry[HighlightOption].x1+1,
							pMenu->MenuEntry[HighlightOption].y1+1,
							pMenu->MenuEntry[HighlightOption].x2-pMenu->MenuEntry[HighlightOption].x1-1,
							pMenu->MenuEntry[HighlightOption].y2-pMenu->MenuEntry[HighlightOption].y1-1,
							MENU_HIGHLIGHT);

				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			}
			else
			{
				//un-highlight:
				OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			}
		}

		if (kbhit())
		{
			Key=getch();

			//MenÅpunkt selektieren:
			switch (Key)
			{
				case 0:
					switch (getch())
					{
						case 72: //Up
						case 75: //Left
							LastTime = 0;
							BlinkState = 0;
							OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
							HighlightOption = (HighlightOption+pMenu->AnzEntrys-1)%pMenu->AnzEntrys;
							break;

						case 77: //Right
						case 80: //Down
							LastTime = 0;
							BlinkState = 0;
							OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
							HighlightOption = (HighlightOption+1)%pMenu->AnzEntrys;
							break;
					}
					break;

				case 13:
				case 32:
					//Restore the old screen:
					OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
					OPM_Del (&BackupOpm);
					DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
					return (HighlightOption);
			}
		}

		//Mit Hotkeys vergleichen:
		for (c=0; c<pMenu->AnzEntrys; c++)
		{
			for (d=0; d<strlen(pMenu->MenuEntry[c].Shortcut); d++)
				if (pMenu->MenuEntry[c].Shortcut[d] == Key)
				{
					//Restore the old screen:
					OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
					OPM_Del (&BackupOpm);
					DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
					return (c);
				}
		}

		//Maus mit Select-Frames vergleichen:
		for (c=0; c<pMenu->AnzEntrys; c++)
		{
			//Ist Cursor in Box ?
			if (Mouse_X >= pMenu->MenuEntry[c].x1+1 &&
				 Mouse_X <= pMenu->MenuEntry[c].x2-1 &&
			    Mouse_Y >= pMenu->MenuEntry[c].y1+1 &&
				 Mouse_Y <= pMenu->MenuEntry[c].y2-1)
			{
				//HÑlt Benutzer den LMB bis jetzt gedrÅckt?
				if (Button_state&1)
				{
					LastTime = 0;
					BlinkState = 0;
					OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
					HighlightOption = c;
				}

				//Hat er ihn jetzt innerhalb einer Box losgelassen ?
				if ((!(Button_state&1)) && (Last_button_state&1))
				{
					//Restore the old screen:
					OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
					OPM_Del (&BackupOpm);
					DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
					return (c);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
//Zeigt ein AuswahlmenÅ an gibt die Zeilennummer des Verzweigungslabels zurÅck:
//------------------------------------------------------------------------------
SILONG SelectionWindow (MENU *pMenu)
{
	SILONG SizeX,SizeY;
	struct OPM BackupOpm;
	MENU	 SelectMenu;
	SILONG c;
	SILONG y;

	SelectMenu.AnzEntrys=0;

	//ggf. bestehendes Werbe-Fenster schliessen:
	EasyPrintWindow (NULL);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, 320, 200, 0, 0);

	SizeX=0;
	SizeY=MENU_BORDER_WIDTH*2+2;
	for (c=0; c<pMenu->AnzEntrys; c++)
	{
		if (pMenu->MenuEntry[c].Text && pMenu->MenuEntry[c].Line!=-1)
		{
			if (PixelStringLength (pMenu->MenuEntry[c].Text)+MENU_BORDER_WIDTH*2+12 > SizeX)
				SizeX = PixelStringLength (pMenu->MenuEntry[c].Text)+MENU_BORDER_WIDTH*2+12;

			SizeY+=FONT_Y_SIZE+6;
		}
		else if (pMenu->MenuEntry[c].Text && pMenu->MenuEntry[c].Line==-1)
		{
			if (PixelStringLength (pMenu->MenuEntry[c].Text)+MENU_BORDER_WIDTH*2+6 > SizeX)
				SizeX = PixelStringLength (pMenu->MenuEntry[c].Text)+MENU_BORDER_WIDTH*2+6;

			SizeY+=FONT_Y_SIZE+4;
		}
		else
			SizeY+=3;
	}

	Window3d (&SetupOpm,
				 RESOLUTION_X/2 - SizeX/2,
				 RESOLUTION_Y/2 - SizeY/2,
		       RESOLUTION_X/2 - SizeX/2+SizeX,
				 RESOLUTION_Y/2 - SizeY/2+SizeY,
				 MENU_BORDER_COLOR_LIGHT,
				 MENU_BORDER_COLOR,
				 MENU_BORDER_COLOR_DARK,
				 MENU_BACK_COLOR);

	y = RESOLUTION_Y/2 - SizeY/2 + MENU_BORDER_WIDTH + 1;

	for (c=0; c<pMenu->AnzEntrys; c++)
	{
		if (pMenu->MenuEntry[c].Text)
		{
			if (pMenu->MenuEntry[c].Line!=-1)
			{
				//Echter Auswahlpunkt:
				pMenu->MenuEntry[c].x1 = RESOLUTION_X/2 - SizeX/2 + MENU_BORDER_WIDTH + 3;
				pMenu->MenuEntry[c].y1 = y+1;
				pMenu->MenuEntry[c].x2 = RESOLUTION_X/2 - SizeX/2 + SizeX - MENU_BORDER_WIDTH - 3;
				pMenu->MenuEntry[c].y2 = y+FONT_Y_SIZE+4;

				//Kasten zeichnen:
				Rectangle3d (&SetupOpm,
								 pMenu->MenuEntry[c].x1,
								 pMenu->MenuEntry[c].y1,
								 pMenu->MenuEntry[c].x2,
								 pMenu->MenuEntry[c].y2,
				 				 MENU_BORDER_COLOR_LIGHT,
				 				 MENU_BORDER_COLOR,
				 				 MENU_BORDER_COLOR_DARK);

				PrintTextInFrame (pMenu->MenuEntry[c].Text,
									   MENU_FONT_COLOR,
									   &SetupOpm,
										pMenu->MenuEntry[c].x1+2,
										pMenu->MenuEntry[c].y1+2,
										pMenu->MenuEntry[c].x2+10,
										pMenu->MenuEntry[c].y2);

				y+=FONT_Y_SIZE+6;

				//MenÅ-Punkt zum auswÑhlen vorbereiten:
				SelectMenu.MenuEntry[SelectMenu.AnzEntrys].Text=NULL;
				SelectMenu.MenuEntry[SelectMenu.AnzEntrys].Shortcut="";
				SelectMenu.MenuEntry[SelectMenu.AnzEntrys].Line=pMenu->MenuEntry[c].Line;
				SelectMenu.MenuEntry[SelectMenu.AnzEntrys].x1=pMenu->MenuEntry[c].x1;
				SelectMenu.MenuEntry[SelectMenu.AnzEntrys].y1=pMenu->MenuEntry[c].y1;
				SelectMenu.MenuEntry[SelectMenu.AnzEntrys].x2=pMenu->MenuEntry[c].x2;
				SelectMenu.MenuEntry[SelectMenu.AnzEntrys].y2=pMenu->MenuEntry[c].y2;
				SelectMenu.AnzEntrys++;
			}
			else
			{
				//Bereich fÅr eine öberschrift:
				pMenu->MenuEntry[c].x1 = RESOLUTION_X/2 - SizeX/2 + MENU_BORDER_WIDTH + 1;
				pMenu->MenuEntry[c].y1 = y;
				pMenu->MenuEntry[c].x2 = RESOLUTION_X/2 - SizeX/2 + SizeX - MENU_BORDER_WIDTH - 1;
				pMenu->MenuEntry[c].y2 = y+FONT_Y_SIZE+4;

				PrintTextInFrame (pMenu->MenuEntry[c].Text,
									   MENU_FONT_COLOR,
									   &SetupOpm,
										pMenu->MenuEntry[c].x1+1,
										pMenu->MenuEntry[c].y1+1,
										pMenu->MenuEntry[c].x2,
										pMenu->MenuEntry[c].y2);

				y+=FONT_Y_SIZE+4;
			}
		}
		else
		{
			//Leerzeile erwÅnscht:
			y+=3;
		}
	}

	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	c = GetMenuAnswer (&SelectMenu, 0);

	//Restore the old screen:
	OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
	OPM_Del (&BackupOpm);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	return (SelectMenu.MenuEntry[c].Line);
}

//------------------------------------------------------------------------------
//Erzeugt ein MenÅ mit den erlaubten Drives:
//------------------------------------------------------------------------------
void SelectTargetDrive (SILONG NeededKbFree)
{
	MENU Menu;
	SILONG c;
	SILONG Free;
  	union REGS Regs;
	struct diskfree_t DiskData;

	SetupNeededKbFree = NeededKbFree;

	//MenÅtitel:
	Menu.MenuEntry[0].Text=Messages[Language].ChooseYourDrive;
	Menu.MenuEntry[0].Shortcut="";
	Menu.MenuEntry[0].Line=-1;
	Menu.AnzEntrys=1;

	for (c=3; c<=26; c++)
	{
    	Regs.h.ah=0x36; //Get Drive Allocation Information
		Regs.h.dl=c;
    	int386(0x21,&Regs,&Regs);

		//Keine Netzlaufwerke !
		if (IsDriveReal(c))
		{
			if (_dos_getdiskfree (c, &DiskData)==0)
			{
				Free = DiskData.avail_clusters * DiskData.sectors_per_cluster * DiskData.bytes_per_sector / 1024;

				if (Free > NeededKbFree)
				{
					Menu.MenuEntry[Menu.AnzEntrys].Text=THMalloc(80);
					Menu.MenuEntry[Menu.AnzEntrys].Shortcut=THMalloc(2);
					Menu.MenuEntry[Menu.AnzEntrys].Line=c+'A'-1;
					sprintf (Menu.MenuEntry[Menu.AnzEntrys].Text, "%s %c:", Messages[Language].Drive, c+'A'-1);
					sprintf (Menu.MenuEntry[Menu.AnzEntrys].Shortcut, "%c", c+'A'-1);
					Menu.AnzEntrys++;
				}
			}
		}
	}

	//WÅrde Åberhaupt ein Eintrag hinzugefÅgt ?
	if (Menu.AnzEntrys==1)
	{
		char Buffer[250];

		sprintf (Buffer, Messages[Language].NotEnoughDiskSpace, NeededKbFree);
		ErrorWindow (Buffer);
		_setvideomode (_TEXTC80);
		exit(-1);
	}

	SetupTargetDrive = SelectionWindow (&Menu);
}

//------------------------------------------------------------------------------
//Erzeugt ein MenÅ mit den erlaubten Drives die ein CD-Rom seien kînnten:
//------------------------------------------------------------------------------
void SelectCDDrive (void)
{
	MENU Menu;
	SILONG c;
	struct diskfree_t DiskData;

	//MenÅtitel:
	Menu.MenuEntry[0].Text=Messages[Language].ChooseYourCDDrive;
	Menu.MenuEntry[0].Shortcut="";
	Menu.MenuEntry[0].Line=-1;
	Menu.AnzEntrys=1;

	for (c=3; c<=26; c++)
	{
		if (_dos_getdiskfree (c, &DiskData)==0)
		{
			Menu.MenuEntry[Menu.AnzEntrys].Text=THMalloc(80);
			Menu.MenuEntry[Menu.AnzEntrys].Shortcut=THMalloc(2);
			Menu.MenuEntry[Menu.AnzEntrys].Line=c+'A'-1;
			sprintf (Menu.MenuEntry[Menu.AnzEntrys].Text, "%s %c:", Messages[Language].Drive, c+'A'-1);
			sprintf (Menu.MenuEntry[Menu.AnzEntrys].Shortcut, "%c", c+'A'-1);
			Menu.AnzEntrys++;
		}
	}

	//Falls eines der Laufwerke Probleme machte ist das nicht so wichtig.
	//(CD nicht eingelegt oder so.)
	WasHardError = FALSE;

	SetupCDDrive = SelectionWindow (&Menu);
}

//------------------------------------------------------------------------------
//LÑ·t den Benutzer was eingeben, gibt Pointer auf static Buffer zurÅck:
//------------------------------------------------------------------------------
char *UserEntersString (char *Headline, char *DefString)
{
	SILONG Size;      	//Fenstergrî·e
	SILONG LastTime=0;	//Zeit-Counter fÅrs Cursorblinken
	SILONG Key;          //fÅr Tastaurabfrage
	BOOL	 BlinkState=0; //Flag Åber den Cursorzustand
	static char Buffer[200];
	struct OPM BackupOpm;

	//ggf. bestehendes Werbe-Fenster schliessen:
	EasyPrintWindow (NULL);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, 320, 200, 0, 0);

	sprintf (Buffer, "%c:\\%s", SetupTargetDrive, DefString);

	//Trick 17: Fenster nur beim ersten mal Wachsen lassen:
	Size=5;

	while (1)
	{
		//Fenster beim Redraw nicht weiter vergrî·ern:
		for (; Size<RESOLUTION_Y/2; Size+=WIN_EXPLODE_SPEED)
		{
			Window3d (&SetupOpm,
				 		3,
				 		RESOLUTION_Y/3*2 - Size,
		       		RESOLUTION_X-1-3,
				 		RESOLUTION_Y/3*2 + Size,
				 		MENU_BORDER_COLOR_LIGHT,
				 		MENU_BORDER_COLOR,
				 		MENU_BORDER_COLOR_DARK,
				 		MENU_BACK_COLOR);

			//Headline-Text schreiben:
			if (!PrintTextInFrame (Headline,
									  	MENU_FONT_COLOR,
									  	&SetupOpm,
									  	3+MENU_BORDER_WIDTH+2,
				 					  	RESOLUTION_Y/3*2 - Size+2+MENU_BORDER_WIDTH,
									  	RESOLUTION_X-1-MENU_BORDER_WIDTH-3,
				 					  	RESOLUTION_Y/3*2 + Size-2-FONT_Y_SIZE-MENU_BORDER_WIDTH))
			{
				//passt nicht rein:
				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
				continue;
			}

			break;
		}
		Rectangle3d (&SetupOpm,
				 	 	3+1+MENU_BORDER_WIDTH,
				 	 	RESOLUTION_Y/3*2 + Size-6-FONT_Y_SIZE,
		       	 	RESOLUTION_X-1-3-1-MENU_BORDER_WIDTH,
				 	 	RESOLUTION_Y/3*2 + Size-MENU_BORDER_WIDTH-1,
				 	 	MENU_BORDER_COLOR_LIGHT,
				 	 	MENU_BORDER_COLOR,
				 	 	MENU_BORDER_COLOR_DARK);

		OPM_Write (&SetupOpm,
					  Buffer,
					  3+MENU_BORDER_WIDTH+2,
					  RESOLUTION_Y/3*2 + Size-6-FONT_Y_SIZE+2,
					  MENU_FONT_COLOR);

 		DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

		Update_input();

		if ( ((SILONG*)0x46C)[0] - LastTime>10 )
		{
			LastTime=((SILONG*)0x46C)[0];
			BlinkState^=1;

			if (BlinkState)
			{
				strcat (Buffer, "#");
			}
			else
			{
				Buffer[strlen(Buffer)-1]=0;
			}
		}
		if (kbhit())
		{
			//ggf. Cursor lîschen:
			if (BlinkState)
			{
				BlinkState = 0;
				Buffer[strlen(Buffer)-1]=0;
			}

			Key=getch();

			//Backspace:
			if (Key==8)
			{
				//Der Anfang "?:\" kann nicht gelîscht werden:
				if (strlen(Buffer)>3)
					Buffer[strlen(Buffer)-1]=0;
			}
			if (Key>='a' && Key<='z') Key=Key+'A'-'a';
			if ((Key>='A' && Key<='Z') || Key=='\\' || Key=='_')
			{
				if (strlen(Buffer)<35)
				{
					//Zeichen anhÑngen:
					Buffer[strlen(Buffer)]=Key;
					Buffer[strlen(Buffer)+1]=0;
				}
			}
			if (Key==13)
			{
				//Restore the old screen:
				OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
				OPM_Del (&BackupOpm);
				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

				return (Buffer);
			}
		}
	}
}

//------------------------------------------------------------------------------
//Verwaltet das Balkenfenster fÅr die Fortschrittsanzeige zum kopieren.
//  1 = Fenster erîffnen (Es darf noch nicht offen sein)
//  0 = Fenster aktualisieren (Es mu· schon offen sein)
// -1 = Fenster schliessen (Es mu· offen sein)
//------------------------------------------------------------------------------
void AdministrateProgressWindow (SILONG NewState)
{
	static struct OPM BackupOpm;
	static BOOL WinOpen = 0;

	switch (NewState)
	{
		case 1:
			if (!WinOpen)
			{
				WinOpen = TRUE;

				//Save the old screen:
				OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
				OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, 320, 200, 0, 0);

				//Inhalt aktualisieren:
				AdministrateProgressWindow (0);
			}
			break;

		case 0:
			if (WinOpen)
			{
				Rectangle3d (&SetupOpm,
							 10,
					 		 RESOLUTION_Y - 31,
		          		 RESOLUTION_X - 10,
					 		 RESOLUTION_Y - 10,
					 		 MENU_BORDER_COLOR_LIGHT,
					 		 MENU_BORDER_COLOR,
					 		 MENU_BORDER_COLOR_DARK);

				Rectangle3d (&SetupOpm,
							 	12,
					 		 	RESOLUTION_Y - 20,
		          		 	RESOLUTION_X - 12,
					 		 	RESOLUTION_Y - 12,
					 		 	MENU_BORDER_COLOR_DARK,
					 		 	MENU_BORDER_COLOR,
					 		 	MENU_BORDER_COLOR_LIGHT);

				OPM_Write (&SetupOpm,
							  Messages[Language].Copying,
							  14,
					 		  RESOLUTION_Y - 29,
					  		  MENU_FONT_COLOR);

				if (SetupNeededKbFree)
				{
					if (SetupCopiedBytes/1024 > SetupNeededKbFree)
						SetupCopiedBytes=SetupNeededKbFree*1024;

					Rectangle3d (&SetupOpm,
							 		13,
					 		 		RESOLUTION_Y - 19,
		          		 		13+ (SetupCopiedBytes/1024*(RESOLUTION_X-26)/SetupNeededKbFree),
					 		 		RESOLUTION_Y - 13,
					 		 		ERROR_BORDER_COLOR_LIGHT,
					 		 		ERROR_BORDER_COLOR,
					 		 		ERROR_BORDER_COLOR_DARK);
				}
				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			}
			break;

		case -1:
			if (WinOpen)
			{
				WinOpen = FALSE;

				//Restore the old screen:
				OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
				OPM_Del (&BackupOpm);
				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			}
			break;
	}
}

//------------------------------------------------------------------------------
//Gibt einen Text parallel zum Kopieren aus. Verschwindet erst beim Aufruf mit
//Parameter NULL
//------------------------------------------------------------------------------
void EasyPrintWindow (char *Text)
{
	static struct OPM BackupOpm;
	static BOOL WinOpen = 0;
	SILONG Size;

	if (Text)
	{
		if (WinOpen) EasyPrintWindow (NULL);

		WinOpen = TRUE;

		//Save the old screen:
		OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
		OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, 320, 200, 0, 0);

		//Fenster immer weiter wachsen lassen:
		for (Size=5; Size<RESOLUTION_Y/2; Size+=WIN_EXPLODE_SPEED)
		{
			Window3d (&SetupOpm,
					 	RESOLUTION_X/2 - Size*1.6,
					 	RESOLUTION_Y/3 - Size,
		          	RESOLUTION_X/2 + Size*1.6,
					 	RESOLUTION_Y/3 + Size,
					 	MENU_BORDER_COLOR_LIGHT,
					 	MENU_BORDER_COLOR,
					 	MENU_BORDER_COLOR_DARK,
					 	MENU_BACK_COLOR);

			if (PrintTextInFrame (Text,
									 	MENU_FONT_COLOR,
									  	&SetupOpm,
									 	RESOLUTION_X/2 - Size*1.6 + MENU_BORDER_WIDTH+2,
									 	RESOLUTION_Y/3 - Size     + MENU_BORDER_WIDTH+2,
									 	RESOLUTION_X/2 + Size*1.6 - MENU_BORDER_WIDTH-2,
									 	RESOLUTION_Y/3 + Size     - MENU_BORDER_WIDTH-1))
			{
				//Wachstum abbrechen, da Text in Fenster passt:
				break;
			}
			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
		}
		DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

		if (Size>=RESOLUTION_Y/2)
			InternalError (ERR_ERRTEXT_TOO_BIG);
	}
	else
	{
		if (WinOpen)
		{
			WinOpen = FALSE;

			//Restore the old screen:
			OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
			OPM_Del (&BackupOpm);
			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
		}
	}
}
