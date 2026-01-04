//------------------------------------------------------------------------------
//Ini.c - Bietet High-Level-funktionen zum Umgang mit ini-Dateien
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Hinweis: Alle Ini-Funktionen setzen vorraus, da· die Datei schon existiert.
//         Sie darf leer sein, aber sie mu· vorhanden sein.
//Hinweis: Leerzeichen vor oder nach dem Gleichheitszeichen sind nicht erlaubt.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//éndert eine Variable in "Filename" (bzw. fÅgt sie ggf. hinzu)
//------------------------------------------------------------------------------
BOOL IniSet (char *Filename, char *Name, char *Value)
{
	FILE 			*Input, *Output;
	BOOL        VarFound;
	char        TempFilename[80];
	char 			Line	[256];
	char 	 		Drive	[_MAX_DRIVE];
	char 	 		Dir	[_MAX_DIR];
	char 	 		FName	[_MAX_FNAME];
	char 	 		Ext	[_MAX_EXT];

	//INI-Datei mu· existieren:
	if (!IfExistsFile (Filename)) return (NULL);

	VarFound = FALSE;

	//Wir kopieren das File in ein Tmp-Filem, lîschen das Origal und renamen..
	_splitpath (Filename, Drive, Dir, NULL, NULL);
	_makepath  (TempFilename, Drive, Dir, "install", "tmp");

	//Open Files:
	Input  = fopen (Filename, "r");

	//War das ôffnen erfolgreich ?
	if (Input==NULL) return (NULL);

	Output = fopen (TempFilename, "w");
	if (Output==NULL)
	{
		fclose (Input);
		return (NULL);
	}

	while (1)
	{
		if (fgets (Line, 255, Input)==0)
		{
			if (!VarFound)
			{
				//Variable war nicht drin; kommt aber jetzt hinzu:
				sprintf (Line, "%s=%s\x0a", Name, Value);
				if (fputs (Line, Output)==0)
				{
					fclose (Output);
					fclose (Input);
					return (NULL);
				}
			}

			fclose (Output);
			fclose (Input);

			if (remove (Filename)!=0) return (NULL);

			_splitpath (Filename, NULL, NULL, FName, Ext);
			_makepath  (Line, NULL, NULL, FName, Ext);
			if (rename (TempFilename, Filename)!=0) return (NULL);
			return (TRUE);
		}

		//Zeilenanfang mit Bezeichner vergleichen:
		if (strnicmp (Line, Name, strlen(Name))==0)
		{
			if (Line[strlen(Name)]=='=')
			{
				VarFound = TRUE;

				//Zeile ersetzen:
				sprintf (Line, "%s=%s\x0a", Name, Value);
			}
		}

		if (fputs (Line, Output)==0)
		{
			fclose (Output);
			fclose (Input);
			return (NULL);
		}
	}
}

//------------------------------------------------------------------------------
//Lîscht ein Variable aus "Filename"
//------------------------------------------------------------------------------
BOOL IniDelete (char *Filename, char *Name)
{
	FILE 			*Input, *Output;
	char        TempFilename[80];
	char 			Line	[256];
	char 	 		Drive	[_MAX_DRIVE];
	char 	 		Dir	[_MAX_DIR];
	char 	 		FName	[_MAX_FNAME];
	char 	 		Ext	[_MAX_EXT];

	//INI-Datei mu· existieren:
	if (!IfExistsFile (Filename)) return (NULL);

	//Wir kopieren das File in ein Tmp-Filem, lîschen das Origal und renamen..
	_splitpath (Filename, Drive, Dir, NULL, NULL);
	_makepath  (TempFilename, Drive, Dir, "install", "tmp");

	//Open Files:
	Input  = fopen (Filename, "r");

	//War das ôffnen erfolgreich ?
	if (Input==NULL) return (NULL);

	Output = fopen (TempFilename, "w");
	if (Output==NULL)
	{
		fclose (Input);
		return (NULL);
	}

	while (1)
	{
		if (fgets (Line, 255, Input)==0)
		{
			fclose (Output);
			fclose (Input);

			if (remove (Filename)!=0) return (NULL);

			_splitpath (Filename, NULL, NULL, FName, Ext);
			_makepath  (Line, NULL, NULL, FName, Ext);
			if (rename (TempFilename, Filename)!=0) return (NULL);
			return (TRUE);
		}

		//Zeilenanfang mit Bezeichner vergleichen:
		if (strnicmp (Line, Name, strlen(Name))==0)
		{
			if (Line[strlen(Name)]=='=')
			{
				//Diese Zeile wird nicht kopiert.
				continue;
			}
		}

		if (fputs (Line, Output)==0)
		{
			fclose (Output);
			fclose (Input);
			return (NULL);
		}
	}
}

