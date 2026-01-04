//------------------------------------------------------------------------------
//Setup.c
//=============
//Hauptfile fr Setup.exe (Scriptgesteuertes DOS-Installprogramm im Auftrag der
//Blue Byte Software GmbH)
//
//Projektbeginn : 2.6.1995
//Autor			 : Thomas Holz
//------------------------------------------------------------------------------

//Das Haupt-include laden:
#include "setup.h"

//------------------------------------------------------------------------------
//Hauptprogramm:
//------------------------------------------------------------------------------
void main (SILONG argc, SIBYTE *argv[])
{
	SCRIPT Script;
	SILONG CurrentLine;
	SILONG AnzOpenIf;
	char 	 Drive	[_MAX_DRIVE];
	char 	 Dir		[_MAX_DIR];
	char 	 FName	[_MAX_FNAME];

	Language				 = 0;
	SetupCDDrive       = 0;
	SetupTargetDrive   = 'C';
	SetupTargetPath[0] = 0;

	printf ("\nStarte BlueByte's Setup-Programm...\n(C) 1995 Blue Byte Software GmbH\n");

	//Setfilenamen ermitteln und sichern:
	SetupFilename = argv[0];

	//Break the path up and join it again to get the SourcePath-Var:
	_splitpath (SetupFilename, Drive, Dir, NULL, NULL);
	_makepath  (SetupSourcePath, Drive, Dir, NULL, NULL);

	//Falls Spiel schon mal konfiguriert wurde, nehmen wir die Einstellungen:
	_splitpath (SetupFilename, Drive, Dir, FName, NULL);
	_makepath  (IniFilename, Drive, Dir, FName, "ini");
	LoadIni ();

	//Programmparameter verwalten:
	switch (argc)
	{
		//Keine Parameter ==> Default Scriptname:
		case 1:
			ScriptFilename = "INSTALL.SCR";
			break;

		//Ein Parameter (Skriptname):
		case 2:
			ScriptFilename = argv[1];
			break;

		//zu viele Parameter:
		default:
			InternalError (ERR_TOO_MANY_PARAMETERS);
			break;
	}

	//Das Scriptfile laden & vorbereiten:
	SKRIPT_Init (&Script, ScriptFilename);

	//Die Offscreen Pixel Map anlegen:
	OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &SetupOpm, NULL);

	SYSTEM_Init ();
	DSA_Init ();
	if (!DSA_OpenScr (&SetupOpm, 0))
		InternalError (ERR_NO_VGA);

	//Initialize mouse pointer (palette is changed !)
	SYSTEM_ShowMousePtr(SYSTEM_MousePointerNormal);

	//Standart-Hintergrund generieren:
	Window3d (&SetupOpm, 0, 0, RESOLUTION_X-1, RESOLUTION_Y-1, SETUP_DEF_BACK_LIGHT, SETUP_DEF_BACK, SETUP_DEF_BACK_DARK, SETUP_DEF_BACK_PAPER);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	//Dieses coole Skript ist objektorientiert und darf daher keine eigenen
	//Daten in der Abarbeitungsroutine haben. Daher braucht man diesen Parameter:
	AnzOpenIf	= 0;

	//Start interpreting the Script:
	CurrentLine = 0;
	while (CurrentLine != -1)
	{
		//Make sure, that the user can cancel:
		kbhit();

		CurrentLine = SCRIPT_ProcessScriptLine (&Script, CurrentLine, &AnzOpenIf);
	}

	//Den Ini-Filenamen ableiten:
	if (*SetupTargetPath)
	{
		//Spiel wird installiert; Ini kommt in den Target-Path:
		_splitpath (SetupTargetPath, Drive, Dir, FName, NULL);
	}
	else
	{
		//Spiel wurde nur neukonfiguriert; Es gibt keinen Target-Path:
		_splitpath (SetupFilename, Drive, Dir, FName, NULL);
	}
	_makepath  (IniFilename, Drive, Dir, FName, "ini");

	UpdateIni();

	if (AnzOpenIf) //File zuende und trotzdem noch IF's offen:
		InternalError (ERR_UNMATCHED_IF);

	//Programmende -> Die Offscreen Pixel Map meucheln:
	OPM_Del (&SetupOpm);
	DSA_CloseScreen (&SetupScreenPort);
	SYSTEM_Exit();
}
