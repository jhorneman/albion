//------------------------------------------------------------------------------
//global.c - definition der globalen Variablen
//------------------------------------------------------------------------------
#include "setup.h"

//Kompletter Filename des Setups-files, z.B. "E:\BATTLE\SETUP.EXE":
UNCHAR *SetupFilename;

//Die Auflîsung des Bildschirmes
SILONG RESOLUTION_X=320;
SILONG RESOLUTION_Y=200;

//Filename des Script-files, z.B. "D:\BI\SETUP.SCR" oder "INSTALL.SCR"
UNCHAR *ScriptFilename;
UNCHAR IniFilename[80];      //Filename der INI-Datei, z.B. d:\bb\setup.ini
UNCHAR VesaIniFilename[80];  //Filename der Vesa-INI-Datei, z.B. d:\bb\vesa.ini

//Die eingestellte Sprache (Default ist 0):
SILONG Language=0;

//Das wird den Benutzer ggf. an etwas erinnern:
UNCHAR youreallyshouldturnoncasesensitivelinking=0;
UNCHAR YouReallyShouldTurnOnCaseSensitiveLinking=1;

//Globales OPM, Palette und Screenport:
struct OPM				SetupOpm;
struct BBPALETTE		SetupPalette;

//Quelle und Ziel (Laufwerk und Pfad)
UNCHAR SetupSourcePath[256];
UNCHAR SetupTargetPath[256];
UNCHAR SetupTargetDrive;
UNCHAR SetupCDDrive;

//Wird am SelectTargetPath initialisiert und fÅr den Progress-bar verwendet.
SILONG SetupNeededKbFree=0;
SILONG SetupCopiedBytes=0;

//Die Daten fÅr das AuswahlmenÅ:
MENU AuswahlMenue;

//Die Daten der Maus:
SILONG Mouse_X;
SILONG Mouse_Y;
SILONG Button_state;
SILONG Last_button_state;
SILONG Ignore_second_left_click;
SILONG Ignore_second_right_click;

//Globale Kommunikationsvarianten der _harderr-Variablen:
unsigned __deverror, __errcode, far *__devhdr;
BOOL WasHardError;

//Definiert die stÑndig fest gewÑhlten Farben:
TH_PALETTE DefaultPal[]=
{
	{   0,      0,   0,   0 }, //Schwarz
	{ 240,     17,  58,   0 }, //GrÅntîne..
	{ 241,     34,  75,   0 },
	{ 242,     45,   0,   0 }, //Orangetîne..
	{ 243,     67,   6,   0 },
	{ 244,     84,  23,   0 },
	{ 245,    100,  41,   0 },
	{ 246,      0,  13,  42 }, //Blautîne..
	{ 247,      0,  28,  59 },
	{ 248,      9,  45,  73 },
	{ 249,     63,  72, 100 },
	{ 250,     16,  16,  16 }, //Grautîne..
	{ 251,     33,  33,  33 },
	{ 252,     50,  50,  50 },
	{ 253,     67,  67,  67 },
	{ 254,     86,  86,  86 },
	{ 255,    100, 100, 100 }, //Wei·
	{  -1,     -1,  -1,  -1 }  //Tabellenende
};
