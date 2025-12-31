/************
 * NAME     : NPCSOUND.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef NPCSOUND_H
#define NPCSOUND_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/* NPC sound effect type */
#define PERMANENT_NPC_EFFECT	(0)
#define MOVING_NPC_EFFECT		(1)
#define SLEEPING_NPC_EFFECT	(2)
#define DISCOVER_NPC_EFFECT	(3)
#define HUNTING_NPC_EFFECT		(4)

#define MAX_NPC_EFFECT_TYPES	(5)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

void Exit_NPC_sound_effects(void);

void Remove_all_NPC_sound_effects(UNSHORT NPC_index);

void Update_NPC_sound_effects(void);

void Add_NPC_sound_effect(UNSHORT NPC_index, UNSHORT NPC_effect_type);
void Remove_NPC_sound_effect(UNSHORT NPC_index, UNSHORT NPC_effect_type);

#endif

