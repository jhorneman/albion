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
void SKRIPT_Init (SCRIPT *Script, char *ScriptFilename)
{
	SILONG	InFileHandle;
	SILONG	ScriptFilelength;
	SILONG	i,j;  //Schleifenindices
	SILONG	l;    //StringlÑnge

	//---------------------------------------------------------------------------
	//Script einlesen:
	//---------------------------------------------------------------------------

	//Scriptfile îffnen:
	InFileHandle = open (ScriptFilename, O_RDONLY | O_BINARY);

	//Mi·erfolg ?
	if (InFileHandle<=0)
	{
		//Programm abbrechen:
		InternalError (ERR_SCRIPT_OPEN, ScriptFilename);
	}

	//Die Datei-LÑnge des Scriptes ermitteln:
	ScriptFilelength = filelength (InFileHandle);

	//Mi·erfolg ?
	if (ScriptFilelength<0)
	{
		//Programm abbrechen:
		InternalError (ERR_SCRIPT_INFO, ScriptFilename);
	}

	//Speicher allokieren:
	Script->ScriptMem = THMalloc (ScriptFilelength+1);

	//Script komplett einlesen:
	if (read (InFileHandle,
				 Script->ScriptMem,
				 ScriptFilelength) != ScriptFilelength)
	{
		//Programm abbrechen:
		InternalError (ERR_SCRIPT_READ, ScriptFilename);
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
	Script->Line = malloc ( Script->AnzLines * sizeof(char*) );

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
		l=strlen (Script->Line[i]);

		//Kommentare abhacken:
		for (j=0; j<l; j++)
		{
			if ( (Script->Line[i])[j]=='/' && (Script->Line[i])[j+1]=='/')
			{
				(Script->Line[i])[j]=0;
				break;
			}
		}

		//Rausfiltern: NewLine, CarriageReturn, Space und Tab am Zeilenanfang:
		while ( (Script->Line[i])[0] != 0 &&
				  ( (Script->Line[i])[0] == 10 ||
				    (Script->Line[i])[0] == 13 ||
				    (Script->Line[i])[0] == 32 ||
				    (Script->Line[i])[0] == 9)) Script->Line[i]++;

		//Rausfiltern: NewLine, CarriageReturn, EOF, Space und Tab am Zeilenende:
		while ( strlen(Script->Line[i]) > 0 &&
				  ( (Script->Line[i])[strlen(Script->Line[i])-1] == 10 ||
				    (Script->Line[i])[strlen(Script->Line[i])-1] == 13 ||
				    (Script->Line[i])[strlen(Script->Line[i])-1] == 26 ||
				    (Script->Line[i])[strlen(Script->Line[i])-1] == 32 ||
				    (Script->Line[i])[strlen(Script->Line[i])-1] == 9))
		{
			//String um ein Zeichen kÅrzer machen:
		 	(Script->Line[i])[strlen(Script->Line[i])-1] = 0;
		}
	}
}

//------------------------------------------------------------------------------
//Gibt die Tokennummer der angegebenen Zeile zurÅck. (-1 fÅr unbekannten Token)
//------------------------------------------------------------------------------
SILONG SCRIPT_GetTokenNumber (SCRIPT *Script, SILONG Linenumber, TOKEN *TokenTable)
{
	SILONG i;

	//Leerzeile, Kommentar oder Label ?
	if ((Script->Line[Linenumber])[0]==0 || (Script->Line[Linenumber])[0]==':')
	{
		//eine Null bedeutet leere Zeile:
		return(0);
	}

	//Alle Tokens der Tabelle durchgehen:
	for (i=0; TokenTable[i].Name; i++)
	{
		//Aktuellen Token mit dem Anfang der aktuellen Zeile vergleichen:
		if (strncmp (TokenTable[i].Name,
						 Script->Line[Linenumber],
						 strlen(TokenTable[i].Name)) == 0 &&
			 ( (Script->Line[Linenumber])[strlen(TokenTable[i].Name)]==32 ||
			   (Script->Line[Linenumber])[strlen(TokenTable[i].Name)]==0 ) )
		{
			//Identifikationswert zurÅckgeben:
			return (TokenTable[i].ID);
		}
	}

	//Sorry, aber den Token gibt's nicht:
	return (-1);
}

