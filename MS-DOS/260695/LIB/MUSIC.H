/************
 * NAME     : MUSIC.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 2-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MUSIC.C
 ************/

#ifndef MUSIC_H
#define MUSIC_H

#include <BBDEF.H>

#include <AIL.H>
#include <DLL.H>

/*
 ** Defines ****************************************************************
 */

#define SONG_VOLUME				(127)
#define AMBIENT_VOLUME			(40)

#define DEFAULT_FREQUENCY		(11000)

#define MAX_SOUND_EFFECTS		(8)

/* Error codes */
#define MUSICERR_AIL_ERROR		(1)
#define MUSICERR_NO_FREE_SONG	(2)
#define MUSICERR_FILE_LOAD		(3)

#define MUSICERR_MAX				(3)

/*
 ** Structure definitions **************************************************
 */

/* Song structure */
struct Song {
	UNLONG Flags;
	UNSHORT Number;
	MEM_HANDLE Memory_handle;
	HSEQUENCE Sequence_handle;
};

/* Bitmasks for Song.Flags */

/* Sound effect structure */
struct Sound_effect {
	UNLONG Flags;
	UNSHORT Priority;
	UNSHORT Volume;
	UNSHORT Frequency;
	UNSHORT Number;
	MEM_HANDLE Memory_handle;
	UNLONG Length;
	HSAMPLE Sample_handle;
};

/* Bitmasks for Sound_effect.Flags */

/* Located sound effect structure */
struct Located_sound_effect {
	struct Sound_effect Effect;

};

/*
 ** External global variables **********************************************
 */

extern BOOLEAN Music_on;

/*
 ** Prototypes *************************************************************
 */

/* Music management functions */
void Init_music(void);
void Exit_music(void);
void Update_music(void);

/* Music access functions */
void Play_song(UNSHORT Song_nr);
void Play_ambient_song(UNSHORT Song_nr);
void Play_jingle(UNSHORT Song_nr);
void Play_sound_effect(UNSHORT Effect_nr, UNSHORT Priority, UNSHORT Volume,
 UNSHORT Variability, UNSHORT Frequency);

/* Located sound effects functions */
void Initialize_located_effects(UNLONG Space_width, UNLONG Space_height);
void Add_located_effect(UNSHORT Effect_nr, UNSHORT Priority, UNSHORT Volume,
 UNSHORT Variability, UNSHORT Frequency, SILONG Effect_X, SILONG Effect_Y);
void Update_located_effects(SILONG Observer_X, SILONG Observer_Y);

/* Music error functions */
void Music_error(UNSHORT Error_code);
void Music_print_error(UNCHAR *buffer, UNBYTE *data);

/* Music support functions */
void End_song(struct Song *Song);

#endif

