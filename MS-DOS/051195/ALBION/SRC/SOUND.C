/************
 * NAME     : SOUND.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 2-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : SOUND.H
 ************/

/*
 ** Includes ***************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <BBDEF.H>
#include <BBERROR.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <SORT.H>
#include <XLOAD.H>

#include <MAIN.H>
#include <GAMEVAR.H>
#include <SOUND.H>
#include <XFTYPES.H>

#include <AIL.H>

/*
 ** Defines ****************************************************************
 */

/* Bitmasks for SOUND_State */

/* This flag is set when AIL has been initialized successfully */
#define SOUNDSYSTEM_ON				(1L << 0)

/* This flag is set when the sound system has been initialized successfully.
	It can only be set when the sound system is on. */
#define SOUNDSYSTEM_INITIALIZED	(1L << 1)

/* These flags are set when the XMIDI and digital sound subsystems have
	been initialized successfully.
	They can only be set when the sound system is on and initialized. */
#define MIDI_ON						(1L << 2)
#define DIGI_ON						(1L << 3)

/* This flag controls the sound effects. The access functions that access
   this bit can be called at any time, but will only have an effect when
   digital sound is on. */
#define SOUND_FX_ON					(1L << 4)

/* This flag controls the located sound effects. It can only be set when
   sound effects are on, and is used to switch off the located sound effects
   when they are not desired, for instance in the combat screen. */
#define LOCATED_SOUND_FX_ON		(1L << 5)

/* Default sample frequency */
#define DEFAULT_FREQUENCY				(11000)

/* Digital channel maxima */
#define MAX_AMBIENT_DIGI_CHANNELS	(4)
#define MAX_FX_DIGI_CHANNELS			(12)

/* Sound effect maxima */
#define MAX_NORMAL_SOUND_EFFECTS		(8)
#define MAX_LOCATED_SOUND_EFFECTS	(64)

/* Error codes */
#define SOUND_ERR_AIL_ERROR		(1)
#define SOUND_ERR_FILE_LOAD		(2)

#define SOUND_ERR_MAX				(2)

/*
 ** Enumerations ***********************************************************
 */

/* Song states */
enum eSong_state {
	SONG_FREE,
	SONG_PLAYING,
	SONG_PAUSED
};

/* Sound effect states */
enum eSound_effect_state {
	SOUND_EFFECT_FREE,
	SOUND_EFFECT_WAITING,
	SOUND_EFFECT_PLAYING,
	SOUND_EFFECT_STOPPED
};

/* Digital channel states */
enum eDigital_channel_state {
	DIGI_CHANNEL_FREE,
	DIGI_CHANNEL_PLAYING
};

/*
 ** Type definitions *******************************************************
 */

typedef enum eSong_state SONG_STATE_T;
typedef enum eSound_effect_state SOUND_EFFECT_STATE_T;
typedef enum eDigital_channel_state DIGI_CHANNEL_STATE_T;

/*
 ** Structure definitions **************************************************
 */

/* Song structure */
struct Song {
	SONG_STATE_T State;
	UNSHORT Flags;

	UNSHORT Number;

	MEM_HANDLE Song_memory_handle;
	HSEQUENCE Sequence_handle;

	MEM_HANDLE Wavelib_memory_handle;
	HWAVE Wave_synth;
};

/* Bitmasks for Song.Flags */
#define AMBIENT_SONG				(1 << 0)

/* Sound effect structure */
struct Sound_effect {
	SOUND_EFFECT_STATE_T State;
	UNSHORT Flags;
	UNSHORT Priority;

	UNSHORT Effect_nr;

	UNSHORT Volume;
	UNSHORT Frequency;
	UNSHORT Stereo_position;

