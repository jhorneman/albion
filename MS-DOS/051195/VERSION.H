/************
 * NAME     : VERSION.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 19-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef VERSION_H
#define VERSION_H

/*
 ** Defines ****************************************************************
 */

#define VERSION_STRING_LENGTH	(100)

/*
 ** Prototypes *************************************************************
 */

void Get_version_string(UNCHAR *String);
UNLONG Get_version_number(void);

#endif

