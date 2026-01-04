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
	BOOL   EnforceVGA=FALSE;
	UNCHAR Drive	[_MAX_DRIVE];
	UNCHAR Dir		[_MAX_DIR];
	UNCHAR FName	[_MAX_FNAME];

	Language				 = 0;
	SetupCDDrive       = 0;
	SetupTargetDrive   = 'C';
	SetupTargetPath[0] = 0;

	printf ("\nStarte BlueByte's Setup-Programm...\n(C) 1995 Blue Byte Software GmbH\n");

	//Handler fr systemnahe Fehler installieren:
	_harderr (HarderrHandler);
	WasHardError = FALSE;

	//Setfilenamen ermitteln und sichern:
	SetupFilename = argv[0];

	//Break the path up and join it again to get the SourcePath-Var:
	_splitpath (SetupFilename, Drive, Dir, NULL, NULL);
	_makepath  (SetupSourcePath, Drive, Dir, NULL, NULL);

	//Falls Spiel schon mal konfiguriert wurde, nehmen wir die Einstellungen:
	_splitpath (SetupFilename, Drive, Dir, FName, NULL);
	_makepath  (IniFilename, Drive, Dir, FName, "ini");
	_makepath  (VesaIniFilename, Drive, Dir, "Vesa", "ini");
	LoadIni ();

	//Programmparameter verwalten:
	switch (argc)
	{
		//Keine Parameter ==> Default Scriptname:
		case 1:
			ScriptFilename = "INSTALL.SCR";
			break;

		//Ein Parameter (Skriptname oder /VGA):
		case 2:
			if (stricmp (argv[1], "/VGA")==0)
			{
				EnforceVGA=TRUE;
				ScriptFilename = "INSTALL.SCR";
			}
			else ScriptFilename = argv[1];
			break;

		//Zwei Parameter (Skriptname oder /VGA):
		case 3:
			if (stricmp (argv[1], "/VGA")==0)
			{
				EnforceVGA=TRUE;
				ScriptFilename = argv[2];
			}
			else if (stricmp (argv[2], "/VGA")==0)
			{
				EnforceVGA=TRUE;
				ScriptFilename = argv[1];
			}
			else InternalError (ERR_TOO_MANY_PARAMETERS);
			break;

		//zu viele Parameter:
		default:
			InternalError (ERR_TOO_MANY_PARAMETERS);
			break;
	}

	//Das Scriptfile laden & vorbereiten:
	SKRIPT_Init (&Script, ScriptFilename);

	//Der Tastaturtreiber von BB bringt nur Žrger:
   SYSTEM_Init_Flags|=SYSTEM_FLAG_NOKEYINT;

	//Blue Byte's Spielzeuge anschalten:
	SYSTEM_Init ();
	DSA_Init ();

	//Versuchen, SVGA zu benutzen:
	if (VESA_IsAvailable() && (!EnforceVGA))
	{
		RESOLUTION_X=640;
		RESOLUTION_Y=480;
	}

	while (1)
	{
		//Die Offscreen Pixel Map anlegen:
		OPM_New (RESOLUTION_X, RESOLUTION_Y, 1, &SetupOpm, NULL);
		if (!DSA_OpenScr (&SetupOpm, 0))
		{
			if (RESOLUTION_X==640)
			{
				//SVGA failed:
				OPM_Del (&SetupOpm);
				RESOLUTION_X=320;
				RESOLUTION_Y=200;
				continue;
			}
			InternalError (ERR_NO_VGA);
		}
		break;
	}

	RefreshFixedPalette ();

	//Initialize mouse pointer (palette is changed !)
	SYSTEM_ShowMousePtr(SYSTEM_MousePointerNormal);

	//Standart-Hintergrund generieren:
	Window3d (&SetupOpm, 0, 0, RESOLUTION_X-1, RESOLUTION_Y-1, SETUP_COLOR_LIGHT, SETUP_COLOR, SETUP_COLOR_DARK, SETUP_COLOR_PAPER);
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


	if (AnzOpenIf) //File zuende und trotzdem noch IF's offen:
		InternalError (ERR_UNMATCHED_IF);

	//Programmende -> Die Offscreen Pixel Map meucheln:
	OPM_Del (&SetupOpm);
	DSA_CloseScreen (&SetupScreenPort);
	SYSTEM_Exit();
}
