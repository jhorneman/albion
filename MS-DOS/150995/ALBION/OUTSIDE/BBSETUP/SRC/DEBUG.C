//------------------------------------------------------------------------------
//Debug.c - Programmroutinen zum debuggen und Error-Handling:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
// Programm verlassen und Meldung ausgeben oder nur Meldung ausgeben:
//------------------------------------------------------------------------------
void InternalError (SISHORT ErrNum, ...)
{
	static UNCHAR Buffer[400];

	//Hilfskonstruktion fr beliebige viele Argumente deklarieren:
	va_list	Vars;

	if (WasHardError)
	{
		switch (__errcode)
		{
			case 0x00: //Schreibschutz:
				sprintf (Buffer, Messages[Language].WriteProtect, (__deverror&127)+'A');
				ErrorWindow (Buffer);
				break;

			case 0x02: //Drive ist offen/leer:
				sprintf (Buffer, Messages[Language].InsertMedium, (__deverror&127)+'A');
				ErrorWindow (Buffer);
				break;

			default:
				sprintf (Buffer, Messages[Language].MediaError, (__deverror&127)+'A');
				ErrorWindow (Buffer);
				if ((__deverror&127)+'A' == SetupCDDrive)
				{
					ErrorWindow (Messages[Language].Advice1ForCD);
					ErrorWindow (Messages[Language].Advice2ForCD);
					ErrorWindow (Messages[Language].Advice3ForCD);
				}
				break;
		}
	}

	//Programmende -> Die Offscreen Pixel Map meucheln:
	OPM_Del (&SetupOpm);
	DSA_CloseScreen (&SetupScreenPort);

	//Textheader ausdrucken:
	printf ("\n\nInterner Fehler %i !\n", ErrNum);

	//Tabelle initialisieren:
	va_start (Vars, ErrNum);

	//Beschreibung ausgeben:
	vprintf (Messages[Language].ErrorMessage[ErrNum-1000], Vars);
	printf ("\n");

	//Daten bereinigen:
	va_end (Vars);

	SYSTEM_Exit();

	exit (-1);
}

//------------------------------------------------------------------------------
//Im Debugger einen automatischen Breakpoint setzen:
//------------------------------------------------------------------------------
void DoDebugBreak (void)
{
	union REGS Regs;

	//Call Breakpoint interrupt:
	int386 (3, &Regs, &Regs);
}

//------------------------------------------------------------------------------
//Sorry Jurie, aber alles was ich brauche ist eine sichere Methode um
//Speicherplatz zu bekommen. Wenn keiner da ist, dann l„uft das Programm nicht,
//so einfach ist das. Daher hier eine start abgespeckte Routine aus meinem
//eigenen Memory-Management System:
//------------------------------------------------------------------------------
UNCHAR *THMalloc (SILONG Size)
{
	UNCHAR *p;

	//Is requested Size ridiculous?:
	if (Size>0xfffff)
		InternalError (ERR_ILLEGAL_MEM);

	p = malloc (Size+10);

	if (!p)
		InternalError (ERR_OUT_OF_MEMORY);

	return(p);
}
