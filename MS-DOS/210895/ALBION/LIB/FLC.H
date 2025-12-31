/************
 * NAME     : FLC.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-11-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef FLC_H
#define FLC_H

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>

/*
 ** Defines ****************************************************************
 */

/* FLC error codes */
#define FLCERR_FILE_ERROR				(1)
#define FLCERR_ALREADY_PLAYING		(2)
#define FLCERR_NOT_FLIC					(3)
#define FLCERR_ILLEGAL_FRAME_CHUNK	(4)
#define FLCERR_ILLEGAL_SUBCHUNK		(5)

#define FLCERR_MAX						(5)

/*
 ** Structure definitions **************************************************
 */

/* Flic file header */
struct Flic_header {
	UNLONG size;					/* Size of entire file */
	UNSHORT type;					/* See below */
	UNSHORT frames;
	UNSHORT width;
	UNSHORT height;
	UNSHORT depth;					/* Bits per pixel : always 8 */
	UNSHORT flags;					/* See below */
	UNLONG speed;					/* In milliseconds for .FLC */
	UNSHORT reserved1;			/* Set to 0 */
	UNLONG created;				/* MS-DOS date and time of file creation */
	UNLONG creator;				/* Set to 0 */
	UNLONG updated;				/* MS-DOS date and time of last file update */
	UNLONG updater;				/* Set to 0 */
	UNSHORT aspectx;
	UNSHORT aspecty;
	UNBYTE reserved2[38];		/* Set to 0 */
	UNLONG oframe1;				/* Offset from SOF to first frame */
	UNLONG oframe2;				/* Offset from SOF to second frame */
	UNBYTE reserved3[40];		/* Set to 0 */
};

/* Values for Flic_header.type */
#define FLI_TYPE			(0xAF11)
#define FLC_TYPE			(0xAF12)

/* Values for Flic_header.flags */
#define FLI_FINISHED		(0x0001)
#define FLI_LOOPED		(0x0002)

/* Flic recording I/O structure */
struct Flic_record {
	UNCHAR *Flic_filename;			/* Filename of flic-file */

	struct OPM *First_frame;		/* Empty OPM */
	struct OPM *Previous_frame;	/* Empty OPM */
	struct OPM *Input_frame;		/* Source OPM */

	UNBYTE *Output_buffer;			/* Buffer used */

	struct BBPALETTE *Palette;		/* Pointer to current palette */

	struct Flic_header Flic;		/* Flic header */

	UNSHORT Width, Height;			/* Flic dimensions */
};

/* Flic playback I/O structure */
struct Flic_playback {
	struct OPM *Output_frame;		/* Target OPM */

	SISHORT Playback_X;				/* Playback coordinates */
	SISHORT Playback_Y;

	UNBYTE *Input_buffer;			/* Buffer containing flic */

	struct BBPALETTE *Palette;		/* Pointer to current palette */

//	struct SCREENPORT *Screen;		/* Pointer to screen */

	BOOLEAN Palette_was_changed;	/* Set if the palette was changed while
											   decoding the current frame */

	UNSHORT Black_colour_index;	/* Palette index of black */
	UNSHORT Trans_colour_index;	/* Palette index of transparent colour /
												 0xFFFF */

	UNSHORT First_colour;			/* First colour that may be modified */
	UNSHORT Max_colours;				/* Number of colours that may be modified */

	struct Flic_header Flic;		/* Flic header */

	UNSHORT Width, Height;			/* Flic dimensions */

	UNSHORT Current_frame;			/* Current frame */

	UNSHORT Nr_frames;				/* Total number of frames */

	UNLONG Previous_frame_offset;	/* Offset to previous frame */
};

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

/*
 ** Prototypes *************************************************************
 */

extern BOOLEAN FLC_Start_flic_recording(struct Flic_record *Flic_IO);
extern BOOLEAN FLC_Stop_flic_recording(void);
extern BOOLEAN FLC_Record_flic_frame(void);

extern BOOLEAN FLC_Start_flic_playback(struct Flic_playback *Flic_IO);
extern void FLC_Restart_flic(void);
extern BOOLEAN FLC_Stop_flic_playback(void);
extern BOOLEAN FLC_Playback_flic_frame(void);

#endif

