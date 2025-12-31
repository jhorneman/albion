//------------------------------------------------------------------------------
//Ini.c - Bietet High-Level-funktionen zum Umgang mit ini-Dateien
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Hinweis: Alle Ini-Funktionen setzen vorraus, daá die Datei schon existiert.
//         Sie darf leer sein, aber sie muá vorhanden sein.
//Hinweis: Leerzeichen vor oder nach dem Gleichheitszeichen sind nicht erlaubt.
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//entfernt alle CodeString-Zeichen die am Ende des Strings vorkommen:
//---------------------------------------------------------------------------
char *StrRemoveEndingCodes (char *String, char *CodeString)
{
   SILONG c;

   for (c=strlen(String)-1; c>=0; c--)
   {
      //cancel the loop, when String[c] is not in CodeString:
      if (strchr (CodeString, String[c])==NULL)
         break;
   }

   //Cut bad things of:
   String[c+1]=0;

   return (String);
}

//------------------------------------------------------------------------------
//Žndert eine Variable in "Filename" (bzw. fgt sie ggf. hinzu)
//------------------------------------------------------------------------------
BOOL IniSet (UNCHAR *Filename, UNCHAR *Section, UNCHAR *Name, UNCHAR *Value)
{
   FILE   *Input, *Output;
   BOOL   VarFound;
   UNCHAR TempFilename[80];
   UNCHAR Line  [256];
   UNCHAR Drive [_MAX_DRIVE];
   UNCHAR Dir   [_MAX_DIR];
   UNCHAR FName [_MAX_FNAME];
   UNCHAR Ext   [_MAX_EXT];
   UNCHAR CurrentSection[80];
   BOOL   WasSection  = FALSE;
   BOOL   WasAnything = FALSE;

   CurrentSection[0]=0;

	//INI-Datei muá existieren:
	if (!IfExistsFile (Filename)) return (NULL);

	VarFound = FALSE;

	//Wir kopieren das File in ein Tmp-Filem, l”schen das Origal und renamen..
	_splitpath (Filename, Drive, Dir, NULL, NULL);
	_makepath  (TempFilename, Drive, Dir, "install", "tmp");

	//Open Files:
	Input  = fopen (Filename, "r");

	//War das ™ffnen erfolgreich ?
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
            if (WasSection)
            {
               //Die Variable fehlte, aber die Section war da!
               fclose (Output);
               fclose (Input);
               if (remove (TempFilename)!=0) return (NULL);
               return (BrutalIniSet (Filename, Section, Name, Value));
            }
            else
            {
               //Variable und Section fehlen:
               if (WasAnything)
                  if (fputs ("\n", Output)==0) { fclose (Output); fclose (Input); return (NULL); }

               sprintf (Line, "[%s]\n", Section);
               if (fputs (Line, Output)==0) { fclose (Output); fclose (Input); return (NULL); }

               sprintf (Line, "%s=%s\n", Name, Value);
               if (fputs (Line, Output)==0) { fclose (Output); fclose (Input); return (NULL); }
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

      StrRemoveEndingCodes (Line, "\xd\xa\1a\x20\x9");

      //Test, ob Zeile eine neue Section einleitet:
      if (strlen(Line)>2 && Line[0]=='[' && Line[strlen(Line)-1]==']')
      {
         strcpy (CurrentSection, Line+1);
         CurrentSection [strlen (CurrentSection)-1] = 0;
      }
      //Sind wir in einer relevanten Section ?
      else if (stricmp (CurrentSection, Section)==0)
      {
         WasSection = TRUE;

         //Zeilenanfang mit Bezeichner vergleichen:
         if (strnicmp (Line, Name, strlen(Name))==0)
         {
            if (Line[strlen(Name)]=='=')
            {
               VarFound = TRUE;

               //Zeile ersetzen:
               sprintf (Line, "%s=%s", Name, Value);
            }
         }
      }

      WasAnything = TRUE;
      if (*Line)
      {
         if (fputs (Line, Output)==0 || fputs ("\n", Output)==0)
         {
            fclose (Output);
            fclose (Input);
            return (NULL);
         }
		}
      else
      {
         if (fputs ("\n", Output)==0)
         {
            fclose (Output);
            fclose (Input);
            return (NULL);
         }
		}
	}
}

