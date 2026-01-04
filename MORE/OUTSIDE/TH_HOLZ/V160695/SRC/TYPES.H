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

	//Ja, nein, etc. in der Landessprache:
	char *Yes;
	char *No;
	char *OK;
	char *Drive;
	char *Exit;
	char *ChooseYourDrive;
	char *ChooseYourCDDrive;
	char *NotEnoughDiskSpace;
}
MESSAGES;


//------------------------------------------------------------------------------
//Die halbwegs objektorientierte Skript-Struktur:
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
//Die Struktur zum VerknÅpfen von Token-ID mit Token-Name:
//------------------------------------------------------------------------------
typedef struct STRUCT_TOKEN
{
	//String mit dem Token selbst;
	char *Name;

	//Ein Wert zur Identifikation:
	SILONG ID;
}
TOKEN;

//------------------------------------------------------------------------------
//Ein Punkt im AuswahlmenÅ
//------------------------------------------------------------------------------
typedef struct STRUCT_MENU_ENTRY
{
	char  *Text;         //Text fÅr den MenÅeintrag (NULL = kein Text)
	char  *Shortcut;     //FÅr Menuverwaltung die Schnell-Tasten
	SILONG Line;         //Sprung zu welcher Zeile (-1 = nur öberschrift)
	SILONG x1,y1,x2,y2;  //Ausma·e des Rechteckes
}
MENU_ENTRY;

//------------------------------------------------------------------------------
//Die Struktur fÅr das Auswahl-MenÅ:
//------------------------------------------------------------------------------
typedef struct STRUCT_MENU
{
	SILONG AnzEntrys;

	MENU_ENTRY MenuEntry[ MAX_MENU_ENTRIES ];
}
MENU;
