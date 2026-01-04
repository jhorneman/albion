//------------------------------------------------------------------------------
//global.h - extern-deklarationen der globalen variablen: (siehe auch global.c)
//------------------------------------------------------------------------------

extern char *SetupFilename;

//Filename des Script-files, z.B. "D:\BI\SETUP.SCR" oder "INSTALL.SCR"
extern char *ScriptFilename;

//Die Texte fÅr die Fehler:
extern MESSAGES Messages [];

//Die eingestellte Sprache (Default ist 0):
extern SILONG Language;

//Globales OPM, Palette und Screenport:
extern struct OPM				SetupOpm;
extern struct BBPALETTE		SetupPalette;
//extern struct BBSCREENPORT	SetupScreenPort;

//Die Tabelle mit den Tokens (siehe Token.c):
extern TOKEN TokenTable[];

//Quelle und Ziel (Laufwerk und Pfad)
extern char SetupSourcePath[256];
extern char SetupTargetPath[256];
extern char SetupTargetDrive;
extern char SetupCDDrive;

//Die Daten fÅr das AuswahlmenÅ:
extern MENU AuswahlMenue;
