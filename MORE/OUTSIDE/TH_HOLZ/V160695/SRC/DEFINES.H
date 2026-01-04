//------------------------------------------------------------------------------
//File fÅr BB's Setup.exe - definition einiger verÑnderbaren Sachen:
//------------------------------------------------------------------------------

//generelles:
#define MAX_LANGUAGES 2					//0=Deutsch, 1=Englisch
#define MAX_ERROR_MESSAGES 50
#define MAX_MENU_ENTRIES 20         //Nie mehr als 20 EintrÑge in einem MenÅ

//Die Auslîsung des Bildschirmes
#define RESOLUTION_X 320
#define RESOLUTION_Y 200

//MenÅfarben:
	//Der Standarthintergrund wenn kein Bild geladen ist:
	#define SETUP_DEF_BACK_LIGHT 15
	#define SETUP_DEF_BACK       7
	#define SETUP_DEF_BACK_DARK  8
	#define SETUP_DEF_BACK_PAPER 30

	//Die Farben fÅr ein normales AuswahlmenÅ:
	#define MENU_BACK_COLOR				30
	#define MENU_BORDER_COLOR_LIGHT	15
	#define MENU_BORDER_COLOR			7
	#define MENU_BORDER_COLOR_DARK	8
	#define MENU_FONT_COLOR      		9

	//Die Farben fÅr den File Browser:
	#define BROWSE_BACK_COLOR				30
	#define BROWSE_BORDER_COLOR_LIGHT	15
	#define BROWSE_BORDER_COLOR			7
	#define BROWSE_BORDER_COLOR_DARK		8
	#define BROWSE_FONT_COLOR      		9

	//Die Farben fÅr eine Error-Box
	#define ERROR_BACK_COLOR			48
	#define ERROR_BORDER_COLOR_LIGHT	12
	#define ERROR_BORDER_COLOR			 4
	#define ERROR_BORDER_COLOR_DARK	 0
	#define ERROR_FONT_COLOR          0

	//blinkender Rahmen um aktive Option:
	#define MENU_HIGHLIGHT           13

//MenÅaussehen:
#define MENU_BORDER_WIDTH			3

//Geschwindigkeit mit der sich Fenster îffnen:
#define WIN_EXPLODE_SPEED			1

//Die y-Grî·e des Fonts (die x-grî·e wird von einer Funktion errechnet)
#define FONT_Y_SIZE	8
