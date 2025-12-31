//------------------------------------------------------------------------------
//Die diversen Texte in den diversen Sprachen:
//------------------------------------------------------------------------------
#include "setup.h"

//FÅr MAX_LANGUAGES siehe defines.h
//FÅr MAX_ERROR_MESSAGES siehe defines.h und types.h

MESSAGES Messages [MAX_LANGUAGES] =
{
	//Deutsch:
	{
		//Die Fehlertexte:
		{
			"Keine VGA-Karte vorhanden.",							//ERR_NO_VGA
			"Setup erlaubt maximal einen Parameter.",       //ERR_TOO_MANY_PARAMETERS
			"Kann Scriptfile nicht îffnen:%s.",             //ERR_SCRIPT_OPEN
			"Kann Script nicht lesen:%s.",                  //ERR_SCRIPT_READ
			"Nicht genug erweiterter Speicherplatz.",       //ERR_OUT_OF_MEMORY
			"Falsche Speicherplatzanforderung.",            //ERR_ILLEGAL_MEM
			"Unbekanntes Token in Zeile %li:%s.",           //ERR_UNKNOWN_TOKEN
			"Sprung zu unbekanntem Label in Zeile %li:%s.", //ERR_UNKNOWN_LABEL
			"Fehler bei DSA_InitSystem.",                   //ERR_BB_INIT_DSA
			"Kann Bild nicht laden:%s",                     //ERR_LOAD_PIC
			"Token wurde nicht eingebaut in Zeile %li:%s.", //ERR_TOKEN_NOT_IMPLEMENTED
			"Offener String in Zeile %li:%s.",              //ERR_UNMATCHED_STRING
			"Falsche Parameter-Anzahl in Zeile %li:%s.",    //ERR_NUM_PARAMETERS
			"Kann nicht nach Laufwerk %c wechseln.",        //ERR_CHANGE_DRIVE
			"Kann nicht in Verzeichnis %s wechseln.",       //ERR_CHANGE_DIR
			"Kann copy-Datei nicht îffnen: %s.",            //ERR_COPY_OPEN
			"Kann %s nicht schreiben. Speicherplatz ?",     //ERR_COPY_WRITE
			"Kann Datei nicht umbenennen in Zeile %i:%s",   //ERR_RENAME_FAILS
			"Kann leere Datei %s nicht erzeugen.",          //ERR_CREATE
			"Integer erwartet in Zeile %li.",               //ERR_INT_EXPECTED
			"Falsche Sprache angegeben in Zeile %li:%s.",   //ERR_WRONG_LANGUAGE
			"ELSE ohne IF in Zeile %li:%s.",                //ERR_ELSE_WITHOUT_IF
			"ENDIF ohne IF in Zeile %li:%s.",               //ERR_ENDIF_WITHOUT_IF
			"Zweites ELSE in Zeile %li gefunden:%s",        //ERR_DOUBLE_ELSE
			"EOF statt ENDIF gefunden.",                    //ERR_UNMATCHED_IF
			"Konnte Verzeichnis %s nicht anlegen.",         //ERR_MKDIR
			"<ERROR>-Text zu lang",                         //ERR_ERRTEXT_TOO_BIG
			"Kein MenÅ definiert in Zeile %li:%s",          //ERR_MENU_DO_SELECT
			"Zu viele MenÅeintrÑge in Zeile %li:%s",        //ERR_MAX_MENU
			"Kann %s nicht anzeigen.",                      //ERR_BROWSE_OPEN
			"Syntax Error in Zeile %li:%s",						//ERR_SYNTAX
			"Fehler beim Dateizugriff",                     //ERR_IO
         "Kein CD-Laufwerk installiert!",                //ERR_NO_CD
         "MenÅ ohne Optionen in Zeile %li:%s",           //ERR_NO_OPTION
			NULL
		},

		//Standardsachen:
		"Ja",
		"Nein",
		"O.K.",
		"Laufwerk",
		"Beenden",
		"Dateien werden kopiert...",
		"Seite %li/%li",
		"Abbrechen",

		//FensterÅberschriften:
		"Hinweis",
		"Frage",
		"Fehler!",

		//ganze SÑtze:
		"Bitte wÑhlen Sie ein Ziellaufwerk:",
		"Bitte wÑhlen Sie Ihr CD-Laufwerk:",
		"Sie brauchen mindestens %liKb Festplattenspeicher zur Installation!",
		"Bitte Zielpfad angeben:",

		//Bei Fehlern:
		"Laufwerk %c ist schreibgeschÅtzt. Installation wird abgebrochen.",
		"In Laufwerk %c kann kein DatentrÑger gefunden werden. Installation wird abgebrochen.",
		"Fehler beim Zugriff auf Laufwerk %c. Installation wird abgebrochen.",
		"Wenn es sich bei dem Daten~trÑger um eine CD handelt, reinigen Sie diese bitte vor~sichtig von mîglichen Finger~ab~drÅcken oder anderen Ver~un~reinigungen.",
		"",
		""
	},

	//Englisch:
	{
		//Die Fehlertexte:
		{
			"Keine VGA-Karte gefunden.",							//ERR_NO_VGA
			"Setup erlaubt maximal einen Parameter.",       //ERR_TOO_MANY_PARAMETERS
			"Kann Scriptfile nicht îffnen:%s.",             //ERR_SCRIPT_OPEN
			"Kann Script nicht lesen:%s.",                  //ERR_SCRIPT_READ
			"Nicht genug erweiterter Speicherplatz.",       //ERR_OUT_OF_MEMORY
			"Falsche Speicherplatzanforderung.",            //ERR_ILLEGAL_MEM
			"Unbekannter Token in Zeile %li:%s.",           //ERR_UNKNOWN_TOKEN
			"Sprung zu unbekanntem Label in Zeile %li:%s.", //ERR_UNKNOWN_LABEL
			"Fehler bei DSA_InitSystem.",                   //ERR_BB_INIT_DSA
			"Kann Bild nicht laden:%s",                     //ERR_LOAD_PIC
			"Token wurde nicht eingebaut in Zeile %li:%s.", //ERR_TOKEN_NOT_IMPLEMENTED
			"Offener String in Zeile %li:%s.",              //ERR_UNMATCHED_STRING
			"Falsche Parameter-Anzahl in Zeile %li:%s.",    //ERR_NUM_PARAMETERS
			"Kann nicht nach Laufwerk %c wechseln.",        //ERR_CHANGE_DRIVE
			"Kann nicht in Verzeichnis %s wechseln.",       //ERR_CHANGE_DIR
			"Kann copy-Datei nicht îffnen: %s.",            //ERR_COPY_OPEN
			"Kann %s nicht schreiben. Speicherplatz ?",     //ERR_COPY_WRITE
			"Kann Datei nicht umbenennen in Zeile %i:%s",   //ERR_RENAME_FAILS
			"Kann leere Datei %s nicht erzeugen.",          //ERR_CREATE
			"Integer erwartet in Zeile %li",                //ERR_INT_EXPECTED
			"Falsche Sprache angegeben in Zeile %li:%s",    //ERR_WRONG_LANGUAGE
			"ELSE ohne IF in Zeile %li:%s.",                //ERR_ELSE_WITHOUT_IF
			"ENDIF ohne IF in Zeile %li:%s.",               //ERR_ENDIF_WITHOUT_IF
			"Zweites ELSE in Zeile %li gefunden:%s",        //ERR_DOUBLE_ELSE
			"EOF statt ENDIF gefunden.",                    //ERR_UNMATCHED_IF
			"Konnte Verzeichnis %s nicht anlegen.",         //ERR_MKDIR
			"<ERROR>-Text zu lang",                         //ERR_ERRTEXT_TOO_BIG
			"Kein MenÅ definiert in Zeile %li:%s",          //ERR_MENU_DO_SELECT
			"Zu viele MenÅeintrÑge in Zeile %li:%s",        //ERR_MAX_MENU
			"Kann %s nicht anzeigen.",                      //ERR_BROWSE_OPEN
			"Syntax Error in Zeile %li:%s",						//ERR_SYNTAX
			"Fehler beim Dateizugriff",                     //ERR_IO
         "Kein CD-Laufwerk gefunden!",                //ERR_NO_CD
         "MenÅ ohne Optionen in Zeile %li:%s",           //ERR_NO_OPTION
			NULL
		},

		//Standardsachen:
		"Ja",
		"Nein",
		"O.K.",
		"Drive",
		"Exit",
		"Dateien werden kopiert...",
		"Seite %li/%li",
		"Abbrechen",

		//FensterÅberschriften:
		"Hinweis",
		"Frage",
		"Fehler!",

		//ganze SÑtze:
		"Bitte wÑhlen Sie ein Ziellaufwerk:",
		"Bitte wÑhlen Sie Ihr CD-Laufwerk:",
		"Sie brauchen mindestens %likb Festplattenspeicher zur Installation!",
		"Bitte Zielpfad angeben:",

		//Bei Fehlern:
		"Laufwerk %c ist schreibgeschÅtzt. Installation wird abgebrochen.",
		"In Laufwerk %c kann kein DatentrÑger gefunden werden. Installation wird abgebrochen.",
		"Fehler beim Zugriff auf Laufwerk %c. Installation wird abgebrochen.",
		"Wenn es sich bei dem Daten~trÑger um eine CD handelt, reinigen Sie diese bitte vor~sichtig von mîglichen Finger~ab~drÅcken oder anderen Ver~un~reinigungen.",
		"",
		""
	}
};
