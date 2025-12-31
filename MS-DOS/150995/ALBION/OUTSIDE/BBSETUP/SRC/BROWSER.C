//------------------------------------------------------------------------------
//Browser.c - Alles was zum Text Browser fÅr Setup/VGA gehîrt:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Hauptfunktion, die von au·en einfach aufgerufen wird um ein File zu zeigen:
//------------------------------------------------------------------------------
void BrowseFile (UNCHAR *Filename)
{
	SILONG c;
	SILONG Choice;  //Ergebnis vom AuswahlmenÅ
	SILONG CursorY; //Cursor im Textfenster (besser gesagt: y-hotspot links-o.)
	struct OPM BackupOpm;
	SCRIPT BrowseScript;
	UNCHAR Buffer[80];

	LoadBrowseFile (&BrowseScript, Filename);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);

	//Der Ñu·ere Bilderrahmen:
	Window3d (&SetupOpm,
				 RESOLUTION_X/2-160,
				 RESOLUTION_Y/2-100,
				 RESOLUTION_X/2+159,
				 RESOLUTION_Y/2+99,
				 MENU_COLOR_LIGHT,
				 MENU_COLOR,
				 MENU_COLOR_DARK,
				 MENU_COLOR);
	DrawWinShadow (&SetupOpm,
				 		RESOLUTION_X/2-160,
				 		RESOLUTION_Y/2-100,
				 		RESOLUTION_X/2+159,
				 		RESOLUTION_Y/2+99);

	//Die Headline:
	Rectangle3d (&SetupOpm,
				 	 RESOLUTION_X/2-160+4,
				 	 RESOLUTION_Y/2-100+4,
				 	 RESOLUTION_X/2+159-4,
				 	 RESOLUTION_Y/2-100+13,
					 MENU_COLOR_HEADLINE_BACK,
					 MENU_COLOR_HEADLINE_BACK,
					 MENU_COLOR_HEADLINE_BACK);
	Rectangle (&SetupOpm,
				  RESOLUTION_X/2-160+5,
				  RESOLUTION_Y/2-100+14,
				  RESOLUTION_X/2+159-4,
				  RESOLUTION_Y/2-100+14,
				  MENU_COLOR_LIGHT);
	PrintCenteredTextInFrame (Filename,
									  MENU_COLOR_HEADLINE_TEXT,
									  &SetupOpm,
				 					  RESOLUTION_X/2-160+5,
				 					  RESOLUTION_Y/2-100+5,
				 					  RESOLUTION_X/2+159-5,
				 					  RESOLUTION_Y/2-100+14);

	//Der innere Bilderrahmen:
	Rectangle3d (&SetupOpm,
				 	RESOLUTION_X/2-160+7,
				 	RESOLUTION_Y/2-100+15,
				 	RESOLUTION_X/2+159-7,
				 	RESOLUTION_Y/2+99-23,
					MENU_COLOR_DARK,
					MENU_COLOR,
					MENU_COLOR_LIGHT);

	//Scroll-Pfeil "line-down":
	DrawButton (&SetupOpm,
				 	RESOLUTION_X/2-160+8,
				 	RESOLUTION_Y/2+100-20,
				 	RESOLUTION_X/2-160+26,
				 	RESOLUTION_Y/2+100-8,
					MENU_COLOR_BUTTON_FRAME);
	DrawArrow (&SetupOpm,
				  RESOLUTION_X/2-160+17,
				  RESOLUTION_Y/2+100-14,
				  BUTTON_COLOR_TEXT,
              -1);

	//Scroll-Pfeil "line-up":
	DrawButton (&SetupOpm,
				 	RESOLUTION_X/2-160+30,
				 	RESOLUTION_Y/2+100 -20,
				 	RESOLUTION_X/2-160+48,
				 	RESOLUTION_Y/2+100-8,
					MENU_COLOR_BUTTON_FRAME);
	DrawArrow (&SetupOpm,
				  RESOLUTION_X/2-160+39,
				  RESOLUTION_Y/2+100-14,
				  BUTTON_COLOR_TEXT,
              1);

	//Scroll-Pfeilrahmen "page-down":
	DrawButton (&SetupOpm,
				 	RESOLUTION_X/2-160+57,
				 	RESOLUTION_Y/2+100-20,
				 	RESOLUTION_X/2-160+75,
				 	RESOLUTION_Y/2+100-8,
					MENU_COLOR_BUTTON_FRAME);
	DrawArrow (&SetupOpm,
				  RESOLUTION_X/2-160+66-3,
				  RESOLUTION_Y/2+100-14,
              BUTTON_COLOR_TEXT, -1);
	DrawArrow (&SetupOpm,
				  RESOLUTION_X/2-160+66+3,
				  RESOLUTION_Y/2+100-14,
              BUTTON_COLOR_TEXT, -1);

	//Scroll-Pfeilrahmen "page-up":
	DrawButton (&SetupOpm,
				 	RESOLUTION_X/2-160+79,
				 	RESOLUTION_Y/2+100-20,
				 	RESOLUTION_X/2-160+97,
				 	RESOLUTION_Y/2+100-8,
					MENU_COLOR_BUTTON_FRAME);
	DrawArrow (&SetupOpm,
				  RESOLUTION_X/2-160+88-3,
				  RESOLUTION_Y/2+100-14,
              BUTTON_COLOR_TEXT, 1);
	DrawArrow (&SetupOpm,
				  RESOLUTION_X/2-160+88+3,
				  RESOLUTION_Y/2+100-14,
              BUTTON_COLOR_TEXT, 1);

	//Scroll-Button "beenden":
	DrawButton (&SetupOpm,
				 	RESOLUTION_X/2+160-75,
				 	RESOLUTION_Y/2+100-20,
				 	RESOLUTION_X/2+160-9,
				 	RESOLUTION_Y/2+100-8,
					MENU_COLOR_BUTTON_FRAME);
	PrintCenteredTextInFrame (Messages[Language].Exit,
									  BUTTON_COLOR_TEXT,
									  &SetupOpm,
				 					  RESOLUTION_X/2+160-75,
				 					  RESOLUTION_Y/2+100-18,
				 					  RESOLUTION_X/2+160-9,
				 					  RESOLUTION_Y/2+100-8);

	//Displayrahmen "Seite x/y":
	Rectangle3d (&SetupOpm,
				    RESOLUTION_X/2-50,
				    RESOLUTION_Y/2+100-20,
				    RESOLUTION_X/2+65,
				    RESOLUTION_Y/2+100-8,
				 	 MENU_COLOR_DARK,
				 	 MENU_COLOR,
				 	 MENU_COLOR_LIGHT);

	CursorY=0;
	Choice =0;

	do
	{
		Rectangle3d (&SetupOpm,
				    	RESOLUTION_X/2-49,
				    	RESOLUTION_Y/2+100-19,
				    	RESOLUTION_X/2+64,
				    	RESOLUTION_Y/2+100-9,
				 	 	BUTTON_COLOR,
				 	 	BUTTON_COLOR,
				 	 	BUTTON_COLOR);

		sprintf (Buffer, Messages[Language].PageAofB, CursorY/20+1, (BrowseScript.AnzLines-20)/20+1);
		PrintCenteredTextInFrame (Buffer,
									  	  BUTTON_COLOR_TEXT,
									  	  &SetupOpm,
				    					  RESOLUTION_X/2-49,
				    					  RESOLUTION_Y/2+100-18,
				    					  RESOLUTION_X/2+64,
				    					  RESOLUTION_Y/2+100-7);

		//Das "Papier" wei· anmalen:
		Rectangle3d (&SetupOpm,
				 		RESOLUTION_X/2-160+7+1,
				 		RESOLUTION_Y/2-100+15+1,
				 		RESOLUTION_X/2+159-7-1,
				 		RESOLUTION_Y/2+99-23-1,
				 	 	BUTTON_COLOR,
				 	 	BUTTON_COLOR,
				 	 	BUTTON_COLOR);

		for (c=0; c<20; c++)
			if (c+CursorY>=0 && c+CursorY<BrowseScript.AnzLines)
			if (BrowseScript.Line[c+CursorY])
			{
				//Wenn der Text zu lang ist kommt die Hammer-Methode:
				if (strlen(BrowseScript.Line[c+CursorY])>38)
					(BrowseScript.Line[c+CursorY])[38]=0;

				PrintTextInFrame (BrowseScript.Line[c+CursorY],
										BUTTON_COLOR_TEXT,
										&SetupOpm,
				 						RESOLUTION_X/2-160+7+1,
				 						RESOLUTION_Y/2-100+15+1+c*FONT_Y_SIZE,
				 						RESOLUTION_X/2+159-7-1+3,
				 						RESOLUTION_Y/2+99-23-1+(c+1)*FONT_Y_SIZE+3);
			}

		//Update Screen with drawn window:
		DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

		//Drei Knîpfe verwalten:
		Choice= GetButtonAnswer
				 (
				 	5,
					Choice,
				 	RESOLUTION_X/2-160+8,
				 	RESOLUTION_Y/2+100-20,
				 	RESOLUTION_X/2-160+26,
				 	RESOLUTION_Y/2+100-8,
					"\xff\x50",
				 	RESOLUTION_X/2-160+30,
				 	RESOLUTION_Y/2+100 -20,
				 	RESOLUTION_X/2-160+48,
				 	RESOLUTION_Y/2+100-8,
					"\xff\x48",
				 	RESOLUTION_X/2-160+57,
				 	RESOLUTION_Y/2+100-20,
				 	RESOLUTION_X/2-160+75,
				 	RESOLUTION_Y/2+100-8,
					"\xff\x51",
				 	RESOLUTION_X/2-160+79,
				 	RESOLUTION_Y/2+100-20,
				 	RESOLUTION_X/2-160+97,
				 	RESOLUTION_Y/2+100-8,
					"\xff\x49",
				 	RESOLUTION_X/2+160-75,
				 	RESOLUTION_Y/2+100-20,
				 	RESOLUTION_X/2+160-9,
				 	RESOLUTION_Y/2+100-8,
					"\x1b"
				 );

		switch (Choice)
		{
			case 0:
            CursorY--;
				break;

			case 1:
            CursorY++;
				break;

			case 2:
            CursorY-=20;
				break;

			case 3:
            CursorY+=20;
				break;
		}

		if (CursorY > BrowseScript.AnzLines-20) CursorY = BrowseScript.AnzLines-20;
		if (CursorY < 0)                        CursorY = 0;
	}
	while (Choice!=4);

	//Restore the old screen:
	OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0);
	OPM_Del (&BackupOpm);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
}

