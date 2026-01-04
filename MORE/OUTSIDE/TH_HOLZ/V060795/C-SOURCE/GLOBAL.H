//------------------------------------------------------------------------------
//global.h - extern-deklarationen der globalen variablen: (siehe auch global.c)
//------------------------------------------------------------------------------

//Kompletter Filename des Setups-files, z.B. "E:\BATTLE\SETUP.EXE":
extern char *SetupFilename;

//Die Auslîsung des Bildschirmes
extern SILONG RESOLUTION_X;
extern SILONG RESOLUTION_Y;

//Filename des Script-files, z.B. "D:\BI\SETUP.SCR" oder "INSTALL.SCR"
extern char *ScriptFilename;
extern char IniFilename[80];      //Filename der INI-Datei, z.B. d:\bb\setup.ini
extern char VesaIniFilename[80];  //Filename der Vesa-INI-Datei, z.B. d:\bb\vesa.ini

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

//Wird am SelectTargetPath initialisiert und fÅr den Progress-bar verwendet.
extern SILONG SetupNeededKbFree;
extern SILONG SetupCopiedBytes;

//Die Daten fÅr das AuswahlmenÅ:
extern MENU AuswahlMenue;

//Die Daten der Maus:
extern SILONG Mouse_X;
extern SILONG Mouse_Y;
extern SILONG Button_state;
extern SILONG Last_button_state;
extern SILONG Ignore_second_left_click;
extern SILONG Ignore_second_right_click;

//Globale Kommunikationsvarianten der _harderr-Variablen:
extern unsigned __deverror, __errcode, far *__devhdr;
extern BOOL WasHardError;

//Definiert die stÑndig fest gewÑhlten Farben:
extern TH_PALETTE DefaultPal[];
