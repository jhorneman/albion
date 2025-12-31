/************
 * NAME     : MUSIC.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 2-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MUSIC.H
 ************/

/*
 ** Includes ***************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <BBDEF.H>
#include <BBERROR.H>
#include <BBMEM.H>

#include <XLOAD.H>

#include <MAIN.H>
#include <GAMEVAR.H>
#include <MUSIC.H>
#include <XFTYPES.H>

/*
 ** Defines ****************************************************************
 */

/* Bitmasks for Sound_state */

/* This flag is set when AIL has been initialized successfully */
#define SOUNDSYSTEM_ON				(1 << 0)

/* This flag is set when the sound system has been initialized successfully.
	It can only be set when the sound system is on. */
#define SOUNDSYSTEM_INITIALIZED	(1 << 1)

/* These flags are set when the XMIDI and digital sound subsystems have
	been initialized successfully.
	They can only be set when the sound system is on and initialized. */
#define MIDI_ON						(1 << 2)
#define DIGI_ON						(1 << 3)

/* These flags control the individual aspects of the sound system. They can
	be set and cleared practically at any time during the execution of the
   program (although they are changed using access functions and not
   directly).
   The access functions make sure that they can NOT be set unless the
	appropriate subsystems are on! */
#define MUSIC_ON						(1 << 4)
#define AMBIENT_ON					(1 << 5)
#define SOUND_FX_ON					(1 << 6)

/* This flag controls the located sound effects. It can only be set when
   sound effects are on, and are used to switch off the located sound effects
   when they are not desired, for instance in the combat screen. */
#define LOCATED_SOUND_FX_ON		(1 << 7)

/* Various volumes */
#define SONG_VOLUME				(127)
#define AMBIENT_VOLUME			(80)
#define SOUND_FX_VOLUME			(100)

/* Default sample frequency */
#define DEFAULT_FREQUENCY		(11000)

/* Digital channel maxima */
#define MAX_DIGI_CHANNELS				(16)
#define MAX_AMBIENT_DIGI_CHANNELS	(4)
#define MAX_FX_DIGI_CHANNELS			(12)

/* Sound effect maxima */
#define MAX_NORMAL_SOUND_EFFECTS		(8)
#define MAX_LOCATED_SOUND_EFFECTS	(40)

/* Error codes */
#define MUSICERR_AIL_ERROR		(1)
#define MUSICERR_NO_FREE_SONG	(2)
#define MUSICERR_FILE_LOAD		(3)
#define MUSICERR_NO_MDI_INI	(4)
#define MUSICERR_NO_DIG_INI	(5)

#define MUSICERR_MAX				(5)

/*
 ** Enumerations ***********************************************************
 */

/* Song states */
enum Song_state {
	SONG_FREE,
	SONG_PLAYING,
	SONG_PAUSED
};

/* Sound effect states */
enum Sound_effect_state {
	SOUND_EFFECT_FREE,
	SOUND_EFFECT_WAITING,
	SOUND_EFFECT_PLAYING,
	SOUND_EFFECT_STOPPED
};

/* Digital channel states */
enum Digital_channel_state {
	DIGI_CHANNEL_FREE,
	DIGI_CHANNEL_PLAYING
};

/*
 ** Type definitions *******************************************************
 */

typedef enum Song_state SONG_STATE_T;
typedef enum Sound_effect_state SOUND_EFFECT_STATE_T;
typedef enum Digital_channel_state DIGI_CHANNEL_STATE_T;

/*
 ** Structure definitions **************************************************
 */

/* Song structure */
struct Song {
	SONG_STATE_T State;
	UNSHORT Number;
	MEM_HANDLE Memory_handle;
	HSEQUENCE Sequence_handle;
};

/* Sound effect structure */
struct Sound_effect {
	SOUND_EFFECT_STATE_T State;
	UNSHORT Flags;
	UNSHORT Priority;

	UNSHORT Volume;
	UNSHORT Frequency;
	UNSHORT Stereo_position;

	MEM_HANDLE Memory_handle;
	UNLONG Length;
};

/* Bitmasks for Sound_effect.Flags */
#define LOOP_SOUND_EFFECT		(1 << 0)

/* Located sound effect structure */
struct Located_sound_effect {
	struct Sound_effect Effect;

	UNSHORT Initial_priority;
	UNSHORT Initial_volume;
	UNSHORT Probability;

	SILONG Effect_X;
	SILONG Effect_Y;
};

/* Digital channel structure */
struct Digital_channel {
	DIGI_CHANNEL_STATE_T State;
	struct Sound_effect *Effect_ptr;
	HSAMPLE Sample_handle;
};

/*
 ** Prototypes *************************************************************
 */

/* Music error functions */
void Music_error(UNSHORT Error_code);
void Music_print_error(UNCHAR *buffer, UNBYTE *data);

/* Music support functions */
void Pause_song(struct Song *Song);
void Continue_song(struct Song *Song);
void End_song(struct Song *Song);

/* Sound effect management functions */
void Update_sound_effects(void);

