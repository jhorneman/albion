/************
 * NAME     : MUSIC.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 2-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MUSIC.H
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <direct.h>

#include <BBDEF.H>
#include <BBERROR.H>

#include <XLOAD.H>

#include <GAMEVAR.H>
#include <MUSIC.H>
#include <XFTYPES.H>

/* global variables */

static BOOLEAN Music_initialized = FALSE;
BOOLEAN Music_on = TRUE;

static MDI_DRIVER *XMIDI_driver;
static DIG_DRIVER *Digital_driver;

static HWAVE Ambient_wave_synth;
static MEM_HANDLE Ambient_wavelib_handle;

static struct Song Song_song;
static struct Song Ambient_song;
static struct Song Jingle_song;

static struct Sound_effect Sound_effect_table[MAX_SOUND_EFFECTS];

static UNCHAR Music_library_name[] = "Music";

static UNCHAR *Music_error_strings[] = {
	"Illegal error code.",
	"AIL error : %s",
	"No free song.",
	"File could not be loaded."
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_music
 * FUNCTION  : Initialize music.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 14:23
 * LAST      : 09.02.95 14:23
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_music(void)
{
	UNSHORT i;

	/* Already initialized ? */
	if (!Music_initialized && (Music_flags & SOUNDSYSTEM_ON))
	{
		/* No -> Set Global Timbre Library filename prefix */
		AIL_set_GTL_filename_prefix("ALBISND");

		/* Switch to music driver directory */
		chdir("DRIVERS");

		/* Install XMIDI driver */
		XMIDI_driver = AIL_install_MDI_INI();
		if (!XMIDI_driver)
		{
			chdir("..");
			Music_error(MUSICERR_AIL_ERROR);
			return;
		}

		#ifdef URGLE
		/* Install XMIDI driver */
		XMIDI_driver = AIL_install_MDI_driver_file("DRIVERS\\SBLASTER.MDI", NULL);
		if (!XMIDI_driver)
		{
			Music_error(MUSICERR_AIL_ERROR);
			return;
		}
		#endif

		/* Set preferences for playback of digital samples */
	   AIL_set_preference(DIG_LATENCY, 30);
	   AIL_set_preference(DIG_USE_STEREO, YES);
	   AIL_set_preference(DIG_HARDWARE_SAMPLE_RATE, DEFAULT_FREQUENCY);

		/* Install digital driver */
		Digital_driver = AIL_install_DIG_INI();
		if (!Digital_driver)
		{
			chdir("..");
			Music_error(MUSICERR_AIL_ERROR);
			return;
		}

		#ifdef URGLE
		/* Install digital driver */
		Digital_driver = AIL_install_DIG_driver_file("DRIVERS\\SBLASTER.DIG", NULL);
		if (!Digital_driver)
		{
			Music_error(MUSICERR_AIL_ERROR);
			return;
		}
		#endif

		/* Back to normal directory */
		chdir("..");

		/* Allocate sequence handles */
		Song_song.Sequence_handle = AIL_allocate_sequence_handle(XMIDI_driver);
		if (!Song_song.Sequence_handle)
		{
			Music_error(MUSICERR_AIL_ERROR);
		}

		Ambient_song.Sequence_handle = AIL_allocate_sequence_handle(XMIDI_driver);
		if (!Ambient_song.Sequence_handle)
		{
			Music_error(MUSICERR_AIL_ERROR);
		}

		Jingle_song.Sequence_handle = AIL_allocate_sequence_handle(XMIDI_driver);
		if (!Jingle_song.Sequence_handle)
		{
			Music_error(MUSICERR_AIL_ERROR);
		}

		/* Allocate sample handles */
		for (i=0;i<MAX_SOUND_EFFECTS;i++)
		{
			Sound_effect_table[i].Sample_handle = AIL_allocate_sample_handle(Digital_driver);
			if (!Sound_effect_table[i].Sample_handle)
			{
				Music_error(MUSICERR_AIL_ERROR);
			}
		}

		/* Music is initialized */
		Music_initialized = TRUE;
	}

	/* Reset songs */
	Song_song.Number = 0;
	Ambient_song.Number = 0;
	Jingle_song.Number = 0;

	/* Reset sound effect table */
	for (i=0;i<MAX_SOUND_EFFECTS;i++)
	{
		Sound_effect_table[i].Number = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_music
 * FUNCTION  : Exit music.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 16:06
 * LAST      : 10.02.95 16:06
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_music(void)
{
	UNSHORT i;

	/* Initialized ? */
	if (Music_initialized)
	{
		/* Yes -> Is a song playing ? */
		if (Song_song.Number)
		{
			/* Yes -> End song */
			End_song(&Song_song);
		}

		/* Is an ambient song playing ? */
		if (Ambient_song.Number)
		{
			/* Yes -> Destroy wave synthesizer */
			AIL_destroy_wave_synthesizer(Ambient_wave_synth);

			/* Remove wave library */
			MEM_Free_pointer(Ambient_wavelib_handle);
			MEM_Free_memory(Ambient_wavelib_handle);

			/* End song */
			End_song(&Ambient_song);
		}

		/* Is a jingle playing ? */
		if (Jingle_song.Number)
		{
			/* Yes -> End song */
			End_song(&Jingle_song);
		}

		/* Release sequence handles */
		AIL_release_sequence_handle(Song_song.Sequence_handle);
		AIL_release_sequence_handle(Ambient_song.Sequence_handle);
		AIL_release_sequence_handle(Jingle_song.Sequence_handle);

		/* End all sound-effects */
		for (i=0;i<MAX_SOUND_EFFECTS;i++)
		{
			/* Active ? */
			if (Sound_effect_table[i].Number)
			{
				/* Yes -> End */
				AIL_end_sample(Sound_effect_table[i].Sample_handle);

				/* Remove sound effect */
				MEM_Free_pointer(Sound_effect_table[i].Memory_handle);
				MEM_Free_memory(Sound_effect_table[i].Memory_handle);

				/* Clear sound effect data */
				Sound_effect_table[i].Number = 0;
			}

			/* Release sample handle */
			AIL_release_sample_handle(Sound_effect_table[i].Sample_handle);
		}

		/* Remove digital driver */
		AIL_uninstall_DIG_driver(Digital_driver);

		/* Remove XMIDI driver */
		AIL_uninstall_MDI_driver(XMIDI_driver);

		/* Music is no longer initialized */
		Music_initialized = FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_music
 * FUNCTION  : Update music handling.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 15:59
 * LAST      : 09.02.95 15:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_music(void)
{
	UNSHORT i;

	/* Initialized ? */
	if (Music_initialized)
	{
		/* Yes -> Is a jingle active ? */
		if (Jingle_song.Number)
		{
			/* Yes -> Has it ended ? */
			if (AIL_sequence_status(Jingle_song.Sequence_handle) == SEQ_DONE)
			{
				/* Yes -> End song */
				End_song(&Jingle_song);
			}
		}

		/* Check all sound effects */
		for (i=0;i<MAX_SOUND_EFFECTS;i++)
		{
			/* Active ? */
			if (Sound_effect_table[i].Number)
			{
				/* Yes -> Ended ? */
				if (AIL_sample_status(Sound_effect_table[i].Sample_handle) == SMP_DONE)
				{
					/* Yes -> Remove sound effect */
					MEM_Free_pointer(Sound_effect_table[i].Memory_handle);
					MEM_Free_memory(Sound_effect_table[i].Memory_handle);

					/* Clear sound effect data */
					Sound_effect_table[i].Number = 0;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_song
 * FUNCTION  : Start playing a song.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 15:04
 * LAST      : 09.02.95 15:04
 * INPUTS    : UNSHORT Song_nr - Number of song to be played (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Play_song(UNSHORT Song_nr)
{
	MEM_HANDLE Handle;
	UNBYTE *Ptr;

	/* Music initialized / on ? */
	if (Music_initialized && (Music_flags & MUSIC_ON))
	{
		/* Yes -> See if this song is already playing */
		if (Song_song.Number == Song_nr)
			return;

		/* Is some song already playing ? */
		if (Song_song.Number)
		{
			/* Yes -> End current song */
			End_song(&Song_song);
		}

		/* Any song ? */
		if (Song_nr)
		{
			/* Yes -> Load new song */
			Handle = Load_subfile(SONG, Song_nr);
			if (!Handle)
			{
				Music_error(MUSICERR_FILE_LOAD);
				return;
			}

			/* Claim song memory */
			Ptr = MEM_Claim_pointer(Handle);

			/* Initialize sequence */
			if (AIL_init_sequence(Song_song.Sequence_handle, Ptr, 0) <= 0)
			{
				Music_error(MUSICERR_AIL_ERROR);
				return;
			}

			/* Insert data in song table */
			Song_song.Flags = 0;
			Song_song.Number = Song_nr;
			Song_song.Memory_handle = Handle;

			/* Start sequence */
			AIL_start_sequence(Song_song.Sequence_handle);

			/* Set the sequence volume */
			AIL_set_sequence_volume(Ambient_song.Sequence_handle,
			 (LONG) SONG_VOLUME, (LONG) 0);

			/* Make sure the sequence repeats forever */
			AIL_set_sequence_loop_count(Song_song.Sequence_handle, (LONG) 0);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_ambient_song
 * FUNCTION  : Start playing an ambient song.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 10:58
 * LAST      : 10.02.95 10:58
 * INPUTS    : UNSHORT Song_nr - Number of song to be played (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Play_ambient_song(UNSHORT Song_nr)
{
	MEM_HANDLE Handle;
	UNBYTE *Ptr;

	/* Music initialized / on / ambient on ? */
	if (Music_initialized && (Music_flags & MUSIC_ON) &&
	 (Music_flags & AMBIENT_ON))
	{
		/* Yes -> See if this song is already playing */
		if (Ambient_song.Number == Song_nr)
			return;

		/* Is some song already playing ? */
		if (Ambient_song.Number)
		{
			/* Yes -> Destroy wave synthesizer */
			AIL_destroy_wave_synthesizer(Ambient_wave_synth);

			/* Remove wave library */
			MEM_Free_pointer(Ambient_wavelib_handle);
			MEM_Free_memory(Ambient_wavelib_handle);

			/* End current song */
			End_song(&Ambient_song);
		}

		/* Any song ? */
		if (Song_nr)
		{
			/* Yes -> Load new song */
			Handle = Load_subfile(SONG, Song_nr);
			if (!Handle)
			{
				Music_error(MUSICERR_FILE_LOAD);
				return;
			}

			/* Load new wave library */
			Ambient_wavelib_handle = Load_subfile(WAVELIB, Song_nr);
			if (!Ambient_wavelib_handle)
			{
				Music_error(MUSICERR_FILE_LOAD);
				return;
			}

			/* Claim wave library memory */
			Ptr = MEM_Claim_pointer(Ambient_wavelib_handle);

			/* Create wave synthesizer */
			Ambient_wave_synth = AIL_create_wave_synthesizer(Digital_driver,
			 XMIDI_driver, (void *) Ptr, (LONG) 4);

			/* Claim song memory */
			Ptr = MEM_Claim_pointer(Handle);

			/* Initialize sequence */
			if (AIL_init_sequence(Ambient_song.Sequence_handle, Ptr, 0) <= 0)
			{
				Music_error(MUSICERR_AIL_ERROR);
				return;
			}

			/* Insert data in song table */
			Ambient_song.Flags = 0;
			Ambient_song.Number = Song_nr;
			Ambient_song.Memory_handle = Handle;

			/* Start sequence */
			AIL_start_sequence(Ambient_song.Sequence_handle);

			/* Set the sequence volume */
			AIL_set_sequence_volume(Ambient_song.Sequence_handle,
			 (LONG) AMBIENT_VOLUME, (LONG) 0);

			/* Make sure the sequence repeats forever */
			AIL_set_sequence_loop_count(Ambient_song.Sequence_handle, (LONG) 0);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_jingle
 * FUNCTION  : Play a jingle.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 11:10
 * LAST      : 10.02.95 11:10
 * INPUTS    : UNSHORT Song_nr - Number of song to be played (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Play_jingle(UNSHORT Song_nr)
{
	MEM_HANDLE Handle;
	UNBYTE *Ptr;

	/* Music initialized / on ? */
	if (Music_initialized && (Music_flags & MUSIC_ON))
	{
		/* Yes -> Is some song already playing ? */
		if (Jingle_song.Number)
		{
			/* Yes -> End current song */
			End_song(&Jingle_song);
		}

		/* Any song ? */
		if (Song_nr)
		{
			/* Yes -> Load new song */
			Handle = Load_subfile(SONG, Song_nr);
			if (!Handle)
			{
				Music_error(MUSICERR_FILE_LOAD);
				return;
			}

			/* Claim song memory */
			Ptr = MEM_Claim_pointer(Handle);

			/* Initialize sequence */
			if (AIL_init_sequence(Jingle_song.Sequence_handle, Ptr, 0) <= 0)
			{
				Music_error(MUSICERR_AIL_ERROR);
				return;
			}

			/* Insert data in song table */
			Jingle_song.Flags = 0;
			Jingle_song.Number = Song_nr;
			Jingle_song.Memory_handle = Handle;

			/* Start sequence */
			AIL_start_sequence(Jingle_song.Sequence_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_sound_effect
 * FUNCTION  : Play a sound effect.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 16:36
 * LAST      : 09.02.95 16:36
 * INPUTS    : UNSHORT Effect_nr - Number of sound effect to be played (1...).
 *             UNSHORT Priority - Priority (0...100).
 *             UNSHORT Volume - Volume (0...127).
 *             UNSHORT Variability - Variability percentage (0...100).
 *             UNSHORT Frequency - Playback frequence in Hz (0 = use default).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Play_sound_effect(UNSHORT Effect_nr, UNSHORT Priority, UNSHORT Volume,
 UNSHORT Variability, UNSHORT Frequency)
{
	MEM_HANDLE Handle;
	SILONG New_frequency;
	SILONG Frequency_deviation;
	UNLONG Length;
	SISHORT New_volume;
	SISHORT Volume_deviation;
	UNSHORT Found = 0;
	UNSHORT Lowest;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Music initialized / sound effects on ? */
	if (Music_initialized && (Music_flags & SOUND_FX_ON))
	{
		/* Load new sample */
		Handle = Load_subfile(DSAMPLE, Effect_nr);
		if (!Handle)
		{
			Music_error(MUSICERR_FILE_LOAD);
			return;
		}

		/* Get sample length */
		Length = MEM_Get_block_size(Handle);

		/* Is a dummy file ? */
		if (Length <= 4)
		{
			/* Yes -> Remove file */
			MEM_Free_memory(Handle);

			/* Exit */
			return;
		}

		/* Yes -> Check all sound effects */
		for (i=0;i<MAX_SOUND_EFFECTS;i++)
		{
			/* Active ? */
			if (!Sound_effect_table[i].Number)
				break;
		}

		/* Found something ? */
		if (i >= MAX_SOUND_EFFECTS)
		{
			/* No -> Search the sound effect with the lowest priority */
			Lowest = 0xFFFF;
			for (i=0;i<MAX_SOUND_EFFECTS;i++)
			{
				if (Sound_effect_table[i].Priority < Lowest)
				{
					Lowest = Sound_effect_table[i].Priority;
					Found = i;
				}
			}

			/* Is the lowest priority lower than the new effect's priority ? */
			if (Lowest < Priority)
			{
				/* Yes -> Install the new effect in the old one's place */
				i = Found;

				/* End old sound effect */
				AIL_end_sample(Sound_effect_table[i].Sample_handle);

				/* Remove old sound effect */
				MEM_Free_pointer(Sound_effect_table[i].Memory_handle);
				MEM_Free_memory(Sound_effect_table[i].Memory_handle);

				/* Clear sound effect data */
				Sound_effect_table[i].Number = 0;
			}
			else
			{
				/* No ->Remove file */
				MEM_Free_memory(Handle);

				/* Exit */
				return;
			}
		}

		/* Claim sample memory */
		Ptr = MEM_Claim_pointer(Handle);

		/* Initialize sample */
		AIL_init_sample(Sound_effect_table[i].Sample_handle);
		AIL_set_sample_address(Sound_effect_table[i].Sample_handle, (void *) Ptr,
		 (ULONG) Length);

		/* Frequency given ? */
		if (!Frequency)
		{
			/* No -> Set to default */
			Frequency = DEFAULT_FREQUENCY;
		}

		/* Implement variability on volume and frequency */
		Volume_deviation = (63 * 100) / Variability;
		New_volume = Volume + (rand() % (2 * Volume_deviation)) - Volume_deviation;

		Frequency_deviation = (5000 * 100) / Variability;
		New_frequency = Frequency + (rand() % (2 * Frequency_deviation)) - Frequency_deviation;

		/* Clip volume and frequency */
		if (New_volume < 1)
		{
			New_volume = 1;
		}
		else
		{
			if (New_volume > 127)
				New_volume = 127;
		}
		Volume = New_volume;

		if (New_frequency < 1000)
		{
			New_frequency = 1000;
		}
		else
		{
			if (New_frequency > 44100)
				New_frequency = 44100;
		}
		Frequency = New_frequency;

		/* Set sample parameters */
		AIL_set_sample_volume(Sound_effect_table[i].Sample_handle, Volume);
		AIL_set_sample_playback_rate(Sound_effect_table[i].Sample_handle, (LONG) Frequency);

		/* Insert data in sound effect table */
		Sound_effect_table[i].Flags = 0;
		Sound_effect_table[i].Priority = Priority;
		Sound_effect_table[i].Volume = Volume;
		Sound_effect_table[i].Frequency = Frequency;
		Sound_effect_table[i].Number = Effect_nr;
		Sound_effect_table[i].Memory_handle = Handle;
		Sound_effect_table[i].Length = Length;

		/* Start sample */
		AIL_start_sample(Sound_effect_table[i].Sample_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Initialize_located_effects
 * FUNCTION  : Initialize located sound effects.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : UNLONG Space_width - Width of effect space.
 *             UNLONG Space_height - Height of effect space.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - All previous located sound effects will be deleted.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Initialize_located_effects(UNLONG Space_width, UNLONG Space_height)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_located_effect
 * FUNCTION  : Add a located sound effect.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : UNSHORT Effect_nr - Number of sound effect to be played (1...).
 *             UNSHORT Priority - Priority (0...100).
 *             UNSHORT Volume - Volume (0...127).
 *             UNSHORT Variability - Variability percentage (0...100).
 *             UNSHORT Frequency - Playback frequence in Hz (0 = use default).
 *             UNLONG Effect_X - X-coordinate of effect in effect space.
 *             UNLONG Effect_Y - Y-coordinate of effect in effect space.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_located_effect(UNSHORT Effect_nr, UNSHORT Priority, UNSHORT Volume,
 UNSHORT Variability, UNSHORT Frequency, SILONG Effect_X, SILONG Effect_Y)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_located_effects
 * FUNCTION  : Update all located sound effects.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : UNLONG Observer_X - X-coordinate of observer in effect space.
 *             UNLONG Observer_Y - Y-coordinate of observer in effect space.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_located_effects(SILONG Observer_X, SILONG Observer_Y)
{
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Music_error
 * FUNCTION  : Report a music error.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 14:14
 * LAST      : 09.02.95 14:14
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : MUSIC.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Music_error(UNSHORT Error_code)
{
	/* Push error on the error stack */
	ERROR_PushError(Music_print_error, Music_library_name, sizeof(UNSHORT),
	 (UNBYTE *) &Error_code);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Music_print_error
 * FUNCTION  : Print a music error.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 14:19
 * LAST      : 09.02.95 14:19
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by Music_error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : XLOAD.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Music_print_error(UNCHAR *buffer, UNBYTE *data)
{
	UNSHORT i;

	/* Get error code */
	i = *((UNSHORT *) data);

	/* Catch illegal errors */
	if (i>MUSICERR_MAX)
		i = 0;

 	/* Print error */
	sprintf((char *)buffer, Music_error_strings[i], (UNCHAR *) AIL_error);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : End_song
 * FUNCTION  : End a song.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 09:52
 * LAST      : 10.02.95 09:52
 * INPUTS    : struct Song *Song - Pointer to song data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
End_song(struct Song *Song)
{
	/* End sequence */
	AIL_end_sequence(Song->Sequence_handle);

	/* Remove song */
	MEM_Free_pointer(Song->Memory_handle);
	MEM_Free_memory(Song->Memory_handle);

	/* Clear song data */
	Song->Number = 0;
}