//------------------------------------------------------------------------------
//Fgt eine Variable in "Filename" ohne Rcksicht hinzu:
//------------------------------------------------------------------------------
BOOL BrutalIniSet (UNCHAR *Filename, UNCHAR *Section, UNCHAR *Name, UNCHAR *Value)
{
   FILE   *Input, *Output;
   BOOL   VarFound;
   UNCHAR TempFilename[80];
   UNCHAR Line  [256];
   UNCHAR Drive [_MAX_DRIVE];
   UNCHAR Dir   [_MAX_DIR];
   UNCHAR FName [_MAX_FNAME];
   UNCHAR Ext   [_MAX_EXT];
   UNCHAR CurrentSection[80];

   CurrentSection[0]=0;

	//INI-Datei muá existieren:
	if (!IfExistsFile (Filename)) return (NULL);

	VarFound = FALSE;

	//Wir kopieren das File in ein Tmp-Filem, l”schen das Origal und renamen..
	_splitpath (Filename, Drive, Dir, NULL, NULL);
	_makepath  (TempFilename, Drive, Dir, "install", "tmp");

	//Open Files:
	Input  = fopen (Filename, "r");

	//War das ™ffnen erfolgreich ?
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

      StrRemoveEndingCodes (Line, "\xd\xa\1a\x20\x9");

      //Test, ob Zeile eine neue Section einleitet:
      if (strlen(Line)>2 && Line[0]=='[' && Line[strlen(Line)-1]==']')
      {
         strcpy (CurrentSection, Line+1);
         CurrentSection [strlen (CurrentSection)-1] = 0;

         if (stricmp (CurrentSection, Section)==0)
         {
            //Zeile einfgen:
            if (fputs (Line, Output)==0 || fputs ("\n", Output)==0)
            {
               fclose (Output);
               fclose (Input);
               return (NULL);
            }
            sprintf (Line, "%s=%s", Name, Value);
         }
      }

      if (*Line)
      {
         if (fputs (Line, Output)==0 || fputs ("\n", Output)==0)
         {
            fclose (Output);
            fclose (Input);
            return (NULL);
         }
		}
      else
      {
         if (fputs ("\n", Output)==0)
         {
            fclose (Output);
            fclose (Input);
            return (NULL);
         }
		}
	}
}

//------------------------------------------------------------------------------
//L”scht ein Variable aus "Filename" (Sections werden nicht gel”scht)
//------------------------------------------------------------------------------
BOOL IniDelete (UNCHAR *Filename, UNCHAR *Section, UNCHAR *Name)
{
   FILE   *Input, *Output;
   UNCHAR TempFilename[80];
   UNCHAR Line  [256];
   UNCHAR Drive [_MAX_DRIVE];
   UNCHAR Dir   [_MAX_DIR];
   UNCHAR FName [_MAX_FNAME];
   UNCHAR Ext   [_MAX_EXT];
   UNCHAR CurrentSection[80];

   CurrentSection[0]=0;

	//INI-Datei muá existieren:
	if (!IfExistsFile (Filename)) return (NULL);

	//Wir kopieren das File in ein Tmp-Filem, l”schen das Origal und renamen..
	_splitpath (Filename, Drive, Dir, NULL, NULL);
	_makepath  (TempFilename, Drive, Dir, "install", "tmp");

	//Open Files:
	Input  = fopen (Filename, "r");

	//War das ™ffnen erfolgreich ?
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

      StrRemoveEndingCodes (Line, "\xd\xa\1a\x20\x9");

      //Test, ob Zeile eine neue Section einleitet:
      if (strlen(Line)>2 && Line[0]=='[' && Line[strlen(Line)-1]==']')
      {
         strcpy (CurrentSection, Line+1);
         CurrentSection [strlen (CurrentSection)-1] = 0;
      }
      //Sind wir in einer relevanten Section ?
      else if (stricmp (CurrentSection, Section)==0)
      {
         //Zeilenanfang mit Bezeichner vergleichen:
         if (strnicmp (Line, Name, strlen(Name))==0)
         {
            if (Line[strlen(Name)]=='=')
            {
               //Diese Zeile wird nicht kopiert.
               continue;
            }
         }
		}

      if (*Line)
      {
         if (fputs (Line, Output)==0 || fputs ("\n", Output)==0)
         {
            fclose (Output);
            fclose (Input);
            return (NULL);
         }
		}
      else
      {
         if (fputs ("\n", Output)==0)
         {
            fclose (Output);
            fclose (Input);
            return (NULL);
         }
		}
	}
}

