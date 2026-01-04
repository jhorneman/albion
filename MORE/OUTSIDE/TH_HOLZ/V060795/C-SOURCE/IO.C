//------------------------------------------------------------------------------
//io.c - Routinen fÅr Disk-IO (Copy, ChangeDir, CreateDir etc.)
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Wechselt Laufwerk und Pfad (Files dÅrfen nicht mit angegeben werden !)
//------------------------------------------------------------------------------
void ChangeDirAndDriveTo (UNCHAR *String)
{
	UNCHAR Tmp [256];
	UNCHAR Drive	[_MAX_DRIVE];
	UNCHAR Dir		[_MAX_DIR];
	unsigned rc;

	//put a backslash at the end of the string:
	if (String[strlen(String)-1]=='\\' || (String[1]==':' && String[2]==0))
		strcpy (Tmp, String);
	else
		sprintf (Tmp, "%s\\", String);

	//Break the path up:
	_splitpath (Tmp, Drive, Dir, NULL, NULL);

	//Ensure case:
	Drive[0] = toupper (Drive[0]);

	//Did user pass a drive wish ?
	if (Drive[0]>='A' && Drive[0]<='Z')
	{
		//Change the drive (1 means drive a, 2 drive b etc.)
		_dos_setdrive (Drive[0]-'A'+1, &rc);

		//Watcom sets the drive without a return code.
		_dos_getdrive (&rc);

		//Thus we have to re-check it (stupid, isn't it ?)
		if (rc != Drive[0]-'A'+1)
		{
			InternalError (ERR_CHANGE_DRIVE, Drive[0]);
			return;
		}
	}

	//Einfach nur ins Top wechseln ?
	if (strcmp (Dir, "\\") != 0)
	{
		//doof: Der Backslash den splitpath unbedingt braucht ist hier stîrend:
		if (strlen(Dir)>0 && Dir[strlen(Dir)-1]=='\\')
			Dir[strlen(Dir)-1] = 0;
	}

	//Soll die Directory Åberhaupt geÑndert werden ?
	if (*Dir)
	{
		if (chdir (Dir) == -1)
		{
			InternalError (ERR_CHANGE_DIR, Dir);
			return;
		}
	}
}

//------------------------------------------------------------------------------
//Kopiert ein File von Source nach Ziel (Beidesmale Filenamen mitangeben)
//(das r/o flag wird dabei nicht mitkopiert)
//------------------------------------------------------------------------------
void CopyOneFile (UNCHAR *Source, UNCHAR *Target)
{
	SILONG InFileHandle;
	SILONG OutFileHandle;
	SILONG Size;
	UNCHAR 	*Buffer;

	//Source-File îffnen:
	InFileHandle = open (Source, O_RDONLY | O_BINARY);
	if (InFileHandle<0)
	{
		//Mi·erfolg:
		InternalError (ERR_COPY_OPEN, Source);
	}

	//Target-File îffnen:
	OutFileHandle = open(Target,O_TRUNC|O_CREAT|O_WRONLY|O_BINARY,S_IWRITE);
	if (OutFileHandle<0)
	{
		//Mi·erfolg:
		close (InFileHandle);
		InternalError (ERR_COPY_OPEN, Target);
	}

	Buffer = THMalloc (32768);
	while (1)
	{
		//Hier ist noch eine genauere Disk-Fehlerabfrage notwendig:
		Size = read (InFileHandle, Buffer, 32768);
		if (Size<=0) break;

		SetupCopiedBytes+=Size;

		if (write (OutFileHandle, Buffer, Size) != Size)
		{
			//Anscheinend nicht genug Speicherplatz:
			InternalError (ERR_COPY_WRITE, Target);
		}
	}

	free (Buffer);
	close (InFileHandle);
	close (OutFileHandle);

	AdministrateProgressWindow (0);
}

