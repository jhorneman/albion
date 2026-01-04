//------------------------------------------------------------------------------
//Window.c - die Handleroutinen fÅr die einzelnen Fenstertypen:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Gibt ein rotes Fenster mit einem OK-Butten aus.
//------------------------------------------------------------------------------
void ErrorWindow (UNCHAR *Text)
{
	GeneralWindow (Text,
						Messages[Language].HeadlineErrorWindow,
						Messages[Language].OK,
						ERROR_COLOR,
						ERROR_COLOR_LIGHT,
						ERROR_COLOR_DARK,
						ERROR_COLOR_HEADLINE_BACK,
						ERROR_COLOR_HEADLINE_TEXT,
						ERROR_COLOR_BUTTON_FRAME,
						ERROR_COLOR_BUTTON_HIGHLIGHT);
}

//------------------------------------------------------------------------------
//Gibt ein normales Fenster mit einem OK-Button aus.
//------------------------------------------------------------------------------
void InfoWindow (UNCHAR *Text)
{
	GeneralWindow (Text,
						Messages[Language].HeadlineInfoWindow,
						Messages[Language].OK,
						MENU_COLOR,
						MENU_COLOR_LIGHT,
						MENU_COLOR_DARK,
						MENU_COLOR_HEADLINE_BACK,
						MENU_COLOR_HEADLINE_TEXT,
						MENU_COLOR_BUTTON_FRAME,
						MENU_COLOR_BUTTON_HIGHLIGHT);
}

//------------------------------------------------------------------------------
//Gibt ein Error/Info-Style Fenster parametrisiert aus.
//------------------------------------------------------------------------------
void GeneralWindow (UNCHAR *Text, UNCHAR *Headline, UNCHAR *Button, UNCHAR Color, UNCHAR ColorLight, UNCHAR ColorDark, UNCHAR ColorHeadlineBack, UNCHAR ColorHeadlineText, UNCHAR ColorButtonFrame, UNCHAR ColorButtonHighlight)
{
	SILONG Size;
	struct OPM BackupOpm;

	//ggf. bestehendes Werbe-Fenster schliessen:
	EasyPrintWindow (NULL);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);

	//Fenster immer weiter wachsen lassen:
	for (Size=10; Size<RESOLUTION_Y/2; Size+=WIN_EXPLODE_SPEED)
	{
		//Standart-Fenster generieren:
		Window3d (&SetupOpm,
					 RESOLUTION_X/2 - Size*1.6,
					 RESOLUTION_Y/2 - Size,
		          RESOLUTION_X/2 + Size*1.6,
					 RESOLUTION_Y/2 + Size,
					 ColorLight,
					 Color,
					 ColorDark,
					 Color);

		//Headline-Box anfertigen:
		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 - Size*1.6 + 4,
					 	 RESOLUTION_Y/2 - Size + 4,
		          	 RESOLUTION_X/2 + Size*1.6 - 4,
					 	 RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + 1,
						 ColorHeadlineBack,
						 ColorHeadlineBack,
						 ColorHeadlineBack);
		Rectangle (&SetupOpm,
					 	RESOLUTION_X/2 - Size*1.6 + 5,
					 	RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + 2,
		          	RESOLUTION_X/2 + Size*1.6 - 4,
					 	RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + 2,
						ColorLight);

		//Headline-Text schreiben:
		if (!PrintCenteredTextInFrame (Headline,
												 ColorHeadlineText,
												 &SetupOpm,
				 	 							 RESOLUTION_X/2 - Size*1.6 + (MENU_BORDER_WIDTH + 3),
				 	 							 RESOLUTION_Y/2 - Size + 5,
				 	 							 RESOLUTION_X/2 + Size*1.6 - (MENU_BORDER_WIDTH + 3),
				 	 							 RESOLUTION_Y/2 - Size + 7 + FONT_Y_SIZE))
		{
			//Passt nicht rein!
			continue;
		}


		//Das Text-Fenster:
		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 - Size*1.6 + 5,
					 	 RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + 3,
		          	 RESOLUTION_X/2 + Size*1.6 - 5,
					 	 RESOLUTION_Y/2 + Size - 34,
					 	 ColorDark,
					 	 Color,
					 	 ColorLight);

		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 - Size*1.6 + (5+1),
					 	 RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + (3+1),
		          	 RESOLUTION_X/2 + Size*1.6 - (5+1),
					 	 RESOLUTION_Y/2 + Size - (34+1),
					 	 BUTTON_COLOR,
					 	 BUTTON_COLOR,
					 	 BUTTON_COLOR);

		if (!PrintTextInFrame (Text,
									  BUTTON_COLOR_TEXT,
									  &SetupOpm,
					 	 			  RESOLUTION_X/2 - Size*1.6 + (5+2),
					 	 			  RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + (3+2),
		          	 			  RESOLUTION_X/2 + Size*1.6 - (5+2),
					 	 			  RESOLUTION_Y/2 + Size - (34+2)))
		{
			//Der Text passt nicht:
			continue;
		}

		//OK-Button machen:
		DrawButton (&SetupOpm,
					 	RESOLUTION_X/2 - Size + MENU_BORDER_WIDTH*1.4+1,
					 	RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (10 + 4 + FONT_Y_SIZE),
		          	RESOLUTION_X/2 + Size - MENU_BORDER_WIDTH*1.4-1,
					 	RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - 9,
						ColorButtonFrame);

		//OK-Button-Text schreiben:
		if (!PrintCenteredTextInFrame (Button,
									  			 BUTTON_COLOR_TEXT,
									  			 &SetupOpm,
					 	 						 RESOLUTION_X/2 - Size + MENU_BORDER_WIDTH*1.4 + (1+1),
					 	 						 RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (9 + 4 + FONT_Y_SIZE)+2,
		          	 						 RESOLUTION_X/2 + Size - MENU_BORDER_WIDTH*1.4 - (1+1),
					 	 						 RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (9)))
		{
			//schon das OK passte nicht rein:
			continue;
		}

		//Wachstum abbrechen, da Text in Fenster passt:
		break;
	}
	if (Size>=RESOLUTION_Y/2)
		InternalError (ERR_ERRTEXT_TOO_BIG);

	DrawWinShadow (&SetupOpm,
					   RESOLUTION_X/2 - Size*1.6,
					   RESOLUTION_Y/2 - Size,
					   RESOLUTION_X/2 + Size*1.6,
					   RESOLUTION_Y/2 + Size);

	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	//Einen Knopf verwalten. Das Ergebnis interessiert uns gar nicht:
	GetButtonAnswer (1,
						  0,
					 	  (SILONG)(RESOLUTION_X/2 - Size + MENU_BORDER_WIDTH*1.4+1),
					 	  (SILONG)(RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (10 + 4 + FONT_Y_SIZE)),
		          	  (SILONG)(RESOLUTION_X/2 + Size - MENU_BORDER_WIDTH*1.4-1),
					 	  (SILONG)(RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - 9),
						  "\x1b");                                        //ShortCut

	//Restore the old screen:
	OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
	OPM_Del (&BackupOpm);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
}