//------------------------------------------------------------------------------
//Gibt einen Pointer auf einen static-buffer mit dem Eintrag zurck (oder NULL)
//------------------------------------------------------------------------------
UNCHAR *IniRead (UNCHAR *Filename, UNCHAR *Section, UNCHAR *Name)
{
	FILE *Input;
	static UNCHAR Buffer[100];
	UNCHAR Line[256];
   UNCHAR CurrentSection[80];

   CurrentSection[0]=0;

	//INI-Datei muá existieren:
	if (!IfExistsFile (Filename)) return (NULL);

	//Open File for text-input:
	Input=fopen (Filename, "r");

	//War das ”ffnen erfolgreich ?
	if (Input==NULL) return (NULL);

	while (1)
	{
		if (fgets (Line, 255, Input)==0)
		{
			fclose (Input);
			return (NULL);
		}

      StrRemoveEndingCodes (Line, "\xd\xa\1a\x20\x9");

      //Test, ob Zeile eine neue Section einleitet:
      if (strlen(Line)>2 && Line[0]=='[' && Line[strlen(Line)-1]==']')
      {
         strcpy (CurrentSection, Line+1);
         CurrentSection [strlen (CurrentSection)-1] = 0;
      }
      //Sind wir in einer relevanten Section ?
      else if (stricmp (CurrentSection, Section)==0)
      {
         //Zeilenanfang mit Bezeichner vergleichen:
         if (strnicmp (Line, Name, strlen(Name))==0)
         {
            if (Line[strlen(Name)]=='=')
            {
               fclose (Input);
               strcpy (Buffer, Line+strlen(Name)+1);
               return (Buffer);
            }
         }
		}
	}
}

//------------------------------------------------------------------------------
//Aktualisiert alle Daten in der INI-Datei:
//------------------------------------------------------------------------------
void UpdateIni (void)
{
	UNCHAR   Buffer  [200];

	//Es ist Vorraussetzung, daá bereits eine INI-Datei existiert. Sonst tut
	//diese Routine keinen Hammerschlag. Sie auch "setup.sam".
	if (IfExistsFile (IniFilename))
	{
		#if FALSE
		itoa (BenchPixelSpeed(), Buffer, 10);
      IniSet (IniFilename, "SYSTEM", "PIXEL_PRO_SEC", Buffer);

      IniSet (IniFilename, "SYSTEM", "INSTALLED", "Y");
		#endif

		if (VESA_IsAvailable())
         IniSet (IniFilename, "SYSTEM", "VESA", "Y");
		else
         IniSet (IniFilename, "SYSTEM", "VESA", "N");

		if (_bios_equiplist()&2)
         IniSet (IniFilename, "SYSTEM", "FPU", "Y");
		else
         IniSet (IniFilename, "SYSTEM", "FPU", "N");

		if (SetupCDDrive)
		{
			sprintf (Buffer, "%c", SetupCDDrive);
         IniSet (IniFilename, "SYSTEM", "CD_ROM_DRIVE", Buffer);
		}

		if(SetupTargetDrive)
		{
			sprintf (Buffer, "%s", SetupSourcePath);
		   IniSet (IniFilename, "SYSTEM", "SOURCE_PATH", Buffer);
		}

		itoa (Language+1, Buffer, 10);
      IniSet (IniFilename, "SYSTEM", "LANGUAGE", Buffer);
	}
}

//------------------------------------------------------------------------------
//Probiert die Ini zu laden:
//------------------------------------------------------------------------------
void LoadIni (void)
{
	UNCHAR *rc;

   rc = IniRead (IniFilename, "SYSTEM", "LANGUAGE");
	if (rc) Language = atoi(rc)-1;

   rc = IniRead (IniFilename, "SYSTEM", "CD_ROM_DRIVE");
	if (rc) SetupCDDrive = rc[0];
}

#if FALSE
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
#endif

//------------------------------------------------------------------------------
//Den Ini-Pfad ableiten:
//------------------------------------------------------------------------------
void CalculateIniFilename (void)
{
   UNCHAR Drive   [_MAX_DRIVE];
   UNCHAR Dir     [_MAX_DIR];
   UNCHAR FName   [_MAX_FNAME];

   if (*SetupTargetPath)
   {
      UNCHAR Buffer[200];

      sprintf (Buffer, "%s\\x.x", SetupTargetPath);

      //Spiel wird installiert; Ini kommt in den Target-Path:
      _splitpath (Buffer, Drive, Dir, FName, NULL);
   }
   else
   {
      //Spiel wurde nur neukonfiguriert; Es gibt keinen Target-Path:
      _splitpath (SetupFilename, Drive, Dir, FName, NULL);
   }
   //Den Ini-Filenamen vom Script ableiten:
   _splitpath (SetupFilename, NULL, NULL, FName, NULL);

   _makepath  (IniFilename, Drive, Dir, FName, "ini");
}