//------------------------------------------------------------------------------
//Eine Shell mit Programm und Parametern ausfÅhren:
//------------------------------------------------------------------------------
void ExecuteShell (SILONG AnzParameters, UNCHAR **Parameter)
{
	UNCHAR	 Buffer[256];
	SILONG c;

	//Der Programmname
	strcpy (Buffer, Parameter[1]);

	//Die Parameter hinzufÅgen:
	for (c=2; c<AnzParameters; c++)
	{
		strcat (Buffer, " ");
		strcat (Buffer, Parameter[c]);
	}

	//Die Shell ausfÅhren:
	system (Buffer);
}

//------------------------------------------------------------------------------
//Legt komplexe Directory an und lîscht im Fehlerfall die von ihm angelegten
//Verzeichnisse auch wieder.
//------------------------------------------------------------------------------
void MakeDirs (UNCHAR *DirString)
{
	UNCHAR   Buffer[256];
	UNCHAR 	*ErrorBuffer[30];
	SILONG AnzErrorBuffer;

	//Kopie vorbereiten:
	Buffer [0] = 0;

	AnzErrorBuffer = 0;

	do
	{
		//Buffer um eine Tiefe erweitern:
		do
		{
			do
			{
				Buffer[strlen(Buffer)+1] = 0;
				Buffer[strlen(Buffer)] = DirString[strlen(Buffer)];
			}
			while (DirString[strlen(Buffer)] != '\\' && strlen(Buffer)!=strlen(DirString));
		}
		while (strlen(Buffer)==2 && Buffer[1]==':');

		//Existiert die Directory bis hierhin schon ?
		if (!IfExists (Buffer))
		{
			//Nein, deshalb legen wir sie jetzt an:
			if (mkdir (Buffer))
			{
				//Oh, oh, Fehler ! Wir gehen alle angelegten Dir's durch..
				while (AnzErrorBuffer>0)
				{
					AnzErrorBuffer--;
					rmdir (ErrorBuffer[AnzErrorBuffer]); //..und lîschen sie !
					free (ErrorBuffer[AnzErrorBuffer]);
				}

				//Danach verabschieden wir uns von den Nichtschwimmern:
				InternalError (ERR_MKDIR, DirString);
			}

			//Kopie anlegen:
			ErrorBuffer[AnzErrorBuffer] = THMalloc (256);
			strcpy (ErrorBuffer[AnzErrorBuffer], Buffer);
			AnzErrorBuffer++;
		}
	} while (strlen(Buffer)!=strlen(DirString));

	//Die belegten ErrorBuffers wieder freigeben:
	while (AnzErrorBuffer>0)
	{
		AnzErrorBuffer--;
		free (ErrorBuffer[AnzErrorBuffer]);
	}
}

