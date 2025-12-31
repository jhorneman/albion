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

/*
 ** Defines ****************************************************************
 */

/*
 ** Macro definitions ******************************************************
 */

#define Play_sound(a)	Play_sound_effect(a, 100, 100, 0, 0)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Sound state access functions */
void Sound_on(void);
void Sound_off(void);

void Music_on(void);
void Music_off(void);

void Sound_effects_on(void);
void Sound_effects_off(void);

void Located_sound_effects_on(void);
void Located_sound_effects_off(void);

/* Volume access functions */
UNSHORT Get_MIDI_volume(void);
void Set_MIDI_volume(UNSHORT New_volume);

UNSHORT Get_digital_volume(void);
void Set_digital_volume(UNSHORT New_volume);

/* Sound system management functions */
BOOLEAN Init_sound_system(void);
void Exit_sound_system(void);
void Update_sound_system(void);

/* Music functions */
void Play_song(UNSHORT Song_nr);
void Play_ambient_song(UNSHORT Song_nr);
void Play_jingle(UNSHORT Song_nr);

void Play_sound_effect(UNSHORT Effect_nr, UNSHORT Priority, UNSHORT Volume,
 UNSHORT Variability, UNSHORT Frequency);

/* Located sound effects functions */
void Initialize_located_effects(UNLONG Listening_range);

void Load_located_effects(UNBYTE *Ptr);

UNBYTE *Get_located_effect_table_address(void);
UNLONG Get_located_effect_table_length(void);

UNSHORT Add_located_effect(UNSHORT Effect_nr, UNSHORT Priority,
 UNSHORT Volume, UNSHORT Probability, UNSHORT Frequency, SILONG Effect_X,
 SILONG Effect_Y);

void Remove_located_effect(UNSHORT Effect_index);

void Remove_located_effects_at(SILONG Effect_X, SILONG Effect_Y);

void Set_listener_position(SILONG X, SILONG Y, UNSHORT Angle);

void Set_effect_position(UNSHORT Effect_index, SILONG Effect_X,
 SILONG Effect_Y);

#endif

