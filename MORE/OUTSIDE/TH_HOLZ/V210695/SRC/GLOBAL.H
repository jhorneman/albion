//------------------------------------------------------------------------------
//global.h - extern-deklarationen der globalen variablen: (siehe auch global.c)
//------------------------------------------------------------------------------

extern char *SetupFilename;

//Filename des Script-files, z.B. "D:\BI\SETUP.SCR" oder "INSTALL.SCR"
extern char *ScriptFilename;
extern char IniFilename[80];

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

//Die Daten der Maus:
extern SILONG Mouse_X;
extern SILONG Mouse_Y;
extern SILONG Button_state;
extern SILONG Last_button_state;
extern SILONG Ignore_second_left_click;
extern SILONG Ignore_second_right_click;
