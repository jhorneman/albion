//------------------------------------------------------------------------------
//global.c - definition der globalen Variablen
//------------------------------------------------------------------------------
#include "setup.h"

//Kompletter Filename des Setups-files, z.B. "E:\BATTLE\SETUP.EXE":
char *SetupFilename;

//Filename des Script-files, z.B. "D:\BI\SETUP.SCR" oder "INSTALL.SCR"
char *ScriptFilename;
char IniFilename[80];

//Die eingestellte Sprache (Default ist 0):
SILONG Language=0;

//Das wird den Benutzer ggf. an etwas erinnern:
char youreallyshouldturnoncasesensitivelinking=0;
char YouReallyShouldTurnOnCaseSensitiveLinking=1;

//Globales OPM, Palette und Screenport:
struct OPM				SetupOpm;
struct BBPALETTE		SetupPalette;
//struct BBSCREENPORT	SetupScreenPort;

//Quelle und Ziel (Laufwerk und Pfad)
char SetupSourcePath[256];
char SetupTargetPath[256];
char SetupTargetDrive;
char SetupCDDrive;

//Die Daten fÅr das AuswahlmenÅ:
MENU AuswahlMenue;

//Die Daten der Maus:
SILONG Mouse_X;
SILONG Mouse_Y;
SILONG Button_state;
SILONG Last_button_state;
SILONG Ignore_second_left_click;
SILONG Ignore_second_right_click;
