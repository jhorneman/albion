/************
 * NAME     : FLC.C
 * AUTHOR   : Jurie Horneman & Thomas H„user, BlueByte
 * START    : 28-11-1994
 * PROJECT  : FLC animation grabber
 * NOTES    :
 * VERSION  : 1.0
 * SEE ALSO : FLC.H
 ************/

/* includes */

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

/* global variables */

static struct Flic_record *FLC_I;
static struct Flic_playback *FLC_O;

static BOOLEAN FLC_Recording_flic = FALSE;
static BOOLEAN FLC_Playing_back_flic = FALSE;

static UNSHORT FLC_Nr_delta_packets;

static UNCHAR FLC_Library_name[] = "FLC";

static UNCHAR *FLC_Error_strings[] = {
	"Illegal error code.",
	"File error.",
	"Already playing a flic.",
	"This is not a flic.",
	"Illegal frame chunk in flic.",
	"Illegal sub-chunk in flic."
};

/* structure definitions */

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

/* prototypes */

void FLC_Error(UNSHORT Error_code);
void FLC_Print_error(UNCHAR *buffer, UNBYTE *data);

UNBYTE *FLC_Create_COLOR_256_chunk(UNBYTE *Buffer);
UNBYTE *FLC_Create_BYTE_RUN_chunk(UNBYTE *Buffer, UNBYTE *Frame);

UNBYTE *FLC_Write_run_packet(UNBYTE *Ptr, UNBYTE Pixel, UNSHORT Length);
UNBYTE *FLC_Write_unpacked_packet(UNBYTE *Ptr, UNBYTE *Run_ptr, UNSHORT Length);

UNBYTE *FLC_Create_DELTA_FLC_chunk(UNBYTE *Buffer, UNBYTE *Frame, UNBYTE *Previous);

UNSHORT FLC_Scan_line_for_delta_packets(UNSHORT *Old, UNSHORT *New,
 struct FLC_delta Output[]);
UNBYTE *FLC_Write_delta_packet(UNBYTE *Buffer, struct FLC_delta *Packet);
UNBYTE *FLC_Write_run_delta_packet(UNBYTE *Ptr, UNSHORT Skip_count, UNSHORT Word,
 UNSHORT Length);
UNBYTE *FLC_Write_unpacked_delta_packet(UNBYTE *Ptr, UNSHORT Skip_count,
 UNSHORT *Run_ptr, UNSHORT Length);

