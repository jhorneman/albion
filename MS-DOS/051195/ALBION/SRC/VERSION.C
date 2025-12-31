/************
 * NAME     : VERSION.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 19-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <stdio.h>

#include <BBDEF.H>

#include <VERSION.H>

/* global variables */

#ifdef ALBION_VERSION_NR
static UNSHORT Albion_version_nr = ALBION_VERSION_NR;
#else
static UNSHORT Albion_version_nr = 0;
#endif

#ifdef ALBION_SUBVERSION_NR
static UNSHORT Albion_subversion_nr = ALBION_SUBVERSION_NR;
#else
static UNSHORT Albion_subversion_nr = 0;
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_version_string
 * FUNCTION  : Get Albion version as a string.
 * FILE      : VERSION.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.08.95 15:39
 * LAST      : 27.10.95 15:27
 * INPUTS    : UNCHAR *String - Pointer to destination string.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_version_string(UNCHAR *String)
{
	/* Build version string */
	_bprintf
	(
		String,
		VERSION_STRING_LENGTH,
		"v%u.%02u (created on %s %s)",
		Albion_version_nr,
		Albion_subversion_nr,
		__DATE__,
		__TIME__
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_version_number
 * FUNCTION  : Get Albion version as a number.
 * FILE      : VERSION.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.95 16:10
 * LAST      : 19.09.95 16:10
 * INPUTS    : None.
 * RESULT    : UNLONG : Version number.
 * BUGS      : No known.
 * NOTES     : - The sub-version number is presumed to have two decimals.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_version_number(void)
{
	/* Build and return version number */
	return ((Albion_version_nr * 100) + Albion_subversion_nr);
}