//------------------------------------------------------------------------------
//Gibt ein YesNo-Fenster mit einem Text aus:
//------------------------------------------------------------------------------
BOOL AssertWindow (UNCHAR *Text)
{
   SILONG rc;
	SILONG Size;
	struct OPM BackupOpm;

	//ggf. bestehendes Werbe-Fenster schliessen:
	EasyPrintWindow (NULL);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);

	//Fenster immer weiter wachsen lassen:
	for (Size=10; Size<RESOLUTION_Y/2; Size+=WIN_EXPLODE_SPEED)
	{
		//Standart-Fenster generieren:
		Window3d (&SetupOpm,
					 RESOLUTION_X/2 - Size*1.6,
					 RESOLUTION_Y/2 - Size,
		          RESOLUTION_X/2 + Size*1.6,
					 RESOLUTION_Y/2 + Size,
					 MENU_COLOR_LIGHT,
					 MENU_COLOR,
					 MENU_COLOR_DARK,
					 MENU_COLOR);

		//Headline-Box anfertigen:
		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 - Size*1.6 + 4,
					 	 RESOLUTION_Y/2 - Size + 4,
		          	 RESOLUTION_X/2 + Size*1.6 - 4,
					 	 RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + 1,
						 MENU_COLOR_HEADLINE_BACK,
						 MENU_COLOR_HEADLINE_BACK,
						 MENU_COLOR_HEADLINE_BACK);
		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 - Size*1.6 + 5,
					 	 RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + 2,
		          	 RESOLUTION_X/2 + Size*1.6 - 4,
					 	 RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + 2,
						 MENU_COLOR_LIGHT,
						 MENU_COLOR_LIGHT,
						 MENU_COLOR_LIGHT);

		//Headline-Text schreiben:
		if (!PrintCenteredTextInFrame (Messages[Language].HeadlineAssertWindow,
												 MENU_COLOR_HEADLINE_TEXT,
												 &SetupOpm,
				 	 							 RESOLUTION_X/2 - Size*1.6 + (MENU_BORDER_WIDTH + 3),
				 	 							 RESOLUTION_Y/2 - Size + 5,
				 	 							 RESOLUTION_X/2 + Size*1.6 - (MENU_BORDER_WIDTH + 3),
				 	 							 RESOLUTION_Y/2 - Size + 7 + FONT_Y_SIZE))
		{
			//Passt nicht rein!
			continue;
		}


		//Das Text-Fenster:
		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 - Size*1.6 + 5,
					 	 RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + 3,
		          	 RESOLUTION_X/2 + Size*1.6 - 5,
					 	 RESOLUTION_Y/2 + Size - 34,
					 	 MENU_COLOR_DARK,
					 	 MENU_COLOR,
					 	 MENU_COLOR_LIGHT);

		Rectangle3d (&SetupOpm,
					 	 RESOLUTION_X/2 - Size*1.6 + (5+1),
					 	 RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + (3+1),
		          	 RESOLUTION_X/2 + Size*1.6 - (5+1),
					 	 RESOLUTION_Y/2 + Size - (34+1),
					 	 BUTTON_COLOR,
					 	 BUTTON_COLOR,
					 	 BUTTON_COLOR);

		if (!PrintTextInFrame (Text,
									  BUTTON_COLOR_TEXT,
									  &SetupOpm,
					 	 			  RESOLUTION_X/2 - Size*1.6 + (5+2),
					 	 			  RESOLUTION_Y/2 - Size + 4 + FONT_Y_SIZE + (3+2),
		          	 			  RESOLUTION_X/2 + Size*1.6 - (5+2),
					 	 			  RESOLUTION_Y/2 + Size - (34+2)))
		{
			//Der Text passt nicht:
			continue;
		}

		//Yes-Button machen:
		DrawButton (&SetupOpm,
					 	RESOLUTION_X/2 - Size + MENU_BORDER_WIDTH*1.4+1,
					 	RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (10 + 4 + FONT_Y_SIZE),
		          	RESOLUTION_X/2 - 2,
					 	RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - 9,
						MENU_COLOR_BUTTON_FRAME);

		//Yes-Button-Text schreiben:
		if (!PrintCenteredTextInFrame (Messages[Language].Yes,
									  			 BUTTON_COLOR_TEXT,
									  			 &SetupOpm,
					 	 						 RESOLUTION_X/2 - Size + MENU_BORDER_WIDTH*1.4 + (1+1),
					 	 						 RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (9 + 4 + FONT_Y_SIZE)+2,
		          	 						 RESOLUTION_X/2 - 2,
					 	 						 RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (9)))
		{
			//schon das Yes passte nicht rein:
			continue;
		}

		//No-Button machen:
		DrawButton (&SetupOpm,
					 	RESOLUTION_X/2 + 2,
					 	RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (10 + 4 + FONT_Y_SIZE),
		          	RESOLUTION_X/2 + Size - MENU_BORDER_WIDTH*1.4-1,
					 	RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - 9,
						MENU_COLOR_BUTTON_FRAME);

		//No-Button-Text schreiben:
		if (!PrintCenteredTextInFrame (Messages[Language].No,
									  			 BUTTON_COLOR_TEXT,
									  			 &SetupOpm,
					 	 						 RESOLUTION_X/2 + 2,
					 	 						 RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (9 + 4 + FONT_Y_SIZE)+2,
		          	 						 RESOLUTION_X/2 + Size - MENU_BORDER_WIDTH*1.4 - (1+1),
					 	 						 RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (9)))
		{
			//schon das No passte nicht rein:
			continue;
		}

		//Wachstum abbrechen, da Text in Fenster passt:
		break;
	}
	if (Size>=RESOLUTION_Y/2)
		InternalError (ERR_ERRTEXT_TOO_BIG);

	DrawWinShadow (&SetupOpm,
					   RESOLUTION_X/2 - Size*1.6,
					   RESOLUTION_Y/2 - Size,
					   RESOLUTION_X/2 + Size*1.6,
					   RESOLUTION_Y/2 + Size);

	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	//Die beiden Knîpfe verwalten:
	rc = GetButtonAnswer (2,
						  		 0,
						  		 //Yes-Button:
					 	  		 (SILONG)(RESOLUTION_X/2 - Size + MENU_BORDER_WIDTH*1.4+1),
					 	  		 (SILONG)(RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (10 + 4 + FONT_Y_SIZE)),
		          	  		 (SILONG)(RESOLUTION_X/2 - 2),
					 	  		 (SILONG)(RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - 9),
						  		 "jJyY",                                         //ShortCut

						  		 //No-Button:
					 	  		 (SILONG)(RESOLUTION_X/2 + 2),
					 	  		 (SILONG)(RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - (10 + 4 + FONT_Y_SIZE)),
		          	  		 (SILONG)(RESOLUTION_X/2 + Size - MENU_BORDER_WIDTH*1.4-1),
					 	  		 (SILONG)(RESOLUTION_Y/2 + Size - MENU_BORDER_WIDTH - 9),
						  		 "nN\x1b");                                      //ShortCut

	//Restore the old screen:
	OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
	OPM_Del (&BackupOpm);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
	return (!rc);
}