void FLC_Decode_COLOR_256_chunk(UNBYTE *Ptr);
void FLC_Decode_DELTA_FLC_chunk(UNBYTE *Ptr);
void FLC_Decode_COLOR_64_chunk(UNBYTE *Ptr);
void FLC_Decode_DELTA_FLI_chunk(UNBYTE *Ptr);
void FLC_Decode_BLACK_chunk(UNBYTE *Ptr);
void FLC_Decode_BYTE_RUN_chunk(UNBYTE *Ptr);
void FLC_Decode_LITERAL_chunk(UNBYTE *Ptr);

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Error
 * FUNCTION  : Report an FLC error.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 11:48
 * LAST      : 30.11.94 11:48
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Error(UNSHORT Error_code)
{
	struct FLC_error Error;

	/* Initialize FLC error structure */
	Error.Code = Error_code;

	/* Push error on the error stack */
	ERROR_PushError(FLC_Print_error, FLC_Library_name, sizeof(struct FLC_error),
	 (UNBYTE *) &Error);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLD_Print_error
 * FUNCTION  : Print an FLC error.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 11:51
 * LAST      : 30.11.94 11:51
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by FLC_Error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : FLC.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Print_error(UNCHAR *buffer, UNBYTE *data)
{
	struct FLC_error *Error;
	UNSHORT i;

	Error = (struct FLC_error *) data;

	/* Get error code */
	i = Error->Code;

	/* Catch illegal errors */
	if (i > FLCERR_MAX)
		i = 0;

 	/* Print error */
	sprintf((char *)buffer,"%s", FLC_Error_strings[i]);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Start_flic_recording
 * FUNCTION  : Start recording a flic.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 11:37
 * LAST      : 30.11.94 11:37
 * INPUTS    : struct Flic_record *Flic_IO - Pointer to initialized flic
 *              recording I/O structure.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * NOTES     : - All elements of the I/O structure should be set to meaningful
 *              values except the flic header and the flic dimensions.
 *             - Only one flic can be recorded at the same time.
 *             - The flic header will be initialized and the first frame will
 *              be encoded.
 *             - Any flic file with the same name will be deleted.
 *             - If a flic is already being recorded, it will be deleted
 *              since it is incomplete.
 *             - The first frame should be in the Input_frame OPM.
 *             - The first frame will be copied to the Previous_frame OPM.
 *             - The width and height of the resulting flic are determined by
 *              the first frame's OPM.
 *             - The speed of the resulting flic is set to 50 milliseconds
 *              per frame.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FLC_Start_flic_recording(struct Flic_record *Flic_IO)
{
	struct Flic_chunk_header *Chunk;
	UNLONG Length;
	SISHORT File;
	UNBYTE *Ptr;

	/* Already recording a flic ? */
	if (FLC_Recording_flic)
	{
		/* Yes -> Delete current flic (which is incomplete) */
		DOS_Delete(FLC_I->Flic_filename);

		/* Stop recording */
		FLC_Recording_flic = FALSE;
	}

	/* Store I/O structure */
	FLC_I = Flic_IO;

	/* Clear flic header */
	BASEMEM_FillMemByte((UNBYTE *) &(FLC_I->Flic), sizeof(struct Flic_header), 0);

	/* Set flic dimensions */
	FLC_I->Width = FLC_I->First_frame->width;
	FLC_I->Height = FLC_I->First_frame->height;

	/* Initialize flic header */
	FLC_I->Flic.size = MISC_UNLONG2INTEL(sizeof(struct Flic_header));
	FLC_I->Flic.type = MISC_UNSHORT2INTEL(FLC_TYPE);
	FLC_I->Flic.width = MISC_UNSHORT2INTEL(FLC_I->Width);
	FLC_I->Flic.height = MISC_UNSHORT2INTEL(FLC_I->Height);
	FLC_I->Flic.depth = MISC_UNSHORT2INTEL(8);
	FLC_I->Flic.speed = MISC_UNSHORT2INTEL(50);
	FLC_I->Flic.aspectx = MISC_UNSHORT2INTEL(1);
	FLC_I->Flic.aspecty = MISC_UNSHORT2INTEL(1);
	FLC_I->Flic.oframe1 = MISC_UNLONG2INTEL(sizeof(struct Flic_header));

	/* Start recording */
	FLC_Recording_flic = TRUE;

	/* Create first frame */
	Chunk = (struct Flic_chunk_header *) FLC_I->Output_buffer;
	Ptr = (UNBYTE *) (Chunk + 1);

	/* Clear chunk header */
	BASEMEM_FillMemByte((UNBYTE *) Chunk, sizeof(struct Flic_chunk_header), 0);

	/* Initialize chunk header */
	Chunk->type = MISC_UNSHORT2INTEL(FLC_FLIC_FRAME);
	Chunk->chunks = MISC_UNSHORT2INTEL(2);

	/* Create colour chunk */
	Ptr = FLC_Create_COLOR_256_chunk(Ptr);

	/* Create first frame byte-run chunk */
	Ptr = FLC_Create_BYTE_RUN_chunk(Ptr, FLC_I->Input_frame->data);

	/* Calculate length of chunk */
	Length = Ptr - FLC_I->Output_buffer;

	/* Store length of chunk */
	Chunk->size = MISC_UNLONG2INTEL(Length);

	/* Update flic header */
	FLC_I->Flic.size = MISC_UNLONG2INTEL(MISC_INTEL2UNLONG(FLC_I->Flic.size)
	 + Length);
	FLC_I->Flic.frames = MISC_UNSHORT2INTEL(1);
	FLC_I->Flic.oframe2 = MISC_UNLONG2INTEL(FLC_I->Flic.size);

	/* Delete any existing flic file with this name */
	DOS_Delete(FLC_I->Flic_filename);

	/* Open flic file */
	File = DOS_Open(FLC_I->Flic_filename, BBDOSFILESTAT_WRITE);
	if (File < 0)
	{
		FLC_Error(FLCERR_FILE_ERROR);
		return(FALSE);
	}

	/* Write header */
	if (DOS_Write(File, (UNBYTE *) &(FLC_I->Flic), sizeof(struct Flic_header))
	 != sizeof(struct Flic_header))
	{
		DOS_Close(File);
 		FLC_Error(FLCERR_FILE_ERROR);
		return(FALSE);
	}

	/* Write first frame */
	if (DOS_Write(File, FLC_I->Output_buffer, Length) != Length)
	{
		DOS_Close(File);
		FLC_Error(FLCERR_FILE_ERROR);
		return(FALSE);
	}

	/* Close flic file */
	DOS_Close(File);

	/* Copy input frame to first and previous frame OPMs */
	OPM_CopyOPMOPM(FLC_I->Input_frame, FLC_I->First_frame, 0, 0,
	 FLC_I->Width, FLC_I->Height, 0, 0);
	OPM_CopyOPMOPM(FLC_I->Input_frame, FLC_I->Previous_frame, 0, 0,
	 FLC_I->Width, FLC_I->Height, 0, 0);

	return(TRUE);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Stop_flic_recording
 * FUNCTION  : Stop recording the current flic.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 11:42
 * LAST      : 30.11.94 11:42
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * NOTES     : - The flic header will be updated and the ring frame will be
 *              created.
 *             - The input frame OPM will not be used this last time.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FLC_Stop_flic_recording(void)
{
	struct Flic_chunk_header *Chunk;
	UNLONG Length;
	SISHORT File;
	UNBYTE *Ptr;

	/* Recording a flic ? */
	if (FLC_Recording_flic)
	{
		/* Create ring frame */
		Chunk = (struct Flic_chunk_header *) FLC_I->Output_buffer;
		Ptr = (UNBYTE *) (Chunk + 1);

		/* Clear chunk header */
		BASEMEM_FillMemByte((UNBYTE *) Chunk, sizeof(struct Flic_chunk_header), 0);

		/* Initialize chunk header */
		Chunk->type = MISC_UNSHORT2INTEL(FLC_FLIC_FRAME);
		Chunk->chunks = MISC_UNSHORT2INTEL(2);

		/* Create colour chunk */
		Ptr = FLC_Create_COLOR_256_chunk(Ptr);

		/* Create ring frame delta-packed chunk */
		Ptr = FLC_Create_DELTA_FLC_chunk(Ptr, FLC_I->First_frame->data,
		 FLC_I->Previous_frame->data);

		/* Calculate length of chunk */
		Length = Ptr - FLC_I->Output_buffer;

		/* Store length of chunk */
		Chunk->size = MISC_UNLONG2INTEL(Length);

		/* Update flic header */
		FLC_I->Flic.size = MISC_UNLONG2INTEL(MISC_INTEL2UNLONG(FLC_I->Flic.size)
		 + Length);
		FLC_I->Flic.flags = MISC_UNSHORT2INTEL(FLI_FINISHED | FLI_LOOPED);

		/* Open flic file */
		File = DOS_Open(FLC_I->Flic_filename, BBDOSFILESTAT_READWRITE);
		if (File < 0)
		{
			FLC_Error(FLCERR_FILE_ERROR);
			return(FALSE);
		}

		/* Write header */
		if (DOS_Write(File, (UNBYTE *) &(FLC_I->Flic),
		 sizeof(struct Flic_header)) != sizeof(struct Flic_header))
		{
			DOS_Close(File);
	 		FLC_Error(FLCERR_FILE_ERROR);
			return(FALSE);
		}

		/* Append ring frame to the end */
		if (!DOS_Seek(File, BBDOS_SEEK_EOF, 0L))
		{
			DOS_Close(File);
			FLC_Error(FLCERR_FILE_ERROR);
			return(FALSE);
		}
		if (DOS_Write(File, FLC_I->Output_buffer, Length) != Length)
		{
			DOS_Close(File);
			FLC_Error(FLCERR_FILE_ERROR);
			return(FALSE);
		}

		/* Close flic file */
		DOS_Close(File);

		/* Stop recording */
		FLC_Recording_flic = FALSE;
	}

	return(TRUE);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Record_flic_frame
 * FUNCTION  : Record a frame in the current flic.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 13:21
 * LAST      : 30.11.94 13:21
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * NOTES     : - The current frame should be in the Input_frame OPM.
 *             - The current frame will be copied to the Previous_frame OPM.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FLC_Record_flic_frame(void)
{
	struct Flic_chunk_header *Chunk;
	UNLONG Length;
	SISHORT File;
	UNBYTE *Ptr;

	/* Recording a flic ? */
	if (FLC_Recording_flic)
	{
		/* Create next frame */
		Chunk = (struct Flic_chunk_header *) FLC_I->Output_buffer;
		Ptr = (UNBYTE *) (Chunk + 1);

		/* Clear chunk header */
		BASEMEM_FillMemByte((UNBYTE *) Chunk, sizeof(struct Flic_chunk_header), 0);

		/* Initialize chunk header */
		Chunk->type = MISC_UNSHORT2INTEL(FLC_FLIC_FRAME);
		Chunk->chunks = MISC_UNSHORT2INTEL(2);

		/* Create colour chunk */
		Ptr = FLC_Create_COLOR_256_chunk(Ptr);

		/* Create next frame delta-packed chunk */
		Ptr = FLC_Create_DELTA_FLC_chunk(Ptr, FLC_I->Input_frame->data,
		 FLC_I->Previous_frame->data);

		/* Calculate length of chunk */
		Length = Ptr - FLC_I->Output_buffer;

		/* Store length of chunk */
		Chunk->size = MISC_UNLONG2INTEL(Length);

		/* Update flic header */
		FLC_I->Flic.size = MISC_UNLONG2INTEL(MISC_INTEL2UNLONG(FLC_I->Flic.size)
		 + Length);
		FLC_I->Flic.frames = MISC_UNSHORT2INTEL(MISC_INTEL2UNSHORT(FLC_I->Flic.frames)
		 + 1);

		/* Open flic file */
		File = DOS_Open(FLC_I->Flic_filename, BBDOSFILESTAT_APPEND);
		if (File < 0)
		{
			FLC_Error(FLCERR_FILE_ERROR);
			return(FALSE);
		}

		/* Append current frame to the end */
		if (DOS_Write(File, FLC_I->Output_buffer, Length) != Length)
		{
			DOS_Close(File);
			FLC_Error(FLCERR_FILE_ERROR);
			return(FALSE);
		}

		/* Close flic file */
		DOS_Close(File);

		/* Copy input frame to previous frame OPMs */
		OPM_CopyOPMOPM(FLC_I->Input_frame, FLC_I->Previous_frame, 0, 0,
		 FLC_I->Width, FLC_I->Height, 0, 0);
	}

	return(TRUE);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Create_COLOR_256_chunk
 * FUNCTION  : Create a COLOR_256 FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 12:33
 * LAST      : 29.11.94 12:33
 * INPUTS    : UNBYTE *Buffer - Pointer to output buffer.
 *             struct BBPALETTE *Palette - Pointer to palette.
 * RESULT    : UNBYTE * : Updated pointer to output buffer.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FLC_Create_COLOR_256_chunk(UNBYTE *Buffer)
{
	struct BBPALETTE *Palette;
	struct Flic_subchunk_header *Chunk;
	UNSHORT i;
	UNBYTE *Ptr;

	Chunk = (struct Flic_subchunk_header *) Buffer;
	Ptr = (UNBYTE *) (Chunk + 1);

	/* Get pointer to palette */
	Palette = FLC_I->Palette;

	/* Clear chunk header */
	BASEMEM_FillMemByte((UNBYTE *) Chunk, sizeof(struct Flic_subchunk_header), 0);

	/* Initialize chunk header */
	Chunk->type = MISC_UNSHORT2INTEL(FLC_COLOR_256);

	/* Write number of packets */
	*((UNSHORT *) Ptr) = MISC_UNSHORT2INTEL(1);
	Ptr += 2;

	/* Write skip count */
	*Ptr++ = 0;

	/* Write number of colours in packet */
	*Ptr++ = 0;

	/* Write colours */
	for (i=0;i<256;i++)
	{
		*Ptr++ = Palette->color[i].red;
		*Ptr++ = Palette->color[i].green;
		*Ptr++ = Palette->color[i].blue;
	}

	/* Write size of chunk in chunk header */
	Chunk->size = MISC_UNLONG2INTEL((UNLONG)(Ptr - Buffer));

	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Create_BYTE_RUN_chunk
 * FUNCTION  : Create a BYTE_RUN run-length encoded FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 12:32
 * LAST      : 29.11.94 12:32
 * INPUTS    : UNBYTE *Buffer - Pointer to output buffer.
 *             UNBYTE *Frame - Pointer to frame that must be encoded.
 * RESULT    : UNBYTE * : Updated pointer to output buffer.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FLC_Create_BYTE_RUN_chunk(UNBYTE *Buffer, UNBYTE *Frame)
{
	struct Flic_subchunk_header *Chunk;
	UNSHORT Pixel1, Pixel2;
	UNSHORT Run_length, Unpacked_length;
	UNSHORT i;
	UNBYTE *Ptr, *Run_base, *Run_ptr, *Run_end;

	Chunk = (struct Flic_subchunk_header *) Buffer;
	Ptr = (UNBYTE *) (Chunk + 1);

	/* Clear chunk header */
	BASEMEM_FillMemByte((UNBYTE *) Chunk, sizeof(struct Flic_subchunk_header), 0);

	/* Initialize chunk header */
	Chunk->type = MISC_UNSHORT2INTEL(FLC_BYTE_RUN);

	/* Do each line */
	for (i=0;i<FLC_I->Height;i++)
	{
		/* Write number of packets (is ignored) */
		*Ptr++ = 0;

		/* Start run */
		Run_base = Frame;
		Run_end = Run_base + FLC_I->Width;
		Run_ptr = Run_base;

		Run_length = 0;
		Unpacked_length = 1;

		/* Read first pixel */
		Pixel1 = *Run_ptr++;

		for (;;)
		{
			/* Read next pixel */
			Pixel2 = *Run_ptr++;

			/* Is the current pixel the same as the first ? */
			if (Pixel1 == Pixel2)
			{
				/* Yes -> First time ? */
				if (!Run_length)
				{
					/* Yes -> Count first pixel too */
					Run_length = 2;

					/* Take this pixel from current unpacked packet (if any) */
					if (Unpacked_length)
						Unpacked_length--;
				}
				else
				{
					/* No -> Increase run length */
					Run_length++;
				}

				/* Is the current run long enough ? */
				if (Run_length == 3)
				{
					/* Yes -> Are there unpacked pixels waiting ? */
					if (Unpacked_length)
					{
						/* Yes -> Write unpacked packet */
						Ptr = FLC_Write_unpacked_packet(Ptr, Run_base,
						 Unpacked_length);
						Unpacked_length = 0;
					}

					Run_base = Run_ptr - 3;
				}

				/* Maximum run size reached ? */
				if (Run_length > 128)
				{
					/* Yes -> Write run packet */
					Ptr = FLC_Write_run_packet(Ptr, Pixel1, 127);

					Run_base += 127;
					Run_length -= 127;
				}
			}
			else
			{
				/* No -> Is the current run long enough ? */
				if (Run_length > 2)
				{
					/* Yes -> Write run packet */
					Ptr = FLC_Write_run_packet(Ptr, Pixel1, Run_length);

					Run_base = Run_ptr - 1;
					Run_length = 0;
				}

				/* Start unpacked */
				Unpacked_length += Run_length + 1;
				Run_length = 0;
				Pixel1 = Pixel2;
			}

			/* End of line ? */
			if (Run_ptr == Run_end)
				break;
		}

		/* Write last packet */
		/* Last run long enough ? */
		if (Run_length > 2)
		{
			/* Yes -> Write run packet */
			Ptr = FLC_Write_run_packet(Ptr, Pixel1, Run_length);
		}
		else
		{
			/* No -> Write unpacked packet */
			Unpacked_length += Run_length;
			Ptr = FLC_Write_unpacked_packet(Ptr, Run_base, Unpacked_length);
		}

		/* Next line */
		Frame += FLC_I->Width;
	}

	/* Write size of chunk in chunk header */
	Chunk->size = MISC_UNLONG2INTEL((UNLONG)(Ptr - Buffer));

	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Write_run_packet
 * FUNCTION  : Write a byte-run RLE-packet.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 12:30
 * LAST      : 29.11.94 12:30
 * INPUTS    : UNBYTE *Ptr - Pointer to output buffer.
 *             UNBYTE Pixel - Pixel colour.
 *             UNSHORT Length - Length of byte-run.
 * RESULT    : UNBYTE * : Updated pointer to output buffer.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FLC_Write_run_packet(UNBYTE *Ptr, UNBYTE Pixel, UNSHORT Length)
{
	/* While the packet is too large for a single run */
	while (Length > 127)
	{
		/* Write maximum run packet */
		*Ptr++ = 127;
		*Ptr++ = Pixel;

		/* Decrease length */
		Length -= 127;
	}

	/* Is anything left ? */
	if (Length)
	{
		/* Yes -> Write run packet */
		*Ptr++ = Length;
		*Ptr++ = Pixel;
	}

	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Write_unpacked_packet
 * FUNCTION  : Write an unpacked RLE-packet.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 12:29
 * LAST      : 29.11.94 12:29
 * INPUTS    : UNBYTE *Ptr - Pointer to output buffer.
 *             UNBYTE *Run_ptr - Pointer to start of unpacked data.
 *             UNSHORT Length - Length of unpacked data.
 * RESULT    : UNBYTE * : Updated pointer to output buffer.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FLC_Write_unpacked_packet(UNBYTE *Ptr, UNBYTE *Run_ptr, UNSHORT Length)
{
	UNSHORT i;

	/* While the packet is too large for a single run */
	while (Length > 127)
	{
		/* Write maximum unpacked packet */
		*Ptr++ = 0x81;

		for (i=0;i<127;i++)
		{
			*Ptr++ = *Run_ptr++;
		}

		/* Decrease length */
		Length -= 127;
	}

	/* Is anything left ? */
	if (Length)
	{
		/* Yes -> Write unpacked packet */
		*Ptr++ = 0 - Length;

		for (i=0;i<Length;i++)
		{
			*Ptr++ = *Run_ptr++;
		}
	}

	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Create_DELTA_FLC_chunk
 * FUNCTION  : Create a DELTA_FLC word-oriented delta-packed FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 12:35
 * LAST      : 29.11.94 12:35
 * INPUTS    : UNBYTE *Buffer - Pointer to output buffer.
 *             UNBYTE *Frame - Pointer to frame that must be encoded.
 *             UNBYTE *Previous - Pointer to previous frame.
 * RESULT    : UNBYTE * : Updated pointer to output buffer.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FLC_Create_DELTA_FLC_chunk(UNBYTE *Buffer, UNBYTE *Frame, UNBYTE *Previous)
{
	struct Flic_subchunk_header *Chunk;
	struct FLC_delta *Delta_packets;
	UNSHORT Line_count = 0, Nr_packets, Line_skip = 0;
	UNSHORT *Line_count_ptr, *Packet_count_ptr;
	UNSHORT i, j, Width;
	UNBYTE *Ptr;

	Chunk = (struct Flic_subchunk_header *) Buffer;
	Ptr = (UNBYTE *) (Chunk + 1);

	/* Get flic width */
	Width = FLC_I->Width;

	/* Allocate space for delta packets */
	Delta_packets = (struct FLC_delta *) BASEMEM_Alloc(Width *
	 sizeof(struct FLC_delta), BASEMEM_Status_Normal);

	/* Clear chunk header */
	BASEMEM_FillMemByte((UNBYTE *) Chunk, sizeof(struct Flic_subchunk_header), 0);

	/* Initialize chunk header */
	Chunk->type = MISC_UNSHORT2INTEL(FLC_DELTA_FLC);

	/* Get pointer to line count */
	Line_count_ptr = (UNSHORT *) Ptr;
	Ptr += 2;

	/* Do each line */
	for (i=0;i<FLC_I->Height;i++)
	{
		/* Clear packet counter */
		FLC_Nr_delta_packets = 0;

		/* Scan line for delta packets */
		Nr_packets = FLC_Scan_line_for_delta_packets((UNSHORT *) Previous,
		 (UNSHORT *) Frame, Delta_packets);

		/* Any packets ? */
		if (Nr_packets)
		{
			/* Yes -> Have any lines been skipped until now ? */
			if (Line_skip)
			{
				*((UNSHORT *) Ptr) = MISC_UNSHORT2INTEL(0 - Line_skip);
				Ptr += 2;
				Line_skip = 0;
			}

			/* Is the width odd ? */
			if (Width % 1)
			{
				/* Yes -> Has the last pixel changed ? */
				if (*(Previous + Width - 1) != *(Frame + Width - 1))
				{
					/* Yes -> Write special word */
					*((UNSHORT *) Ptr) = MISC_UNSHORT2INTEL(*(Frame + Width - 1)
					 | 0x8000);
					Ptr += 2;
				}
			}

			/* Keep pointer to packet count */
			Packet_count_ptr = (UNSHORT *) Ptr;
			Ptr += 2;

			/* Write packets */
			for (j=0;j<Nr_packets;j++)
			{
				Ptr = FLC_Write_delta_packet(Ptr, &(Delta_packets[j]));
			}

			/* Write packet count */
			*Packet_count_ptr = MISC_UNSHORT2INTEL(FLC_Nr_delta_packets);

			/* Increase line counter */
			Line_count++;
		}
		else
		{
			/* No -> Increase line skip */
			Line_skip++;
		}

		/* Next line */
		Previous += FLC_I->Width;
		Frame += FLC_I->Width;
	}

	/* Write line count */
	*Line_count_ptr = MISC_UNSHORT2INTEL(Line_count);

	/* Free space for delta packets */
	BASEMEM_Free((UNBYTE *) Delta_packets);

	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Scan_line_for_delta_packets.
 * FUNCTION  : Scan a line for delta packets.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 13:49
 * LAST      : 29.11.94 13:49
 * INPUTS    : UNSHORT *Old - Pointer to old line.
 *             UNSHORT *New - Pointer to new line.
 *             struct FLC_delta Output[] - Array of FLC_delta structures.
 * RESULT    : UNSHORT : Number of delta packets.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
FLC_Scan_line_for_delta_packets(UNSHORT *Old, UNSHORT *New,
 struct FLC_delta Output[])
{
	UNSHORT Skip = 0, Length = 0, Index = 0, Nr_real_packets = 0;
	UNSHORT i;
	UNSHORT *Start;

	/* Analyze line */
	for (i=0;i<FLC_I->Width / 2;i++)
	{
		/* Is there a difference ? */
		if (*Old++ != *New++)
		{
			/* Yes -> Were we already scanning a delta packet ? */
			if (!Length)
			{
				/* No -> Remember where this new packet begins */
				Start = New - 1;
			}

			/* Increase packet length */
			Length++;
		}
		else
		{
			/* No -> Were we scanning a delta packet ? */
			if (Length)
			{
				/* Yes -> Store delta packet */
				Output[Index].Column_skip = Skip;
				Output[Index].Packet_length = Length;
				Output[Index].Data_ptr = Start;
				Index++;

				/* Next packet */
				Nr_real_packets++;

				Length = 0;
				Skip = 1;
			}
			else
			{
				/* No -> Increase column skip */
				Skip++;

				/* Has the column skip grown too large ? */
				if (Skip > 127)
				{
					/* Yes -> Store dummy packet */
					Output[Index].Column_skip = 127;
					Output[Index].Packet_length = 1;
					Output[Index].Data_ptr = New - 1;
					Index++;

					Skip -= 128;
				}
			}
		}
	}

	/* Were we scanning a delta packet ? */
	if (Length)
	{
		/* Yes -> Store delta packet */
		Output[Index].Column_skip = Skip;
		Output[Index].Packet_length = Length;
		Output[Index].Data_ptr = Start;
		Index++;

		/* Next packet */
	   Nr_real_packets++;
	}

	/* Did we find any REAL packets ? */
	if (Nr_real_packets)
	{
		/* Yes -> Include dummy packets */
		Nr_real_packets = Index;
	}

	return(Nr_real_packets);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Write_delta_packet
 * FUNCTION  : Run-length encode a delta packet and write it in a buffer.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 14:04
 * LAST      : 29.11.94 14:04
 * INPUTS    : UNBYTE *Buffer - Pointer to output buffer.
 *             struct FLC_delta *Packet - Pointer to delta packet.
 * RESULT    : UNBYTE * : Updated pointer to output buffer.
 * BUGS      : No known.
 * NOTES     : - One delta packet may be divided into several packets by the
 *              run-length encoding process. The packet counter will be
 *              increased accordingly.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FLC_Write_delta_packet(UNBYTE *Buffer, struct FLC_delta *Packet)
{
	UNSHORT Pixel1, Pixel2;
	UNSHORT Run_length, Unpacked_length;
	UNSHORT Skip_count;
	UNSHORT *Run_base, *Run_ptr, *Run_end;
	UNBYTE *Ptr;

	Ptr = Buffer;

	/* Start run */
	Run_base = Packet->Data_ptr;
	Run_end = Run_base + Packet->Packet_length;
	Run_ptr = Run_base;

	Run_length = 0;
	Unpacked_length = 1;
	Skip_count = Packet->Column_skip;

	/* Very short packet ? */
	if (Packet->Packet_length < 3)
	{
		/* Yes -> Just do it */
		Ptr = FLC_Write_unpacked_delta_packet(Ptr, Skip_count, Run_base,
		 Packet->Packet_length);

		return(Ptr);
	}

	/* Read first pixel */
	Pixel1 = *Run_ptr++;

	for (;;)
	{
		/* Read next pixel */
		Pixel2 = *Run_ptr++;

		/* Is the current pixel the same as the first ? */
		if (Pixel1 == Pixel2)
		{
			/* Yes -> First time ? */
			if (!Run_length)
			{
				/* Yes -> Count first pixel too */
				Run_length = 2;

				/* Take this pixel from current unpacked packet (if any) */
				if (Unpacked_length)
					Unpacked_length--;
			}
			else
			{
				/* No -> Increase run length */
				Run_length++;
			}

			/* Is the current run long enough ? */
			if (Run_length == 3)
			{
				/* Yes -> Are there unpacked pixels waiting ? */
				if (Unpacked_length)
				{
					/* Yes -> Write unpacked packet */
					Ptr = FLC_Write_unpacked_delta_packet(Ptr, Skip_count, Run_base,
					 Unpacked_length);
					Skip_count = 0;
					Unpacked_length = 0;
				}

				Run_base = Run_ptr - 3;
			}

			/* Maximum run size reached ? */
			if (Run_length > 128)
			{
				/* Yes -> Write run packet */
				Ptr = FLC_Write_run_delta_packet(Ptr, Skip_count, Pixel1, 127);
				Skip_count = 0;

				Run_base += 127;
				Run_length -= 127;
			}
		}
		else
		{
			/* No -> Is the current run long enough ? */
			if (Run_length > 2)
			{
				/* Yes -> Write run packet */
				Ptr = FLC_Write_run_delta_packet(Ptr, Skip_count, Pixel1,
				 Run_length);
				Skip_count = 0;

				Run_base = Run_ptr - 1;
				Run_length = 0;
			}

			/* Start unpacked */
			Unpacked_length += Run_length + 1;
			Run_length = 0;
			Pixel1 = Pixel2;
		}

		/* End of line ? */
		if (Run_ptr == Run_end)
			break;
	}

	/* Write last packet */
	/* Last run long enough ? */
	if (Run_length > 2)
	{
		/* Yes -> Write run packet */
		Ptr = FLC_Write_run_delta_packet(Ptr, Skip_count, Pixel1, Run_length);
	}
	else
	{
		/* No -> Write unpacked packet */
		Unpacked_length += Run_length;
		Ptr = FLC_Write_unpacked_delta_packet(Ptr, Skip_count, Run_base,
		 Unpacked_length);
	}

	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Write_run_delta_packet
 * FUNCTION  : Write a word-run delta packet.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 14:17
 * LAST      : 29.11.94 14:17
 * INPUTS    : UNBYTE *Ptr - Pointer to output buffer.
 *             UNSHORT Skip_count - Skip count.
 *             UNSHORT Word - Pixel pair.
 *             UNSHORT Length - Length of run.
 * RESULT    : UNBYTE * : Updated pointer to output buffer.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FLC_Write_run_delta_packet(UNBYTE *Ptr, UNSHORT Skip_count, UNSHORT Word,
 UNSHORT Length)
{
	/* Skip count is now measured in bytes */
	Skip_count *= 2;

	/* While the packet is too large for a single run */
	while (Length > 127)
	{
		/* Write maximum run packet */
		*Ptr++ = Skip_count;
		*Ptr++ = 0x81;

		*((UNSHORT *) Ptr) = MISC_UNSHORT2INTEL(Word);
		Ptr += 2;

		/* Skip count is no longer needed */
		Skip_count = 0;

		/* Increase packet counter */
		FLC_Nr_delta_packets++;

		/* Decrease length */
		Length -= 127;
	}

	/* Anything left ? */
	if (Length)
	{
		/* Yes -> Write run packet */
		*Ptr++ = Skip_count;
		*Ptr++ = 0 - Length;

		*((UNSHORT *) Ptr) = MISC_UNSHORT2INTEL(Word);
		Ptr += 2;

		/* Increase packet counter */
		FLC_Nr_delta_packets++;
	}

	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Write_unpacked_delta_packet
 * FUNCTION  : Write an unpacked delta packet.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.11.94 14:19
 * LAST      : 29.11.94 14:19
 * INPUTS    : UNBYTE *Ptr - Pointer to output buffer.
 *             UNSHORT Skip_count - Column skip count.
 *             UNBYTE *Run_ptr - Pointer to start of unpacked data.
 *             UNSHORT Length - Length of unpacked data.
 * RESULT    : UNBYTE * : Updated pointer to output buffer.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
FLC_Write_unpacked_delta_packet(UNBYTE *Ptr, UNSHORT Skip_count, UNSHORT *Run_ptr,
 UNSHORT Length)
{
	UNSHORT i, *WPtr;

	/* Skip count is now measured in bytes */
	Skip_count *= 2;

	/* While the packet is too large for a single run */
	while (Length > 127)
	{
		/* Write maximum unpacked packet */
		*Ptr++ = Skip_count;
		*Ptr++ = 127;

		WPtr = (UNSHORT *) Ptr;

		for (i=0;i<127;i++)
		{
			*WPtr++ = MISC_UNSHORT2INTEL(*Run_ptr++);
		}

		Ptr = (UNBYTE *) WPtr;

		/* Skip count is no longer needed */
		Skip_count = 0;

		/* Increase packet counter */
		FLC_Nr_delta_packets++;

		/* Decrease length */
		Length -= 127;
	}

	/* Anything left ? */
	if (Length)
	{
		/* Yes -> Write unpacked packet */
		*Ptr++ = Skip_count;
		*Ptr++ = Length;

		WPtr = (UNSHORT *) Ptr;

		for (i=0;i<Length;i++)
		{
			*WPtr++ = MISC_UNSHORT2INTEL(*Run_ptr++);
		}

		Ptr = (UNBYTE *) WPtr;

		/* Increase packet counter */
		FLC_Nr_delta_packets++;
	}

	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Start_flic_playback
 * FUNCTION  : Start playing back a flic.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 20:24
 * LAST      : 30.11.94 20:24
 * INPUTS    : struct Flic_playback *Flic_IO - Pointer to initialized flic
 *              recording I/O structure.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * NOTES     : - The input buffer, palette, screen, black colour index and
 *              output OPM elements of the I/O structure need to be set to
 *              meaningful values.
 *             - The flic will be played back at coordinates (0, 0). Other
 *              positions can be achieved by use of virtual OPMs.
 *             - Only one flic can be played at the same time.
 *             - The flic header will be initialized and the first frame will
 *              be decoded.
 *             - If a flic is already being played, playing back will fail.
 *             - This function will call FLC_Playback_flic_frame().
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FLC_Start_flic_playback(struct Flic_playback *Flic_IO)
{
	UNSHORT Flic_type;

	/* Already recording a flic ? */
	if (FLC_Playing_back_flic)
	{
		/* Yes -> Error */
		FLC_Error(FLCERR_ALREADY_PLAYING);
		return(FALSE);
	}

	/* Store I/O structure */
	FLC_O = Flic_IO;

	/* Copy flic header */
	memcpy((UNBYTE *) &(FLC_O->Flic), FLC_O->Input_buffer,
	 sizeof(struct Flic_header));

	/* Is this an .FLI- or .FLC-animation ? */
	Flic_type = MISC_INTEL2UNSHORT(FLC_O->Flic.type);
	if ((Flic_type != FLI_TYPE) && (Flic_type != FLC_TYPE))
	{
		/* No -> Error */
		FLC_Error(FLCERR_NOT_FLIC);
		return(FALSE);
	}

	/* Set flic dimensions */
	FLC_O->Width = MISC_INTEL2UNSHORT(FLC_O->Flic.width);
	FLC_O->Height = MISC_INTEL2UNSHORT(FLC_O->Flic.height);

	/* Initialize playback */
	FLC_O->Current_frame = 0;
	FLC_O->Nr_frames = MISC_INTEL2UNSHORT(FLC_O->Flic.frames);
	FLC_O->Previous_frame_offset = 0;

	/* Start playback */
	FLC_Playing_back_flic = TRUE;

	/* Display first frame */
	FLC_Playback_flic_frame();

	return(TRUE);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Stop_flic_playback
 * FUNCTION  : Stop playing back the current flic.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 20:26
 * LAST      : 30.11.94 20:26
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
FLC_Stop_flic_playback(void)
{
	/* Playing back a flic ? */
	if (FLC_Playing_back_flic)
	{
		/* Yes -> Stop it */
		FLC_Playing_back_flic = FALSE;
	}

	return(TRUE);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Playback_flic_frame
 * FUNCTION  : Play back a frame in the current flic.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 20:27
 * LAST      : 30.11.94 20:27
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (no errors) or FALSE (an error ocurred).
 * BUGS      : No known.
 * NOTES     : - The current frame will be created in the Output_frame OPM.
 *             - The palette will be automatically updated.
 *             - The speed of the input flic is ignored.
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
FLC_Playback_flic_frame(void)
{
	struct Flic_chunk_header *Chunk;
	struct Flic_subchunk_header *Subchunk;
	UNLONG Offset;
	UNSHORT Nr_subchunks, i;
	UNBYTE *Ptr;

	/* Playing back a flic ? */
	if (FLC_Playing_back_flic)
	{
		/* Yes -> Get offset to current frame */
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
				/* No -> Get offset to previous frame */
				Offset = FLC_O->Previous_frame_offset;

				/* Get previous frame chunk */
				Chunk = (struct Flic_chunk_header *) (FLC_O->Input_buffer + Offset);

				/* Add length of previous frame chunk to offset */
				Offset += MISC_INTEL2UNLONG(Chunk->size);
			}
		}
		else
		{
			/* Yes -> Playing for the first time ? */
			if (FLC_O->Previous_frame_offset)
			{
				/* No -> Get offset to last frame */
				Offset = FLC_O->Previous_frame_offset;

				/* Get last frame chunk */
				Chunk = (struct Flic_chunk_header *) (FLC_O->Input_buffer + Offset);

				/* Add length of last frame chunk to offset */
				Offset += MISC_INTEL2UNLONG(Chunk->size);
			}
			else
			{
				/* Yes -> Get offset to first frame */
				Offset = MISC_INTEL2UNLONG(FLC_O->Flic.oframe1);
			}
		}

		/* Store new offset */
		FLC_O->Previous_frame_offset = Offset;

		/* Get current frame chunk */
		Chunk = (struct Flic_chunk_header *) (FLC_O->Input_buffer + Offset);

		/* Is this a frame chunk ? */
		if (MISC_INTEL2UNSHORT(Chunk->type) != FLC_FLIC_FRAME)
		{
			/* No -> Error */
			FLC_Error(FLCERR_ILLEGAL_FRAME_CHUNK);
			return(FALSE);
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
					FLC_Decode_COLOR_256_chunk(Ptr);
					break;
				/* Word-oriented delta-frame sub-chunk */
				case FLC_DELTA_FLC:
					FLC_Decode_DELTA_FLC_chunk(Ptr);
					break;
				/* 64-level colour palette sub-chunk */
				case FLC_COLOR_64:
					FLC_Decode_COLOR_64_chunk(Ptr);
					break;
				/* Byte-oriented delta-frame sub-chunk */
				case FLC_DELTA_FLI:
					FLC_Decode_DELTA_FLI_chunk(Ptr);
					break;
				/* Black frame sub-chunk */
				case FLC_BLACK:
					FLC_Decode_BLACK_chunk(Ptr);
					break;
				/* Byte-run sub-chunk */
				case FLC_BYTE_RUN:
					FLC_Decode_BYTE_RUN_chunk(Ptr);
					break;
				/* Literal sub-chunk */
				case FLC_LITERAL:
					FLC_Decode_LITERAL_chunk(Ptr);
					break;
				/* Postage stamp sub-chunk (is ignored) */
				case FLC_PSTAMP:
					break;
				/* No known sub-chunk */
				default:
					/* Error */
					FLC_Error(FLCERR_ILLEGAL_SUBCHUNK);
					break;
			}

			/* Go to next sub-chunk */
			Ptr = ((UNBYTE *) Subchunk) + MISC_INTEL2UNLONG(Subchunk->size);
		}

		/* Next frame */
		FLC_O->Current_frame++;

		/* Time to loop ? */
		if (FLC_O->Current_frame >= FLC_O->Nr_frames)
		{
			/* Yes -> Back to frame 0 */
			FLC_O->Current_frame = 0;
		}
	}

	return(TRUE);
}


/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Decode_COLOR_256_chunk
 * FUNCTION  : Decode a COLOR_256 FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:09
 * LAST      : 30.11.94 21:09
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Decode_COLOR_256_chunk(UNBYTE *Ptr)
{
	struct BBPALETTE *Palette;
	UNSHORT Packets, Index, Length;
	UNSHORT i;

	/* Get pointer to palette */
	Palette = FLC_O->Palette;

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
			/* Copy colour */
			Palette->color[Index].red = *Ptr++;
			Palette->color[Index].green = *Ptr++;
			Palette->color[Index].blue = *Ptr++;
			Palette->color[Index].alpha = 0;

			/* Next colour */
			Index++;
		}

		/* Update palette part */
		DSA_SetPal(FLC_O->Screen, Palette, Index - Length, Length,
		 Index - Length);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Decode_DELTA_FLC_chunk
 * FUNCTION  : Decode a DELTA_FLC FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:00
 * LAST      : 30.11.94 21:00
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Decode_DELTA_FLC_chunk(UNBYTE *Ptr)
{
	UNSHORT X, Y = 0;
	UNSHORT Height, Special_word, Nr_packets;
	UNSHORT Colour1, Colour2;
	UNSHORT i;
	SIBYTE Counter;

	/* Get number of lines */
	Height = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
	Ptr += 2;

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
					Nr_packets = Special_word;
					break;
				/* Last pixel */
				case 2:
					/* Set last pixel */
					OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X +
					 FLC_O->Width - 1, FLC_O->Playback_Y + Y,
					 Special_word & 0x00FF);

					/* The next special word is always the packet counter */
					Nr_packets = MISC_INTEL2UNSHORT(*((UNSHORT *) Ptr));
					Ptr += 2;

					break;
				/* Line skip */
				case 3:
					/* Skip lines */
					Y -= Special_word;
					break;
			}
		/* Until the number of packets is known */
		} while (Nr_packets == 0xFFFF);

		/* Start a new line */
		X = 0;

		/* Any packets in this line ? */
		if (Nr_packets)
		{
			/* Skip columns */
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
						OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
						 FLC_O->Playback_Y + Y, *Ptr++);
						X++;
						OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
						 FLC_O->Playback_Y + Y, *Ptr++);
						X++;
					}
				}
				else
				{
					/* Run -> Get packet length */
					Counter = 0 - Counter;

					/* Get colours */
					Colour1 = *Ptr++;
					Colour2 = *Ptr++;

					/* Write run packet */
					while (Counter--)
					{
						OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
						 FLC_O->Playback_Y + Y, Colour1);
						X++;
						OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
						 FLC_O->Playback_Y + Y, Colour2);
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
 * NAME      : FLC_Decode_COLOR_64_chunk
 * FUNCTION  : Decode a COLOR_64 FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:00
 * LAST      : 30.11.94 21:00
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Decode_COLOR_64_chunk(UNBYTE *Ptr)
{
	struct BBPALETTE *Palette;
	UNSHORT Packets, Index, Length;
	UNSHORT i;

	/* Get pointer to palette */
	Palette = FLC_O->Palette;

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
			/* Copy colour */
			Palette->color[Index].red = *Ptr++ << 2;
			Palette->color[Index].green = *Ptr++ << 2;
			Palette->color[Index].blue = *Ptr++ << 2;
			Palette->color[Index].alpha = 0;

			/* Next colour */
			Index++;
		}

		/* Update palette part */
		DSA_SetPal(FLC_O->Screen, Palette, Index - Length, Length,
		 Index - Length);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Decode_DELTA_FLI_chunk
 * FUNCTION  : Decode a DELTA_FLI FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:00
 * LAST      : 30.11.94 21:00
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Decode_DELTA_FLI_chunk(UNBYTE *Ptr)
{
	UNSHORT X, Y;
	UNSHORT Height, Nr_packets, Colour;
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
						OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
						 FLC_O->Playback_Y + Y, *Ptr++);
						X++;
					}
				}
				else
				{
					/* Run -> Get packet length */
					Counter = -Counter;

					/* Get colour */
					Colour = *Ptr++;

					/* Write run packet */
					while (Counter--)
					{
						OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
						 FLC_O->Playback_Y + Y, Colour);
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
 * NAME      : FLC_Decode_BLACK_chunk
 * FUNCTION  : Decode a BLACK FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 21:00
 * LAST      : 30.11.94 21:00
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Decode_BLACK_chunk(UNBYTE *Ptr)
{
	/* Display black frame */
	OPM_FillBox(FLC_O->Output_frame, FLC_O->Playback_X, FLC_O->Playback_Y,
	 FLC_O->Width, FLC_O->Height, FLC_O->Black_colour_index);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : FLC_Decode_BYTE_RUN_chunk
 * FUNCTION  : Decode a BYTE_RUN FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman, Thomas H„user
 * FIRST     : 30.11.94 21:00
 * LAST      : 30.11.94 21:00
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Decode_BYTE_RUN_chunk(UNBYTE *Ptr)
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
					OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
					 FLC_O->Playback_Y + Y, *Ptr++);
					X++;
				}
			}
			else
			{
				/* Run -> Get colour */
				Colour = *Ptr++;

				/* Write run packet */
				while (Counter--)
				{
					OPM_SetPixel(FLC_O->Output_frame, FLC_O->Playback_X + X,
					 FLC_O->Playback_Y + Y, Colour);
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
 * NAME      : FLC_Decode_LITERAL_chunk
 * FUNCTION  : Decode a LITERAL FLC-chunk.
 * FILE      : FLC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.11.94 21:00
 * LAST      : 30.11.94 21:00
 * INPUTS    : UNBYTE *Ptr - Pointer to sub-chunk data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
FLC_Decode_LITERAL_chunk(UNBYTE *Ptr)
{
	struct OPM Temp_OPM;

	/* Create temporary OPM */
	OPM_New(FLC_O->Width, FLC_O->Height, 1, &Temp_OPM, Ptr);

	/* Copy literal image */
	OPM_CopyOPMOPM(&Temp_OPM, FLC_O->Output_frame, 0, 0, FLC_O->Width,
	 FLC_O->Height, FLC_O->Playback_X, FLC_O->Playback_Y);

	/* Destroy temporary OPM */
	OPM_Del(&Temp_OPM);
}

