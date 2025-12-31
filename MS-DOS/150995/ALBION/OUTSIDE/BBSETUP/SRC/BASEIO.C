//------------------------------------------------------------------------------
//Basis io-routinen (eigendlich die Aufgabe der Biblithek)
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
//Ist Laufwerk physikalisch vorhanden oder nur ASSIGNed ?
//Int 21h, AH=44h, AL=0Eh - Drive=1=A:>
//------------------------------------------------------------------------------

/* GeÑndert am 30-8-1995 von Jurie Horneman */
/* Nochmal geÑndert am 6-9-1995 von Jurie Horneman */

BOOL IsDriveReal (SILONG Drive)
{
	/* Pfusch um das Problem mit das schlechte Drive-auswahl-Fenster zu
	  lîsen : alle Laufwerke zwischen C und H sind "echt" */

	if ((Drive >= 3) && (Drive <= 26))
		return TRUE;
	else
		return FALSE;
}

//------------------------------------------------------------------------------
//Gibt wahr zurÅck falls das File oder die Directory existiert
//------------------------------------------------------------------------------
BOOL IfExists (UNCHAR *Filename)
{
	unsigned rc;

	//Das ist schlauer als ein probe-open, da dies auch dir's annimmt:
	if (_dos_getfileattr (Filename, &rc) == 0)
		return (TRUE);
	else
		return (FALSE);
}

//------------------------------------------------------------------------------
//Gibt wahr zurÅck falls das File existiert
//------------------------------------------------------------------------------
BOOL IfExistsFile (UNCHAR *Filename)
{
	SILONG	Handle;

	Handle = open (Filename, O_RDONLY|O_BINARY);
	if (Handle>0)
	{
  		close (Handle);
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}

//------------------------------------------------------------------------------
//Gibt wahr zurÅck falls das Verzeichnis existiert
//------------------------------------------------------------------------------
BOOL IfExistsDir (UNCHAR *Filename)
{
	UNCHAR  Buffer[200];
	DIR    *DirInfo;

	sprintf (Buffer, "%s\\*.*", Filename);
	DirInfo = opendir (Buffer);

	if (DirInfo)
	{
		closedir (DirInfo);
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}

//------------------------------------------------------------------------------
//Legt 'ne leere Datei an. Wei· nicht wozu, aber er kann es:
//------------------------------------------------------------------------------
void CreateFile (UNCHAR *Filename)
{
	SILONG OutFileHandle;

	//File îffnen:
	OutFileHandle = open (Filename, O_CREAT|O_WRONLY|O_BINARY,S_IWRITE);
	if (OutFileHandle<0)
	{
		//Mi·erfolg:
		InternalError (ERR_CREATE, Filename);
	}

	close (OutFileHandle);
}

//------------------------------------------------------------------------------
//Gibt TRUE zurÅck, falls Setup in einer Windows-DosBox lÑuft
//------------------------------------------------------------------------------
BOOL IfWindows (void)
{
	union REGS Regs;

	Regs.w.ax=0x1600;

	//Quelle: PC-Intern, Seite 961
	int386 (0x2f, &Regs, &Regs);

	switch (Regs.h.al)
	{
		case 0x00:
		case 0x80:
			Regs.w.ax=0x4680;
			int386 (0x2f, &Regs, &Regs); //und wieder Multiplex
			if (Regs.h.al==0x80)
			{
				//kein Windows:
				return (FALSE);
			}
			else
			{
				//Windows im Real oder Standartmodus:
				return (TRUE);
			}

		default:
			//Windows/386 V2.x
			return (TRUE);
	}
}

//------------------------------------------------------------------------------
//Dummy-Funktion die spÑter mal den Wert des "Lastdrive"-Settings liefern soll!
//------------------------------------------------------------------------------
SILONG GetLastdrive (void)
{
   return (26);
}