//------------------------------------------------------------------------------
//LÑd ein Textfile ohne énderungen in eine SCRIPT-Struktur:
//------------------------------------------------------------------------------
void LoadBrowseFile (SCRIPT *Script, UNCHAR *Filename)
{
	SILONG	i;
	SILONG	InFileHandle;
	SILONG	ScriptFilelength;

	//---------------------------------------------------------------------------
	//Script einlesen:
	//---------------------------------------------------------------------------

	//Textfile îffnen:
	InFileHandle = open (Filename, O_RDONLY | O_BINARY);

	//Mi·erfolg ?
	if (InFileHandle<=0)
	{
		//Programm abbrechen:
		InternalError (ERR_BROWSE_OPEN, ScriptFilename);
	}

	//Die Datei-LÑnge des Scriptes ermitteln:
	ScriptFilelength = filelength (InFileHandle);

	//Mi·erfolg ?
	if (ScriptFilelength<0)
	{
		//Programm abbrechen:
		InternalError (ERR_BROWSE_OPEN, Filename);
	}

	//Speicher allokieren:
	Script->ScriptMem = THMalloc (ScriptFilelength+1);

	//Script komplett einlesen:
	if (read (InFileHandle,
				 Script->ScriptMem,
				 ScriptFilelength) != ScriptFilelength)
	{
		//Programm abbrechen:
		InternalError (ERR_SCRIPT_READ, Filename);
	}

	//Und mit Null beenden:
	Script->ScriptMem [ScriptFilelength] = 0;

	close (InFileHandle);

	//---------------------------------------------------------------------------
	//Script aufbereiten:
	//---------------------------------------------------------------------------

	Script->AnzLines = 0;

	//Das ganze Skript durchlaufen:
	for (i=0; i<ScriptFilelength; i++)
	{
		//Und die Textzeilen zÑhlen:
		if (Script->ScriptMem[i] == 13)
			Script->AnzLines++;
	}

	//Speicher fÅr den Array-Zugriff reservieren:
	Script->Line = (UNCHAR**)THMalloc ( (Script->AnzLines+1) * sizeof(UNCHAR*) );

	Script->AnzLines = 0;

	//Array-Access-Pointer fÅr die erste Zeile einrichten:
	Script->Line[ Script->AnzLines ] = Script->ScriptMem;
	Script->AnzLines++;


	//Das ganze Skript erneut durchlaufen:
	for (i=0; i<ScriptFilelength; i++)
	{
		//Und die Textzeilen zÑhlen:
		if (Script->ScriptMem[i] == 13)
		{
			//Array-Access-Pointer einrichten:
			Script->Line[ Script->AnzLines ] = Script->ScriptMem+i+1;

			//In c-String konvertieren:
			Script->ScriptMem[i] = 0;

			//Rausfiltern: NewLine, CarriageReturn am Zeilenanfang:
			while ( (Script->Line[Script->AnzLines-1])[0] != 0 &&
				  	( (Script->Line[Script->AnzLines-1])[0] == 10 ||
				    	(Script->Line[Script->AnzLines-1])[0] == 13)) Script->Line[Script->AnzLines-1]++;

			//Rausfiltern: NewLine, CarriageReturn, EOF am Zeilenende:
			while ( strlen(Script->Line[Script->AnzLines-1]) > 0 &&
				  	( (Script->Line[Script->AnzLines-1])[strlen(Script->Line[Script->AnzLines-1])-1] == 10 ||
				    	(Script->Line[Script->AnzLines-1])[strlen(Script->Line[Script->AnzLines-1])-1] == 13 ||
				    	(Script->Line[Script->AnzLines-1])[strlen(Script->Line[Script->AnzLines-1])-1] == 26))
			{
				//String um ein Zeichen kÅrzer machen:
		 		(Script->Line[Script->AnzLines])[strlen(Script->Line[Script->AnzLines-1])-1] = 0;
			}

			Script->AnzLines++;
		}
	}
	Script->AnzLines--;
}
