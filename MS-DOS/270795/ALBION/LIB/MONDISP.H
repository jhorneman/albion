/************
 * NAME     : MONDISP.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 9-3-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MONDISP_H
#define MONDISP_H

#include <BBDEF.H>
#include <COMBAT.H>

/*
 ** Defines ****************************************************************
 */

/* Monster display types */
#define APPEAR_MONDISP			(0)
#define STAND_MONDISP			(1)
#define MOVE_MONDISP				(2)
#define PARADE_MONDISP			(3)
#define SHORT_RANGE_MONDISP	(4)
#define LONG_RANGE_MONDISP    (5)
#define MAGIC_MONDISP         (6)
#define DEFLECT_MONDISP       (7)
#define FLEE_MONDISP          (8)
#define DAMAGE_MONDISP        (9)
#define GIVE_UP_MONDISP       (10)
#define DIE_MONDISP           (11)

#define MAX_MONDISPS				(12)

/*
 ** Type definitions *******************************************************
 */

typedef void (*Monster_display_handler) (struct Combat_participant *Monster_part);

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

void Do_monster_display(struct Combat_participant *Monster_part,
 UNSHORT Mondisp_type);

void Default_appear_mondisp_handler(struct Combat_participant *Monster_part);
void Default_move_mondisp_handler(struct Combat_participant *Monster_part);
void Default_short_range_mondisp_handler(struct Combat_participant *Monster_part);
void Default_long_range_mondisp_handler(struct Combat_participant *Monster_part);
void Default_magic_mondisp_handler(struct Combat_participant *Monster_part);
void Default_deflect_mondisp_handler(struct Combat_participant *Monster_part);
void Default_flee_mondisp_handler(struct Combat_participant *Monster_part);
void Default_damage_mondisp_handler(struct Combat_participant *Monster_part);
void Default_give_up_mondisp_handler(struct Combat_participant *Monster_part);
void Default_die_mondisp_handler(struct Combat_participant *Monster_part);

BOOLEAN Monster_has_animation(struct Combat_participant *Monster_part,
 UNSHORT Anim_type);

#endif