void Swap_sound_effects(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_sound_effects(UNLONG A, UNLONG B, UNBYTE *Data);

void Update_located_sound_effects(void);

/* Sound effect support functions */
void Start_sound_effect(struct Sound_effect *Sound_effect);
void Update_sound_effect(struct Sound_effect *Effect);
void Stop_sound_effect(struct Sound_effect *Effect);
void Delete_sound_effect(struct Sound_effect *Effect);

/* Digital channel support functions */
void Stop_all_digital_channels(void);
void Stop_digital_channel(UNSHORT Channel_index);

/*
 ** Global variables *******************************************************
 */

/* Global soundsystem state */
UNLONG Sound_state;

UNSHORT Sound_effects_volume = SOUND_FX_VOLUME;

/* XMIDI and digital drivers */
static MDI_DRIVER *XMIDI_driver;
static DIG_DRIVER *Digital_driver;

/* Wave synth data */
static HWAVE Ambient_wave_synth;
static MEM_HANDLE Ambient_wavelib_handle;

/* Song data */
static struct Song Song_song;
static struct Song Ambient_song;
static struct Song Jingle_song;

/* Dimensions of effect space */
static UNLONG Effect_space_width;
static UNLONG Effect_space_height;
static UNLONG Effect_listening_range;

/* Position of listener within effect space */
static SILONG Listener_X;
static SILONG Listener_Y;
static UNSHORT Listener_angle;

/* Digital channel data */
static struct Digital_channel Digital_channels[MAX_FX_DIGI_CHANNELS];

/* Sound effect data */
static struct Sound_effect Sound_effect_table[MAX_NORMAL_SOUND_EFFECTS];
static struct Located_sound_effect Located_sound_effect_table[MAX_LOCATED_SOUND_EFFECTS];

/* Error handling data */
static UNCHAR Music_library_name[] = "Music";

static UNCHAR *Music_error_strings[] = {
	"Illegal error code.",
	"AIL error : %s",
	"No free song.",
	"File could not be loaded.",
	"MIDI sound has not been set up.",
	"Digital sound has not been set up."
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Sound_on
 * FUNCTION  : Switch music and sound effects on.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 14.07.95 17:54
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may safely ignore the sound state.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Sound_on(void)
{
	/* Switch music and sound effects on */
	Music_on();
	Sound_effects_on();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Sound_on
 * FUNCTION  : Switch music and sound effects off.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 14.07.95 17:54
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may safely ignore the sound state.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Sound_off(void)
{
	/* Switch music and sound effects off */
	Music_off();
	Sound_effects_off();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Music_on
 * FUNCTION  : Switch music on.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 23.07.95 15:45
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Music_on(void)
{
	/* Sound system initialized / MIDI on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & MIDI_ON))
	{
		/* Yes -> Music off ? */
		if (!(Sound_state & MUSIC_ON))
		{
			/* Yes -> Continue all normal songs */
			Continue_song(&Song_song);
			Continue_song(&Jingle_song);

			/* Music is on */
			Sound_state |= MUSIC_ON;
		}

		/* Digital sound on / ambient music off ? */
		if ((Sound_state & DIGI_ON) && !(Sound_state & AMBIENT_ON))
		{
			/* Yes -> Continue ambient song */
			Continue_song(&Ambient_song);

			/* Ambient music is on */
			Sound_state |= AMBIENT_ON;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Music_on
 * FUNCTION  : Switch music off.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 23.07.95 15:45
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Music_off(void)
{
	/* Sound system initialized / MIDI on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & MIDI_ON))
	{
		/* Yes -> Music on ? */
		if (Sound_state & MUSIC_ON)
		{
			/* Yes -> Pause all normal songs */
			Pause_song(&Song_song);
			Pause_song(&Jingle_song);

			/* Music is off */
			Sound_state &= ~MUSIC_ON;
		}

		/* Digital sound on / ambient music on ? */
		if ((Sound_state & DIGI_ON) &&
		 (Sound_state & AMBIENT_ON))
		{
			/* Yes -> Pause ambient song */
			Pause_song(&Ambient_song);

			/* Ambient music is off */
			Sound_state &= ~AMBIENT_ON;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Sound_effects_on
 * FUNCTION  : Switch sound effects on.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 23.07.95 15:51
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Sound_effects_on(void)
{
	/* Sound system initialized / digital sound on / sound effects off ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & DIGI_ON) &&
	 !(Sound_state & SOUND_FX_ON))
	{
		/* Yes -> Sound effects are on */
		Sound_state |= SOUND_FX_ON;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Sound_effects_off
 * FUNCTION  : Switch sound effects off.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 23.07.95 15:54
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Sound_effects_off(void)
{
	UNSHORT i;

	/* Sound system initialized / digital sound on / sound effects on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & DIGI_ON) &&
	 (Sound_state & SOUND_FX_ON))
	{
		/* Yes -> Stop all sound effects */
		Stop_all_digital_channels();

		/* Delete all normal sound effects */
		for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
		{
			/* State is not FREE ? */
			if (Sound_effect_table[i].State != SOUND_EFFECT_FREE)
			{
				/* Yes -> Delete sound effect */
				Delete_sound_effect(&(Sound_effect_table[i]));
			}
		}

		/* Delete all located sound effects */
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* State is not FREE ? */
			if (Located_sound_effect_table[i].Effect.State != SOUND_EFFECT_FREE)
			{
				/* Yes -> Delete sound effect */
				Delete_sound_effect(&(Located_sound_effect_table[i].Effect));
			}
		}

		/* Sound effects are off */
		Sound_state &= ~SOUND_FX_ON;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Located_sound_effects_on
 * FUNCTION  : Switch located sound effects on.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 23.07.95 16:01
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Located_sound_effects_on(void)
{
	/* Sound system initialized / digital sound on / sound effects on /
	 located sound effects off ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & DIGI_ON) &&
	 (Sound_state & SOUND_FX_ON) &&
	 !(Sound_state & LOCATED_SOUND_FX_ON))
	{
		/* Yes -> Located sound effects are on */
		Sound_state |= LOCATED_SOUND_FX_ON;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Located_sound_effects_off
 * FUNCTION  : Switch located sound effects off.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 23.07.95 16:02
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Located_sound_effects_off(void)
{
	UNSHORT i;

	/* Sound system initialized / digital sound on / sound effects on /
	 located sound effects on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & DIGI_ON) &&
	 (Sound_state & SOUND_FX_ON) &&
	 (Sound_state & LOCATED_SOUND_FX_ON))
	{
		/* Yes -> Delete all located sound effects */
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* State is not FREE ? */
			if (Located_sound_effect_table[i].Effect.State != SOUND_EFFECT_FREE)
			{
				/* Yes -> Delete sound effect */
				Delete_sound_effect(&(Located_sound_effect_table[i].Effect));
			}
		}

		/* Located sound effects are off */
		Sound_state &= ~LOCATED_SOUND_FX_ON;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_sound_system
 * FUNCTION  : Initialize sound system.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 14:23
 * LAST      : 23.07.95 19:00
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_sound_system(void)
{
	UNLONG Return;
	UNSHORT i;

	/* Has AIL been initialized ? */
	if (No_AIL)
	{
		/* No */
		Sound_state = 0;
	}
	else
	{
		/* Yes */
		Sound_state = SOUNDSYSTEM_ON;
	}

	/* Reset songs */
	Song_song.State = SONG_FREE;
	Ambient_song.State = SONG_FREE;
	Jingle_song.State = SONG_FREE;

	/* Reset digital channel list */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		Digital_channels[i].State = DIGI_CHANNEL_FREE;
	}

	/* Reset sound effect lists */
	for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
	{
		Sound_effect_table[i].State = SOUND_EFFECT_FREE;
	}
	for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
	{
		Located_sound_effect_table[i].Effect.State = SOUND_EFFECT_FREE;
	}

	/* Sound system on / not initialized ? */
	if ((Sound_state & SOUNDSYSTEM_ON) &&
	 !(Sound_state & SOUNDSYSTEM_INITIALIZED))
	{
		/* Yes -> Set Global Timbre Library filename prefix */
		AIL_set_GTL_filename_prefix("ALBISND");

		/* Switch to music driver directory */
		Change_directory("DRIVERS");

		/* Try to install XMIDI driver */
		Return = AIL_install_MDI_INI(&XMIDI_driver);

		/* Act depending on return value */
		switch (Return)
		{
			/* Success */
			case AIL_INIT_SUCCESS:
			{
				/* MIDI is on */
				Sound_state |= MIDI_ON;
				break;
			}
			/* No MIDI INI-file found */
			case AIL_NO_INI_FILE:
			{
				/* Report */
				Music_error(MUSICERR_NO_MDI_INI);
				break;
			}
			/* Error */
			default:
			{
				/* Report */
				Music_error(MUSICERR_AIL_ERROR);

				/* Exit */
				Change_directory("..");

				return FALSE;
			}
		}

		/* Set preferences for playback of digital samples */
		AIL_set_preference(DIG_LATENCY, 30);
		AIL_set_preference(DIG_USE_STEREO, YES);
		AIL_set_preference(DIG_HARDWARE_SAMPLE_RATE, DEFAULT_FREQUENCY);

		/* Install digital driver */
		Return = AIL_install_DIG_INI(&Digital_driver);

		/* Act depending on return value */
		switch (Return)
		{
			/* Success */
			case AIL_INIT_SUCCESS:
			{
				/* Digital sound is on */
				Sound_state |= DIGI_ON;
				break;
			}
			/* No digital INI-file found */
			case AIL_NO_INI_FILE:
			{
				/* Report */
				Music_error(MUSICERR_NO_DIG_INI);
				break;
			}
			/* Error */
			default:
			{
				/* Report */
				Music_error(MUSICERR_AIL_ERROR);

				/* Exit */
				Change_directory("..");

				return FALSE;
			}
		}

		/* Back to normal directory */
		Change_directory("..");

		/* MIDI sound on ? */
		if (Sound_state & MIDI_ON)
		{
			/* Yes -> Allocate normal song sequence handle */
			Song_song.Sequence_handle =
			 AIL_allocate_sequence_handle(XMIDI_driver);

			if (!Song_song.Sequence_handle)
			{
				Music_error(MUSICERR_AIL_ERROR);
			}

			/* Allocate ambient sequence handle */
			/* (this handle must be allocated even if digital sound if off
				because it makes the exit logic much simpler). */
			Ambient_song.Sequence_handle =
			 AIL_allocate_sequence_handle(XMIDI_driver);

			if (!Ambient_song.Sequence_handle)
			{
				Music_error(MUSICERR_AIL_ERROR);
			}

			/* Allocate jingle sequence handle */
			Jingle_song.Sequence_handle =
			 AIL_allocate_sequence_handle(XMIDI_driver);

			if (!Jingle_song.Sequence_handle)
			{
				Music_error(MUSICERR_AIL_ERROR);
			}
		}

		/* Digital sound on ? */
		if (Sound_state & DIGI_ON)
		{
			/* Yes -> Allocate sample handles */
			for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
			{
				Digital_channels[i].Sample_handle =
				 AIL_allocate_sample_handle(Digital_driver);

				if (!Digital_channels[i].Sample_handle)
				{
					Music_error(MUSICERR_AIL_ERROR);
				}
			}
		}

		/* Sound system is initialized */
		Sound_state |= SOUNDSYSTEM_INITIALIZED;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_sound_system
 * FUNCTION  : Exit sound system.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 16:06
 * LAST      : 23.07.95 15:23
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The drivers must be uninstalled in reverse order!!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_sound_system(void)
{
	UNSHORT i;

	/* Sound system initialized ? */
	if (Sound_state & SOUNDSYSTEM_INITIALIZED)
	{
		/* Yes -> Digital sound on ? */
		if (Sound_state & DIGI_ON)
		{
			/* Yes -> Stop all sound effects */
			Stop_all_digital_channels();

			/* Delete all normal sound effects */
			for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
			{
				/* State is not FREE ? */
				if (Sound_effect_table[i].State != SOUND_EFFECT_FREE)
				{
					/* Yes -> Delete sound effect */
					Delete_sound_effect(&(Sound_effect_table[i]));
				}
			}

			/* Delete all located sound effects */
			for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
			{
				/* State is not FREE ? */
				if (Located_sound_effect_table[i].Effect.State !=
				 SOUND_EFFECT_FREE)
				{
					/* Yes -> Delete sound effect */
					Delete_sound_effect(&(Located_sound_effect_table[i].Effect));
				}
			}

			/* Release all sample handles */
			for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
			{
				/* Release sample handle */
				AIL_release_sample_handle(Digital_channels[i].Sample_handle);
			}

			/* Remove digital driver */
			AIL_uninstall_DIG_driver(Digital_driver);

			/* Digital sound is off */
			Sound_state &= ~DIGI_ON;
		}

		/* MIDI sound on ? */
		if (Sound_state & MIDI_ON)
		{
			/* Yes -> Is a song playing ? */
			if (Song_song.State != SONG_FREE)
			{
				/* Yes -> End song */
				End_song(&Song_song);
			}

			/* Digital sound on / is an ambient song playing ? */
			if ((Sound_state & DIGI_ON) && (Ambient_song.State != SONG_FREE))
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
			if (Jingle_song.State != SONG_FREE)
			{
				/* Yes -> End song */
				End_song(&Jingle_song);
			}

			/* Release sequence handles */
			AIL_release_sequence_handle(Song_song.Sequence_handle);
			AIL_release_sequence_handle(Ambient_song.Sequence_handle);
			AIL_release_sequence_handle(Jingle_song.Sequence_handle);

			/* Remove XMIDI driver */
			AIL_uninstall_MDI_driver(XMIDI_driver);

			/* MIDI is off */
			Sound_state &= ~MIDI_ON;
		}

		/* Sound system is no longer initialized */
		Sound_state &= ~SOUNDSYSTEM_INITIALIZED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_sound_system
 * FUNCTION  : Update music and sound effects.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 15:59
 * LAST      : 13.07.95 20:06
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_sound_system(void)
{
	UNSHORT i;

	/* Sound system initialized / music on / MIDI on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & MUSIC_ON) &&
	 (Sound_state & MIDI_ON))
	{
		/* Yes -> Is a jingle playing ? */
		if (Jingle_song.State != SONG_PLAYING)
		{
			/* Yes -> Has it ended ? */
			if (AIL_sequence_status(Jingle_song.Sequence_handle) == SEQ_DONE)
			{
				/* Yes -> End song */
				End_song(&Jingle_song);
			}
		}
	}

	/* Sound system initialized / sound effects on / digital sound on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & SOUND_FX_ON) && (Sound_state & DIGI_ON))
	{
		/* Yes -> Update sound effects */
		Update_sound_effects();
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
 * LAST      : 13.07.95 11:31
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

	/* Sound system initialized / music on / MIDI on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & MUSIC_ON) &&
	 (Sound_state & MIDI_ON))
	{
		/* Yes -> See if this song is already playing */
		if (Song_song.Number == Song_nr)
			return;

		/* Is some song already playing ? */
		if (Song_song.State != SONG_FREE)
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
			Song_song.Number = Song_nr;
			Song_song.Memory_handle = Handle;

			/* Indicate song is playing */
			Song_song.State = SONG_PLAYING;

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
 * LAST      : 13.07.95 11:32
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

	/* Sound system initialized / ambient on / MIDI on / digital sound on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & AMBIENT_ON) &&
	 (Sound_state & MIDI_ON) &&
	 (Sound_state & DIGI_ON))
	{
		/* Yes -> See if this song is already playing */
		if (Ambient_song.Number == Song_nr)
			return;

		/* Is some song already playing ? */
		if (Ambient_song.State != SONG_FREE)
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
			 XMIDI_driver, (void *) Ptr, (LONG) MAX_AMBIENT_DIGI_CHANNELS);

			/* Claim song memory */
			Ptr = MEM_Claim_pointer(Handle);

			/* Initialize sequence */
			if (AIL_init_sequence(Ambient_song.Sequence_handle, Ptr, 0) <= 0)
			{
				Music_error(MUSICERR_AIL_ERROR);
				return;
			}

			/* Insert data in song table */
			Ambient_song.Number = Song_nr;
			Ambient_song.Memory_handle = Handle;

			/* Indicate song is playing */
			Ambient_song.State = SONG_PLAYING;

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
 * LAST      : 13.07.95 11:33
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

	/* Sound system initialized / music on / MIDI on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & MUSIC_ON) &&
	 (Sound_state & MIDI_ON))
	{
		/* Yes -> Is some song already playing ? */
		if (Jingle_song.State != SONG_FREE)
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
			Jingle_song.Number = Song_nr;
			Jingle_song.Memory_handle = Handle;

			/* Indicate song is playing */
			Jingle_song.State = SONG_PLAYING;

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
 * LAST      : 18.07.95 11:52
 * INPUTS    : UNSHORT Effect_nr - Number of sound effect to be played (1...).
 *             UNSHORT Priority - Priority (0...100).
 *             UNSHORT Volume - Volume percentage (0...100).
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
	struct Sound_effect *Effect;
	MEM_HANDLE Handle;
	SILONG New_frequency;
	SILONG Frequency_deviation;
	UNLONG Length;
	SISHORT New_volume;
	SISHORT Volume_deviation;
	UNSHORT Index;
	UNSHORT Lowest;
	UNSHORT i;

	/* Sound system initialized / sound effects on / digital sound on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & DIGI_ON) &&
	 (Sound_state & SOUND_FX_ON))
	{
		/* Find free sound effect */
		Index = 0xFFFF;
		for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
		{
			/* Is this sound effect free ? */
			if (Sound_effect_table[i].State == SOUND_EFFECT_FREE)
			{
				/* Yes -> Found it */
				Index = i;
				break;
			}
		}

		/* Found something ? */
		if (Index == 0xFFFF)
			return;

		/* Load new sample */
		Handle = Load_subfile(DSAMPLE, Effect_nr);
		if (!Handle)
		{
			Music_error(MUSICERR_FILE_LOAD);
			return;
		}

		/* Get sample length */
		Length = MEM_Get_block_size(Handle);

		/* Volume given ? */
		if (!Volume)
		{
			/* No -> Set to default */
			Volume = 100;
		}

		/* Frequency given ? */
		if (!Frequency)
		{
			/* No -> Set to default */
			Frequency = DEFAULT_FREQUENCY;
		}

		/* Any variability ? */
		if (Variability)
		{
			/* Yes -> Implement variability on volume */
			Volume_deviation = (50 * 100) / Variability;
			New_volume = Volume + ((rand() % (2 * Volume_deviation)) -
			 Volume_deviation) / 100;

			/* Clip volume */
			Volume = min(max(New_volume, 50), 150);

			/* Implement variability on frequency */
			Frequency_deviation = (5000 * 100) / Variability;
			New_frequency = Frequency + ((rand() % (2 * Frequency_deviation)) -
			 Frequency_deviation) / 100;

			/* Clip frequency */
			Frequency = min(max(New_frequency, 1000), 44100);
		}

		/* Insert data in sound effect table */
		Effect = &(Sound_effect_table[Index]);

		Effect->Priority 			= Priority;
		Effect->Flags				= 0;
		Effect->Volume 			= Volume;
		Effect->Frequency			= Frequency;
		Effect->Stereo_position	= 64;
		Effect->Memory_handle	= Handle;
		Effect->Length 			= Length;

		/* Set sound effect state to WAITING */
		Effect->State = SOUND_EFFECT_WAITING;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Initialize_located_effects
 * FUNCTION  : Initialize located sound effects.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.07.95 11:27
 * LAST      : 23.07.95 15:26
 * INPUTS    : UNLONG Space_width - Width of effect space.
 *             UNLONG Space_height - Height of effect space.
 *             UNLONG Listening_range - Listening range.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - All previous located sound effects will be deleted.
 *             - The sound effects will be initialized even if sound effects
 *              are off.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Initialize_located_effects(UNLONG Space_width, UNLONG Space_height,
 UNLONG Listening_range)
{
	UNSHORT i;

	/* Sound system initialized / digital sound on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & DIGI_ON))
	{
		/* Yes -> Delete all located sound effects */
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* State is not FREE ? */
			if (Located_sound_effect_table[i].Effect.State != SOUND_EFFECT_FREE)
			{
				/* Yes -> Delete sound effect */
				Delete_sound_effect(&(Located_sound_effect_table[i].Effect));
			}
		}

		/* Store new effect space dimensions */
		Effect_space_width = Space_width;
		Effect_space_height = Space_height;

		/* Store listening range */
		Effect_listening_range = Listening_range;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_located_effect
 * FUNCTION  : Add a located sound effect.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.07.95 11:53
 * LAST      : 23.07.95 15:29
 * INPUTS    : UNSHORT Effect_nr - Number of sound effect to be played (1...).
 *             UNSHORT Priority - Priority (0...100).
 *             UNSHORT Volume - Volume (0...127).
 *             UNSHORT Probability - Probability that the effect is played
 *              (0...100%).
 *             UNSHORT Frequency - Playback frequence in Hz (0 = use default).
 *             UNLONG Effect_X - X-coordinate of effect in effect space.
 *             UNLONG Effect_Y - Y-coordinate of effect in effect space.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The sound effect will not be added if sound effects are off.
 *              This means located sound effects will not appear when sound
 *              effects are switched on.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_located_effect(UNSHORT Effect_nr, UNSHORT Priority, UNSHORT Volume,
 UNSHORT Probability, UNSHORT Frequency, SILONG Effect_X, SILONG Effect_Y)
{
	struct Sound_effect *Effect;
	MEM_HANDLE Handle;
	SILONG New_frequency;
	SILONG Frequency_deviation;
	UNLONG Length;
	SISHORT New_volume;
	SISHORT Volume_deviation;
	UNSHORT Index;
	UNSHORT Lowest;
	UNSHORT i;

	/* Sound system initialized / sound effects on / digital sound on ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & DIGI_ON) &&
	 (Sound_state & SOUND_FX_ON))
	{
		/* Find free sound effect */
		Index = 0xFFFF;
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* Is this sound effect free ? */
			if (Located_sound_effect_table[i].Effect.State ==
			 SOUND_EFFECT_FREE)
			{
				/* Yes -> Found it */
				Index = i;
				break;
			}
		}

		/* Found something ? */
		if (Index == 0xFFFF)
			return;

		/* Load new sample */
		Handle = Load_subfile(DSAMPLE, Effect_nr);
		if (!Handle)
		{
			Music_error(MUSICERR_FILE_LOAD);
			return;
		}

		/* Get sample length */
		Length = MEM_Get_block_size(Handle);

		/* Volume given ? */
		if (!Volume)
		{
			/* No -> Set to default */
			Volume = 100;
		}

		/* Frequency given ? */
		if (!Frequency)
		{
			/* No -> Set to default */
			Frequency = DEFAULT_FREQUENCY;
		}

		/* Insert data in sound effect table */
		Effect = &(Located_sound_effect_table[Index].Effect);

		Effect->Priority 			= 0;
		Effect->Flags				= 0;
		Effect->Volume 			= 0;
		Effect->Frequency 		= Frequency;
		Effect->Stereo_position	= 64;
		Effect->Memory_handle	= Handle;
		Effect->Length 			= Length;

		/* Should this sound effect be looped indefinitely ? */
		if (Probability == 100)
			Effect->Flags |= LOOP_SOUND_EFFECT;

		/* Insert additional data */
		Located_sound_effect_table[Index].Initial_priority = Priority;
		Located_sound_effect_table[Index].Initial_volume = Volume;
		Located_sound_effect_table[Index].Probability = Probability;

		Located_sound_effect_table[Index].Effect_X = Effect_X;
		Located_sound_effect_table[Index].Effect_Y = Effect_Y;

		/* Set sound effect state to WAITING */
		Effect->State = SOUND_EFFECT_WAITING;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_listener_position
 * FUNCTION  : Set position of listener within effect space.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.07.95 12:47
 * LAST      : 18.07.95 12:47
 * INPUTS    : SILONG X - X-coordinate of listener in effect space.
 *             SILONG Y - Y-coordinate of listener in effect space.
 *             UNSHORT Angle - Angle of listener.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The listener position will be set even if sound effects
 *              are off.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_listener_position(SILONG X, SILONG Y, UNSHORT Angle)
{
	/* Set position data */
	Listener_X = X;
	Listener_Y = Y;
	Listener_angle = Angle;
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
 * SEE ALSO  : BBERROR.H
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
 * NAME      : Pause_song
 * FUNCTION  : Pause a playing song.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 18:18
 * LAST      : 14.07.95 18:18
 * INPUTS    : struct Song *Song - Pointer to song data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function does not need to check the sound state, as the
 *              song wouldn't be playing unless everything is in order.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pause_song(struct Song *Song)
{
	/* Is this song playing ? */
	if (Song->State == SONG_PLAYING)
	{
		/* Yes -> Stop sequence */
		AIL_stop_sequence(Song->Sequence_handle);

		/* Set song state to PAUSED */
		Song->State = SONG_PAUSED;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Continue_song
 * FUNCTION  : Continue a paused song.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 18:18
 * LAST      : 14.07.95 18:18
 * INPUTS    : struct Song *Song - Pointer to song data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function does not need to check the sound state, as it
 *              wouldn't be called and the song wouldn't have been paused
 *              unless everything is in order.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Continue_song(struct Song *Song)
{
	/* Is this song paused ? */
	if (Song->State == SONG_PAUSED)
	{
		/* Yes -> Resume sequence */
		AIL_resume_sequence(Song->Sequence_handle);

		/* Set song state to PLAYING */
		Song->State = SONG_PLAYING;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : End_song
 * FUNCTION  : End a song.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 09:52
 * LAST      : 14.07.95 18:19
 * INPUTS    : struct Song *Song - Pointer to song data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function does not need to check the sound state, as the
 *              song wouldn't be playing unless everything is in order.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
End_song(struct Song *Song)
{
	/* Is this song not free ? */
	if (Song->State != SONG_FREE)
	{
		/* Yes -> End sequence */
		AIL_end_sequence(Song->Sequence_handle);

		/* Remove song */
		MEM_Free_pointer(Song->Memory_handle);
		MEM_Free_memory(Song->Memory_handle);

		/* Set song state to FREE */
		Song->State = SONG_FREE;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_sound_effects
 * FUNCTION  : Check all normal and located and sound effects and allocate
 *              digital channels.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.07.95 19:54
 * LAST      : 17.07.95 13:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may assume that it is only called when the
 *              sound state is OK.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_sound_effects(void)
{
	struct Sound_effect *Active_sound_effect_list[MAX_NORMAL_SOUND_EFFECTS +
	 MAX_LOCATED_SOUND_EFFECTS];
	UNSHORT Nr_active_sound_effects;
	UNSHORT i;

	/* Stop all sound effects that have ended */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		/* Is this channel playing ? */
		if (Digital_channels[i].State == DIGI_CHANNEL_PLAYING)
		{
			/* Yes -> Has the sample ended ? */
			if (AIL_sample_status(Digital_channels[i].Sample_handle) ==
			 SMP_DONE)
			{
				/* Yes -> Stop */
				Stop_digital_channel(i);
			}
		}
	}

	/* Delete all normal sound effects that have been stopped */
	for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
	{
		/* State is STOPPED ? */
		if (Sound_effect_table[i].State == SOUND_EFFECT_STOPPED)
		{
			/* Yes -> Delete sound effect */
			Delete_sound_effect(&(Sound_effect_table[i]));
		}
	}

	/* Located sound effects on ? */
	if (Sound_state & LOCATED_SOUND_FX_ON)
	{
		/* Yes -> Determine if any located sound effects should be started */
		Update_located_sound_effects();
	}

	/* Clear active sound effect counter */
	Nr_active_sound_effects = 0;

	/* Add waiting normal sound effects to active list */
	for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
	{
		/* State is WAITING or PLAYING ? */
		if ((Sound_effect_table[i].State == SOUND_EFFECT_WAITING) ||
		 (Sound_effect_table[i].State == SOUND_EFFECT_PLAYING))
		{
			/* Yes -> Add to list */
			Active_sound_effect_list[Nr_active_sound_effects] =
			 &(Sound_effect_table[i]);

			/* Count up */
			Nr_active_sound_effects++;
		}
	}

	/* Located sound effects on ? */
	if (Sound_state & LOCATED_SOUND_FX_ON)
	{
		/* Yes -> Add waiting located sound effects to active list */
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* State is WAITING or PLAYING ? */
			if ((Located_sound_effect_table[i].Effect.State ==
			 SOUND_EFFECT_WAITING) ||
			 (Located_sound_effect_table[i].Effect.State ==
			 SOUND_EFFECT_PLAYING))
			{
				/* Yes -> Add to list */
				Active_sound_effect_list[Nr_active_sound_effects] =
				 &(Located_sound_effect_table[i].Effect);

				/* Count up */
				Nr_active_sound_effects++;
			}
		}
	}

	/* Sort the list of active sound effects */
	Shuttlesort(Swap_sound_effects, Compare_sound_effects,
	 Nr_active_sound_effects, (UNBYTE *) &(Active_sound_effect_list[0]));

	/* Stop any sound effects that are playing now, but
	 are no longer among the top few (if any) */
	if (Nr_active_sound_effects > MAX_FX_DIGI_CHANNELS)
	{
		for (i=MAX_FX_DIGI_CHANNELS;i<Nr_active_sound_effects;i++)
		{
			/* State is PLAYING ? */
			if (Active_sound_effect_list[i]->State == SOUND_EFFECT_PLAYING)
			{
				/* Yes -> Stop sound effect */
				Stop_sound_effect(Active_sound_effect_list[i]);
			}
		}
	}

	/* Then start any sound effects that are not playing now, but
	 are among the top few (if any) */
	for (i=0;i<min(MAX_FX_DIGI_CHANNELS, Nr_active_sound_effects);i++)
	{
		/* Act depending on sound effect state */
		switch (Active_sound_effect_list[i]->State)
		{
			/* Waiting to be played */
			case SOUND_EFFECT_WAITING:
			{
				/* Start it */
				Start_sound_effect(Active_sound_effect_list[i]);
				break;
			}
			/* Already playing */
			case SOUND_EFFECT_PLAYING:
			{
				/* Update it */
				Update_sound_effect(Active_sound_effect_list[i]);
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Swap_sound_effects
 * FUNCTION  : Swap two sound effects (for sorting).
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.07.95 16:36
 * LAST      : 17.07.95 13:48
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Swap_sound_effects(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Sound_effect *T, **List;

	List = (struct Sound_effect **) Data;

	/* Swap pointers to sound effects */
	T = List[A];
	List[A] = List[B];
	List[B] = T;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_sound_effects
 * FUNCTION  : Compare two sound effects (for sorting).
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.07.95 16:36
 * LAST      : 17.07.95 13:49
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : BOOLEAN : element A should come after element B.
 * BUGS      : No known.
 * NOTES     : - This function will make sure the sound effect with the
 *              highest priority comes first. If the priorities are the same,
 *              the effect that's already playing (if any) will come first.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_sound_effects(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Sound_effect *T, **List;

	List = (struct Sound_effect **) Data;

	/* Do these sound effects have the same priority ? */
	if (List[A]->Priority == List[B]->Priority)
	{
		/* Yes -> Element that is already playing comes first */
		return (List[B]->State == SOUND_EFFECT_PLAYING);
	}
	else
	{
		/* No -> Element with the highest priority comes first */
		return (List[A]->Priority < List[B]->Priority);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_located_sound_effects
 * FUNCTION  : Determine if any located sound effects should be started.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.07.95 11:59
 * LAST      : 18.07.95 18:07
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may assume that it is only called when the
 *              sound state is OK.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_located_sound_effects(void)
{
	double Angle;
	SILONG X, Y;
	SILONG New_X;
	UNLONG Length;
	SISHORT Stereo;
	UNSHORT Volume;
	UNSHORT i;

	/* Check located sound effects */
	for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
	{
		/* Skip if state is FREE */
		if (Located_sound_effect_table[i].Effect.State == SOUND_EFFECT_FREE)
			continue;

		/* State is STOPPED ? */
		if (Located_sound_effect_table[i].Effect.State == SOUND_EFFECT_STOPPED)
		{
			/* Yes -> Is this a one-shot sound effect ? */
			if (!Located_sound_effect_table[i].Probability)
			{
				/* Yes -> Delete sound effect */
				Delete_sound_effect(&(Located_sound_effect_table[i].Effect));
				continue;
			}

			/* Should this sound effect be started again ? */
			if ((rand() % 100) > Located_sound_effect_table[i].Probability)
			{
				/* No -> Skip */
				continue;
			}
		}

		/* Calculate coordinates of effect relative to the listener */
		X = Located_sound_effect_table[i].Effect_X - Listener_X;
		Y = Located_sound_effect_table[i].Effect_Y - Listener_Y;

		/* Calculate vector length */
		Length = (X * X) + (Y * Y);
		Length = (SILONG) sqrt((double) Length);

		/* Too far away ? */
		if (Length > Effect_listening_range)
		{
			/* Yes -> Set state to STOPPED */
			Stop_sound_effect(&(Located_sound_effect_table[i].Effect));
		}
		else
		{
			/* No -> Calculate volume depending on distance */
			Volume = (Located_sound_effect_table[i].Initial_volume *
			 (100 - ((Length * 100) / Effect_listening_range))) / 100;

			/* Set volume */
			Located_sound_effect_table[i].Effect.Volume = Volume;

			/* Set priority depending on distance */
			Located_sound_effect_table[i].Effect.Priority =
			 (Located_sound_effect_table[i].Initial_priority *
			 (100 - ((Length * 100) / Effect_listening_range))) / 100;

			/* Rotate effect around the listener */
			Angle = ((double) Listener_angle * 2 * PI) / 360;

			New_X = (SILONG) (((double) X * cos(Angle)) -
			 ((double) Y * sin(Angle)));

			/* Calculate stereo position depending on effect position
			 (there's an extra distortion here to increase the 3D effect) */
			Stereo = 64 + ((New_X * 128) / (SILONG) Effect_listening_range);
			Stereo = min(max(Stereo, 0), 127);

			/* Set stereo position */
			Located_sound_effect_table[i].Effect.Stereo_position = Stereo;

			/* Set state to WAITING, unless it is already PLAYING */
			if (Located_sound_effect_table[i].Effect.State !=
				SOUND_EFFECT_PLAYING)
			{
				Located_sound_effect_table[i].Effect.State =	SOUND_EFFECT_WAITING;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Start_sound_effect
 * FUNCTION  : Start playing a sound effect.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.07.95 17:44
 * LAST      : 19.07.95 10:57
 *	INPUTS    : struct Sound_effect *Effect - Pointer to sound effect data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may assume that it is only called when the
 *              sound state is OK.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Start_sound_effect(struct Sound_effect *Effect)
{
	struct Digital_channel *Channel;
	UNSHORT Channel_index;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Exit if this effect is not waiting to be played */
	if (Effect->State != SOUND_EFFECT_WAITING)
		return;

	/* Exit if a digital channel is already playing this effect */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		if ((Digital_channels[i].State != DIGI_CHANNEL_FREE) &&
		 (Digital_channels[i].Effect_ptr == Effect))
		{
			return;
		}
	}

	/* Find a free digital channel */
	Channel_index = 0xFFFF;
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		/* Is this channel free ? */
		if (Digital_channels[i].State == DIGI_CHANNEL_FREE)
		{
			/* Yes -> Found it */
			Channel_index = i;
			break;
		}
	}

	/* Found anything ? */
	if (Channel_index != 0xFFFF)
	{
		/* Yes -> Get channel data */
		Channel = &(Digital_channels[Channel_index]);

		/* Create reference to sound effect */
		Channel->Effect_ptr = Effect;

		/* Set digital channel state to playing */
		Channel->State = DIGI_CHANNEL_PLAYING;

		/* Set sound effect state to playing */
		Effect->State = SOUND_EFFECT_PLAYING;

		/* Claim sample memory */
		Ptr = MEM_Claim_pointer(Effect->Memory_handle);

		/* Initialize sample */
		AIL_init_sample(Channel->Sample_handle);
		AIL_set_sample_address(Channel->Sample_handle, (void *) Ptr,
		 (ULONG) Effect->Length);

		/* Set sample parameters */
		AIL_set_sample_volume(Channel->Sample_handle, (Effect->Volume *
		 Sound_effects_volume) / 100);
		AIL_set_sample_playback_rate(Channel->Sample_handle,
		 (LONG) Effect->Frequency);
		AIL_set_sample_pan(Channel->Sample_handle,
		 (LONG) Effect->Stereo_position);

		/* Should this sample be looped indefinitely ? */
		if (Effect->Flags & LOOP_SOUND_EFFECT)
		{
			/* Yes */
			AIL_set_sample_loop_count(Channel->Sample_handle, (LONG) 0);
		}

		/* Start sample */
		AIL_start_sample(Channel->Sample_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_sound_effect
 * FUNCTION  : Update a playing sound effect.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.07.95 18:15
 * LAST      : 19.07.95 10:57
 *	INPUTS    : struct Sound_effect *Effect - Pointer to sound effect data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may assume that it is only called when the
 *              sound state is OK.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_sound_effect(struct Sound_effect *Effect)
{
	struct Digital_channel *Channel;
	UNSHORT i;

	/* Exit if this effect is not playing */
	if (Effect->State != SOUND_EFFECT_PLAYING)
		return;

	/* Check all digital channels */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		/* Is this channel playing / is it playing the sound effect ? */
		if ((Digital_channels[i].State == DIGI_CHANNEL_PLAYING) &&
		 (Digital_channels[i].Effect_ptr == Effect))
		{
			/* Yes -> Get channel data */
			Channel = &(Digital_channels[i]);

			/* Update sample parameters */
			AIL_set_sample_volume(Channel->Sample_handle, (Effect->Volume *
			 Sound_effects_volume) / 100);
			AIL_set_sample_playback_rate(Channel->Sample_handle,
			 (LONG) Effect->Frequency);
			AIL_set_sample_pan(Channel->Sample_handle,
			 (LONG) Effect->Stereo_position);

			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Stop_sound_effect
 * FUNCTION  : Stop playing a sound effect.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.07.95 14:20
 * LAST      : 19.07.95 11:00
 *	INPUTS    : struct Sound_effect *Effect - Pointer to sound effect data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will set the sound effect's state to STOPPED,
 *              even if the sound effect wasn't playing to begin with.
 *             - This function does not need to check the sound state, as the
 *              effect wouldn't be playing unless everything is in order.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Stop_sound_effect(struct Sound_effect *Effect)
{
	UNSHORT i;

	/* Exit if this sound effect is FREE */
	if (Effect->State == SOUND_EFFECT_FREE)
		return;

	/* Is this sound effect playing ? */
	if (Effect->State == SOUND_EFFECT_PLAYING)
	{
		/* Yes -> Check all digital channels */
		for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
		{
			/* Is this channel playing / is it playing the sound effect ? */
			if ((Digital_channels[i].State == DIGI_CHANNEL_PLAYING) &&
			 (Digital_channels[i].Effect_ptr == Effect))
			{
				/* Yes -> Stop playing this channel */
				Stop_digital_channel(i);
				break;
			}
		}
	}

	/* Set sound effect's state to STOPPED */
	Effect->State = SOUND_EFFECT_STOPPED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_sound_effect
 * FUNCTION  : Delete a sound effect.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.07.95 14:29
 * LAST      : 23.07.95 15:38
 *	INPUTS    : struct Sound_effect *Effect - Pointer to sound effect data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may safely ignore the sound state.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Delete_sound_effect(struct Sound_effect *Effect)
{
	/* Exit if this sound effect is FREE */
	if (Effect->State == SOUND_EFFECT_FREE)
		return;

	/* Is this sound effect playing ? */
	if (Effect->State == SOUND_EFFECT_PLAYING)
	{
		/* Yes -> Stop it */
		Stop_sound_effect(Effect);
	}

	/* Free sample memory */
	MEM_Free_pointer(Effect->Memory_handle);
	MEM_Free_memory(Effect->Memory_handle);

	/* Set sound effect's state to FREE */
	Effect->State = SOUND_EFFECT_FREE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Stop_all_digital_channels
 * FUNCTION  : Stop all digital channels that are currently playing.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 18:01
 * LAST      : 19.07.95 11:01
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Stop_all_digital_channels(void)
{
	UNSHORT i;

	/* Stop all digital channels */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		Stop_digital_channel(i);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Stop_digital_channel
 * FUNCTION  : Stop playing a digital channel.
 * FILE      : MUSIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.07.95 14:26
 * LAST      : 15.07.95 14:26
 *	INPUTS    : UNSHORT Channel_index - Digital channel index (0...
 *              MAX_FX_DIGI_CHANNELS - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Stop_digital_channel(UNSHORT Channel_index)
{
	/* Exit if channel index is illegal */
	if (Channel_index >= MAX_FX_DIGI_CHANNELS)
		return;

	/* Sound system on / digital sound on / sound effects on /
	  is this channel playing ? */
	if ((Sound_state & SOUNDSYSTEM_INITIALIZED) &&
	 (Sound_state & DIGI_ON) &&
	 (Sound_state & SOUND_FX_ON) &&
	 (Digital_channels[Channel_index].State == DIGI_CHANNEL_PLAYING))
	{
		/* Yes -> Has the sample ended ? */
		if (AIL_sample_status(Digital_channels[Channel_index].Sample_handle)
		 != SMP_DONE)
		{
			/* No -> Stop playing */
			AIL_end_sample(Digital_channels[Channel_index].Sample_handle);
		}

		/* Set digital channel state to FREE */
		Digital_channels[Channel_index].State = DIGI_CHANNEL_FREE;

		/* Set corresponding sound effect's state to STOPPED */
		Digital_channels[Channel_index].Effect_ptr->State = SOUND_EFFECT_STOPPED;

		/* Destroy reference to sound effect */
		Digital_channels[Channel_index].Effect_ptr = NULL;
	}
}

