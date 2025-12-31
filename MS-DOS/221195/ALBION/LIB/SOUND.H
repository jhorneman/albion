/************
 * NAME     : SOUND.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 2-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : SOUND.C
 ************/

#ifndef SOUND_H
#define SOUND_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Macro definitions ******************************************************
 */

#define Play_sound(a)	SOUND_Play_sound_effect(a, 100, 100, 0, 0)

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
void SOUND_Sound_on(void);
void SOUND_Sound_off(void);

void SOUND_Music_on(void);
void SOUND_Music_off(void);

void SOUND_Sound_effects_on(void);
void SOUND_Sound_effects_off(void);

void SOUND_Located_sound_effects_on(void);
void SOUND_Located_sound_effects_off(void);

/* Volume access functions */
UNSHORT SOUND_Get_MIDI_volume(void);
void SOUND_Set_MIDI_volume(UNSHORT New_volume);

UNSHORT SOUND_Get_digital_volume(void);
void SOUND_Set_digital_volume(UNSHORT New_volume);

/* Sound system management functions */
BOOLEAN SOUND_Init_sound_system(void);
void SOUND_Exit_sound_system(void);

void SOUND_Update_sound_system(void);

/* Music functions */
void SOUND_Play_song(UNSHORT Song_nr);
void SOUND_Play_ambient_song(UNSHORT Song_nr);
void SOUND_Play_jingle(UNSHORT Song_nr);

void SOUND_Play_sound_effect(UNSHORT Effect_nr, UNSHORT Priority,
 UNSHORT Volume, UNSHORT Variability, UNSHORT Frequency);

/* Located sound effects functions */
void SOUND_Initialize_located_effects(UNLONG Listening_range);

void SOUND_Load_located_effects(UNBYTE *Ptr);

UNBYTE *SOUND_Get_located_effect_table_address(void);
UNLONG SOUND_Get_located_effect_table_length(void);

UNSHORT SOUND_Add_located_effect(UNSHORT Effect_nr, UNSHORT Priority,
 UNSHORT Volume, UNSHORT Probability, UNSHORT Frequency, SILONG Effect_X,
 SILONG Effect_Y);

void SOUND_Remove_located_effect(UNSHORT Effect_index);

void SOUND_Remove_located_effects_at(SILONG Effect_X, SILONG Effect_Y);

void SOUND_Set_listener_position(SILONG X, SILONG Y, UNSHORT Angle);

void SOUND_Set_effect_position(UNSHORT Effect_index, SILONG Effect_X,
 SILONG Effect_Y);

#endif

