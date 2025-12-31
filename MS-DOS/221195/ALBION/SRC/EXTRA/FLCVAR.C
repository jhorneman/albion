/************
 * NAME     : FLCVAR.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 12-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#include <BBDEF.H>

#include <FLC.H>
#include <FLCVAR.H>

/*
 ** Global variables *******************************************************
 */

/* Global recording data structure */
struct Flic_record *FLC_I;

/* Global playback data structure */
struct Flic_playback *FLC_O;

BOOLEAN FLC_Recording_flic = FALSE;
BOOLEAN FLC_Playing_back_flic = FALSE;

/* Error handling data */
UNCHAR FLC_Library_name[] = "FLC";

UNCHAR *FLC_Error_strings[] = {
	"Illegal error code.",
	"File error.",
	"Already playing a flic.",
	"This is not a flic.",
	"Illegal frame chunk in flic.",
	"Illegal sub-chunk in flic.",
	"Frame too big for input buffer."
};