	MEM_HANDLE Sample_memory_handle;
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
void SOUND_Error(UNSHORT Error_code);
void SOUND_Print_error(UNCHAR *buffer, UNBYTE *data);

/* Music support functions */
void SOUND_Pause_song(struct Song *Song);
void SOUND_Continue_song(struct Song *Song);
void SOUND_End_song(struct Song *Song);

/* Sound effect management functions */
void SOUND_Update_sound_effects(void);

void SOUND_Swap_sound_effects(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN SOUND_Compare_sound_effects(UNLONG A, UNLONG B, UNBYTE *Data);

void SOUND_Update_located_sound_effects(void);

/* Sound effect support functions */
void SOUND_Start_sound_effect(struct Sound_effect *Sound_effect);
void SOUND_Update_sound_effect(struct Sound_effect *Effect);
void SOUND_Stop_sound_effect(struct Sound_effect *Effect);
void SOUND_Delete_sound_effect(struct Sound_effect *Effect);

/* Digital channel support functions */
void SOUND_Stop_all_digital_channels(void);
void SOUND_Stop_digital_channel(UNSHORT Channel_index);

/*
 ** Global variables *******************************************************
 */

/* Global soundsystem state */
static UNLONG SOUND_State;

/* Global volumes */
static UNSHORT SOUND_Global_MIDI_volume = 127;
static UNSHORT SOUND_Global_digital_volume = 127;

/* XMIDI and digital drivers */
static MDI_DRIVER *SOUND_XMIDI_driver;
static DIG_DRIVER *SOUND_Digital_driver;

/* Song data */
static struct Song SOUND_Song_song;
static struct Song SOUND_Ambient_song;
static struct Song SOUND_Jingle_song;

/* Sound effect listening range */
static UNLONG SOUND_Effect_listening_range;

/* Located sound effects restart counter */
static UNSHORT SOUND_Located_sound_effects_restart_counter;

/* Position of listener within effect space */
static SILONG SOUND_Listener_X;
static SILONG SOUND_Listener_Y;
static UNSHORT SOUND_Listener_angle;

/* Digital channel data */
static struct Digital_channel SOUND_Digital_channels[MAX_FX_DIGI_CHANNELS];

/* Sound effect data */
static struct Sound_effect SOUND_Sound_effects[MAX_NORMAL_SOUND_EFFECTS];
static struct Located_sound_effect SOUND_Located_sound_effects[MAX_LOCATED_SOUND_EFFECTS];

/* Error handling data */
static UNCHAR SOUND_Library_name[] = "Sound";

static UNCHAR *SOUND_Error_strings[SOUND_ERR_MAX + 1] = {
	"Illegal error code.",
	"AIL error : %s",
	"File could not be loaded."
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Sound_on
 * FUNCTION  : Switch music and sound effects on.
 * FILE      : SOUND.C
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
SOUND_Sound_on(void)
{
	/* Switch music and sound effects on */
	SOUND_Music_on();
	SOUND_Sound_effects_on();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Sound_off
 * FUNCTION  : Switch music and sound effects off.
 * FILE      : SOUND.C
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
SOUND_Sound_off(void)
{
	/* Switch music and sound effects off */
	SOUND_Music_off();
	SOUND_Sound_effects_off();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Music_on
 * FUNCTION  : Switch music on.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 20.10.95 11:54
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Music_on(void)
{
	/* Sound system initialized / MIDI on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & MIDI_ON))
	{
		/* Yes -> Continue all normal songs */
		SOUND_Continue_song(&SOUND_Song_song);
		SOUND_Continue_song(&SOUND_Jingle_song);

		/* Digital sound on ? */
		if (SOUND_State & DIGI_ON)
		{
			/* Yes -> Continue ambient song */
			SOUND_Continue_song(&SOUND_Ambient_song);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Music_on
 * FUNCTION  : Switch music off.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 20.10.95 11:55
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Music_off(void)
{
	/* Sound system initialized / MIDI on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & MIDI_ON))
	{
		/* Yes -> Pause all normal songs */
		SOUND_Pause_song(&SOUND_Song_song);
		SOUND_Pause_song(&SOUND_Jingle_song);

		/* Digital sound on ? */
		if (SOUND_State & DIGI_ON)
		{
			/* Yes -> Pause ambient song */
			SOUND_Pause_song(&SOUND_Ambient_song);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Sound_effects_on
 * FUNCTION  : Switch sound effects on.
 * FILE      : SOUND.C
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
SOUND_Sound_effects_on(void)
{
	/* Sound system initialized / digital sound on / sound effects off ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & DIGI_ON) &&
	 !(SOUND_State & SOUND_FX_ON))
	{
		/* Yes -> Sound effects are on */
		SOUND_State |= SOUND_FX_ON;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Sound_effects_off
 * FUNCTION  : Switch sound effects off.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 20.10.95 11:56
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Sound_effects_off(void)
{
	UNSHORT i;

	/* Sound system initialized / digital sound on / sound effects on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & DIGI_ON) &&
	 (SOUND_State & SOUND_FX_ON))
	{
		/* Yes -> Stop all sound effects */
		SOUND_Stop_all_digital_channels();

		/* Delete all normal sound effects.
			The effects are deleted directly. If they were stopped, they
			would immediately be deleted by the sound effect logic. */
		for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
		{
			/* State is not FREE ? */
			if (SOUND_Sound_effects[i].State != SOUND_EFFECT_FREE)
			{
				/* Yes -> Delete sound effect */
				SOUND_Delete_sound_effect(&(SOUND_Sound_effects[i]));
			}
		}

		/* Stop all located sound effects.
		   These effects are stopped because they may be restarted at a later
		   time. */
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* State is not FREE ? */
			if (SOUND_Located_sound_effects[i].Effect.State != SOUND_EFFECT_FREE)
			{
				/* Yes -> Stop sound effect */
				SOUND_Stop_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
			}
		}

		/* Sound effects are off */
		SOUND_State &= ~SOUND_FX_ON;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Located_sound_effects_on
 * FUNCTION  : Switch located sound effects on.
 * FILE      : SOUND.C
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
SOUND_Located_sound_effects_on(void)
{
	/* Sound system initialized / digital sound on / sound effects on /
	 located sound effects off ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & DIGI_ON) &&
	 (SOUND_State & SOUND_FX_ON) &&
	 !(SOUND_State & LOCATED_SOUND_FX_ON))
	{
		/* Yes -> Located sound effects are on */
		SOUND_State |= LOCATED_SOUND_FX_ON;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Located_sound_effects_off
 * FUNCTION  : Switch located sound effects off.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.07.95 17:54
 * LAST      : 18.09.95 16:45
 *	INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Located_sound_effects_off(void)
{
	UNSHORT i;

	/* Sound system initialized / digital sound on / sound effects on /
	 located sound effects on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & DIGI_ON) &&
	 (SOUND_State & SOUND_FX_ON) &&
	 (SOUND_State & LOCATED_SOUND_FX_ON))
	{
		/* Yes -> Stop all located sound effects */
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* State is not FREE ? */
			if (SOUND_Located_sound_effects[i].Effect.State != SOUND_EFFECT_FREE)
			{
				/* Yes -> Stop sound effect */
				SOUND_Stop_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
			}
		}

		/* Located sound effects are off */
		SOUND_State &= ~LOCATED_SOUND_FX_ON;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Get_MIDI_volume
 * FUNCTION  : Get global MIDI music volume.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:11
 * LAST      : 12.10.95 21:38
 *	INPUTS    : None.
 * RESULT    : UNSHORT : Volume (0...127).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
SOUND_Get_MIDI_volume(void)
{
	return SOUND_Global_MIDI_volume;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Set_MIDI_volume
 * FUNCTION  : Set global MIDI music volume.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:12
 * LAST      : 12.10.95 21:33
 *	INPUTS    : UNSHORT New_volume - New volume (0...127).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Although the ambient music is played as an XMIDI track, to
 *              the user it appears to be a sound effect. Therefore it's
 *              volume is not changed here.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Set_MIDI_volume(UNSHORT New_volume)
{
	/* Set MIDI volume */
	SOUND_Global_MIDI_volume = New_volume;

	/* Sound system initialized / MIDI sound on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & MIDI_ON))
	{
		/* Yes -> Is some song already playing ? */
		if (SOUND_Song_song.State != SONG_FREE)
		{
			/* Yes -> Set the sequence volume */
			AIL_set_sequence_volume
			(
				SOUND_Song_song.Sequence_handle,
				(LONG) SOUND_Global_MIDI_volume,
				(LONG) 0
			);
		}

		/* Is some jingle already playing ? */
		if (SOUND_Jingle_song.State != SONG_FREE)
		{
  			/* Yes -> Set the sequence volume */
			AIL_set_sequence_volume
			(
				SOUND_Jingle_song.Sequence_handle,
				(LONG) SOUND_Global_MIDI_volume,
				(LONG) 0
			);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Get_digital_volume
 * FUNCTION  : Get global digital sound volume.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:12
 * LAST      : 12.10.95 21:33
 *	INPUTS    : None.
 * RESULT    : UNSHORT : Volume (0...127).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
SOUND_Get_digital_volume(void)
{
	return SOUND_Global_digital_volume;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Set_digital_volume
 * FUNCTION  : Set global digital sound volume.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:12
 * LAST      : 12.10.95 21:38
 *	INPUTS    : UNSHORT New_volume - New volume (0...127).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The digital master volume will automatically influence
 *              the volume of any ambient songs.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Set_digital_volume(UNSHORT New_volume)
{
	/* Set digital volume */
	SOUND_Global_digital_volume = New_volume;

	/* Sound system initialized / digital sound on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & DIGI_ON))
	{
		/* Yes -> Set master digital volume */
		AIL_set_digital_master_volume
		(
			SOUND_Digital_driver,
			(LONG) New_volume
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Init_sound_system
 * FUNCTION  : Initialize sound system.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 14:23
 * LAST      : 02.11.95 15:12
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
SOUND_Init_sound_system(void)
{
	UNLONG Return;
	UNSHORT i;

	/* Has AIL been initialized ? */
	if (No_AIL)
	{
		/* No */
		SOUND_State = 0;
	}
	else
	{
		/* Yes */
		SOUND_State = SOUNDSYSTEM_ON;
	}

	/* Reset variables */
	SOUND_Effect_listening_range						= 1;
	SOUND_Located_sound_effects_restart_counter	= 0;

	/* Reset volumes */
	SOUND_Global_MIDI_volume		= 127;
	SOUND_Global_digital_volume	= 127;

	/* Clear song structures */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &SOUND_Song_song,
		sizeof(struct Song),
		0
	);
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &SOUND_Ambient_song,
		sizeof(struct Song),
		0
	);
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &SOUND_Jingle_song,
		sizeof(struct Song),
		0
	);

	/* Reset songs */
	SOUND_Song_song.State		= SONG_FREE;
	SOUND_Ambient_song.State	= SONG_FREE;
	SOUND_Jingle_song.State		= SONG_FREE;

	/* Reset digital channels */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		/* Clear digital channel structure */
		BASEMEM_FillMemByte
		(
			(UNBYTE *) &(SOUND_Digital_channels[i]),
			sizeof(struct Digital_channel),
			0
		);

		/* Reset digital channel state */
		SOUND_Digital_channels[i].State = DIGI_CHANNEL_FREE;
	}

	/* Reset sound effect list */
	for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
	{
		/* Clear sound effect structure */
		BASEMEM_FillMemByte
		(
			(UNBYTE *) &(SOUND_Sound_effects[i]),
			sizeof(struct Sound_effect),
			0
		);

		/* Reset sound effect state */
		SOUND_Sound_effects[i].State = SOUND_EFFECT_FREE;
	}

	/* Reset located sound effect list */
	for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
	{
		/* Clear located sound effect structure */
		BASEMEM_FillMemByte
		(
			(UNBYTE *) &(SOUND_Located_sound_effects[i]),
			sizeof(struct Located_sound_effect),
			0
		);

		/* Reset located sound effect state */
		SOUND_Located_sound_effects[i].Effect.State = SOUND_EFFECT_FREE;
	}

	/* Sound system on / not initialized ? */
	if ((SOUND_State & SOUNDSYSTEM_ON) &&
	 !(SOUND_State & SOUNDSYSTEM_INITIALIZED))
	{
		/* Yes -> Set Global Timbre Library filename prefix */
		AIL_set_GTL_filename_prefix("ALBISND");

		/* Select data path 0 */
		Select_data_path(0);

		/* Switch to music driver directory */
		Change_directory("DRIVERS");

		/* Try to install XMIDI driver */
		Return = AIL_install_MDI_INI(&SOUND_XMIDI_driver);

		/* Act depending on return value */
		switch (Return)
		{
			/* Success */
			case AIL_INIT_SUCCESS:
			{
				/* MIDI is on */
				SOUND_State |= MIDI_ON;
				break;
			}
			/* No MIDI INI-file found */
			case AIL_NO_INI_FILE:
			{
				break;
			}
			/* Error */
			default:
 			{
				/* Report */
				SOUND_Error(SOUND_ERR_AIL_ERROR);

				/* Exit */
				Change_directory("..");

				return FALSE;
			}
		}

		/* Set preferences for playback of digital samples */
		AIL_set_preference(DIG_LATENCY,					30);
		AIL_set_preference(DIG_USE_STEREO,				YES);
		AIL_set_preference(DIG_USE_16_BITS,				YES);
		AIL_set_preference(DIG_HARDWARE_SAMPLE_RATE,	DEFAULT_FREQUENCY);

		/* Install digital driver */
		Return = AIL_install_DIG_INI(&SOUND_Digital_driver);

		/* Act depending on return value */
		switch (Return)
		{
			/* Success */
			case AIL_INIT_SUCCESS:
			{
				/* Digital sound is on */
				SOUND_State |= DIGI_ON;
				break;
			}
			/* No digital INI-file found */
			case AIL_NO_INI_FILE:
			{
				break;
			}
			/* Error */
			default:
			{
				/* Report */
				SOUND_Error(SOUND_ERR_AIL_ERROR);

				/* Exit */
				Change_directory("..");

				return FALSE;
			}
		}

		/* Back to normal directory */
		Change_directory("..");

		/* MIDI sound on ? */
		if (SOUND_State & MIDI_ON)
		{
			/* Yes -> Allocate normal song sequence handle */
			SOUND_Song_song.Sequence_handle =
			 AIL_allocate_sequence_handle(SOUND_XMIDI_driver);

			if (!SOUND_Song_song.Sequence_handle)
			{
				SOUND_Error(SOUND_ERR_AIL_ERROR);
			}

			/* Allocate ambient sequence handle */
			/* (this handle must be allocated even if digital sound is off
				because it makes the exit logic much simpler). */
			SOUND_Ambient_song.Sequence_handle =
			 AIL_allocate_sequence_handle(SOUND_XMIDI_driver);

			if (!SOUND_Ambient_song.Sequence_handle)
			{
				SOUND_Error(SOUND_ERR_AIL_ERROR);
			}

			/* Allocate jingle sequence handle */
			SOUND_Jingle_song.Sequence_handle =
			 AIL_allocate_sequence_handle(SOUND_XMIDI_driver);

			if (!SOUND_Jingle_song.Sequence_handle)
			{
				SOUND_Error(SOUND_ERR_AIL_ERROR);
			}
		}

		/* Digital sound on ? */
		if (SOUND_State & DIGI_ON)
		{
			/* Yes -> Allocate sample handles */
			for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
			{
				SOUND_Digital_channels[i].Sample_handle =
				 AIL_allocate_sample_handle(SOUND_Digital_driver);

				if (!SOUND_Digital_channels[i].Sample_handle)
				{
					SOUND_Error(SOUND_ERR_AIL_ERROR);
				}
			}
		}

		/* Sound system is initialized */
		SOUND_State |= SOUNDSYSTEM_INITIALIZED;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Exit_sound_system
 * FUNCTION  : Exit sound system.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 16:06
 * LAST      : 02.11.95 15:12
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The drivers MUST be uninstalled in reverse order!!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Exit_sound_system(void)
{
	UNSHORT i;

	/* Sound system initialized ? */
	if (SOUND_State & SOUNDSYSTEM_INITIALIZED)
	{
		/* Yes -> Digital sound on ? */
		if (SOUND_State & DIGI_ON)
		{
			/* Yes -> Stop all sound effects */
			SOUND_Stop_all_digital_channels();

			/* Delete all normal sound effects */
			for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
			{
				/* State is not FREE ? */
				if (SOUND_Sound_effects[i].State != SOUND_EFFECT_FREE)
				{
					/* Yes -> Delete sound effect */
					SOUND_Delete_sound_effect(&(SOUND_Sound_effects[i]));
				}
			}

			/* Delete all located sound effects */
			for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
			{
				/* State is not FREE ? */
				if (SOUND_Located_sound_effects[i].Effect.State !=
				 SOUND_EFFECT_FREE)
				{
					/* Yes -> Delete sound effect */
					SOUND_Delete_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
				}
			}

			/* Release all sample handles */
			for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
			{
				/* Release sample handle */
				AIL_release_sample_handle(SOUND_Digital_channels[i].Sample_handle);
			}

			/* Remove digital driver */
			AIL_uninstall_DIG_driver(SOUND_Digital_driver);

			/* Clear digital driver handle */
			SOUND_Digital_driver = NULL;

			/* Digital sound is off */
			SOUND_State &= ~DIGI_ON;
		}

		/* MIDI sound on ? */
		if (SOUND_State & MIDI_ON)
		{
			/* Yes -> Is a song playing ? */
			if (SOUND_Song_song.State != SONG_FREE)
			{
				/* Yes -> End song */
				SOUND_End_song(&SOUND_Song_song);
			}

			/* Digital sound on / is an ambient song playing ? */
			if ((SOUND_State & DIGI_ON) &&
			 (SOUND_Ambient_song.State != SONG_FREE))
			{
				/* Yes -> End song */
				SOUND_End_song(&SOUND_Ambient_song);
			}

			/* Is a jingle playing ? */
			if (SOUND_Jingle_song.State != SONG_FREE)
			{
				/* Yes -> End song */
				SOUND_End_song(&SOUND_Jingle_song);
			}

			/* Release sequence handles */
			AIL_release_sequence_handle(SOUND_Song_song.Sequence_handle);
			AIL_release_sequence_handle(SOUND_Ambient_song.Sequence_handle);
			AIL_release_sequence_handle(SOUND_Jingle_song.Sequence_handle);

			/* Remove XMIDI driver */
			AIL_uninstall_MDI_driver(SOUND_XMIDI_driver);

			/* Clear XMIDI driver handle */
			SOUND_XMIDI_driver = NULL;

			/* MIDI is off */
			SOUND_State &= ~MIDI_ON;
		}

		/* Sound system is no longer initialized */
		SOUND_State &= ~SOUNDSYSTEM_INITIALIZED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Update_sound_system
 * FUNCTION  : Update music and sound effects.
 * FILE      : SOUND.C
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
SOUND_Update_sound_system(void)
{
	/* Sound system initialized / MIDI on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & MIDI_ON))
	{
		/* Yes -> Is a jingle playing ? */
		if (SOUND_Jingle_song.State != SONG_PLAYING)
		{
			/* Yes -> Has it ended ? */
			if (AIL_sequence_status(SOUND_Jingle_song.Sequence_handle) == SEQ_DONE)
			{
				/* Yes -> End song */
				SOUND_End_song(&SOUND_Jingle_song);
			}
		}
	}

	/* Sound system initialized / sound effects on / digital sound on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & SOUND_FX_ON) &&
	 (SOUND_State & DIGI_ON))
	{
		/* Yes -> Update sound effects */
		SOUND_Update_sound_effects();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Play_song
 * FUNCTION  : Start playing a song.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 15:04
 * LAST      : 21.10.95 20:04
 * INPUTS    : UNSHORT Song_nr - Number of song to be played (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Play_song(UNSHORT Song_nr)
{
	MEM_HANDLE Handle;
	long Return;
	UNLONG Length;
	UNBYTE *Ptr;

	/* Sound system initialized / MIDI on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & MIDI_ON))
	{
		/* Yes -> Is some song already playing ? */
		if (SOUND_Song_song.State != SONG_FREE)
		{
			/* Yes -> Is this song already playing ? */
			if (SOUND_Song_song.Number == Song_nr)
			{
				/* Yes -> Exit */
				return;
			}

			/* No -> End current song */
			SOUND_End_song(&SOUND_Song_song);
		}

		/* Any song ? */
		if (Song_nr)
		{
			/* Yes -> Load new song */
			Handle = Load_subfile(SONG, Song_nr);
			if (!Handle)
			{
				SOUND_Error(SOUND_ERR_FILE_LOAD);
				return;
			}

			/* Claim and lock song memory */
			Ptr = MEM_Claim_pointer(Handle);
			Length = MEM_Get_block_size(Handle);

			BASEMEM_Lock_region(Ptr, Length);

			/* Initialize sequence */
			Return = AIL_init_sequence
			(
				SOUND_Song_song.Sequence_handle,
				Ptr,
				0
			);
			if (Return <= 0)
			{
				SOUND_Error(SOUND_ERR_AIL_ERROR);
				return;
			}

			/* Insert data in song table */
			SOUND_Song_song.Flags					&= ~AMBIENT_SONG;
			SOUND_Song_song.Number					= Song_nr;
			SOUND_Song_song.Song_memory_handle	= Handle;

			/* Indicate song is playing */
			SOUND_Song_song.State = SONG_PLAYING;

			/* Start sequence */
			AIL_start_sequence(SOUND_Song_song.Sequence_handle);

			/* Set the sequence volume */
			AIL_set_sequence_volume
			(
				SOUND_Song_song.Sequence_handle,
				(LONG) SOUND_Global_MIDI_volume,
				(LONG) 0
			);

			/* Make sure the sequence repeats forever */
			AIL_set_sequence_loop_count
			(
				SOUND_Song_song.Sequence_handle,
				(LONG) 0
			);
		}
		else
		{
			/* No -> Store the song number */
			SOUND_Song_song.Number = 0;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Play_ambient_song
 * FUNCTION  : Start playing an ambient song.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 10:58
 * LAST      : 21.10.95 20:04
 * INPUTS    : UNSHORT Song_nr - Number of song to be played (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Play_ambient_song(UNSHORT Song_nr)
{
	MEM_HANDLE Song_handle;
	MEM_HANDLE Wavelib_handle;
	HWAVE Wave_synth;
	long Return;
	UNLONG Length;
	UNBYTE *Ptr;

//	return;		// CHECK

	/* Sound system initialized / MIDI on / digital sound on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & MIDI_ON) &&
	 (SOUND_State & DIGI_ON))
	{
		/* Yes -> Is some song already playing ? */
		if (SOUND_Ambient_song.State != SONG_FREE)
		{
			/* Yes -> Is this song already playing ? */
			if (SOUND_Ambient_song.Number == Song_nr)
			{
				/* Yes -> Exit */
				return;
			}

			/* No -> End current song */
			SOUND_End_song(&SOUND_Ambient_song);
		}

		/* Any song ? */
		if (Song_nr)
		{
			/* Yes -> Load new song */
			Song_handle = Load_subfile(SONG, Song_nr);
			if (!Song_handle)
			{
				SOUND_Error(SOUND_ERR_FILE_LOAD);
				return;
			}

			/* Load new wave library */
			Wavelib_handle = Load_subfile(WAVELIB, Song_nr);

			/* Success ? */
			if (!Wavelib_handle)
			{
				/* No -> Remove song memory */
				MEM_Free_memory(Song_handle);

				/* Out of memory ? */
				if (XLD_Last_error == XLD_OUT_OF_MEMORY)
				{
					/* Yes -> Ignore this error */
					ERROR_PopError();
				}
				else
				{
					/* No -> Report error */
					SOUND_Error(SOUND_ERR_FILE_LOAD);
				}

				return;
			}

			/* Claim and lock wave library memory */
			Ptr = MEM_Claim_pointer(Wavelib_handle);
			Length = MEM_Get_block_size(Wavelib_handle);

			BASEMEM_Lock_region(Ptr, Length);

			/* Create wave synthesizer */
			Wave_synth = AIL_create_wave_synthesizer
			(
				SOUND_Digital_driver,
				SOUND_XMIDI_driver,
				(void *) Ptr,
				(LONG) MAX_AMBIENT_DIGI_CHANNELS
			);

			/* Claim and lock song memory */
			Ptr = MEM_Claim_pointer(Song_handle);
			Length = MEM_Get_block_size(Song_handle);

			BASEMEM_Lock_region(Ptr, Length);

			/* Initialize sequence */
			Return = AIL_init_sequence
			(
				SOUND_Ambient_song.Sequence_handle,
				Ptr,
				0
			);
			if (Return <= 0)
			{
				SOUND_Error(SOUND_ERR_AIL_ERROR);
				return;
			}

			/* Insert data in song data */
			SOUND_Ambient_song.Flags						|= AMBIENT_SONG;
			SOUND_Ambient_song.Number						= Song_nr;
			SOUND_Ambient_song.Song_memory_handle		= Song_handle;
			SOUND_Ambient_song.Wavelib_memory_handle	= Wavelib_handle;
			SOUND_Ambient_song.Wave_synth					= Wave_synth;

			/* Indicate song is playing */
			SOUND_Ambient_song.State = SONG_PLAYING;

			/* Start sequence */
			AIL_start_sequence(SOUND_Ambient_song.Sequence_handle);

			/* Set the sequence volume */
			AIL_set_sequence_volume
			(
				SOUND_Ambient_song.Sequence_handle,
				(LONG) SOUND_Global_digital_volume,
				(LONG) 0
			);

			/* Make sure the sequence repeats forever */
			AIL_set_sequence_loop_count
			(
				SOUND_Ambient_song.Sequence_handle,
				(LONG) 0
			);
		}
		else
		{
			/* No -> Store the song number */
			SOUND_Ambient_song.Number = 0;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Play_jingle
 * FUNCTION  : Play a jingle.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 11:10
 * LAST      : 21.10.95 20:04
 * INPUTS    : UNSHORT Song_nr - Number of song to be played (1...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Play_jingle(UNSHORT Song_nr)
{
	MEM_HANDLE Handle;
	long Return;
	UNLONG Length;
	UNBYTE *Ptr;

	/* Sound system initialized / MIDI on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & MIDI_ON))
	{
		/* Yes -> Is some song already playing ? */
		if (SOUND_Jingle_song.State != SONG_FREE)
		{
			/* Yes -> End current song */
			SOUND_End_song(&SOUND_Jingle_song);
		}

		/* Any song ? */
		if (Song_nr)
		{
			/* Yes -> Load new song */
			Handle = Load_subfile(SONG, Song_nr);
			if (!Handle)
			{
				SOUND_Error(SOUND_ERR_FILE_LOAD);
				return;
			}

			/* Claim and lock song memory */
			Ptr = MEM_Claim_pointer(Handle);
			Length = MEM_Get_block_size(Handle);

			BASEMEM_Lock_region(Ptr, Length);

			/* Initialize sequence */
			Return = AIL_init_sequence
			(
				SOUND_Jingle_song.Sequence_handle,
				Ptr,
				0
			);
			if (Return <= 0)
			{
				SOUND_Error(SOUND_ERR_AIL_ERROR);
				return;
			}

			/* Insert data in song table */
			SOUND_Song_song.Flags						&= ~AMBIENT_SONG;
			SOUND_Jingle_song.Number					= Song_nr;
			SOUND_Jingle_song.Song_memory_handle	= Handle;

			/* Indicate song is playing */
			SOUND_Jingle_song.State = SONG_PLAYING;

			/* Start sequence */
			AIL_start_sequence(SOUND_Jingle_song.Sequence_handle);

			/* Set the sequence volume */
			AIL_set_sequence_volume
			(
				SOUND_Jingle_song.Sequence_handle,
				(LONG) SOUND_Global_MIDI_volume,
				(LONG) 0
			);
		}
		else
		{
			/* No -> Store the song number */
			SOUND_Jingle_song.Number = 0;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Play_sound_effect
 * FUNCTION  : Play a sound effect.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 16:36
 * LAST      : 27.07.95 17:30
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
SOUND_Play_sound_effect(UNSHORT Effect_nr, UNSHORT Priority, UNSHORT Volume,
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
	UNSHORT i;

	/* Sound system initialized / sound effects on / digital sound on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & DIGI_ON) &&
	 (SOUND_State & SOUND_FX_ON))
	{
		/* Find free sound effect */
		Index = 0xFFFF;
		for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
		{
			/* Is this sound effect free ? */
			if (SOUND_Sound_effects[i].State == SOUND_EFFECT_FREE)
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

		/* Success ? */
		if (!Handle)
		{
			/* No -> Out of memory ? */
			if (XLD_Last_error == XLD_OUT_OF_MEMORY)
			{
				/* Yes -> Ignore this error */
				ERROR_PopError();
			}
			else
			{
				/* No -> Report error */
				SOUND_Error(SOUND_ERR_FILE_LOAD);
			}
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
		Effect = &(SOUND_Sound_effects[Index]);

		Effect->Priority 					= Priority;
		Effect->Flags						= 0;
		Effect->Effect_nr					= Effect_nr;
		Effect->Volume 					= Volume;
		Effect->Frequency					= Frequency;
		Effect->Stereo_position			= 64;
		Effect->Sample_memory_handle	= Handle;
		Effect->Length 					= Length;

		/* Set sound effect state to WAITING */
		Effect->State = SOUND_EFFECT_WAITING;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Initialize_located_effects
 * FUNCTION  : Initialize located sound effects.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.07.95 11:27
 * LAST      : 26.09.95 14:38
 * INPUTS    : UNLONG Listening_range - Listening range.
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
SOUND_Initialize_located_effects(UNLONG Listening_range)
{
	MEM_HANDLE Handle;
	UNLONG Length;
	UNSHORT i;

	/* Sound system initialized / digital sound on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & DIGI_ON))
	{
		/* Yes -> Loading a game ? */
		if (Loading_game)
		{
			/* Yes -> Load all samples */
			for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
			{
				/* State is not FREE ? */
				if (SOUND_Located_sound_effects[i].Effect.State != SOUND_EFFECT_FREE)
				{
					/* Yes -> Load sample */
					Handle = Load_subfile(DSAMPLE,
					 SOUND_Located_sound_effects[i].Effect.Effect_nr);

					/* Success ? */
					if (Handle)
					{
						/* Yes -> Get sample length */
						Length = MEM_Get_block_size(Handle);

						/* Store data */
						SOUND_Located_sound_effects[i].Effect.Sample_memory_handle	= Handle;
						SOUND_Located_sound_effects[i].Effect.Length 			= Length;
					}
					else
					{
						/* No -> Out of memory ? */
						if (XLD_Last_error == XLD_OUT_OF_MEMORY)
						{
							/* Yes -> Ignore this error */
							ERROR_PopError();
							ERROR_PopError();
						}
						else
						{
							/* No -> Report error */
							SOUND_Error(SOUND_ERR_FILE_LOAD);
						}

						/* Delete sound effect */
						SOUND_Delete_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
					}
				}
			}
		}
		else
		{
			/* No -> Delete all located sound effects */
			for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
			{
				/* State is not FREE ? */
				if (SOUND_Located_sound_effects[i].Effect.State != SOUND_EFFECT_FREE)
				{
					/* Yes -> Delete sound effect */
					SOUND_Delete_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
				}
			}
		}

		/* Store listening range */
		SOUND_Effect_listening_range = max(1, Listening_range);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Load_located_effects
 * FUNCTION  : Load located sound effects.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 14:54
 * LAST      : 26.09.95 16:09
 * INPUTS    : UNBYTE *Ptr - Pointer to saved sound effect data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - All previous located sound effects will be deleted.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Load_located_effects(UNBYTE *Ptr)
{
	UNSHORT i;

	/* Delete all located sound effects */
	for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
	{
		/* State is not FREE ? */
		if (SOUND_Located_sound_effects[i].Effect.State != SOUND_EFFECT_FREE)
		{
			/* Yes -> Delete sound effect */
			SOUND_Delete_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
		}
	}

	/* Copy sound effect data */
	memcpy
	(
		(UNBYTE *) &(SOUND_Located_sound_effects[0]),
		Ptr,
		MAX_LOCATED_SOUND_EFFECTS * sizeof(struct Located_sound_effect)
	);

	/* Prepare data */
	for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
	{
		/* Clear memory handle to avoid errors */
		SOUND_Located_sound_effects[i].Effect.Sample_memory_handle	= NULL;

		/* Clear sample length in case the samples have changed */
		SOUND_Located_sound_effects[i].Effect.Length = 0;

		/* State is not FREE ? */
		if (SOUND_Located_sound_effects[i].Effect.State != SOUND_EFFECT_FREE)
		{
			/* Yes -> Set state to WAITING, so the effect will be re-activated */
			SOUND_Located_sound_effects[i].Effect.State = SOUND_EFFECT_WAITING;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Get_located_effect_table_address
 * FUNCTION  : Get address of located sound effect table.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 15:07
 * LAST      : 26.09.95 15:07
 * INPUTS    : None.
 * RESULT    : UNBYTE * : Pointer to located sound effect table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
SOUND_Get_located_effect_table_address(void)
{
	return ((UNBYTE *) &(SOUND_Located_sound_effects[0]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Get_located_effect_table_length
 * FUNCTION  : Get length of located sound effect table.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 15:08
 * LAST      : 26.09.95 15:08
 * INPUTS    : None.
 * RESULT    : UNLONG : Length of located sound effect table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
SOUND_Get_located_effect_table_length(void)
{
	return (MAX_LOCATED_SOUND_EFFECTS * sizeof(struct Located_sound_effect));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Add_located_effect
 * FUNCTION  : Add a located sound effect.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.07.95 11:53
 * LAST      : 20.10.95 13:48
 * INPUTS    : UNSHORT Effect_nr - Number of sound effect to be played (1...).
 *             UNSHORT Priority - Priority (0...100).
 *             UNSHORT Volume - Volume (0...127).
 *             UNSHORT Probability - Probability that the effect is played
 *              (0...100%).
 *             UNSHORT Frequency - Playback frequence in Hz (0 = use default).
 *             SILONG Effect_X - X-coordinate of effect in effect space.
 *             SILONG Effect_Y - Y-coordinate of effect in effect space.
 * RESULT    : UNSHORT : Located sound effect index (0...) / 0xFFFF.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
SOUND_Add_located_effect(UNSHORT Effect_nr, UNSHORT Priority, UNSHORT Volume,
 UNSHORT Probability, UNSHORT Frequency, SILONG Effect_X, SILONG Effect_Y)
{
	struct Sound_effect *Effect;
	MEM_HANDLE Handle;
	UNLONG Length;
	UNSHORT Index = 0xFFFF;
	UNSHORT i;

	/* Sound system initialized / sound effects on / digital sound on ? */
	if ((SOUND_State & SOUNDSYSTEM_INITIALIZED) &&
	 (SOUND_State & DIGI_ON))
	{
		/* Find free sound effect */
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* Is this sound effect free ? */
			if (SOUND_Located_sound_effects[i].Effect.State == SOUND_EFFECT_FREE)
			{
				/* Yes -> Found it */
				Index = i;
				break;
			}
		}

		/* Found something ? */
		if (Index == 0xFFFF)
			return 0xFFFF;

		/* Load new sample */
		Handle = Load_subfile(DSAMPLE, Effect_nr);

		/* Success ? */
		if (!Handle)
		{
			/* No -> Out of memory ? */
			if (XLD_Last_error == XLD_OUT_OF_MEMORY)
			{
				/* Yes -> Ignore this error */
				ERROR_PopError();
				ERROR_PopError();
			}
			else
			{
				/* No -> Report error */
				SOUND_Error(SOUND_ERR_FILE_LOAD);
			}

			return 0xFFFF;
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
		Effect = &(SOUND_Located_sound_effects[Index].Effect);

		Effect->Priority 					= Priority;
		Effect->Flags						= 0;
		Effect->Effect_nr		  			= Effect_nr;
		Effect->Volume 					= Volume;
		Effect->Frequency 				= Frequency;
		Effect->Stereo_position			= 64;
		Effect->Sample_memory_handle	= Handle;
		Effect->Length 					= Length;

		/* Should this sound effect be looped indefinitely ? */
		if (Probability == 100)
			Effect->Flags |= LOOP_SOUND_EFFECT;

		/* Insert additional data */
		SOUND_Located_sound_effects[Index].Initial_priority	= Priority;
		SOUND_Located_sound_effects[Index].Initial_volume	= Volume;
		SOUND_Located_sound_effects[Index].Probability		= Probability;

		SOUND_Located_sound_effects[Index].Effect_X			= Effect_X;
		SOUND_Located_sound_effects[Index].Effect_Y			= Effect_Y;

		/* Set sound effect state to WAITING */
		Effect->State = SOUND_EFFECT_WAITING;
	}

	return Index;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Remove_located_effect
 * FUNCTION  : Remove a located sound effect.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 17:32
 * LAST      : 26.07.95 17:32
 * INPUTS    : UNSHORT Effect_index - Located sound effect index (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Remove_located_effect(UNSHORT Effect_index)
{
	/* Exit if effect index is illegal */
	if (Effect_index >= MAX_LOCATED_SOUND_EFFECTS)
		return;

	/* Exit if this effect is free */
	if (SOUND_Located_sound_effects[Effect_index].Effect.State ==
	 SOUND_EFFECT_FREE)
		return;

	/* Delete sound effect */
	SOUND_Delete_sound_effect(&(SOUND_Located_sound_effects[Effect_index].Effect));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Remove_located_effects_at
 * FUNCTION  : Remove all located sound effects at a certain position.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.95 16:01
 * LAST      : 25.07.95 16:01
 * INPUTS    : UNLONG Effect_X - X-coordinate of effect in effect space.
 *             UNLONG Effect_Y - Y-coordinate of effect in effect space.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Remove_located_effects_at(SILONG Effect_X, SILONG Effect_Y)
{
	UNSHORT i;

	/* Check all located effects */
	for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
	{
		/* Is this sound effect not free / in the right place ? */
		if ((SOUND_Located_sound_effects[i].Effect.State != SOUND_EFFECT_FREE) &&
		 (SOUND_Located_sound_effects[i].Effect_X == Effect_X) &&
		 (SOUND_Located_sound_effects[i].Effect_Y == Effect_Y))
		{
			/* Yes -> Delete it */
			SOUND_Delete_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Set_listener_position
 * FUNCTION  : Set position of listener within effect space.
 * FILE      : SOUND.C
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
SOUND_Set_listener_position(SILONG X, SILONG Y, UNSHORT Angle)
{
	/* Set position data */
	SOUND_Listener_X		= X;
	SOUND_Listener_Y		= Y;
	SOUND_Listener_angle	= Angle;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Set_effect_position
 * FUNCTION  : Set position of located effect within effect space.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 13:37
 * LAST      : 26.07.95 13:37
 * INPUTS    : UNSHORT Effect_index - Located sound effect index (0...).
 *             SILONG Effect_X - X-coordinate of effect in effect space.
 *             SILONG Effect_Y - Y-coordinate of effect in effect space.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Set_effect_position(UNSHORT Effect_index, SILONG Effect_X,
 SILONG Effect_Y)
{
	/* Exit if effect index is illegal */
	if (Effect_index >= MAX_LOCATED_SOUND_EFFECTS)
		return;

	/* Exit if this effect is free */
	if (SOUND_Located_sound_effects[Effect_index].Effect.State ==
	 SOUND_EFFECT_FREE)
		return;

	/* Set effect position data */
	SOUND_Located_sound_effects[Effect_index].Effect_X = Effect_X;
	SOUND_Located_sound_effects[Effect_index].Effect_Y = Effect_Y;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Error
 * FUNCTION  : Report a sound system error.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 14:14
 * LAST      : 09.02.95 14:14
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Error(UNSHORT Error_code)
{
	/* Push error on the error stack */
	ERROR_PushError
	(
		SOUND_Print_error,
		SOUND_Library_name,
		sizeof(UNSHORT),
		(UNBYTE *) &Error_code
	);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Print_error
 * FUNCTION  : Print a sound system error.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.02.95 14:19
 * LAST      : 09.02.95 14:19
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by SOUND_Error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Print_error(UNCHAR *buffer, UNBYTE *data)
{
	UNSHORT i;

	/* Get error code */
	i = *((UNSHORT *) data);

	/* Catch illegal errors */
	if (i > SOUND_ERR_MAX)
		i = 0;

 	/* Print error */
	_bprintf
	(
		buffer,
		BBERROR_OUTSTRINGSIZE,
		SOUND_Error_strings[i],
		(UNCHAR *) AIL_error
	);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Pause_song
 * FUNCTION  : SOUND_Pause a playing song.
 * FILE      : SOUND.C
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
SOUND_Pause_song(struct Song *Song)
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
 * NAME      : SOUND_Continue_song
 * FUNCTION  : Continue a paused song.
 * FILE      : SOUND.C
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
SOUND_Continue_song(struct Song *Song)
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
 * NAME      : SOUND_End_song
 * FUNCTION  : End a song.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 09:52
 * LAST      : 21.10.95 20:14
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
SOUND_End_song(struct Song *Song)
{
	UNLONG Length;
	UNBYTE *Ptr;

	/* Is this song not free ? */
	if (Song->State != SONG_FREE)
	{
		/* Yes -> End sequence */
		AIL_end_sequence(Song->Sequence_handle);

		/* Unlock song memory */
		Ptr = MEM_Get_pointer(Song->Song_memory_handle);
		Length = MEM_Get_block_size(Song->Song_memory_handle);

		BASEMEM_Unlock_region(Ptr, Length);

		/* Remove song memory */
		MEM_Free_pointer(Song->Song_memory_handle);
		MEM_Free_memory(Song->Song_memory_handle);

		/* Clear handle */
		Song->Song_memory_handle = NULL;

		/* Is this an ambient song ? */
		if (Song->Flags & AMBIENT_SONG)
		{
			/* Yes -> Does a wave synthesizer exist ? */
			if (Song->Wave_synth)
			{
				/* Yes -> Destroy it */
				AIL_destroy_wave_synthesizer(Song->Wave_synth);

				/* Clear handle */
				Song->Wave_synth = NULL;
			}

			/* Unlock wave library */
			Ptr = MEM_Get_pointer(Song->Wavelib_memory_handle);
			Length = MEM_Get_block_size(Song->Wavelib_memory_handle);

			BASEMEM_Unlock_region(Ptr, Length);

			/* Remove wave library */
			MEM_Free_pointer(Song->Wavelib_memory_handle);
			MEM_Free_memory(Song->Wavelib_memory_handle);

			/* Clear handle */
			Song->Wavelib_memory_handle = NULL;
		}

		/* Set song state to FREE */
		Song->State = SONG_FREE;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Update_sound_effects
 * FUNCTION  : Check all normal and located and sound effects and allocate
 *              digital channels.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.07.95 19:54
 * LAST      : 26.09.95 15:04
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
SOUND_Update_sound_effects(void)
{
	struct Sound_effect *Active_sound_effect_list[MAX_NORMAL_SOUND_EFFECTS +
	 MAX_LOCATED_SOUND_EFFECTS];
	UNSHORT Nr_active_sound_effects;
	UNSHORT i;

	/* Exit if still loading a saved game (data may be invalid) */
	if (Loading_game)
		return;

	/* Stop all sound effects that have ended */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		/* Is this channel playing ? */
		if (SOUND_Digital_channels[i].State == DIGI_CHANNEL_PLAYING)
		{
			/* Yes -> Has the sample ended ? */
			if (AIL_sample_status(SOUND_Digital_channels[i].Sample_handle) ==
			 SMP_DONE)
			{
				/* Yes -> Stop */
				SOUND_Stop_digital_channel(i);
			}
		}
	}

	/* Delete all normal sound effects that have been stopped */
	for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
	{
		/* State is STOPPED ? */
		if (SOUND_Sound_effects[i].State == SOUND_EFFECT_STOPPED)
		{
			/* Yes -> Delete sound effect */
			SOUND_Delete_sound_effect(&(SOUND_Sound_effects[i]));
		}
	}

	/* Located sound effects on ? */
	if (SOUND_State & LOCATED_SOUND_FX_ON)
	{
		/* Yes -> Determine if any located sound effects should be started */
		SOUND_Update_located_sound_effects();
	}

	/* Clear active sound effect counter */
	Nr_active_sound_effects = 0;

	/* Add waiting normal sound effects to active list */
	for (i=0;i<MAX_NORMAL_SOUND_EFFECTS;i++)
	{
		/* State is WAITING or PLAYING ? */
		if ((SOUND_Sound_effects[i].State == SOUND_EFFECT_WAITING) ||
		 (SOUND_Sound_effects[i].State == SOUND_EFFECT_PLAYING))
		{
			/* Yes -> Add to list */
			Active_sound_effect_list[Nr_active_sound_effects] =
			 &(SOUND_Sound_effects[i]);

			/* Count up */
			Nr_active_sound_effects++;
		}
	}

	/* Located sound effects on ? */
	if (SOUND_State & LOCATED_SOUND_FX_ON)
	{
		/* Yes -> Add waiting located sound effects to active list */
		for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
		{
			/* State is WAITING or PLAYING ? */
			if ((SOUND_Located_sound_effects[i].Effect.State ==
			 SOUND_EFFECT_WAITING) ||
			 (SOUND_Located_sound_effects[i].Effect.State ==
			 SOUND_EFFECT_PLAYING))
			{
				/* Yes -> Add to list */
				Active_sound_effect_list[Nr_active_sound_effects] =
				 &(SOUND_Located_sound_effects[i].Effect);

				/* Count up */
				Nr_active_sound_effects++;
			}
		}
	}

	/* Sort the list of active sound effects */
	Shuttlesort
	(
		SOUND_Swap_sound_effects,
		SOUND_Compare_sound_effects,
		Nr_active_sound_effects,
		(UNBYTE *) &(Active_sound_effect_list[0])
	);

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
				SOUND_Stop_sound_effect(Active_sound_effect_list[i]);
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
				SOUND_Start_sound_effect(Active_sound_effect_list[i]);
				break;
			}
			/* Already playing */
			case SOUND_EFFECT_PLAYING:
			{
				/* Update it */
				SOUND_Update_sound_effect(Active_sound_effect_list[i]);
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Swap_sound_effects
 * FUNCTION  : Swap two sound effects (for sorting).
 * FILE      : SOUND.C
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
SOUND_Swap_sound_effects(UNLONG A, UNLONG B, UNBYTE *Data)
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
 * NAME      : SOUND_Compare_sound_effects
 * FUNCTION  : Compare two sound effects (for sorting).
 * FILE      : SOUND.C
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
SOUND_Compare_sound_effects(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Sound_effect **List;

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
 * NAME      : SOUND_Update_located_sound_effects
 * FUNCTION  : Determine if any located sound effects should be started.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.07.95 11:59
 * LAST      : 14.09.95 15:00
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
SOUND_Update_located_sound_effects(void)
{
	double Angle;
	SILONG X, Y;
	SILONG New_X;
	UNLONG Rough_effect_listening_range;
	UNLONG Length;
	SISHORT Stereo;
	UNSHORT Volume;
	UNSHORT i;

	/* Calculate rough effect listening range */
	Rough_effect_listening_range = (SOUND_Effect_listening_range *
	 SOUND_Effect_listening_range);

	/* Update restart counter */
	if (SOUND_Located_sound_effects_restart_counter < 20)
		SOUND_Located_sound_effects_restart_counter++;
	else
		SOUND_Located_sound_effects_restart_counter = 0;

	/* Check located sound effects */
	for (i=0;i<MAX_LOCATED_SOUND_EFFECTS;i++)
	{
		/* Skip if state is FREE */
		if (SOUND_Located_sound_effects[i].Effect.State == SOUND_EFFECT_FREE)
			continue;

		/* State is STOPPED ? */
		if (SOUND_Located_sound_effects[i].Effect.State == SOUND_EFFECT_STOPPED)
		{
			/* Yes -> Is this a one-shot sound effect ? */
			if (!SOUND_Located_sound_effects[i].Probability)
			{
				/* Yes -> Delete it */
				SOUND_Delete_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
				continue;
			}

			/* Is this a permanent sound effect ? */
			if (SOUND_Located_sound_effects[i].Probability < 100)
			{
				/* No -> Should this sound effect be started again ? */
				if (SOUND_Located_sound_effects_restart_counter ||
				 ((rand() % 100) > SOUND_Located_sound_effects[i].Probability))
				{
					/* No -> Skip */
					continue;
				}
			}
		}

		/* Calculate coordinates of effect relative to the listener */
		X = SOUND_Located_sound_effects[i].Effect_X - SOUND_Listener_X;
		Y = SOUND_Located_sound_effects[i].Effect_Y - SOUND_Listener_Y;

		/* Calculate rough vector length */
		Length = (X * X) + (Y * Y);

		/* Too far away ? */
		if (Length >= Rough_effect_listening_range)
		{
			/* Yes -> Set state to STOPPED */
			SOUND_Stop_sound_effect(&(SOUND_Located_sound_effects[i].Effect));
		}
		else
		{
			/* No -> Calculate real vector length */
			Length = (SILONG) sqrt(((double) X * (double) X) +
			 ((double) Y * (double) Y));

			/* Calculate volume depending on distance */
			Volume = (SOUND_Located_sound_effects[i].Initial_volume *
			 (100 - ((Length * 100) / SOUND_Effect_listening_range))) / 100;

			/* Set volume */
			SOUND_Located_sound_effects[i].Effect.Volume = Volume;

			/* Set priority depending on distance */
			SOUND_Located_sound_effects[i].Effect.Priority =
			 (SOUND_Located_sound_effects[i].Initial_priority *
			 (100 - ((Length * 100) / SOUND_Effect_listening_range))) / 100;

			/* Rotate effect around the listener */
			Angle = ((double) SOUND_Listener_angle * 2 * PI) / 360;

			New_X = (SILONG) (((double) X * cos(Angle)) -
			 ((double) Y * sin(Angle)));

			/* Calculate stereo position depending on effect position
			 (there's an extra distortion here to increase the 3D effect) */
			Stereo = 64 + ((New_X * 128) / (SILONG) SOUND_Effect_listening_range);
			Stereo = min(max(Stereo, 0), 127);

			/* Set stereo position */
			SOUND_Located_sound_effects[i].Effect.Stereo_position = Stereo;

			/* Set state to WAITING, unless it is already PLAYING */
			if (SOUND_Located_sound_effects[i].Effect.State !=
			 SOUND_EFFECT_PLAYING)
			{
				SOUND_Located_sound_effects[i].Effect.State =	SOUND_EFFECT_WAITING;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Start_sound_effect
 * FUNCTION  : Start playing a sound effect.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.07.95 17:44
 * LAST      : 05.09.95 19:14
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
SOUND_Start_sound_effect(struct Sound_effect *Effect)
{
	struct Digital_channel *Channel;
	UNLONG Length;
	UNSHORT Channel_index;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Exit if this effect is not waiting to be played */
	if (Effect->State != SOUND_EFFECT_WAITING)
		return;

	/* Exit if a digital channel is already playing this effect */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		if ((SOUND_Digital_channels[i].State != DIGI_CHANNEL_FREE) &&
		 (SOUND_Digital_channels[i].Effect_ptr == Effect))
		{
			return;
		}
	}

	/* Find a free digital channel */
	Channel_index = 0xFFFF;
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		/* Is this channel free ? */
		if (SOUND_Digital_channels[i].State == DIGI_CHANNEL_FREE)
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
		Channel = &(SOUND_Digital_channels[Channel_index]);

		/* Create reference to sound effect */
		Channel->Effect_ptr = Effect;

		/* Set digital channel state to playing */
		Channel->State = DIGI_CHANNEL_PLAYING;

		/* Set sound effect state to playing */
		Effect->State = SOUND_EFFECT_PLAYING;

		/* Claim and lock sample memory */
		Ptr = MEM_Claim_pointer(Effect->Sample_memory_handle);
		Length = MEM_Get_block_size(Effect->Sample_memory_handle);

		BASEMEM_Lock_region(Ptr, Length);

		/* Initialize sample */
		AIL_init_sample(Channel->Sample_handle);
		AIL_set_sample_address
		(
			Channel->Sample_handle,
			(void *) Ptr,
			(ULONG) Effect->Length
		);

		/* Set sample parameters */
		AIL_set_sample_volume
		(
			Channel->Sample_handle,
			(LONG) Effect->Volume
		);
		AIL_set_sample_playback_rate
		(
			Channel->Sample_handle,
			(LONG) Effect->Frequency
		);
		AIL_set_sample_pan
		(
			Channel->Sample_handle,
			(LONG) Effect->Stereo_position
		);

		/* Should this sample be looped indefinitely ? */
		if (Effect->Flags & LOOP_SOUND_EFFECT)
		{
			/* Yes */
			AIL_set_sample_loop_count
			(
				Channel->Sample_handle,
				(LONG) 0
			);
		}

		/* Start sample */
		AIL_start_sample(Channel->Sample_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Update_sound_effect
 * FUNCTION  : Update a playing sound effect.
 * FILE      : SOUND.C
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
SOUND_Update_sound_effect(struct Sound_effect *Effect)
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
		if ((SOUND_Digital_channels[i].State == DIGI_CHANNEL_PLAYING) &&
		 (SOUND_Digital_channels[i].Effect_ptr == Effect))
		{
			/* Yes -> Get channel data */
			Channel = &(SOUND_Digital_channels[i]);

			/* Update sample parameters */
			AIL_set_sample_volume
			(
				Channel->Sample_handle,
				(LONG) Effect->Volume
			);
			AIL_set_sample_playback_rate
			(
				Channel->Sample_handle,
				(LONG) Effect->Frequency
			);
			AIL_set_sample_pan
			(
				Channel->Sample_handle,
				(LONG) Effect->Stereo_position
			);

			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Stop_sound_effect
 * FUNCTION  : SOUND_Stop playing a sound effect.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.07.95 14:20
 * LAST      : 01.08.95 13:15
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
SOUND_Stop_sound_effect(struct Sound_effect *Effect)
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
			if ((SOUND_Digital_channels[i].State == DIGI_CHANNEL_PLAYING) &&
			 (SOUND_Digital_channels[i].Effect_ptr == Effect))
			{
				/* Yes -> Stop playing this channel */
				SOUND_Stop_digital_channel(i);
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
 * NAME      : SOUND_Delete_sound_effect
 * FUNCTION  : Delete a sound effect.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.07.95 14:29
 * LAST      : 01.08.95 13:15
 *	INPUTS    : struct Sound_effect *Effect - Pointer to sound effect data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may safely ignore the sound state.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Delete_sound_effect(struct Sound_effect *Effect)
{
	/* Exit if this sound effect is FREE */
	if (Effect->State == SOUND_EFFECT_FREE)
		return;

	/* Is this sound effect playing ? */
	if (Effect->State == SOUND_EFFECT_PLAYING)
	{
		/* Yes -> Stop it */
		SOUND_Stop_sound_effect(Effect);
	}

	/* Free sample memory */
	MEM_Free_memory(Effect->Sample_memory_handle);

	/* Set sound effect's state to FREE */
	Effect->State = SOUND_EFFECT_FREE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Stop_all_digital_channels
 * FUNCTION  : Stop all digital channels that are currently playing.
 * FILE      : SOUND.C
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
SOUND_Stop_all_digital_channels(void)
{
	UNSHORT i;

	/* Stop all digital channels */
	for (i=0;i<MAX_FX_DIGI_CHANNELS;i++)
	{
		SOUND_Stop_digital_channel(i);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : SOUND_Stop_digital_channel
 * FUNCTION  : Stop playing a digital channel.
 * FILE      : SOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.07.95 14:26
 * LAST      : 02.11.95 14:10
 *	INPUTS    : UNSHORT Channel_index - Digital channel index (0...
 *              MAX_FX_DIGI_CHANNELS - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function does not need to check the sound state, as the
 *              effect wouldn't be playing unless everything is in order.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
SOUND_Stop_digital_channel(UNSHORT Channel_index)
{
	UNLONG Length;
	UNBYTE *Ptr;

	/* Exit if channel index is illegal */
	if (Channel_index >= MAX_FX_DIGI_CHANNELS)
		return;

	/* Is this channel playing ? */
	if (SOUND_Digital_channels[Channel_index].State == DIGI_CHANNEL_PLAYING)
	{
		/* Yes -> Has the sample ended ? */
		if (AIL_sample_status(SOUND_Digital_channels[Channel_index].Sample_handle)
		 != SMP_DONE)
		{
			/* No -> Stop playing */
			AIL_end_sample(SOUND_Digital_channels[Channel_index].Sample_handle);
		}

		/* Set digital channel state to FREE */
		SOUND_Digital_channels[Channel_index].State = DIGI_CHANNEL_FREE;

		/* Set corresponding sound effect's state to STOPPED */
		SOUND_Digital_channels[Channel_index].Effect_ptr->State = SOUND_EFFECT_STOPPED;

		/* Unlock sound effect */
		Ptr = MEM_Get_pointer(SOUND_Digital_channels[Channel_index].Effect_ptr->Sample_memory_handle);
		Length = MEM_Get_block_size(SOUND_Digital_channels[Channel_index].Effect_ptr->Sample_memory_handle);

		BASEMEM_Unlock_region(Ptr, Length);

		/* Free pointer */
		MEM_Free_pointer(SOUND_Digital_channels[Channel_index].Effect_ptr->Sample_memory_handle);

		/* Destroy reference to sound effect */
		SOUND_Digital_channels[Channel_index].Effect_ptr = NULL;
	}
}

