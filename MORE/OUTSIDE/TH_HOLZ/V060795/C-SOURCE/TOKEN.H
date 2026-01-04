//------------------------------------------------------------------------------
//token.h - enthÑlt die defines der Token-Tabelle fÅr den Script-Interpreter
//------------------------------------------------------------------------------

//Vermischtes:
#define TOKEN_GOTO           1000
#define TOKEN_END            1001
#define TOKEN_TEST_COMPUTER  1002

//If-Befehle:
#define TOKEN_IF_EXISTS      2000
#define TOKEN_IF_NOT_EXISTS  2001
#define TOKEN_IF_LANGUAGE    2002
#define TOKEN_IF_WINDOWS     2003
#define TOKEN_ELSE           2004
#define TOKEN_ENDIF          2005

//Disk-Befehle:
#define TOKEN_COPY                  3000
#define TOKEN_INSTALL               3001
#define TOKEN_MD                    3002
#define TOKEN_CREATE                3003
#define TOKEN_DELETE                3004
#define TOKEN_RENAME                3005
#define TOKEN_EXECUTE_SILENT        3006
#define TOKEN_EXECUTE_OWN_SCREEN    3007
#define TOKEN_CD                    3008
#define TOKEN_SELECT_TARGET_DRIVE	3009
#define TOKEN_SELECT_TARGET_PATH    3010
#define TOKEN_CD_SOURCE             3011
#define TOKEN_CD_TARGET             3012
#define TOKEN_CHECK_CD              3013
#define TOKEN_SELECT_CD_ROM_DRIVE   3014

//MenÅ-Befehle:
#define TOKEN_MENU_START            4000
#define TOKEN_MENU_ENTRY        		4001
#define TOKEN_MENU_END              4002
#define TOKEN_PRINT                 4003
#define TOKEN_TEXT                  4004
#define TOKEN_ASSERT                4005
#define TOKEN_INFO                  4006
#define TOKEN_ERROR                 4007
#define TOKEN_LOAD_BACKGROUND       4008
#define TOKEN_SET_LANGUAGE          4009
