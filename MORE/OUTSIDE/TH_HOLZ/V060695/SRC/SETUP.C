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

	printf ("\nStarte BlueByte's Setup-Programm...\n(C) 1995 Blue Byte Software GmbH\n");

	//Setfilenamen ermitteln und sichern:
	SetupFilename = argv[0];

	//Break the path up and join it again to get the SourcePath-Var:
	_splitpath (SetupFilename, Drive, Dir, NULL, NULL);
	_makepath  (SetupSourcePath, Drive, Dir, NULL, NULL);

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

	//Den Screen access beantragen:
	//if (!DSA_InitSystem())
	//	InternalError (ERR_BB_INIT_DSA);

	if (!DSA_OpenScr (&SetupOpm, 0))
		InternalError (ERR_NO_VGA);

	//Standart-Hintergrund generieren:
	Rectangle3d (&SetupOpm, 0, 0, RESOLUTION_X-1, RESOLUTION_Y-1, SETUP_DEF_BACK_LIGHT, SETUP_DEF_BACK, SETUP_DEF_BACK_DARK);
	Rectangle3d (&SetupOpm, 2, 2, RESOLUTION_X-3, RESOLUTION_Y-3, SETUP_DEF_BACK_DARK, SETUP_DEF_BACK, SETUP_DEF_BACK_LIGHT);
	DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

	//Dieses coole Skript ist objektorientiert und darf daher keine eigenen
	//Daten in der Abarbeitungsroutine haben. Daher braucht man diesen Parameter:
	AnzOpenIf	= 0;

	//Start interpreting the Script:
	CurrentLine = 0;
	while (CurrentLine != -1)
	{
		CurrentLine = SCRIPT_ProcessScriptLine (&Script, CurrentLine, &AnzOpenIf);
	}
	if (AnzOpenIf) //File zuende und trotzdem noch IF's offen:
		InternalError (ERR_UNMATCHED_IF);

	getch();
	//...

	//Programmende -> Die Offscreen Pixel Map meucheln:
	OPM_Del (&SetupOpm);
	DSA_CloseScreen (&SetupScreenPort);
	//DSA_ExitSystem ();
}