//------------------------------------------------------------------------------
//équivalent zu xcopy %Source% %Target% /s
//------------------------------------------------------------------------------
void Copy (UNCHAR *Source, UNCHAR *Target, BOOL IncludeSubDirs)
{
	DIR    *DirInfo;
	UNCHAR   *Buffer1, *Buffer2, *Buffer3;
	UNCHAR 	 *Drive, *Dir, *FName, *Ext;

	//Wegen Rekursion Stack sparen:
	Buffer1 = THMalloc (200);
	Buffer2 = THMalloc (200);
	Buffer3 = THMalloc (200);
	Drive   = THMalloc (_MAX_DRIVE);
	Dir     = THMalloc (_MAX_DIR);
	FName   = THMalloc (_MAX_FNAME);
	Ext     = THMalloc (_MAX_EXT);

	DirInfo = opendir (Source);

	//ersten Eintrag vorbereiten:
	if (DirInfo) DirInfo = readdir (DirInfo);

	if (WasHardError) InternalError (ERR_IO);

	//Kopiert angegebene Files eines Verzeichnisses:
	while (DirInfo)
	{
		if ((DirInfo->d_attr & (_A_SUBDIR | _A_VOLID))==0)
		{
			//Wildcards durch viele Aufrufe ersetzen:
			_splitpath (Source, Drive, Dir, FName, Ext);
			_makepath (Buffer2, Drive, Dir, DirInfo->d_name, NULL);

			_splitpath (Target, Drive, Dir, FName, Ext);
			_makepath (Buffer3, Drive, Dir, DirInfo->d_name, NULL);

			//Bekommt simple Aufgaben ohne Wildcards:
			CopyOneFile (Buffer2, Buffer3);
		}
		DirInfo = readdir (DirInfo);
	}
	closedir (DirInfo);

	if (IncludeSubDirs)
	{
		_splitpath (Source, Drive, Dir, NULL, NULL);
		_makepath  (Buffer1, Drive, Dir, "*", "*");
		DirInfo = opendir (Buffer1);

		//ersten Eintrag vorbereiten:
		if (DirInfo) DirInfo = readdir (DirInfo);

		//HÑlt nach unterverzeichnissen Ausschau:
		while (DirInfo)
		{
			if (DirInfo->d_attr & _A_SUBDIR)
			{
				//NatÅrlich wird wie bei "dir" auch "." und ".." gezeigt..
				if (strcmp (DirInfo->d_name, ".")!=0 && //aber die vergessen wir mal..
			    	strcmp (DirInfo->d_name, "..")!=0)
				{
					//Erst mal neue Sub-Target-Dir anlegen:
					_splitpath (Target, Drive, Dir, FName, Ext);
					_makepath (Buffer1, Drive, Dir, NULL, NULL);
					sprintf (Buffer2, "%s%s", Buffer1, DirInfo->d_name);
					mkdir (Buffer2);

					//Dann diese kopieren:
					sprintf (Buffer1, "%s\\%s%s", Buffer2, FName, Ext);
					_splitpath (Source, Drive, Dir, FName, Ext);
					_makepath (Buffer2, Drive, Dir, NULL, NULL);
					sprintf (Buffer3, "%s%s\\%s%s", Buffer2, DirInfo->d_name, FName, Ext);
					Copy (Buffer3, Buffer1, IncludeSubDirs);
				}
			}

			DirInfo = readdir (DirInfo);
		}
		closedir (DirInfo);
	}

	free (Buffer1);
	free (Buffer2);
	free (Buffer3);
	free (Drive);
	free (Dir);
	free (FName);
	free (Ext);
}

