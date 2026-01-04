//------------------------------------------------------------------------------
//Browser.c - Alles was zum Text Browser fÅr Setup/VGA gehîrt:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Hauptfunktion, die von au·en einfach aufgerufen wird um ein File zu zeigen:
//------------------------------------------------------------------------------
void BrowseFile (char *Filename)
{
	SILONG c;
	SILONG Choice;  //Ergebnis vom AuswahlmenÅ
	SILONG CursorY; //Cursor im Textfenster (besser gesagt: y-hotspot links-o.)
	struct OPM BackupOpm;
	SCRIPT BrowseScript;

	LoadBrowseFile (&BrowseScript, Filename);

	//Save the old screen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &BackupOpm, NULL);
	OPM_CopyOPMOPM (&SetupOpm, &BackupOpm, 0, 0, 320, 200, 0, 0);

	//Der Ñu·ere Bilderrahmen:
	Rectangle3d (&SetupOpm, 0, 0, RESOLUTION_X-1, RESOLUTION_Y-1,
				 	 BROWSE_BORDER_COLOR_LIGHT,
				 	 BROWSE_BORDER_COLOR,
				 	 BROWSE_BORDER_COLOR_DARK);

	//Der innere Bilderrahmen:
	Rectangle3d (&SetupOpm,
					 MENU_BORDER_WIDTH,
					 MENU_BORDER_WIDTH,
					 RESOLUTION_X-MENU_BORDER_WIDTH*2-FONT_Y_SIZE*2,
					 RESOLUTION_Y-MENU_BORDER_WIDTH*2-FONT_Y_SIZE*2,
				 	 BROWSE_BORDER_COLOR_DARK,
				 	 BROWSE_BORDER_COLOR,
				 	 BROWSE_BORDER_COLOR_LIGHT);

	//Scroll-Pfeilrahmen "oben":
	Rectangle3d (&SetupOpm,
					 RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE*2,
					 MENU_BORDER_WIDTH,
					 RESOLUTION_X-MENU_BORDER_WIDTH,
					 MENU_BORDER_WIDTH+(RESOLUTION_Y-FONT_Y_SIZE*2-MENU_BORDER_WIDTH*2)/2,
				 	 BROWSE_BORDER_COLOR_LIGHT,
				 	 BROWSE_BORDER_COLOR,
				 	 BROWSE_BORDER_COLOR_DARK);

	//Scroll-Pfeilrahmen "unten":
	Rectangle3d (&SetupOpm,
					 RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE*2,
					 MENU_BORDER_WIDTH+(RESOLUTION_Y-FONT_Y_SIZE*2-MENU_BORDER_WIDTH*2)/2+2,
					 RESOLUTION_X-MENU_BORDER_WIDTH,
					 RESOLUTION_Y-MENU_BORDER_WIDTH-FONT_Y_SIZE*2-2,
				 	 BROWSE_BORDER_COLOR_LIGHT,
				 	 BROWSE_BORDER_COLOR,
				 	 BROWSE_BORDER_COLOR_DARK);

	//Pfeile mit Trick zeichnen:
	for (c=RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE*2+2; c<=RESOLUTION_X-MENU_BORDER_WIDTH-2; c++)
	{
		//Vîllig krank, ich wei·, aber es geht:
		OPM_VerLine (&SetupOpm,
						 c,
						 MENU_BORDER_WIDTH+2+abs(c-(RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE)),
						 abs(c-(RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE))>1
						  ? MENU_BORDER_WIDTH + FONT_Y_SIZE+1 - (MENU_BORDER_WIDTH+2+abs(c-(RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE)))
						  : MENU_BORDER_WIDTH+(RESOLUTION_Y-FONT_Y_SIZE*2-MENU_BORDER_WIDTH*2)/2 - (MENU_BORDER_WIDTH+2+abs(c-(RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE)))-1,
						 BROWSE_FONT_COLOR);

		//dito:
		OPM_VerLine (&SetupOpm,
						 c,
						 abs(c-(RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE))>1
						  ? RESOLUTION_Y-MENU_BORDER_WIDTH-FONT_Y_SIZE*2-4-FONT_Y_SIZE+1
						  : MENU_BORDER_WIDTH+(RESOLUTION_Y-FONT_Y_SIZE*2-MENU_BORDER_WIDTH*2)/2+4,
						 RESOLUTION_Y-MENU_BORDER_WIDTH-FONT_Y_SIZE*2-3-abs(c-(RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE))
						  -
						 ((abs(c-(RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE))>1)
						  ? RESOLUTION_Y-MENU_BORDER_WIDTH-FONT_Y_SIZE*2-4-FONT_Y_SIZE+1
						  : MENU_BORDER_WIDTH+(RESOLUTION_Y-FONT_Y_SIZE*2-MENU_BORDER_WIDTH*2)/2+4),
						 BROWSE_FONT_COLOR);
	}

	//Rahmen "exit":
	Rectangle3d (&SetupOpm,
					 MENU_BORDER_WIDTH,
					 RESOLUTION_Y-MENU_BORDER_WIDTH-FONT_Y_SIZE*2,
					 RESOLUTION_X-MENU_BORDER_WIDTH,
					 RESOLUTION_Y-MENU_BORDER_WIDTH,
				 	 BROWSE_BORDER_COLOR_LIGHT,
				 	 BROWSE_BORDER_COLOR,
				 	 BROWSE_BORDER_COLOR_DARK);

	//Text "exit":
	PrintTextInFrame (Messages[Language].Exit,
							BROWSE_FONT_COLOR,
							&SetupOpm,
							RESOLUTION_X/2 - PixelStringLength (Messages[Language].Exit)/2,
					 		RESOLUTION_Y-MENU_BORDER_WIDTH-FONT_Y_SIZE*2+FONT_Y_SIZE/2,
							RESOLUTION_X,
					 		RESOLUTION_Y);

	CursorY=0;
	Choice =0;

	do
	{
		//Das "Papier" wei· anmalen:
		Rectangle3d (&SetupOpm,
					 	MENU_BORDER_WIDTH+1,
					 	MENU_BORDER_WIDTH+1,
					 	RESOLUTION_X-MENU_BORDER_WIDTH*2-FONT_Y_SIZE*2-1,
					 	RESOLUTION_Y-MENU_BORDER_WIDTH*2-FONT_Y_SIZE*2-1,
				 	 	BROWSE_BACK_COLOR,
				 	 	BROWSE_BACK_COLOR,
				 	 	BROWSE_BACK_COLOR);

		for (c=0; c<19; c++)
			if (c+CursorY>=0 && c+CursorY<BrowseScript.AnzLines)
			if (BrowseScript.Line[c+CursorY])
			{
				//Wenn der Text zu lang ist kommt die Hammer-Methode:
				if (strlen(BrowseScript.Line[c+CursorY])>36)
					(BrowseScript.Line[c+CursorY])[36]=0;

				PrintTextInFrame (BrowseScript.Line[c+CursorY],
										BROWSE_FONT_COLOR,
										&SetupOpm,
					 					MENU_BORDER_WIDTH+2,
					 					MENU_BORDER_WIDTH+2+c*(FONT_Y_SIZE+1),
					 					RESOLUTION_X-MENU_BORDER_WIDTH*2-FONT_Y_SIZE*2-1,
					 					RESOLUTION_Y-MENU_BORDER_WIDTH*2-FONT_Y_SIZE*2-1);
			}

		//Update Screen with drawn window:
		DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

		//Drei Knîpfe verwalten:
		Choice=GetButtonAnswer
				 (
				 	3,
					Choice,
					RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE*2,
					MENU_BORDER_WIDTH,
		         RESOLUTION_X-MENU_BORDER_WIDTH,
					MENU_BORDER_WIDTH+(RESOLUTION_Y-FONT_Y_SIZE*2-MENU_BORDER_WIDTH*2)/2,
					"",
					RESOLUTION_X-MENU_BORDER_WIDTH-FONT_Y_SIZE*2,
					MENU_BORDER_WIDTH+(RESOLUTION_Y-FONT_Y_SIZE*2-MENU_BORDER_WIDTH*2)/2+2,
					RESOLUTION_X-MENU_BORDER_WIDTH,
					RESOLUTION_Y-MENU_BORDER_WIDTH-FONT_Y_SIZE*2-2,
					"",
					MENU_BORDER_WIDTH,
					RESOLUTION_Y-MENU_BORDER_WIDTH-FONT_Y_SIZE*2,
					RESOLUTION_X-MENU_BORDER_WIDTH,
					RESOLUTION_Y-MENU_BORDER_WIDTH,
					"\x1b"
				 );

		switch (Choice)
		{
			case 0:
				CursorY-=19;
				break;

			case 1:
				CursorY+=19;
				break;
		}

		if (CursorY >= BrowseScript.AnzLines)   CursorY = (BrowseScript.AnzLines-1)/19*19;
		if (CursorY < 0)                        CursorY = 0;
	}
	while (Choice!=2);

	//Restore the old screen:
	OPM_CopyOPMOPM (&BackupOpm, &SetupOpm, 0, 0, 320, 200, 0, 0);
	OPM_Del (&BackupOpm);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
}

//------------------------------------------------------------------------------
//LÑd ein Textfile ohne énderungen in eine SCRIPT-Struktur:
//------------------------------------------------------------------------------
void LoadBrowseFile (SCRIPT *Script, char *Filename)
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
	Script->Line = (char**)THMalloc ( (Script->AnzLines+1) * sizeof(char*) );

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