//------------------------------------------------------------------------------
//Verwaltet beliebig viele Buttons. Aufruf immer:
// - AnzButtons
// - AnzButtons * ⁄ SILONG x1, y1, x2, y2
//                ¿ UNCHAR *KeyboardShortcuts
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
		Menu.MenuEntry[c].Shortcut = va_arg (Vars, UNCHAR *);
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
	SILONG LastMouseX,LastMouseY;
	SILONG HighlightOption;
	SILONG Key;
	BOOL	 RefreshHighlightFrame=1;
	BOOL	 HighlightPushed=0;
	struct OPM BackupOpm;

	HighlightOption = DefButton;

	//What a silly user !
	if (pMenu->AnzEntrys<=0)
		return (0);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);

	while (1)
	{
		Last_button_state=Button_state;
		LastMouseX=Mouse_X;
		LastMouseY=Mouse_Y;
		Update_input();

		if (RefreshHighlightFrame)
		{
			//highlight the active option:
			OPM_Box (&SetupOpm,
						pMenu->MenuEntry[HighlightOption].x1-1,
						pMenu->MenuEntry[HighlightOption].y1-1,
						pMenu->MenuEntry[HighlightOption].x2-pMenu->MenuEntry[HighlightOption].x1+3,
						pMenu->MenuEntry[HighlightOption].y2-pMenu->MenuEntry[HighlightOption].y1+3,
						MENU_COLOR_BUTTON_HIGHLIGHT);
			OPM_Box (&SetupOpm,
						pMenu->MenuEntry[HighlightOption].x1-2,
						pMenu->MenuEntry[HighlightOption].y1-2,
						pMenu->MenuEntry[HighlightOption].x2-pMenu->MenuEntry[HighlightOption].x1+5,
						pMenu->MenuEntry[HighlightOption].y2-pMenu->MenuEntry[HighlightOption].y1+5,
						MENU_COLOR_BUTTON_HIGHLIGHT);

			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

			RefreshHighlightFrame = 0;
		}

		if (kbhit())
		{
			Key=getch();
			if (!Key) Key=getch()+1000;

			//Um Konflikte zu vermeiden gehen Tasten bei gedrÅckter Maus nicht:
			if (!(Button_state&1))
			{
				//Mit Hotkeys vergleichen:
				for (c=0; c<pMenu->AnzEntrys; c++)
				{
					for (d=0; d<strlen(pMenu->MenuEntry[c].Shortcut); d++)
					{
						if ((pMenu->MenuEntry[c].Shortcut[d] == 255 && pMenu->MenuEntry[c].Shortcut[d+1] == Key-1000) ||
					    	(pMenu->MenuEntry[c].Shortcut[d] == Key))
						{
							//Restore the old screen:
							OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
							OPM_Del (&BackupOpm);
							DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
							return (c);
						}

						if (pMenu->MenuEntry[c].Shortcut[d] == 255)
						{
							//Das Format \xff\x?? im Shortcur-String steht fÅr
							//zusammengesetzte Taste wie \0\x48 (Cursor Up)
							d++;
						}
					}
				}


				//MenÅpunkt selektieren:
				switch (Key)
				{
					case 1072: //Up
					case 1075: //Left
						RefreshHighlightFrame=1;
						OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
						HighlightOption = (HighlightOption+pMenu->AnzEntrys-1)%pMenu->AnzEntrys;
						break;

					case 1077: //Right
					case 1080: //Down
						RefreshHighlightFrame=1;
						OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
						HighlightOption = (HighlightOption+1)%pMenu->AnzEntrys;
						break;

					case 13:
					case 32:
						//Restore the old screen:
						OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
						OPM_Del (&BackupOpm);
						DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
						return (HighlightOption);
				}
			}
		}

		for (c=0; c<pMenu->AnzEntrys; c++)
		{
			//Ist Cursor in Box ?
			if (Mouse_X >= pMenu->MenuEntry[c].x1+1 &&
				 	Mouse_X <= pMenu->MenuEntry[c].x2-1 &&
			    	Mouse_Y >= pMenu->MenuEntry[c].y1+1 &&
				 	Mouse_Y <= pMenu->MenuEntry[c].y2-1)
			{
				break;
			}
		}

		//Cursor in box => set highlight cursor
		if (HighlightOption != c && !(Button_state&1) && c<pMenu->AnzEntrys)
		if (Mouse_X!=LastMouseX || Mouse_Y!=LastMouseY)
		{
			HighlightOption = c;
			OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
			RefreshHighlightFrame = 1;
		}

		//Cursor moves when button pushed:
		if (Button_state&1)
		if (Mouse_X!=LastMouseX || Mouse_Y!=LastMouseY)
		{
			if (HighlightOption==c && HighlightPushed==0)
			{
				PushButton (&SetupOpm,
								pMenu->MenuEntry[c].x1,
								pMenu->MenuEntry[c].y1,
								pMenu->MenuEntry[c].x2,
								pMenu->MenuEntry[c].y2);

				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
				HighlightPushed = 1;
			}
			else if (HighlightOption!=c && HighlightPushed==1)
			{
				UnPushButton (&SetupOpm,
									pMenu->MenuEntry[HighlightOption].x1,
									pMenu->MenuEntry[HighlightOption].y1,
									pMenu->MenuEntry[HighlightOption].x2,
									pMenu->MenuEntry[HighlightOption].y2);

				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
				HighlightPushed = 0;
			}
		}

		//DrÅckt der User den LMB?
		if ((Button_state&1) && (!(Last_button_state&1)) && HighlightOption == c && HighlightPushed==0)
		{
			PushButton (&SetupOpm,
							pMenu->MenuEntry[c].x1,
							pMenu->MenuEntry[c].y1,
							pMenu->MenuEntry[c].x2,
							pMenu->MenuEntry[c].y2);

			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			HighlightPushed = 1;
		}

		//Hat er ihn jetzt innerhalb der Highlight-Box losgelassen ?
		if ((!(Button_state&1)) && (Last_button_state&1) && HighlightOption==c && HighlightPushed)
		{
			//Restore the old screen:
			OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
			OPM_Del (&BackupOpm);
			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			return (c);
		}

		//Hat er ihn woanders losgelassen?
		if ((!(Button_state&1)) && (Last_button_state&1) && HighlightOption!=c && (!HighlightPushed) && c<pMenu->AnzEntrys)
		{
			HighlightOption = c;
			OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
			RefreshHighlightFrame = 1;
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
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);

	SizeX=0;
	SizeY=MENU_BORDER_WIDTH*2+2+32;
	for (c=0; c<pMenu->AnzEntrys; c++)
	{
		if (pMenu->MenuEntry[c].Text && pMenu->MenuEntry[c].Line!=-1)
		{
			if (PixelStringLength (pMenu->MenuEntry[c].Text)+MENU_BORDER_WIDTH*2+48 > SizeX)
				SizeX = PixelStringLength (pMenu->MenuEntry[c].Text)+MENU_BORDER_WIDTH*2+48;

			SizeY+=FONT_Y_SIZE+8;
		}
		else if (pMenu->MenuEntry[c].Text && pMenu->MenuEntry[c].Line==-1)
		{
			if (PixelStringLength (pMenu->MenuEntry[c].Text)+MENU_BORDER_WIDTH*2+36 > SizeX)
				SizeX = PixelStringLength (pMenu->MenuEntry[c].Text)+MENU_BORDER_WIDTH*2+36;
		}
		else SizeY+=3;
	}

	Window3d (&SetupOpm,
				 RESOLUTION_X/2 - SizeX/2,
				 RESOLUTION_Y/2 - SizeY/2,
		       RESOLUTION_X/2 - SizeX/2+SizeX,
				 RESOLUTION_Y/2 - SizeY/2+SizeY,
				 MENU_COLOR_LIGHT,
				 MENU_COLOR,
				 MENU_COLOR_DARK,
				 MENU_COLOR);
	DrawWinShadow (&SetupOpm,
						RESOLUTION_X/2 - SizeX/2,
						RESOLUTION_Y/2 - SizeY/2,
		      		RESOLUTION_X/2 - SizeX/2+SizeX,
						RESOLUTION_Y/2 - SizeY/2+SizeY);

	//Headline-Bar:
	Rectangle3d (&SetupOpm,
					 RESOLUTION_X/2 - SizeX/2 + MENU_BORDER_WIDTH+1,
					 RESOLUTION_Y/2 - SizeY/2 + MENU_BORDER_WIDTH+1,
		      	 RESOLUTION_X/2 - SizeX/2+SizeX - MENU_BORDER_WIDTH-1,
					 RESOLUTION_Y/2 - SizeY/2 + MENU_BORDER_WIDTH+12,
					 MENU_COLOR_HEADLINE_BACK,
					 MENU_COLOR_HEADLINE_BACK,
					 MENU_COLOR_HEADLINE_BACK);
	Rectangle (&SetupOpm,
					RESOLUTION_X/2 - SizeX/2 + MENU_BORDER_WIDTH+2,
					RESOLUTION_Y/2 - SizeY/2 + MENU_BORDER_WIDTH+13,
		      	RESOLUTION_X/2 - SizeX/2+SizeX - MENU_BORDER_WIDTH-1,
					RESOLUTION_Y/2 - SizeY/2 + MENU_BORDER_WIDTH+13,
					MENU_COLOR_LIGHT);

	y = RESOLUTION_Y/2 - SizeY/2 + MENU_BORDER_WIDTH + 1+25;

	for (c=0; c<pMenu->AnzEntrys; c++)
	{
		if (pMenu->MenuEntry[c].Text)
		{
			if (pMenu->MenuEntry[c].Line!=-1)
			{
				//Echter Auswahlpunkt:
				pMenu->MenuEntry[c].x1 = RESOLUTION_X/2 - SizeX/2 + MENU_BORDER_WIDTH + 22;
				pMenu->MenuEntry[c].y1 = y+1;
				pMenu->MenuEntry[c].x2 = RESOLUTION_X/2 - SizeX/2 + SizeX - MENU_BORDER_WIDTH - 22;
				pMenu->MenuEntry[c].y2 = y+FONT_Y_SIZE+4;

				//Kasten zeichnen:
				DrawButton (&SetupOpm,
								pMenu->MenuEntry[c].x1,
								pMenu->MenuEntry[c].y1,
								pMenu->MenuEntry[c].x2,
								pMenu->MenuEntry[c].y2,
				 				MENU_COLOR_BUTTON_FRAME);

				PrintCenteredTextInFrame (pMenu->MenuEntry[c].Text,
													BUTTON_COLOR_TEXT,
									   			&SetupOpm,
													pMenu->MenuEntry[c].x1,
													pMenu->MenuEntry[c].y1+2,
													pMenu->MenuEntry[c].x2,
													pMenu->MenuEntry[c].y2);

				y+=FONT_Y_SIZE+8;

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
				pMenu->MenuEntry[c].x1 = RESOLUTION_X/2 - SizeX/2 + MENU_BORDER_WIDTH+1;
				pMenu->MenuEntry[c].y1 = RESOLUTION_Y/2 - SizeY/2 + MENU_BORDER_WIDTH+2;
				pMenu->MenuEntry[c].x2 = RESOLUTION_X/2 - SizeX/2+SizeX - MENU_BORDER_WIDTH-1;
				pMenu->MenuEntry[c].y2 = RESOLUTION_Y/2 - SizeY/2 + MENU_BORDER_WIDTH+12;

				PrintCenteredTextInFrame (pMenu->MenuEntry[c].Text,
												  MENU_COLOR_HEADLINE_TEXT,
												  &SetupOpm,
												  pMenu->MenuEntry[c].x1+1,
												  pMenu->MenuEntry[c].y1+1,
												  pMenu->MenuEntry[c].x2,
												  pMenu->MenuEntry[c].y2);

				//y+=FONT_Y_SIZE+5;
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
	OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
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
		UNCHAR Buffer[250];

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
UNCHAR *UserEntersString (UNCHAR *Headline, UNCHAR *DefString)
{
	SILONG Size;      	//Fenstergrî·e
	SILONG LastTime=0;	//Zeit-Counter fÅrs Cursorblinken
	SILONG Key;          //fÅr Tastaurabfrage
	BOOL	 BlinkState=0; //Flag Åber den Cursorzustand
	static UNCHAR Buffer[200];
	struct OPM BackupOpm;

	//ggf. bestehendes Werbe-Fenster schliessen:
	EasyPrintWindow (NULL);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);

	sprintf (Buffer, "%c:\\%s", SetupTargetDrive, DefString);

	//Feste Grî·e:
	Size=16;

	while (1)
	{
		Window3d (&SetupOpm,
				 	RESOLUTION_X/2-155,
				 	RESOLUTION_Y/3*2 - Size,
		       	RESOLUTION_X/2+155,
				 	RESOLUTION_Y/3*2 + Size,
				 	MENU_COLOR_LIGHT,
				 	MENU_COLOR,
				 	MENU_COLOR_DARK,
				 	MENU_COLOR);
		DrawWinShadow (&SetupOpm,
				 			RESOLUTION_X/2-155,
				 			RESOLUTION_Y/3*2 - Size,
		       			RESOLUTION_X/2+155,
				 			RESOLUTION_Y/3*2 + Size);

		Rectangle3d (&SetupOpm,
				 			RESOLUTION_X/2-155+4,
				 			RESOLUTION_Y/3*2 - Size+4,
		       			RESOLUTION_X/2+155-4,
				 			RESOLUTION_Y/3*2 - Size+4+10,
				 			MENU_COLOR_HEADLINE_BACK,
				 			MENU_COLOR_HEADLINE_BACK,
				 			MENU_COLOR_HEADLINE_BACK);
		Rectangle (&SetupOpm,
				 		RESOLUTION_X/2-155+5,
				 		RESOLUTION_Y/3*2 - Size+4+11,
		       		RESOLUTION_X/2+155-4,
				 		RESOLUTION_Y/3*2 - Size+4+11,
				 		MENU_COLOR_LIGHT);

		//Headline-Text schreiben:
		PrintTextInFrame (Headline,
								MENU_COLOR_HEADLINE_TEXT,
								&SetupOpm,
				 			 	RESOLUTION_X/2-155+5,
				 			 	RESOLUTION_Y/3*2 - Size+5,
		       			 	RESOLUTION_X/2+155-5,
				 			 	RESOLUTION_Y/3*2 - Size+5+12);

		Rectangle3d (&SetupOpm,
				 		 RESOLUTION_X/2-155+5,
				 		 RESOLUTION_Y/3*2 - Size+4+12,
		       		 RESOLUTION_X/2+155-5,
				 		 RESOLUTION_Y/3*2 + Size-MENU_BORDER_WIDTH-2,
				 	 	 MENU_COLOR_DARK,
				 	 	 MENU_COLOR,
				 	 	 MENU_COLOR_LIGHT);

		Rectangle3d (&SetupOpm,
				 		 RESOLUTION_X/2-155+6,
				 		 RESOLUTION_Y/3*2 - Size+4+13,
		       		 RESOLUTION_X/2+155-6,
				 		 RESOLUTION_Y/3*2 + Size-MENU_BORDER_WIDTH-3,
				 	 	 BUTTON_COLOR,
				 	 	 BUTTON_COLOR,
				 	 	 BUTTON_COLOR);

		OPM_Write (&SetupOpm,
					  Buffer,
				 	  RESOLUTION_X/2-155+6,
				 	  RESOLUTION_Y/3*2 - Size+4+14,
					  BUTTON_COLOR_TEXT);

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
				OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
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
				OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);

				//Inhalt aktualisieren:
				AdministrateProgressWindow (0);
			}
			break;

		case 0:
			if (WinOpen)
			{
				Window3d (&SetupOpm,
				 			RESOLUTION_X/2-155,
				 			RESOLUTION_Y/3*2 - 16,
		       			RESOLUTION_X/2+155,
				 			RESOLUTION_Y/3*2 + 16,
				 			MENU_COLOR_LIGHT,
				 			MENU_COLOR,
				 			MENU_COLOR_DARK,
				 			MENU_COLOR);
				DrawWinShadow (&SetupOpm,
				 					RESOLUTION_X/2-155,
				 					RESOLUTION_Y/3*2 - 16,
		       					RESOLUTION_X/2+155,
				 					RESOLUTION_Y/3*2 + 16);

				Rectangle3d (&SetupOpm,
				 					RESOLUTION_X/2-155+4,
				 					RESOLUTION_Y/3*2 - 16+4,
		       					RESOLUTION_X/2+155-4,
				 					RESOLUTION_Y/3*2 - 16+4+10,
				 					MENU_COLOR_HEADLINE_BACK,
				 					MENU_COLOR_HEADLINE_BACK,
				 					MENU_COLOR_HEADLINE_BACK);
				Rectangle (&SetupOpm,
				 				RESOLUTION_X/2-155+5,
				 				RESOLUTION_Y/3*2 - 16+4+11,
		       				RESOLUTION_X/2+155-4,
				 				RESOLUTION_Y/3*2 - 16+4+11,
				 				MENU_COLOR_LIGHT);

				//Headline-Text schreiben:
				PrintTextInFrame (Messages[Language].Copying,
										MENU_COLOR_HEADLINE_TEXT,
										&SetupOpm,
				 			 			RESOLUTION_X/2-155+5,
				 			 			RESOLUTION_Y/3*2 - 16+5,
		       			 			RESOLUTION_X/2+155-5,
				 			 			RESOLUTION_Y/3*2 - 16+5+12);

				Rectangle3d (&SetupOpm,
				 		 		RESOLUTION_X/2-155+5,
				 		 		RESOLUTION_Y/3*2 - 16+4+12,
		       		 		RESOLUTION_X/2+155-5,
				 		 		RESOLUTION_Y/3*2 + 16-MENU_BORDER_WIDTH-2,
				 	 	 		MENU_COLOR_DARK,
				 	 	 		MENU_COLOR,
				 	 	 		MENU_COLOR_LIGHT);

				if (SetupNeededKbFree)
				{
					if (SetupCopiedBytes/1024 > SetupNeededKbFree)
						SetupCopiedBytes=SetupNeededKbFree*1024;

					Rectangle3d (&SetupOpm,
				 		 			RESOLUTION_X/2-155+6,
				 		 			RESOLUTION_Y/3*2 - 16+4+13,
		       		 			RESOLUTION_X/2-155+6 +((155-6)*2)*SetupCopiedBytes/1024/SetupNeededKbFree,
				 		 			RESOLUTION_Y/3*2 + 16-MENU_BORDER_WIDTH-3,
				 	 	 			BUTTON_COLOR_LIGHT,
				 	 	 			BUTTON_COLOR,
				 	 	 			BUTTON_COLOR_DARK);
				}
				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			}
			break;

		case -1:
			if (WinOpen)
			{
				WinOpen = FALSE;

				//Restore the old screen:
				OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
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
void EasyPrintWindow (UNCHAR *Text)
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
		OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);

		//Fenster immer weiter wachsen lassen:
		for (Size=5; Size<RESOLUTION_Y/2; Size+=WIN_EXPLODE_SPEED)
		{
			Window3d (&SetupOpm,
					 	RESOLUTION_X/2 - Size*1.6,
					 	RESOLUTION_Y/3 - Size,
		          	RESOLUTION_X/2 + Size*1.6,
					 	RESOLUTION_Y/3 + Size,
					 	MENU_COLOR_LIGHT,
					 	MENU_COLOR,
					 	MENU_COLOR_DARK,
					 	MENU_COLOR);

			if (PrintTextInFrame (Text,
					  		  			BUTTON_COLOR_TEXT,
									  	&SetupOpm,
									 	RESOLUTION_X/2 - Size*1.6 + MENU_BORDER_WIDTH+2,
									 	RESOLUTION_Y/3 - Size     + MENU_BORDER_WIDTH+2,
									 	RESOLUTION_X/2 + Size*1.6 - MENU_BORDER_WIDTH-2,
									 	RESOLUTION_Y/3 + Size     - MENU_BORDER_WIDTH-1))
			{
				//Wachstum abbrechen, da Text in Fenster passt:
				break;
			}
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
			OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
			OPM_Del (&BackupOpm);
			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
		}
	}
}
