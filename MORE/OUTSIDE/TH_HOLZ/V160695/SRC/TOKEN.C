//------------------------------------------------------------------------------
//Token.c - enthÑlt die definition der Token-Tabelle fÅr den Script-Interpreter
//------------------------------------------------------------------------------
#include "setup.h"

TOKEN TokenTable[]=
{
	//Vermischtes:
	{ "GOTO",					TOKEN_GOTO },
	{ "END_SETUP",       	TOKEN_END_SETUP },
	{ "TEST_COMPUTER",   	TOKEN_TEST_COMPUTER },

	//If-Befehle:
	{ "IF_EXISTS",       	TOKEN_IF_EXISTS },
	{ "IF_NOT_EXISTS",   	TOKEN_IF_NOT_EXISTS },
	{ "IF_LANGUAGE",     	TOKEN_IF_LANGUAGE },
	{ "IF_WINDOWS",      	TOKEN_IF_WINDOWS },
	{ "ELSE",            	TOKEN_ELSE },
	{ "ENDIF",           	TOKEN_ENDIF },

	//Disk-Befehle:
	{ "COPY",            	TOKEN_COPY },
	{ "INSTALL",         	TOKEN_INSTALL },
	{ "MAKEDIR",         	TOKEN_MD },
	{ "MKDIR",           	TOKEN_MD },
	{ "MD",              	TOKEN_MD },
	{ "CREATE",          	TOKEN_CREATE },
	{ "DELETE",          	TOKEN_DELETE },
	{ "RENAME",          	TOKEN_RENAME },
	{ "EXECUTE_SILENT",     TOKEN_EXECUTE_SILENT },
	{ "EXECUTE_OWN_SCREEN", TOKEN_EXECUTE_OWN_SCREEN },
	{ "CD",              	TOKEN_CD },
	{ "CHDIR",           	TOKEN_CD },
	{ "SELECT_TARGET_DRIVE",TOKEN_SELECT_TARGET_DRIVE },
	{ "SELECT_TARGET_PATH", TOKEN_SELECT_TARGET_PATH },
	{ "CD_SOURCE",          TOKEN_CD_SOURCE },
	{ "CD_TARGET",          TOKEN_CD_TARGET },
	{ "CHECK_CD",           TOKEN_CHECK_CD },
	{ "SELECT_CD_ROM_DRIVE",TOKEN_SELECT_CD_ROM_DRIVE },

	//MenÅ-Befehle:
	{ "MENU_RESET",      	TOKEN_MENU_RESET },
	{ "MENU_ADD_ENTRY",   	TOKEN_MENU_ADD_ENTRY },
	{ "MENU_DO_SELECT",    	TOKEN_MENU_DO_SELECT },
	{ "PRINT",           	TOKEN_PRINT },
	{ "BROWSE_FILE",        TOKEN_BROWSE_FILE },
	{ "ASSERT",          	TOKEN_ASSERT },
	{ "ERROR",           	TOKEN_ERROR },
	{ "LOAD_BACKGROUND", 	TOKEN_LOAD_BACKGROUND },
	{ "SET_LANGUAGE",    	TOKEN_SET_LANGUAGE },

	//Ende der Tabelle:
	{ NULL, 0 }
};
