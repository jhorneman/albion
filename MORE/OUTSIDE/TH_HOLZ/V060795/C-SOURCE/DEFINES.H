//------------------------------------------------------------------------------
//File fÅr BB's Setup.exe - definition einiger verÑnderbaren Sachen:
//------------------------------------------------------------------------------

//generelles:
#define MAX_LANGUAGES 2					//0=Deutsch, 1=Englisch
#define MAX_ERROR_MESSAGES 50
#define MAX_MENU_ENTRIES 20         //Nie mehr als 20 EintrÑge in einem MenÅ

//MenÅfarben:
	//Der Standarthintergrund wenn kein Bild geladen ist:
	#define SETUP_COLOR_LIGHT 249
	#define SETUP_COLOR       248
	#define SETUP_COLOR_DARK  247
	#define SETUP_COLOR_PAPER 246

	//Die Button-Colors:
	#define BUTTON_COLOR       253
	#define BUTTON_COLOR_LIGHT 255
	#define BUTTON_COLOR_DARK  251
	#define BUTTON_COLOR_TEXT  0

	//Die Farben fÅr ein normales AuswahlmenÅ:
	#define MENU_COLOR                  248
	#define MENU_COLOR_LIGHT            249
	#define MENU_COLOR_DARK             247
	#define MENU_COLOR_HEADLINE_BACK    246
	#define MENU_COLOR_HEADLINE_TEXT    254
	#define MENU_COLOR_BUTTON_FRAME     246
	#define MENU_COLOR_BUTTON_HIGHLIGHT 249

	//Die Farben fÅr eine Error-Box
	#define ERROR_COLOR                  244
	#define ERROR_COLOR_LIGHT            245
	#define ERROR_COLOR_DARK             243
	#define ERROR_COLOR_HEADLINE_BACK    242
	#define ERROR_COLOR_HEADLINE_TEXT    255
	#define ERROR_COLOR_BUTTON_FRAME     242
	#define ERROR_COLOR_BUTTON_HIGHLIGHT 245

//MenÅaussehen:
#define MENU_BORDER_WIDTH			3
#define SHADOW_WIDTH             3

//Geschwindigkeit mit der sich Fenster îffnen:
#define WIN_EXPLODE_SPEED			1

//Die y-Grî·e des Fonts (die x-grî·e wird von einer Funktion errechnet)
#define FONT_Y_SIZE	8
