/************
 * NAME     : FLCVAR.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 12-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef FLCVAR_H
#define FLCVAR_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/* FLC error codes */
#define FLCERR_FILE_ERROR				(1)
#define FLCERR_ALREADY_PLAYING		(2)
#define FLCERR_NOT_FLIC					(3)
#define FLCERR_ILLEGAL_FRAME_CHUNK	(4)
#define FLCERR_ILLEGAL_SUBCHUNK		(5)
#define FLCERR_FRAME_TOO_BIG			(6)

#define FLCERR_MAX						(6)

/*
 ** Structure definitions **************************************************
 */

/* Flic chunk (frame) header */
struct Flic_chunk_header {
	UNLONG size;					/* Size of chunk, including sub-chunks & header */
	UNSHORT type;					/* See below */
	UNSHORT chunks;				/* Number of sub-chunks */
	UNBYTE reserved[8];			/* Set to 0 */
};

/* Values for Flic_chunk_header.type */
#define FLC_FLIC_FRAME		(0xF1FA)

/* Flic sub-chunk (palette, delta frame) header */
struct Flic_subchunk_header {
	UNLONG size;					/* Size of chunk, including header */
	UNSHORT type;					/* See below */
};

/* Values for Flic_subchunk_header.type */
#define FLC_COLOR_256		(4)
#define FLC_DELTA_FLC		(7)
#define FLC_COLOR_64			(11)
#define FLC_DELTA_FLI		(12)
#define FLC_BLACK				(13)
#define FLC_BYTE_RUN			(15)
#define FLC_LITERAL			(16)
#define FLC_PSTAMP			(18)

/* FLC delta packet data */
struct FLC_delta {
	UNSHORT Column_skip;
	UNSHORT Packet_length;
	UNSHORT *Data_ptr;
};

/* FLC error */
struct FLC_error {
	UNSHORT Code;
};

/*
 ** Prototypes *************************************************************
 */

/* Error handling functions */
extern void FLC_Error(UNSHORT Error_code);
extern void FLC_Print_error(UNCHAR *buffer, UNBYTE *data);

/*
 ** Global variables *******************************************************
 */

/* Global recording data structure */
extern struct Flic_record *FLC_I;

/* Global playback data structure */
extern struct Flic_playback *FLC_O;

extern BOOLEAN FLC_Recording_flic;
extern BOOLEAN FLC_Playing_back_flic;

/* Error handling data */
extern UNCHAR FLC_Library_name[];

extern UNCHAR *FLC_Error_strings[];

#endif