//------------------------------------------------------------------------------
//Gibt die Zeilen des Labels zurÅck. (-1 fÅr unbekannten Label)
//------------------------------------------------------------------------------
SILONG SCRIPT_FindLabelLine (SCRIPT *Script, char *Label)
{
	SILONG i;

	//Alle Zeilen des Skripts durchgehen:
	for (i=0; i<Script->AnzLines; i++)
	{
		//Ist Zeile ein Label ?
		if ((Script->Line[i])[0]==':')
		{
			//Aktuellen Label mit dem Parameter vergleichen:
			if (strcmp (Script->Line[i]+1, Label) ==0 )
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
	SILONG Token;
	SILONG AnzParameter;
	SILONG NewLine;
	char **Parameter;

	if (Line >= Script->AnzLines || Line<0)
	{
		//Illegale Zeile oder Ende des Scriptes:
		return (-1);
	}

	//Token der aktuellen Zeile identifizieren:
	Token = SCRIPT_GetTokenNumber (Script, Line, &TokenTable);

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
			if (NewLine == -1)		InternalError (ERR_UNKNOWN_LABEL, Line+1, Script->Line[Line]);
			return (NewLine);
			break;

		case TOKEN_END_SETUP:
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			return (-1);
			break;

		case TOKEN_TEST_COMPUTER:
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

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

		case TOKEN_IF_WINDOWS:
			AnzOpenIf[0]++;
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

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
		case TOKEN_COPY:
			if (AnzParameter != 3)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			CopyOneFile (Parameter[1], Parameter[2]);
			break;

		case TOKEN_INSTALL:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
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
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_RENAME:
			if (AnzParameter != 3)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			if (rename (Parameter[1], Parameter[2]))
				InternalError (ERR_RENAME_FAILS, Line+1, Script->Line[Line]);
			break;

		case TOKEN_EXECUTE_SILENT:
			ExecuteShell (AnzParameter, Parameter);
			break;

		case TOKEN_EXECUTE_OWN_SCREEN:
			if (AnzParameter == 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);

			//Bildschirm abschalten:
			DSA_CloseScreen (&SetupScreenPort);

			//Programm ausfÅhren:
			ExecuteShell (AnzParameter, Parameter);

			//Bildschirm wieder einschalten:
			if (!DSA_OpenScr (&SetupOpm, 0))
				InternalError (ERR_NO_VGA);

			//Und wieder mit altem Inhalt fÅllen:
			DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);
			DSA_SPal (&SetupPalette, 0, 256, 0);
			DSA_APal ();
			break;

		case TOKEN_CD:
			if (AnzParameter != 2)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			ChangeDirAndDriveTo (Parameter[1]);
			break;

		case TOKEN_SELECT_TARGET_DRIVE:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_SELECT_TARGET_PATH:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_CD_SOURCE:
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			ChangeDirAndDriveTo (SetupSourcePath);
			break;

		case TOKEN_CD_TARGET:
			if (AnzParameter != 1)	InternalError (ERR_NUM_PARAMETERS, Line+1, Script->Line[Line]);
			ChangeDirAndDriveTo (SetupTargetPath);
			break;

		case TOKEN_CHECK_CD:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_SELECT_CD_ROM_DRIVE:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		//------------------------------------------------------------------------
		//MenÅ-Befehle:
		//------------------------------------------------------------------------
		case TOKEN_MENU_START:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_MENU_ENTRY:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_MENU_END:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_PRINT:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_TEXT:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_ASSERT:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
			break;

		case TOKEN_ERROR:
			InternalError (ERR_TOKEN_NOT_IMPLEMENTED, Line+1, Script->Line[Line]);
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
				OPM_CopyOPMOPM (&LoadOpm,        //Quelle
									 &SetupOpm,       //Ziel
									 0,               //Quell-x
									 0,               //Quell-y
									 LoadOpm.width,   //Dimension der Quelle
									 LoadOpm.height,  //Dimension der Quelle
									 (RESOLUTION_X-LoadOpm.width)/2,   //Position im Ziel
									 (RESOLUTION_Y-LoadOpm.height)/2); //Position im Ziel

				//Und anzeigen:
				DSA_SPal (&SetupPalette, 0, 256, 0);
				DSA_APal ();
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
//Zerlegt eine Zeile in seine Parameter; Gibt diese als char** zurÅck sowie
//die Anzahl der Parameter als call-by-reference
//------------------------------------------------------------------------------
char **SCRIPT_ParseScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzParameter)
{
	SILONG AnzP;
	SILONG c,l;
	static char *rcTable[20]; //Max 20 Parameter;
	static char Buffer[256];  //Arbeitsversion fÅr aktuelle Zeile

	strcpy (Buffer, Script->Line[Line]);

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
					break;
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
//Konvertiert einen universellen String-Parameter in einen String
//------------------------------------------------------------------------------
SILONG SCRIPT_GetIntParameter (char *Parameter, SILONG Line)
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
					return (Line);   //Jau, daher ab nach Hause.

				break;

			case TOKEN_ENDIF:
				AnzOpenIf--;

				if (AnzOpenIf == 0)
					return (Line); //Das war das ersehnt ENDIF ==> ab nach Hause !
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
