/************
 * NAME     : FASTFLC.C
 * AUTHOR   : Jurie Horneman & Thomas H„user, BlueByte
 * START    : 12-9-1995
 * PROJECT  : Fast FLC animation playback
 * NOTES    :
 * VERSION  : 1.0
 * SEE ALSO : FASTFLC.H, FLC.H
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <stdio.h>
#include <string.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBDOS.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMISC.H>
#include <BBERROR.H>

#include <FLC.H>
#include <FASTFLC.H>
#include <FLCVAR.H>

/*
 ** Prototypes *************************************************************
 */

/* Flic playback support functions */
static BOOLEAN FASTFLC_Decode_frame_chunk(UNBYTE *Frame_chunk_ptr);

static void FASTFLC_Decode_COLOR_256_chunk(UNBYTE *Ptr);
static void FASTFLC_Decode_DELTA_FLC_chunk(UNBYTE *Ptr);
static void FASTFLC_Decode_COLOR_64_chunk(UNBYTE *Ptr);
static void FASTFLC_Decode_DELTA_FLI_chunk(UNBYTE *Ptr);
static void FASTFLC_Decode_BLACK_chunk(UNBYTE *Ptr);
static void FASTFLC_Decode_BYTE_RUN_chunk(UNBYTE *Ptr);
static void FASTFLC_Decode_LITERAL_chunk(UNBYTE *Ptr);

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Start_flic_playback
 * FUNCTION  : Start playing back a flic.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 20:24
 * LAST      : 12.09.95 14:32
 * INPUTS    : struct Flic_playback *Flic_IO - Pointer to initialized flic
 *              recording I/O structure.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * NOTES     : - The input buffer, palette, screen, black colour index and
 *              output OPM elements of the I/O structure must have been set to
 *              meaningful values.
 *             - It is a good idea to clear the entire Flic_playback
 *              structure before filling it with values!!
 *             - Only one flic can be played at the same time.
 *             - The flic header will be initialized.
 *             - If a flic is already being played, playing back will fail.
 *             - To stream, open a file and insert the BBDOS filehandle and
 *              an offset into the file in the Flic_playback structure.
 *              Then set the STREAMING flag.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FASTFLC_Start_flic_playback(struct Flic_playback *Flic_IO)
{
	BOOLEAN Result;
	UNLONG Read_bytes;
	UNSHORT Flic_type;

	/* Already recording a flic ? */
	if (FLC_Playing_back_flic)
	{
		/* Yes -> Error */
		FLC_Error(FLCERR_ALREADY_PLAYING);
		return FALSE;
	}

	/* Store I/O structure */
	FLC_O = Flic_IO;

	/* Streaming ? */
	if (FLC_O->Flags & FLC_STREAM)
	{
		/* Yes -> Seek to the start of the flic */
		Result = DOS_Seek
		(
			FLC_O->Input_file_handle,
			BBDOS_SEEK_START,
			FLC_O->File_base_offset
		);

		/* Error ? */
		if (!Result)
		{
			/* Yes -> Report and exit */
			FLC_Error(FLCERR_FILE_ERROR);
			return FALSE;
		}

		/* Load flic header */
		Read_bytes = DOS_Read
		(
			FLC_O->Input_file_handle,
			(UNBYTE *) &(FLC_O->Flic),
			sizeof(struct Flic_header)
		);

		/* Error ? */
		if (Read_bytes != sizeof(struct Flic_header))
		{
			/* Yes -> Report and exit */
			FLC_Error(FLCERR_FILE_ERROR);
			return FALSE;
		}
	}
	else
	{
		/* No -> Copy flic header */
		memcpy
		(
			(UNBYTE *) &(FLC_O->Flic),
			FLC_O->Input_buffer,
			sizeof(struct Flic_header)
		);
	}

	/* Is this an .FLI- or .FLC-animation ? */
	Flic_type = MISC_INTEL2UNSHORT(FLC_O->Flic.type);
	if ((Flic_type != FLI_TYPE) && (Flic_type != FLC_TYPE))
	{
		/* No -> Error */
		FLC_Error(FLCERR_NOT_FLIC);
		return FALSE;
	}

	/* Set flic dimensions */
	FLC_O->Width	= MISC_INTEL2UNSHORT(FLC_O->Flic.width);
	FLC_O->Height	= MISC_INTEL2UNSHORT(FLC_O->Flic.height);

	/* Set number of animation frames */
	FLC_O->Nr_frames = MISC_INTEL2UNSHORT(FLC_O->Flic.frames);

	/* Initialize playback */
	FASTFLC_Restart_flic();

	/* Start playback */
	FLC_Playing_back_flic = TRUE;

	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Restart_flic
 * FUNCTION  : Go back to frame 0 of the current flic.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 15:40
 * LAST      : 11.08.95 15:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FASTFLC_Restart_flic(void)
{
	/* Playing back a flic ? */
	if (FLC_Playing_back_flic)
	{
		/* Yes -> Restart */
		FLC_O->Current_frame			= 0;
		FLC_O->Next_frame_offset	= 0;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Stop_flic_playback
 * FUNCTION  : Stop playing back the current flic.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 20:26
 * LAST      : 12.09.95 14:29
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FASTFLC_Stop_flic_playback(void)
{
	/* Playing back a flic ? */
	if (FLC_Playing_back_flic)
	{
		/* Yes -> Stop it */
		FLC_Playing_back_flic = FALSE;
	}

	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Playback_flic_frame
 * FUNCTION  : Play back a frame in the current flic.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 20:27
 * LAST      : 12.09.95 10:46
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * NOTES     : - The current frame will be created in the Output_frame OPM.
 *             - It is assumed the flic contains a header and just as many
 *              consecutive frame chunks as stated in Flic.frames, plus a
 *              ring frame. These frames should start at Input_buffer
 *              + oframe1.Prefix chunks will be ignored correctly, but any
 *              weird chunks inbetween will cause an error.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FASTFLC_Playback_flic_frame(void)
{
	struct Flic_chunk_header *Chunk;
	BOOLEAN Result;
	UNLONG Seek_position;
	UNLONG Offset;
	UNLONG Read_bytes;
	UNBYTE *Ptr;

	/* Playing back a flic ? */
	if (FLC_Playing_back_flic)
	{
		/* Yes -> Clear flag */
		FLC_O->Palette_was_changed = FALSE;

		/* Get offset to current frame */
		/* First frame ? */
		if (FLC_O->Current_frame)
		{
			/* No -> Second frame ? */
			if (FLC_O->Current_frame == 1)
			{
				/* Yes -> Get offset to second frame */
				Offset = MISC_INTEL2UNLONG(FLC_O->Flic.oframe2);
			}
			else
			{
				/* No -> Get offset to next frame */
				Offset = FLC_O->Next_frame_offset;
			}
		}
		else
		{
			/* Yes -> Playing for the first time ? */
			if (FLC_O->Next_frame_offset)
			{
				/* No -> Get offset to next frame */
				Offset = FLC_O->Next_frame_offset;
			}
			else
			{
				/* Yes -> Get offset to first frame */
				Offset = MISC_INTEL2UNLONG(FLC_O->Flic.oframe1);
			}
		}

		/* Streaming ? */
		if (FLC_O->Flags & FLC_STREAM)
		{
			/* Yes -> Get current seek position */
			Seek_position = DOS_GetSeekPosition(FLC_O->Input_file_handle) -
			 FLC_O->File_base_offset;

			/* Is this the position we want ? */
			if (Seek_position != Offset)
			{
				/* No -> Seek to current frame chunk */
				Result = DOS_Seek
				(
					FLC_O->Input_file_handle,
					BBDOS_SEEK_START,
					FLC_O->File_base_offset + Offset
				);

				/* Error ? */
				if (!Result)
				{
					/* Yes -> Report, stop playback and exit */
					FLC_Error(FLCERR_FILE_ERROR);

					FASTFLC_Stop_flic_playback();

					return FALSE;
				}
			}

			/* Load chunk header into input buffer */
			Read_bytes = DOS_Read
			(
				FLC_O->Input_file_handle,
				FLC_O->Input_buffer,
				sizeof(struct Flic_chunk_header)
			);

			/* Error ? */
			if (Read_bytes != sizeof(struct Flic_chunk_header))
			{
				/* Yes -> Report, stop playback and exit */
				FLC_Error(FLCERR_FILE_ERROR);

				FASTFLC_Stop_flic_playback();

				return FALSE;
			}

			/* Is the next frame chunk too big ? */
			Chunk = (struct Flic_chunk_header *) FLC_O->Input_buffer;
			if (Chunk->size > FLC_O->Input_buffer_length)
			{
				/* Yes -> Report, stop playback and exit */
				FLC_Error(FLCERR_FRAME_TOO_BIG);

				FASTFLC_Stop_flic_playback();

				return FALSE;
			}

			/* Load rest of frame chunk */
			Read_bytes = DOS_Read
			(
				FLC_O->Input_file_handle,
				FLC_O->Input_buffer + sizeof(struct Flic_chunk_header),
				Chunk->size - sizeof(struct Flic_chunk_header)
			);

			/* Error ? */
			if (Read_bytes != (Chunk->size - sizeof(struct Flic_chunk_header)))
			{
				/* Yes -> Report, stop playback and exit */
				FLC_Error(FLCERR_FILE_ERROR);

				FASTFLC_Stop_flic_playback();

				return FALSE;
			}

			/* Set pointer to frame chunk in input buffer */
			Ptr = FLC_O->Input_buffer;
		}
		else
		{
			/* No -> Calculate pointer to frame chunk in input buffer */
			Ptr = FLC_O->Input_buffer + Offset;
		}

		/* Store offset to next frame */
		Chunk = (struct Flic_chunk_header *) Ptr;
		FLC_O->Next_frame_offset = Offset + Chunk->size;

		/* Decode current frame chunk */
		FASTFLC_Decode_frame_chunk(Ptr);

		/* Next frame */
		FLC_O->Current_frame++;

		/* Time to loop ? */
		if (FLC_O->Current_frame >= FLC_O->Nr_frames)
		{
			/* Yes -> Back to frame 0 */
			FLC_O->Current_frame = 0;
		}
	}

	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Decode_frame_chunk
 * FUNCTION  : Decode a frame chunk from the current flic.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 18:40
 * LAST      : 11.08.95 18:40
 * INPUTS    : UNBYTE *Frame_chunk_ptr - Pointer to frame chunk.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * NOTES     : - The current frame will be created in the Output_frame OPM.
 *             - It is assumed the flic contains a header and just as many
 *              consecutive frame chunks as stated in Flic.frames, plus a
 *              ring frame. These frames should start at Input_buffer
 *              + oframe1.Prefix chunks will be ignored correctly, but any
 *              weird chunks inbetween will cause an error.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FASTFLC_Decode_frame_chunk(UNBYTE *Frame_chunk_ptr)
{
	struct Flic_chunk_header *Chunk;
	struct Flic_subchunk_header *Subchunk;
	UNSHORT Nr_subchunks;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Get current frame chunk */
	Chunk = (struct Flic_chunk_header *) Frame_chunk_ptr;

	/* Is this a frame chunk ? */
	if (MISC_INTEL2UNSHORT(Chunk->type) != FLC_FLIC_FRAME)
	{
		/* No -> Error */
		FLC_Error(FLCERR_ILLEGAL_FRAME_CHUNK);
		return FALSE;
	}

	/* Get number of sub-chunks in this frame chunk */
	Nr_subchunks = MISC_INTEL2UNSHORT(Chunk->chunks);

	/* Handle all sub-chunks */
	Ptr = (UNBYTE *) (Chunk + 1);
	for (i=0;i<Nr_subchunks;i++)
	{
		/* Get sub-chunk */
		Subchunk = (struct Flic_subchunk_header *) Ptr;
		Ptr += sizeof(struct Flic_subchunk_header);

		/* Handle chunk types */
		switch(Subchunk->type)
		{
			/* 256-level colour palette sub-chunk */
			case FLC_COLOR_256:
			{
				FASTFLC_Decode_COLOR_256_chunk(Ptr);
				break;
			}
			/* Word-oriented delta-frame sub-chunk */
			case FLC_DELTA_FLC:
			{
				FASTFLC_Decode_DELTA_FLC_chunk(Ptr);
				break;
			}
			/* 64-level colour palette sub-chunk */
			case FLC_COLOR_64:
			{
				FASTFLC_Decode_COLOR_64_chunk(Ptr);
				break;
			}
			/* Byte-oriented delta-frame sub-chunk */
			case FLC_DELTA_FLI:
			{
				FASTFLC_Decode_DELTA_FLI_chunk(Ptr);
				break;
			}
			/* Black frame sub-chunk */
			case FLC_BLACK:
			{
				FASTFLC_Decode_BLACK_chunk(Ptr);
				break;
			}
			/* Byte-run sub-chunk */
			case FLC_BYTE_RUN:
			{
				FASTFLC_Decode_BYTE_RUN_chunk(Ptr);
				break;
			}
			/* Literal sub-chunk */
			case FLC_LITERAL:
			{
				FASTFLC_Decode_LITERAL_chunk(Ptr);
				break;
			}
			/* Postage stamp sub-chunk (is ignored) */
			case FLC_PSTAMP:
			{
				break;
			}
			/* No known sub-chunk */
			default:
			{
				/* Error */
				FLC_Error(FLCERR_ILLEGAL_SUBCHUNK);
				break;
			}
		}

		/* Go to next sub-chunk */
		Ptr = ((UNBYTE *) Subchunk) + MISC_INTEL2UNLONG(Subchunk->size);
	}

	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Decode_COLOR_256_chunk
 * FUNCTION  : Decode a COLOR_256 FLC-chunk.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:09
 * LAST      : 04.09.95 10:41
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FASTFLC_Decode_COLOR_256_chunk(UNBYTE *Ptr)
{
	struct BBPALETTE *Palette;
	UNSHORT Packets;
	UNSHORT Index;
	UNSHORT Length;
	UNSHORT i;

	/* Get pointer to palette */
	Palette = FLC_O->Palette;

	/* Exit if no palette was given */
	if (!Palette)
		return;

	/* Read number of packets */
	Packets = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
	Ptr += 2;

	/* Handle all packets */
	Index = 0;
	while (Packets--)
	{
		/* Skip colours */
		Index += (UNSHORT) *Ptr++;

		/* Get packet length */
		Length = (UNSHORT) *Ptr++;

		/* If 0, 256 is meant */
		if (!Length)
			Length = 256;

		/* Copy colours */
		for (i=0;i<Length;i++)
		{
			/* May this colour be modified ? */
			if ((Index >= FLC_O->First_colour) &&
			 (Index < FLC_O->First_colour + FLC_O->Max_colours))
			{
				/* Yes -> Copy colour */
				Palette->color[Index].red		= *Ptr++;
				Palette->color[Index].green	= *Ptr++;
				Palette->color[Index].blue		= *Ptr++;
				Palette->color[Index].alpha	= 0;

				/* Indicate the palette has been changed */
				FLC_O->Palette_was_changed = TRUE;
			}

			/* Next colour */
			Index++;
		}

		/* Update palette part */
/*		DSA_SetPal(FLC_O->Screen, Palette, Index - Length, Length,
		 Index - Length); */
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Decode_DELTA_FLC_chunk
 * FUNCTION  : Decode a DELTA_FLC FLC-chunk.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 12.09.95 16:32
 * LAST      : 12.09.95 16:32
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FASTFLC_Decode_DELTA_FLC_chunk(UNBYTE *Ptr)
{
	UNSHORT Height;
	UNSHORT Special_word;
	UNSHORT Nr_packets;
	UNSHORT Colours;
	UNSHORT i;
	SIBYTE Counter;
	UNBYTE *Screen_ptr;
	UNBYTE *Work_ptr;

	/* Get number of lines */
	Height = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
	Ptr += 2;

	/* Calculate initial screen pointer */
	Screen_ptr = (UNBYTE *) 0xA0000 +
	 (FLC_O->Playback_Y * FLC_O->Width) + FLC_O->Playback_X;

	/* Do all lines */
	for (i=0;i<Height;i++)
	{
		/* Read special words */
		Nr_packets = 0xFFFF;
		do
		{
			/* Read special word */
			Special_word = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
			Ptr += 2;

			/* Handle each special word type */
			switch ((Special_word & 0xC000) >> 14)
			{
				/* Packets counter */
				case 0:
				{
					Nr_packets = Special_word;
					break;
				}
				/* Last pixel */
				case 2:
				{
					/* Set last pixel */
					*(Screen_ptr + FLC_O->Width - 1) = Special_word & 0x00FF;

					/* The next special word is always the packet counter */
					Nr_packets = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
					Ptr += 2;

					break;
				}
				/* Line skip */
				case 3:
				{
					/* Skip lines (Special_word is negative) */
					Screen_ptr += FLC_O->Width *
					 (SILONG)(0 - (SISHORT) Special_word);

					break;
				}
			}
		/* Until the number of packets is known */
		} while (Nr_packets == 0xFFFF);

		/* Start a new line */
		Work_ptr = Screen_ptr;

		/* Any packets in this line ? */
		if (Nr_packets)
		{
			/* Skip columns */
		   Work_ptr += *Ptr++;

			/* Do all packets */
			do
			{
				/* Read counter */
				Counter = (SIBYTE) *Ptr++;

				/* Unpacked or run packet ? */
				if (Counter >= 0)
				{
					/* Unpacked -> Write unpacked packet */
					while (Counter--)
					{
						/* Get two colours */
						*((UNSHORT *) Work_ptr) = *((UNSHORT *) Ptr);

						Work_ptr += 2;
						Ptr += 2;
					}
				}
				else
				{
					/* Run -> Get packet length */
					Counter = 0 - Counter;

					/* Get two colours */
					Colours = *((UNSHORT *) Ptr);
					Ptr += 2;

					/* Write run packet */
					while (Counter--)
					{
						*((UNSHORT *) Work_ptr) = Colours;
						Work_ptr += 2;
					}
				}

				/* Count down */
				Nr_packets--;

				/* Last packet ? */
				if (Nr_packets)
				{
					/* No -> Skip more columns */
				   Work_ptr += *Ptr++;
				}

			} while (Nr_packets);
		}

		/* Next line */
		Screen_ptr += FLC_O->Width;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Decode_COLOR_64_chunk
 * FUNCTION  : Decode a COLOR_64 FLC-chunk.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:00
 * LAST      : 04.09.95 10:41
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FASTFLC_Decode_COLOR_64_chunk(UNBYTE *Ptr)
{
	struct BBPALETTE *Palette;
	UNSHORT Packets;
	UNSHORT Index;
	UNSHORT Length;
	UNSHORT i;

	/* Get pointer to palette */
	Palette = FLC_O->Palette;

	/* Exit if no palette was given */
	if (!Palette)
		return;

	/* Read number of packets */
	Packets = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
	Ptr += 2;

	/* Handle all packets */
	Index = 0;
	while (Packets--)
	{
		/* Skip colours */
		Index += (UNSHORT) *Ptr++;

		/* Get packet length */
		Length = (UNSHORT) *Ptr++;

		/* If 0, 256 is meant */
		if (!Length)
			Length = 256;

		/* Copy colours */
		for (i=0;i<Length;i++)
		{
			/* May this colour be modified ? */
			if ((Index >= FLC_O->First_colour) &&
			 (Index < FLC_O->First_colour + FLC_O->Max_colours))
			{
				/* Yes -> Copy colour */
				Palette->color[Index].red		= *Ptr++ << 2;
				Palette->color[Index].green	= *Ptr++ << 2;
				Palette->color[Index].blue		= *Ptr++ << 2;
				Palette->color[Index].alpha	= 0;

				/* Indicate the palette has been changed */
				FLC_O->Palette_was_changed = TRUE;
			}

			/* Next colour */
			Index++;
		}

		/* Update palette part */
/*		DSA_SetPal(FLC_O->Screen, Palette, Index - Length, Length,
		 Index - Length); */
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Decode_DELTA_FLI_chunk
 * FUNCTION  : Decode a DELTA_FLI FLC-chunk.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:00
 * LAST      : 31.07.95 11:25
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FASTFLC_Decode_DELTA_FLI_chunk(UNBYTE *Ptr)
{
	UNSHORT X, Y;
	UNSHORT Height;
	UNSHORT Nr_packets;
	UNSHORT Colour;
	UNSHORT i;
	SIBYTE Counter;

	/* Skip lines */
	Y = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
	Ptr += 2;

	/* Get number of lines */
	Height = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
	Ptr += 2;

	/* Do all lines */
	for (i=0;i<Height;i++)
	{
		/* Start a new line */
		X = 0;

		/* Read number of packets */
		Nr_packets = *Ptr++;

		/* Any packets ? */
		if (Nr_packets)
		{
			/* Yes -> Skip columns */
			X += (UNSHORT)(*Ptr++);

			/* Do all packets */
			do
			{
				/* Read counter */
				Counter = (SIBYTE) *Ptr++;

				/* Unpacked or run packet ? */
				if (Counter >= 0)
				{
					/* Unpacked -> Write unpacked packet */
					while (Counter--)
					{
						/* Get current pixel */
						Colour = *Ptr++;

						/* No -> Set pixel */
						OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
							FLC_O->Playback_Y + Y, Colour);

						/* Next X */
						X++;
					}
				}
				else
				{
					/* Run -> Get packet length */
					Counter = -Counter;

					/* Get colour */
					Colour = *Ptr++;

					/* No -> Write run packet */
					while (Counter--)
					{
						/* Set pixel */
						OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
							FLC_O->Playback_Y + Y, Colour);

						/* Next X */
						X++;
					}
				}

				/* Count down */
				Nr_packets--;

				/* Last packet ? */
				if (Nr_packets)
				{
					/* No -> Skip more columns */
					X += (UNSHORT)(*Ptr++);
				}

			} while (Nr_packets);
		}

		/* Next line */
		Y++;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Decode_BLACK_chunk
 * FUNCTION  : Decode a BLACK FLC-chunk.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 21:00
 * LAST      : 31.07.95 11:21
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FASTFLC_Decode_BLACK_chunk(UNBYTE *Ptr)
{
	/* No -> Display black frame */
	OPM_FillBox
	(
		FLC_O->Output_frame,
		FLC_O->Playback_X,
		FLC_O->Playback_Y,
		FLC_O->Width,
		FLC_O->Height,
		FLC_O->Black_colour_index
	);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Decode_BYTE_RUN_chunk
 * FUNCTION  : Decode a BYTE_RUN FLC-chunk.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:00
 * LAST      : 31.07.95 11:20
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FASTFLC_Decode_BYTE_RUN_chunk(UNBYTE *Ptr)
{
	UNSHORT X, Y;
	UNSHORT Colour;
	SIBYTE Counter;

	/* Do all lines */
	for (Y=0;Y<FLC_O->Height;Y++)
	{
		/* Start a new line */
		X = 0;

		/* Skip number of packets */
		Ptr++;

		do
		{
			/* Read counter */
			Counter = (SIBYTE) *Ptr++;

			/* Unpacked or run packet ? */
			if (Counter < 0 )
			{
				/* Unpacked -> Get packet length */
				Counter =- Counter;

				/* Write unpacked packet */
				while (Counter--)
				{
					/* Get current pixel */
					Colour = *Ptr++;

					/* No -> Set pixel */
					OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
						FLC_O->Playback_Y + Y, Colour);

					/* Next X */
					X++;
				}
			}
			else
			{
				/* Run -> Get colour */
				Colour = *Ptr++;

				/* No -> Write run packet */
				while (Counter--)
				{
					/* Set pixel */
					OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
						FLC_O->Playback_Y + Y, Colour);

					/* Next X */
					X++;
				}
			}
		}
		/* Until the line is full */
		while (X < FLC_O->Width);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FASTFLC_Decode_LITERAL_chunk
 * FUNCTION  : Decode a LITERAL FLC-chunk.
 * FILE      : FASTFLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 21:00
 * LAST      : 31.07.95 11:32
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FASTFLC_Decode_LITERAL_chunk(UNBYTE *Ptr)
{
	struct OPM Temp_OPM;

	/* No -> Create temporary OPM */
	OPM_New
	(
		FLC_O->Width,
		FLC_O->Height,
		1,
		&Temp_OPM,
		Ptr
	);

	/* Copy literal image */
	OPM_CopyOPMOPM
	(
		&Temp_OPM,
		FLC_O->Output_frame,
		0,
		0,
		FLC_O->Width,
		FLC_O->Height,
		FLC_O->Playback_X,
		FLC_O->Playback_Y
	);

	/* Destroy temporary OPM */
	OPM_Del(&Temp_OPM);
}

