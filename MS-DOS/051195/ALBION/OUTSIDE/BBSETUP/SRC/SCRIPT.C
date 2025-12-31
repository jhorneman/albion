//------------------------------------------------------------------------------
//Script.c - Routinen fÅrs Skripthandling fÅr setup.exe
//------------------------------------------------------------------------------
//Beim compilieren sind keine besonderen Optionen notwendig. Ich empfehle
//aber 32-Bit code und "case-sensitive-linking" zu wÑhlen. Das Programm ist
//fÅr den empfindlichsten Warning-Level geschrieben und man sollte diese
//Idee konsequent fortfÅhren.
//------------------------------------------------------------------------------
//Hinweis: Die Routinen haben so umstÑndliche Parameter, damit man sie leicht
// 		  in eine C++ Klasse einbauen kann.
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//LÑd und initialisiert ein Scriptfile:
//------------------------------------------------------------------------------

// Parameter ScriptFilename changed to ScriptFname - JH

void SKRIPT_Init (SCRIPT *Script, UNCHAR *ScriptFname)
{
	SILONG	InFileHandle;
	SILONG	ScriptFilelength;
	SILONG	i,j;  //Schleifenindices
	SILONG	l;    //StringlÑnge

	//---------------------------------------------------------------------------
	//Script einlesen:
	//---------------------------------------------------------------------------

	//Scriptfile îffnen:
	InFileHandle = open (ScriptFname, O_RDONLY | O_BINARY);

	//Mi·erfolg ?
	if (InFileHandle<=0)
	{
		//Programm abbrechen:
		InternalError (ERR_SCRIPT_OPEN, ScriptFname);
	}

	//Die Datei-LÑnge des Scriptes ermitteln:
	ScriptFilelength = filelength (InFileHandle);

	//Mi·erfolg ?
	if (ScriptFilelength<0)
	{
		//Programm abbrechen:
      InternalError (ERR_SCRIPT_READ, ScriptFname);
	}

	//Speicher allokieren:
	Script->ScriptMem = THMalloc (ScriptFilelength+1);

	//Script komplett einlesen:
	if (read (InFileHandle,
				 Script->ScriptMem,
				 ScriptFilelength) != ScriptFilelength)
	{
		//Programm abbrechen:
		InternalError (ERR_SCRIPT_READ, ScriptFname);
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
	// Cast to UNCHAR** changed to SIBYTE** - JH
	Script->Line = (SIBYTE**)THMalloc ( (Script->AnzLines+1) * sizeof(UNCHAR*) );

	Script->AnzLines = 0;

	//Array-Access-Pointer fÅr die erste Zeile einrichten:
	// Cast to SIBYTE* added - JH
	Script->Line[ Script->AnzLines ] = (SIBYTE *) Script->ScriptMem;
	Script->AnzLines++;


	//Das ganze Skript erneut durchlaufen:
	for (i=0; i<ScriptFilelength; i++)
	{
		//Und die Textzeilen zÑhlen:
		if (Script->ScriptMem[i] == 13)
		{
			//Array-Access-Pointer einrichten:
			// Cast to SIBYTE* added - JH
			Script->Line[ Script->AnzLines ] = (SIBYTE *) (Script->ScriptMem+i+1);
			Script->AnzLines++;

			//In c-String konvertieren:
			Script->ScriptMem[i] = 0;
		}
	}
	Script->AnzLines--;

	//---------------------------------------------------------------------------
	//Script vereinfachen:
	//---------------------------------------------------------------------------

	//Das ganze Skript schon wieder durchlaufen:
	for (i=0; i<Script->AnzLines; i++)
	{
		// Cast to char* added - JH
		l=strlen ((char *) Script->Line[i]);

		//Kommentare abhacken:
		for (j=0; j<l; j++)
		{
			if ( (Script->Line[i])[j]=='/' && (Script->Line[i])[j+1]=='/')
			{
				(Script->Line[i])[j]=0;
				break;
			}
		}

		//aus "\n" einen echtes \n machen:
		for (j=0; j<l; j++)
		{
			if ( (Script->Line[i])[j]=='\\' && (Script->Line[i])[j+1]=='n')
			{
				(Script->Line[i])[j]='\n';

				// Casts to char* added - JH
				strcpy ((char *) (Script->Line[i]+j+1), (char *) (Script->Line[i]+j+2));
				l--; j--;
			}
		}

		//Rausfiltern: NewLine, CarriageReturn, Space und Tab am Zeilenanfang:
		while ( (Script->Line[i])[0] != 0 &&
				  ( (Script->Line[i])[0] == 10 ||
				    (Script->Line[i])[0] == 13 ||
				    (Script->Line[i])[0] == 32 ||
				    (Script->Line[i])[0] == 9)) Script->Line[i]++;

		//Rausfiltern: NewLine, CarriageReturn, EOF, Space und Tab am Zeilenende:
		// Casts to char* added - JH
		while ( strlen((char *) Script->Line[i]) > 0 &&
				  ( (Script->Line[i])[strlen((char *) Script->Line[i])-1] == 10 ||
				    (Script->Line[i])[strlen((char *) Script->Line[i])-1] == 13 ||
				    (Script->Line[i])[strlen((char *) Script->Line[i])-1] == 26 ||
				    (Script->Line[i])[strlen((char *) Script->Line[i])-1] == 32 ||
				    (Script->Line[i])[strlen((char *) Script->Line[i])-1] == 9))
		{
			//String um ein Zeichen kÅrzer machen:
			// Cast to char* added - JH
		 	(Script->Line[i])[strlen((char *) Script->Line[i])-1] = 0;
		}
	}
}

//------------------------------------------------------------------------------
//Gibt die Tokennummer der angegebenen Zeile zurÅck. (-1 fÅr unbekannten Token)
//------------------------------------------------------------------------------

// Parameter TokenTable changed to TokenTab - JH

SILONG SCRIPT_GetTokenNumber (SCRIPT *Script, SILONG Linenumber, TOKEN *TokenTab)
{
	SILONG i;

	//Leerzeile, Kommentar oder Label ?
	if ((Script->Line[Linenumber])[0]==0 || (Script->Line[Linenumber])[0]==':')
	{
		//eine Null bedeutet leere Zeile:
		return(0);
	}

	//Alle Tokens der Tabelle durchgehen:
	for (i=0; TokenTab[i].Name; i++)
	{
		//Aktuellen Token mit dem Anfang der aktuellen Zeile vergleichen:
		// Cast to char* added - JH
		if (strncmp (TokenTab[i].Name,
						 (char *) Script->Line[Linenumber],
						 strlen(TokenTab[i].Name)) == 0 &&
			 ( (Script->Line[Linenumber])[strlen(TokenTab[i].Name)]==32 ||
			   (Script->Line[Linenumber])[strlen(TokenTab[i].Name)]==0 ) )
		{
			//Identifikationswert zurÅckgeben:
			return (TokenTab[i].ID);
		}
	}

	//Sorry, aber den Token gibt's nicht:
	return (-1);
}

//------------------------------------------------------------------------------
//Gibt die Zeilen des Labels zurÅck. (-1 fÅr unbekannten Label)
//------------------------------------------------------------------------------
SILONG SCRIPT_FindLabelLine (SCRIPT *Script, UNCHAR *Label)
{
	SILONG i;

	//Alle Zeilen des Skripts durchgehen:
	for (i=0; i<Script->AnzLines; i++)
	{
		//Ist Zeile ein Label ?
		if ((Script->Line[i])[0]==':')
		{
			//Aktuellen Label mit dem Parameter vergleichen:
			// Cast to char* added - JH
			if (strcmp ((char *) Script->Line[i]+1, Label) ==0 )
			{
				//Zeilennummer zurÅckgeben:
				return (i);
			}
		}
	}

	//Sorry, aber den Label gibt's nicht:
	return (-1);
}

//------------------------------------------------------------------------------
//Eine Zeile aus dem Script bearbeiten und die nÑchste Zeilennummer zurÅckgeben
//------------------------------------------------------------------------------
SILONG SCRIPT_ProcessScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzOpenIf)
{
	// DeppFlag added - JH
	BOOLEAN DeppFlag;

	SILONG Token;
	SILONG AnzParameter;
	SILONG NewLine;
	UNCHAR **Parameter;

	if (Line >= Script->AnzLines || Line<0)
	{
		//Illegale Zeile oder Ende des Scriptes:
		return (-1);
	}

	//Token der aktuellen Zeile identifizieren:
	// & operator in front of TokenTable removed - JH
	Token = SCRIPT_GetTokenNumber (Script, Line, TokenTable);

	if (Token == 0)
	{
		//Leerzeile:
		return (Line+1);
	}

	if (Token == -1)
	{
		//Unbekannter Token:
		InternalError (ERR_UNKNOWN_TOKEN, Line+1, Script->Line[Line]);
	}

	//Eine Zeile in die Parameter zerlegen. Dabei kommt in AnzParameter
	//die Anzahl der Paramter zurÅck. (Der Token gilt wie beim main(arg..)
	//als Parameter. Der rc ist ein Pointer auf eine Stringtabelle in
	//der die einzelnen Parameter liegen. (Paramter 0 ist gleich Token)
	Parameter = SCRIPT_ParseScriptLine (Script, Line, &AnzParameter);

	switch (Token)
	{
		//------------------------------------------------------------------------
		//Vermischtes:
		//------------------------------------------------------------------------
		case TOKEN_GOTO:
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			NewLine = SCRIPT_FindLabelLine (Script, Parameter[1]);
			AnzOpenIf[0] = SCRIPT_ScanForOpenIfs (Script, NewLine);
			if (NewLine == -1)		InternalError (ERR_UNKNOWN_LABEL, Line+1, Script->Line[Line]);
			return (NewLine);
			break;

		case TOKEN_END:
			AnzOpenIf[0] = 0; //Fehlermeldung verhindern
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			return (-1);
			break;

		case TOKEN_UPDATE_INI:
			{
				if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);

            CalculateIniFilename ();
				UpdateIni();
            if (VESA_IsAvailable ()) VESA_UpdateIni();
				break;
			}

		//------------------------------------------------------------------------
		//If-Befehle:
		//------------------------------------------------------------------------
		case TOKEN_IF_EXISTS:
			AnzOpenIf[0]++;
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (IfExists (Parameter[1]))
				return (Line+1);
			else
				return (SCRIPT_ScanForIfComplement (Script, Line, TOKEN_ELSE));
			break;

		case TOKEN_IF_NOT_EXISTS:
			AnzOpenIf[0]++;
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (!IfExists (Parameter[1]))
				return (Line+1);
			else
				return (SCRIPT_ScanForIfComplement (Script, Line, TOKEN_ELSE));
			break;

		case TOKEN_IF_LANGUAGE:
			AnzOpenIf[0]++;
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (SCRIPT_GetIntParameter (Parameter[1], Line) == Language+1)
				return (Line+1);
			else
				return (SCRIPT_ScanForIfComplement (Script, Line, TOKEN_ELSE));
			break;

		#if FALSE
		case TOKEN_IF_WINDOWS:
			AnzOpenIf[0]++;
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (IfWindows())
				return (Line+1);
			else
				return (SCRIPT_ScanForIfComplement (Script, Line, TOKEN_ELSE));
			break;
		#endif

		case TOKEN_ELSE:
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (AnzOpenIf[0] == 0)  InternalError (ERR_ENDIF_WITHOUT_IF, Line+1, Script->Line[Line]);
			NewLine = SCRIPT_ScanForIfComplement (Script, Line, TOKEN_ENDIF);
			return (NewLine);
			break;

		case TOKEN_ENDIF:
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (AnzOpenIf[0] == 0)  InternalError (ERR_ENDIF_WITHOUT_IF, Line+1, Script->Line[Line]);
			AnzOpenIf[0]--;
			return (Line+1);
			break;

		//------------------------------------------------------------------------
		//Disk-Befehle:
		//------------------------------------------------------------------------

		#if FALSE
		/*																							 */
		/*__TOKEN_WRITE_SOURCE_PATH___________________________________________*/
		/*																							 */
		/*	 (Erweitert/Erstellt durch: Andreas Nitsche)								 */
 		/*																							 */
 		/*	 Schreibt den Quellpfad in eine bereits geîffnete .INI-Datei		 */
 		/*																							 */

		case TOKEN_WRITE_SOURCE_PATH:
		{
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);

			CalculateIniFilename ();
			if (IfExistsFile (IniFilename))
			{
				if(SetupTargetDrive)
				{
					char Buffer[1025];
					sprintf (Buffer, "%s", SetupSourcePath);
		         IniSet (IniFilename, "SYSTEM", "SOURCE_PATH", Buffer);
				}
				else
				{
					// Fehlerbehandlung
				}
			}
			break;
		}
		#endif

		case TOKEN_COPY:
			if (AnzParameter!=3 && AnzParameter!=4) InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (AnzParameter==4)
			{
				if (stricmp (Parameter[3], "/s")!=0)    InternalError (ERR_SYNTAX, Line+1, Script->Line[Line]);
			}

			// Calculation of DeppFlag added - JH
			if (AnzParameter == 4)
				DeppFlag = TRUE;
			else
				DeppFlag = FALSE;

			AdministrateProgressWindow (1);

			// AnzParameter == 4 expression replaced by DeppFlag - JH
			Copy (Parameter[1], Parameter[2], DeppFlag);

			AdministrateProgressWindow (-1);
			break;

		case TOKEN_INSTALL:
			if (AnzParameter!=2 && AnzParameter!=3) InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);

			AdministrateProgressWindow (1);

			if (AnzParameter == 3)
				Install (Parameter[1], Parameter[2], FALSE);
			else
				Install (Parameter[1], "", FALSE);

			AdministrateProgressWindow (-1);
			break;

		case TOKEN_INSTALL_DIRS:
			if (AnzParameter!=2 && AnzParameter!=3) InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);

			AdministrateProgressWindow (1);

			if (AnzParameter == 3)
				Install (Parameter[1], Parameter[2], TRUE);
			else
				Install (Parameter[1], "", TRUE);

			AdministrateProgressWindow (-1);
			break;

		case TOKEN_MD:
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			MakeDirs (Parameter[1]); //Verzeichnis(se) anlegen
			break;

		case TOKEN_CREATE:
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			CreateFile (Parameter[1]);
  			break;

		case TOKEN_DELETE:
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			Delete (Parameter[1]);
			break;

		case TOKEN_RENAME:
			if (AnzParameter != 3)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			remove (Parameter[2]);
			if (rename (Parameter[1], Parameter[2]))
				InternalError (ERR_RENAME_FAILS, Line+1, Script->Line[Line]);
			break;

		case TOKEN_EXECUTE_SILENT:
			ExecuteShell (AnzParameter, Parameter);

			//Und sicherheitshalber wieder mit altem Inhalt fÅllen:
         Init_mouse_pointer(10, 14, 1, 1, Mouse_graphics);
			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			DSA_SPal (&SetupPalette, 0, 256, 0);
			DSA_APal ();
			RefreshFixedPalette ();
			break;

		case TOKEN_EXECUTE_OWN_SCREEN:
			if (AnzParameter == 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);

			//Bildschirm abschalten:
			DSA_CloseScreen (&SetupScreenPort);
			SYSTEM_Exit();

			//Programm ausfÅhren:
			ExecuteShell (AnzParameter, Parameter);

			SYSTEM_Init ();
			DSA_Init ();

			//Bildschirm wieder einschalten:
			if (!DSA_OpenScr (&SetupOpm, 0))
				InternalError (ERR_NO_VGA);

			//Und wieder mit altem Inhalt fÅllen:
         Init_mouse_pointer(10, 14, 1, 1, Mouse_graphics);
			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			DSA_SPal (&SetupPalette, 0, 256, 0);
			DSA_APal ();
			break;

		case TOKEN_CD:
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			ChangeDirAndDriveTo (Parameter[1]);
			break;

		/*																							 */
		/*__TOKEN_SELECT_TARGET_DRIVE_________________________________________*/
		/*																							 */
		/*	 (Erweitert/Erstellt durch: Andreas Nitsche)								 */
 		/*																							 */
 		/*	 Auswahlfenster zum Selektieren des Ziellaufwerks. Bei Abbruch 	 */
		/*	 des Fensters wird auf das Label <Parameter[2]> gesprungen.			 */
		/*																							 */

		case TOKEN_SELECT_TARGET_DRIVE:
		{
			if (AnzParameter != 3)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			{
				BOOL ErrorFlag;

		  		ErrorFlag =	SelectTargetDrive
								(
									SCRIPT_GetIntParameter (Parameter[1],Line)
								);
		  		if (!ErrorFlag)
		  		{
			  		NewLine = SCRIPT_FindLabelLine (Script, Parameter[2]);
			  		AnzOpenIf[0] = SCRIPT_ScanForOpenIfs (Script, NewLine);
			  		if (NewLine == -1) InternalError (ERR_UNKNOWN_LABEL, Line+1, Script->Line[Line]);
			  		return (NewLine);
				}
			}
 			break;
		}

		case TOKEN_SELECT_TARGET_PATH:
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			strcpy (SetupTargetPath, UserEntersString (Messages[Language].SelectTargetPath, Parameter[1]));
			MakeDirs (SetupTargetPath);
			break;

		case TOKEN_CD_SOURCE:
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			ChangeDirAndDriveTo (SetupSourcePath);
			break;

		case TOKEN_CD_TARGET:
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			ChangeDirAndDriveTo (SetupTargetPath);
			break;

		#if FALSE
		case TOKEN_CHECK_CD:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;
		#endif

		case TOKEN_SELECT_CD_ROM_DRIVE:
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			{
				BOOL ErrorFlag;

		  		ErrorFlag =	SelectCDDrive();
				if (!ErrorFlag)
		  		{
			  		NewLine = SCRIPT_FindLabelLine (Script, Parameter[1]);
			  		AnzOpenIf[0] = SCRIPT_ScanForOpenIfs (Script, NewLine);
			  		if (NewLine == -1) InternalError (ERR_UNKNOWN_LABEL, Line+1, Script->Line[Line]);
			  		return (NewLine);
				}
			}
 			break;


		//------------------------------------------------------------------------
		//MenÅ-Befehle:
		//------------------------------------------------------------------------
		case TOKEN_MENU_START:
			//Wir nehmen etwas verlorenen Speicher in kauf:
			AuswahlMenue.AnzEntrys = 0;
			break;

		case TOKEN_MENU_ENTRY:
			if (AnzParameter > 3)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (AuswahlMenue.AnzEntrys>=MAX_MENU_ENTRIES-1) InternalError (ERR_MAX_MENU, Line+1, Script->Line[Line]);
			if (AnzParameter == 1)
			{
				//Kein Parameter ==> Leerzeile:
				AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Text = NULL;
				AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Line = -1;
			}
			else if (AnzParameter == 2)
			{
				//Ein Parameter ==> Kommentarzeile (Text kopieren):
				AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Text = THMalloc (strlen(Parameter[1]));
				strcpy (AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Text, Parameter[1]);

				AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Line = -1;
			}
			else if (AnzParameter == 3)
			{
				//Zwei Parameter ==> Kommentarzeile (Text kopieren, Label verwalten):
				AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Text = THMalloc (strlen(Parameter[1]));
				strcpy (AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Text, Parameter[1]);

				AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Line = SCRIPT_FindLabelLine (Script, Parameter[2]);
				if (AuswahlMenue.MenuEntry[ AuswahlMenue.AnzEntrys ].Line == -1)		InternalError (ERR_UNKNOWN_LABEL, Line+1, Script->Line[Line]);
			}
			AuswahlMenue.AnzEntrys++;
			break;

		case TOKEN_MENU_END:
			if (AuswahlMenue.AnzEntrys==0) InternalError (ERR_MENU_DO_SELECT, Line+1, Script->Line[Line]);
         if (AnzParameter != 1)
         {
            InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
         }
         else
         {
            SILONG c;

            for (c=0; c<AuswahlMenue.AnzEntrys; c++)
               if (AuswahlMenue.MenuEntry[c].Text && AuswahlMenue.MenuEntry[c].Line!=-1)
                  break;

            if (c>=AuswahlMenue.AnzEntrys) InternalError (ERR_NO_OPTION, Line+1, Script->Line[Line]);
         }
			NewLine = SelectionWindow (&AuswahlMenue);

			AnzOpenIf[0] = SCRIPT_ScanForOpenIfs (Script, NewLine);
			return (NewLine);
			break;

		case TOKEN_PRINT:
			if (AnzParameter==2)
				EasyPrintWindow (Parameter[1]);
			else if (AnzParameter==1)
				EasyPrintWindow (NULL);
			else
				InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			break;

		case TOKEN_TEXT:
			if (AnzParameter!=2) InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			BrowseFile (Parameter[1]);
			break;

		case TOKEN_ASSERT:
			if (AnzParameter!=3 && AnzParameter!=4) InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (!AssertWindow (Parameter[1]))
			{
				NewLine = SCRIPT_FindLabelLine (Script, Parameter[2]);
				AnzOpenIf[0] = SCRIPT_ScanForOpenIfs (Script, NewLine);
				return (NewLine);
			}
			else if (AnzParameter==4)
			{
				NewLine = SCRIPT_FindLabelLine (Script, Parameter[3]);
				AnzOpenIf[0] = SCRIPT_ScanForOpenIfs (Script, NewLine);
				return (NewLine);
			}
			break;

		case TOKEN_INFO:
			if (AnzParameter != 2) InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			InfoWindow (Parameter[1]);
			break;

		case TOKEN_ERROR:
			if (AnzParameter != 2) InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			ErrorWindow (Parameter[1]);
			break;

		case TOKEN_LOAD_BACKGROUND:
			{
				struct OPM LoadOpm;

				//Paramter testen:
				if (AnzParameter != 2) InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);

				//Bild laden:
				if (!LBM_DisplayLBM (Parameter[1], &LoadOpm, &SetupPalette, LBMSTAT_MAKENEWOPM))
					InternalError (ERR_LOAD_PIC, Parameter[1]);

				//LoadOpm in SetupOpm kopieren:
				ScaleCopyOpm (&LoadOpm,        //Quelle
								  &SetupOpm);      //Ziel
				/*OPM_CopyOPMOPM (&LoadOpm,        //Quelle
									 &SetupOpm,       //Ziel
									 0,               //Quell-x
									 0,               //Quell-y
									 LoadOpm.width,   //Dimension der Quelle
									 LoadOpm.height,  //Dimension der Quelle
									 (RESOLUTION_X-LoadOpm.width)/2,   //Position im Ziel
									 (RESOLUTION_Y-LoadOpm.height)/2); //Position im Ziel*/

				//Und anzeigen:
				DSA_SPal (&SetupPalette, 0, 256, 0);
				DSA_APal ();
				RefreshFixedPalette();
				DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

				OPM_Del (&LoadOpm);
			}
			break;

		case TOKEN_SET_LANGUAGE:
			if (AnzParameter != 2) InternalError (ERR_NUM_PARAMETERS, Line, Script->Line[Line]);
			if (SCRIPT_GetIntParameter (Parameter[1], Line)<1 || SCRIPT_GetIntParameter (Parameter[1], Line) > MAX_LANGUAGES)
				InternalError (ERR_WRONG_LANGUAGE, Line, Script->Line[Line]);
			Language = SCRIPT_GetIntParameter (Parameter[1], Line+1)-1;
			break;

		//------------------------------------------------------------------------
		//Token ohne case:
		//------------------------------------------------------------------------
		default:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;
	}

	return (Line+1);
}