//------------------------------------------------------------------------------
//Gibt einen Pointer auf einen static-buffer mit dem Eintrag zurÅck (oder NULL)
//------------------------------------------------------------------------------
char *IniRead (char *Filename, char *Name)
{
	FILE *Input;
	static char Buffer[100];
	char Line[256];

	//INI-Datei mu· existieren:
	if (!IfExistsFile (Filename)) return (NULL);

	//Open File for text-input:
	Input=fopen (Filename, "r");

	//War das îffnen erfolgreich ?
	if (Input==NULL) return (NULL);

	while (1)
	{
		if (fgets (Line, 255, Input)==0)
		{
			fclose (Input);
			return (NULL);
		}

		//Zeilenanfang mit Bezeichner vergleichen:
		if (strnicmp (Line, Name, strlen(Name))==0)
		{
			if (Line[strlen(Name)]=='=')
			{
				fclose (Input);
				strcpy (Buffer, Line+strlen(Name)+1);

				//Rausfiltern: NewLine, CarriageReturn, EOF, Space und Tab am Zeilenende:
				while ( strlen(Buffer) > 0 &&
				  		( Buffer[strlen(Buffer)-1] == 10 ||
				    		Buffer[strlen(Buffer)-1] == 13 ||
				    		Buffer[strlen(Buffer)-1] == 26 ||
				    		Buffer[strlen(Buffer)-1] == 32 ||
				    		Buffer[strlen(Buffer)-1] == 9))
				{
					//String um ein Zeichen kÅrzer machen:
		 			Buffer[strlen(Buffer)-1] = 0;
				}
				return (Buffer);
			}
		}
	}
}

//------------------------------------------------------------------------------
//Aktualisiert alle Daten in der INI-Datei:
//------------------------------------------------------------------------------
void UpdateIni (void)
{
	char   Buffer  [80];

	//Es ist Vorraussetzung, da· bereits eine INI-Datei existiert. Sonst tut
	//diese Routine keinen Hammerschlag. Sie auch "setup.sam".
	if (IfExistsFile (IniFilename))
	{
		itoa (Language+1, Buffer, 10);
		IniSet (IniFilename, "LANGUAGE", Buffer);

		if (SetupCDDrive)
		{
			sprintf (Buffer, "%c", SetupCDDrive);
			IniSet (IniFilename, "CD_ROM_DRIVE", Buffer);
		}

		if (_bios_equiplist()&2)
			IniSet (IniFilename, "FPU", "Y");
		else
			IniSet (IniFilename, "FPU", "N");

		if (VESA_IsAvailable())
			IniSet (IniFilename, "VESA", "Y");
		else
			IniSet (IniFilename, "VESA", "N");

		IniSet (IniFilename, "INSTALLED", "Y");

		itoa (BenchPixelSpeed(), Buffer, 10);
		IniSet (IniFilename, "PIXEL_PRO_SEC", Buffer);
	}
}

//------------------------------------------------------------------------------
//Probiert die Ini zu laden:
//------------------------------------------------------------------------------
void LoadIni (void)
{
	char *rc;

	rc = IniRead (IniFilename, "LANGUAGE");
	if (rc) Language = atoi(rc)-1;

	rc = IniRead (IniFilename, "CD_ROM_DRIVE");
	if (rc) SetupCDDrive = rc[0];
}

//------------------------------------------------------------------------------
//Probiert wieviele Pixel man pro sec in die Grafikkarte schieben kann:
//------------------------------------------------------------------------------
SILONG BenchPixelSpeed (void)
{
	SILONG Time;
	SILONG CountFrames;

	Time=*((long*)0x46C);
	CountFrames=0;

	//Eine halbe Sekunde lang benchen:
	while (*((long*)0x46C)-Time < 9)
	{
		//bench, bench:
		DSA_CopyMainOPMToScr (DSA_CMOPMTS_ALWAYS);

		CountFrames++;
	}

	//So viele Pixel werden pro Sekunde kopiert:
	return (CountFrames * RESOLUTION_X * RESOLUTION_Y * 2);
}
