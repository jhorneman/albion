//------------------------------------------------------------------------------
//types.h - die Typendefinitionen von setup.exe
//------------------------------------------------------------------------------

//Wozu hab' ich denn bitteschîn bbdef.h, wenn ich DAS selbermachen mu· ?
#define BOOL char

//Die verschiedenen Nachrichten fÅr die einzelnen Sprachen:
typedef struct STRUCT_MESSAGES
{
	//Die Texte fÅr die Fehler:
	char *ErrorMessage [MAX_ERROR_MESSAGES];

	//Ja, nein und o.k. in der Landessprache:
	char *Yes;
	char *No;
	char *OK;
}
MESSAGES;


//Die halbwegs objektorientierte Skript-Struktur:
typedef struct STRUCT_SCRIPT
{
	//Der Speicher in dem das gesamte Skript liegt:
	char *ScriptMem;

	//Die Anzahl der Zeilen des Skripts:
	SILONG	AnzLines;

	//Die einzelnen Zeilen des Skripes:
	SIBYTE **Line;
}
SCRIPT;

typedef struct STRUCT_TOKEN
{
	//String mit dem Token selbst;
	char *Name;

	//Ein Wert zur Identifikation:
	SILONG ID;
}
TOKEN;