//------------------------------------------------------------------------------
//Zerlegt eine Zeile in seine Parameter; Gibt diese als UNCHAR** zurÅck sowie
//die Anzahl der Parameter als call-by-reference
//------------------------------------------------------------------------------
UNCHAR **SCRIPT_ParseScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzParameter)
{
	SILONG AnzP;
	SILONG c,l;
	static UNCHAR *rcTable[20]; //Max 20 Parameter;
	static UNCHAR Buffer[256];  //Arbeitsversion fÅr aktuelle Zeile

	// Cast to char* added - JH
	strcpy (Buffer, (char *) Script->Line[Line]);

	l = strlen (Buffer);

	AnzP=0;
	for (c=0; c<l; c++)
	{
		rcTable[AnzP] = Buffer + c;

		//Haben wir einen String-Paramter erwischt ?
		if ( Buffer[c] == '"' )
		{
			//AnfÅhrungszeichen Åberspringen:
			rcTable[AnzP]++;

			//zweites AnfÅhrungszeichen suchen:
			for (c++; c<l; c++)
			{
				if ( Buffer[c] == '"' && Buffer[c] != '\\')
				{
					//String hier beenden:
					Buffer[c] = 0;

					if (c!=l-1) c++;
					break;
				}
			}

			if (c==l)
			{
				//Ich vermisse das zweite AnfÅhrungszeichen:
				InternalError (ERR_UNMATCHED_STRING, Line+1, Script->Line[Line]);
			}

			//String hier beenden:
			Buffer[c] = 0;
		}
		//Haben wir hier einen normalen Parameter ?
		else if ( Buffer[c] != ' ' )
		{
			//nÑchstes Leerzeichen suchen:
			for (c++; c<l; c++)
			{
				if ( Buffer[c] == ' ')
					break;
			}

			//String hier beenden:
			Buffer[c] = 0;
		}
		else if ( Buffer[c] == ' ' )
		{
			//Leerzeichen zwischen Parametern ignorieren:
			continue;
		}

		//NÑchster Parameter:
		AnzP++;
	}

	//Die Anzahl der Parameter ggf. als call-by-reference zurÅckgeben:
	if (AnzParameter)
	{
		AnzParameter[0] = AnzP;
	}

	//Die Tabelle mit den einzelnen Parametern:
	return (rcTable);
}

