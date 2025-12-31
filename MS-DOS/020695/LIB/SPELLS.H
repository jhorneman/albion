/************
 * NAME     : SPELLS.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-5-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef SPELLS_H
#define SPELLS_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Magical spell handlers */
void C0_Spell_2_handler(UNSHORT Strength);
void Do_C0_Spell_2(UNSHORT Member_nr, UNSHORT Strength);

void C0_Spell_9_handler(UNSHORT Strength);
void Do_C0_Spell_9(UNSHORT Member_nr, UNSHORT Strength);

void C0_Spell_16_handler(UNSHORT Strength);
void C0_Spell_17_handler(UNSHORT Strength);
void C0_Spell_18_handler(UNSHORT Strength);
void C0_Spell_19_handler(UNSHORT Strength);
void C0_Spell_21_handler(UNSHORT Strength);

void C1_Spell_1_handler(UNSHORT Strength);
void C1_Spell_2_handler(UNSHORT Strength);
void C1_Spell_3_handler(UNSHORT Strength);
void C1_Spell_5_handler(UNSHORT Strength);
void C1_Spell_7_handler(UNSHORT Strength);
void C1_Spell_11_handler(UNSHORT Strength);

void C2_Spell_7_handler(UNSHORT Strength);

void C5_Spell_2_handler(UNSHORT Strength);
void C5_Spell_4_handler(UNSHORT Strength);
void C5_Spell_8_handler(UNSHORT Strength);
void C5_Spell_9_handler(UNSHORT Strength);

#endif
