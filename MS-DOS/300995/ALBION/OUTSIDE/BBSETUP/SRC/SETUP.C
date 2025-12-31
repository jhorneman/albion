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

	printf ("\nStarting Blue Byte's setup-program...\n(C) 1995 Blue Byte Software GmbH\n");

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
	if ((!EnforceVGA) && VESA_IsAvailable())
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
   //SYSTEM_ShowMousePtr(SYSTEM_MousePointerNormal);
   Init_mouse_pointer(10, 14, 1, 1, Mouse_graphics);
   //erster Parameter war mal '9'; Aufruf ist auch noch 2x in script.c!

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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_mouse_pointer
 * FUNCTION  : Initialize a new mouse pointer.
 * FILE      : MOUSE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.94 17:42
 * LAST      : 18.07.95 10:33
 * INPUTS    : UNSHORT Width - Width of mouse graphics.
 *             UNSHORT Height - Height of mouse graphics.
 *             UNSHORT Hotspot_X - Hotspot X-coordinate.
 *             UNSHORT Hotspot_Y - Hotspot Y-coordinate.
 *             UNBYTE *Graphics - Pointer to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_mouse_pointer(UNSHORT Width, UNSHORT Height, UNSHORT Hotspot_X,
 UNSHORT Hotspot_Y, UNBYTE *Graphics)
{
	static UNCHAR Info_chunk[] = "INFO";
	static UNCHAR Body_chunk[] = "BODY";
	static UNCHAR End_chunk[] = "ENDE";

	struct GTO *GTO;
	UNLONG Size;
	UNBYTE *Ptr;

	/* Calculate size of sprite image */
	Size = Width * Height;

	/* Allocate memory for new mouse GTO */
   Ptr = (UNBYTE*)THMalloc (sizeof(struct GTO) + Size + 4);

	/* Prepare mouse GTO */
	GTO = (struct GTO *) Ptr;

	Write_chunk_name((UNCHAR *) GTO->infochunk, Info_chunk);
	GTO->name[0] = 0;
	GTO->transcol = 0;
	GTO->plane = 0xFF;
	GTO->xoffset = 0 - Hotspot_X;
	GTO->yoffset = 0 - Hotspot_Y;
	GTO->width = Width;
	GTO->height = Height;
	Write_chunk_name((UNCHAR *) GTO->bodychunk, Body_chunk);

	/* Copy graphics */
	memcpy(Ptr + sizeof(struct GTO), Graphics, Size);

	/* Insert ENDE chunk */
	Write_chunk_name(Ptr + sizeof(struct GTO) + Size, End_chunk);

	/* Set new mouse pointer */
	SYSTEM_HideMousePtr();
	SYSTEM_ShowMousePtr(GTO);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_chunk_name
 * FUNCTION  : Write a four-character chunk name.
 * FILE      : MOUSE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.94 10:53
 * LAST      : 01.09.94 10:53
 * INPUTS    : UNBYTE *Address - Destination address of chunk name.
 *             UNCHAR Chunk_name[] - Chunk name.
 * RESULT    : UNBYTE * : Updated pointer.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Write_chunk_name(UNBYTE *Address, UNCHAR Chunk_name[])
{
	UNSHORT i;

	for (i=0;i<4;i++)
	{
		*Address++ = Chunk_name[i];
	}

	return Address;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_AIL_system
 * FUNCTION  : Dummy function needed by BBSYSTEM.
 * FILE      : SETUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09-07-95 04:15pm
 * LAST      : 09-07-95 04:15pm
 * INPUTS    : None.
 * RESULT    : BOOLEAN : FALSE (AIL not installed).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_AIL_system(void)
{
	return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_AIL_system
 * FUNCTION  : Dummy function needed by BBSYSTEM.
 * FILE      : SETUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09-07-95 04:15pm
 * LAST      : 09-07-95 04:15pm
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_AIL_system(void)
{
}

