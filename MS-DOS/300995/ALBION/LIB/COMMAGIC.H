/************
 * NAME     : COMMAGIC.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 24-4-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMMAGIC_H
#define COMMAGIC_H

#include <BBDEF.H>

#include <COMBAT.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Magical spell handlers */
void C0_Spell_1_handler(UNSHORT Strength);
void Do_C0_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_3_handler(UNSHORT Strength);
void Do_C0_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_4_handler(UNSHORT Strength);
void Do_C0_Spell_4(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_5_handler(UNSHORT Strength);
void Do_C0_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_6_handler(UNSHORT Strength);
void Do_C0_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_7_handler(UNSHORT Strength);
void Do_C0_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);
void C0_Spell_8_handler(UNSHORT Strength);
void Do_C0_Spell_8(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_10_handler(UNSHORT Strength);
void Do_C0_Spell_10(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_11_handler(UNSHORT Strength);
void Do_C0_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);
void C0_Spell_12_handler(UNSHORT Strength);
void Do_C0_Spell_12(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_13_handler(UNSHORT Strength);
void Do_C0_Spell_13(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_14_handler(UNSHORT Strength);
void Do_C0_Spell_14(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);
void Handle_C0_Spell_14_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength, struct COMOB *Trap_COMOB);

void C0_Spell_15_handler(UNSHORT Strength);
void Do_C0_Spell_15(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C0_Spell_20_handler(UNSHORT Strength);
void Do_C0_Spell_20(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C1_Spell_4_handler(UNSHORT Strength);
void C1_Spell_6_handler(UNSHORT Strength);

void C1_Spell_8_handler(UNSHORT Strength);
void Do_C1_Spell_8(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C1_Spell_9_handler(UNSHORT Strength);

void C1_Spell_10_handler(UNSHORT Strength);
void Do_C1_Spell_10(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C2_Spell_1_handler(UNSHORT Strength);
void Do_C2_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C2_Spell_2_handler(UNSHORT Strength);
void Do_C2_Spell_2(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C2_Spell_3_handler(UNSHORT Strength);
void C2_Spell_4_handler(UNSHORT Strength);

void C2_Spell_5_handler(UNSHORT Strength);
void Do_C2_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C2_Spell_6_handler(UNSHORT Strength);
void Do_C2_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C2_Spell_8_handler(UNSHORT Strength);
void Do_C2_Spell_8(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C2_Spell_9_handler(UNSHORT Strength);
void C2_Spell_10_handler(UNSHORT Strength);

void C2_Spell_11_handler(UNSHORT Strength);
void Do_C2_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_1_handler(UNSHORT Strength);
void Do_C3_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_2_handler(UNSHORT Strength);
void Do_C3_Spell_2(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_3_handler(UNSHORT Strength);
void Do_C3_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_4_handler(UNSHORT Strength);
void Do_C3_Spell_4(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_5_handler(UNSHORT Strength);
void Do_C3_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_6_handler(UNSHORT Strength);
void Do_C3_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_7_handler(UNSHORT Strength);
void Do_C3_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);
void Handle_C3_Spell_7_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength, struct COMOB *Trap_COMOB);

void C3_Spell_8_handler(UNSHORT Strength);

void C3_Spell_9_handler(UNSHORT Strength);
void Do_C3_Spell_9(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);
void Handle_C3_Spell_9_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength, struct COMOB *Trap_COMOB);

void C3_Spell_10_handler(UNSHORT Strength);

void C3_Spell_11_handler(UNSHORT Strength);
void Do_C3_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_12_handler(UNSHORT Strength);
void Do_C3_Spell_12(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_13_handler(UNSHORT Strength);
void C3_Spell_14_handler(UNSHORT Strength);
void Do_C3_Spell_14(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C3_Spell_15_handler(UNSHORT Strength);
void Do_C3_Spell_15(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C5_Spell_1_handler(UNSHORT Strength);
void Do_C5_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C5_Spell_3_handler(UNSHORT Strength);
void Do_C5_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C5_Spell_5_handler(UNSHORT Strength);
void Do_C5_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C5_Spell_7_handler(UNSHORT Strength);
void Do_C5_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

#endif
