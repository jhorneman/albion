//------------------------------------------------------------------------------
//Debug.c - Programmroutinen zum debuggen und Error-Handling:
//------------------------------------------------------------------------------
#include "setup.h"

//------------------------------------------------------------------------------
// Programm verlassen und Meldung ausgeben oder nur Meldung ausgeben:
//------------------------------------------------------------------------------
void InternalError (SISHORT ErrNum, ...)
{
	//Hilfskonstruktion fr beliebige viele Argumente deklarieren:
	va_list	Vars;

	_setvideomode (_TEXTC80);

	//Textheader ausdrucken:
	printf ("\n\nInterer Fehler %i !\n", ErrNum);

	//Tabelle initialisieren:
	va_start (Vars, ErrNum);

	//Beschreibung ausgeben:
	vprintf (Messages[Language].ErrorMessage[ErrNum-1000], Vars);
	printf ("\n");

	//Daten bereinigen:
	va_end (Vars);

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
char *THMalloc (SILONG Size)
{
	char *p;

	//Is requested Size ridiculous?:
	if (Size>0xfffff)
		InternalError (ERR_ILLEGAL_MEM);

	p = malloc (Size+10);

	if (!p)
		InternalError (ERR_OUT_OF_MEMORY);

	return(p);
}