//------------------------------------------------------------------------------
//Konvertiert einen universellen String-Parameter in einen Integer
//------------------------------------------------------------------------------
SILONG SCRIPT_GetIntParameter (UNCHAR *Parameter, SILONG Line)
{
	SILONG c,l;

	l = strlen (Parameter);
	for (c=0; c<l; c++)
		if (Parameter[c]!=32 && Parameter[c]!='+' && Parameter[c]!='-' &&
			 (Parameter[c]<'0' || Parameter[c]>'9'))
		{
			//String enthÑlt illegale Zeichen:
			InternalError (ERR_INT_EXPECTED, Line+1);
		}

	return (atoi(Parameter));
}

//------------------------------------------------------------------------------
//Sucht nach ENDIF oder ELSE; gibt Zeilennummer zurÅck.
//------------------------------------------------------------------------------
SILONG SCRIPT_ScanForIfComplement (SCRIPT *Script, SILONG Line, SILONG AllowedToken)
{
	SILONG AnzOpenIf;
	SILONG Token;

	//Wir mÅssen tiefer geschachtelte if-strukturen natÅrlich Åberspringen:
	AnzOpenIf = 1; //und suchen das Komplement zu unserem if-problem.

	//Die aufrufende Line war das IF oder oder ELSE:
	Line++;

	while (1)
	{
		if (Line >= Script->AnzLines) //File zuende...
			InternalError (ERR_UNMATCHED_IF);

		Token = SCRIPT_GetTokenNumber (Script, Line, TokenTable);

		//Wir mÅssen erst einmal zum Ende der Schachtelung:
		switch (Token)
		{
			//Hier mÅssen alle IF's stehen (auch ErgÑnzungen) !
			case TOKEN_IF_EXISTS:
			case TOKEN_IF_NOT_EXISTS:
			case TOKEN_IF_LANGUAGE:
			case TOKEN_IF_WINDOWS:
				AnzOpenIf++;
				break;

			case TOKEN_ELSE:
				if (AllowedToken != TOKEN_ELSE) //Oops; hier darf kein ELSE sein !
					InternalError (ERR_DOUBLE_ELSE, Line+1, Script->Line[Line]);

				if (AnzOpenIf == 1) //Passt es zum aufrufenden IF ?
					return (Line+1); //Jau, daher ab nach Hause.

				break;

			case TOKEN_ENDIF:
				AnzOpenIf--;

				if (AnzOpenIf == 0)
					return (Line); //Das war das ersehnte ENDIF ==> ab nach Hause !
				break;

			case -1:
				//Unbekannter Token:
				InternalError (ERR_UNKNOWN_TOKEN, Line+1, Script->Line[Line]);
				break;
		}

		//Weitersuchen: (DurchkÑmmt die WÅste !)
		Line++;
	}
}