//------------------------------------------------------------------------------
//Lîscht Dateien und Unterverzeichnisse; auch mit Wildcards (Eine Art Staub-
//sauger, die man nach Partys braucht. Die auch kaputte StÅhle mit aufsaugt.)
//------------------------------------------------------------------------------
void Delete (UNCHAR *Filespec)
{
	SILONG c,l;
	BOOL	 ContainsWildcards;
	DIR    *DirInfo;
	UNCHAR   *Buffer, *FilespecBuffer;
	UNCHAR 	 *Drive, *Dir, *FName, *Ext;

	//Endet auf Blashslash?
	if (Filespec[strlen(Filespec)-1]=='\\')
	{
		FilespecBuffer = THMalloc(200);
		sprintf (FilespecBuffer, "%s*.*", Filespec);
		Filespec = FilespecBuffer;
	}
	else
		FilespecBuffer = NULL;

	ContainsWildcards = FALSE;

	l=strlen (Filespec);
	for (c=0; c<l; c++)
	{
		ContainsWildcards |= (Filespec[c]=='*');
		ContainsWildcards |= (Filespec[c]=='?');
	}

	//Wegen Rekursion Stack sparen:
	Buffer  = THMalloc (200);
	Drive   = THMalloc (_MAX_DRIVE);
	Dir     = THMalloc (_MAX_DIR);
	FName   = THMalloc (_MAX_FNAME);
	Ext     = THMalloc (_MAX_EXT);

	//Wenn es Wildcards hat, die entschlÅsseln und die Files lîschen:
	if (ContainsWildcards)
	{
		DirInfo = opendir (Filespec);

		//ersten Dir-Eintrag vorbereiten:
		if (DirInfo) DirInfo = readdir (DirInfo);

		if (WasHardError) InternalError (ERR_IO);

		while (DirInfo)
		{
			if ((DirInfo->d_attr & (_A_SUBDIR | _A_VOLID))==0)
			{
				//Wildcards durch viele Aufrufe ersetzen:
				_splitpath (Filespec, Drive, Dir, FName, Ext);
				_makepath (Buffer, Drive, Dir, DirInfo->d_name, NULL);

				//File killen:
				if (AssertWindow(Buffer))
					remove (Buffer);
			}
			DirInfo = readdir (DirInfo);
		}
		closedir (DirInfo);
	}
	//Wenn es ein einfaches File ist, es direkt lîschen:
	else if (IfExistsFile(Filespec))
	{
		if (AssertWindow (Filespec))
			remove (Filespec);
	}
	//Wenn es ein Verzeichnis ist, dann den Inhalt und dann es selbst lîschen:
	else if (IfExistsDir(Filespec))
	{
		//Erst mal die Datein im Verzeichnis killen:
		sprintf (Buffer, "%s\\*.*", Filespec);

		Delete (Buffer);

		//Dann schauen, was noch fÅr Verzeichnisse drin sind:
		DirInfo = opendir (Filespec);

		while (DirInfo)
		{
			if (DirInfo->d_attr & _A_SUBDIR)
			{
				//NatÅrlich wird wie bei "dir" auch "." und ".." gezeigt..
				if (strcmp (DirInfo->d_name, ".")!=0 && //aber die vergessen wir mal..
			   	strcmp (DirInfo->d_name, "..")!=0)
				{
					sprintf (Buffer, "%s\\%s", Filespec, DirInfo->d_name);
					Delete (Buffer);
				}
			}
			DirInfo = readdir (DirInfo);
		}
		closedir (DirInfo);

		//Und dann das Verzeichnis selbst:
		rmdir (Filespec);
	}
	else
	{
		//Nothing; Wahrscheinlich war es ein lîschen nochmal zur Sicherheit..
	}

	free (Buffer);
	free (Drive);
	free (Dir);
	free (FName);
	free (Ext);
	if (FilespecBuffer) free (FilespecBuffer);
}

//------------------------------------------------------------------------------
//équivalent zu xcopy %SourcePath%Source% %TargetPath%Target% /s
//------------------------------------------------------------------------------
void Install (UNCHAR *Source, UNCHAR *Target, BOOL IncludeSubDirs)
{
	UNCHAR Buffer1[100];
	UNCHAR Buffer2[100];

	if (SetupSourcePath[strlen(SetupSourcePath)-1]=='\\')
		sprintf (Buffer1, "%s%s", SetupSourcePath, Source);
	else
		sprintf (Buffer1, "%s\\%s", SetupSourcePath, Source);

	if (SetupTargetPath[strlen(SetupTargetPath)-1]=='\\')
		sprintf (Buffer2, "%s%s", SetupTargetPath, Target);
	else
		sprintf (Buffer2, "%s\\%s", SetupTargetPath, Target);

	Copy (Buffer1, Buffer2, IncludeSubDirs);
}

//------------------------------------------------------------------------------
//FÑngt Diskettenfehler ab; siehe _harderr in c-doku
//------------------------------------------------------------------------------
int __far HarderrHandler (unsigned deverror, unsigned errcode, unsigned far *devhdr)
{
	static Retrys = 0;

	WasHardError = TRUE;

	__deverror = deverror;
	__errcode  = errcode;
	__devhdr	  = devhdr;

	//Interrupt-Time => Man darf so gut wie nichts:
	switch (errcode)
	{
		case 0x00: //Schreibschutz:
			return (_HARDERR_FAIL);
			break;

		case 0x02: //Drive ist offen/leer:
			return (_HARDERR_FAIL);
			break;

		default:
			if (Retrys>2)
				return (_HARDERR_FAIL);
			else
			{
				Retrys++;
				return (_HARDERR_RETRY);
			}
			break;
	}
}