//------------------------------------------------------------------------------
//Gibt an wieviele IF's in Zeile x offen sind. Nîtig fÅr SprÅnge.
//------------------------------------------------------------------------------
SILONG SCRIPT_ScanForOpenIfs (SCRIPT *Script, SILONG Line)
{
	SILONG AnzOpenIf;
	SILONG CurLine;
	SILONG Token;

	CurLine   = 0;
	AnzOpenIf = 0;

	//Die aufrufende Line war das IF oder oder ELSE:
	CurLine++;

	while (1)
	{
		if (Line == CurLine)
			return (AnzOpenIf);

		Token = SCRIPT_GetTokenNumber (Script, Line, TokenTable);

		//Wir mÅssen erst einmal zum Ende der Schachtelung:
		switch (Token)
		{
			//Hier mÅssen alle IF's stehen (auch ErgÑnzungen) !
			case TOKEN_IF_EXISTS:
			case TOKEN_IF_NOT_EXISTS:
			case TOKEN_IF_LANGUAGE:
			case TOKEN_IF_WINDOWS:
				AnzOpenIf++;
				break;

			case TOKEN_ENDIF:
				AnzOpenIf--;
				break;
		}

		//Weitersuchen: (DurchkÑmmt die WÅste !)
		CurLine++;
	}
}
