/************
 * NAME     : COMSHOW.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 10-5-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMSHOW_H
#define COMSHOW_H

#include <BBDEF.H>
#include <COMBVAR.H>

/*
 ** Defines ****************************************************************
 */

/* Combat show types */
#define SHOW_INIT					(0)
#define SHOW_EXIT					(1)
#define SHOW_STANDING			(2)
#define SHOW_MOVE					(3)
#define SHOW_CLOSE_RANGE		(4)
#define SHOW_LONG_RANGE			(5)
#define SHOW_FLEE					(6)
#define SHOW_CAST_SPELL			(7)
#define SHOW_DAMAGE				(8)
#define SHOW_DEATH				(9)
#define SHOW_DEFLECT				(10)
#define SHOW_GIVE_UP				(11)
#define SHOW_APPEAR				(12)

#define MAX_SHOW_TYPES			(13)

#define MAX_PROJECTILES			(3)

/*
 ** Type definitions *******************************************************
 */

typedef void (*Combat_show_handler) (struct Combat_participant *Part,
 union Combat_show_parms *P);

/*
 ** Structure definitions **************************************************
 */

/* Set of command show methods */
struct Combat_show_set {
	Combat_show_handler Handlers[MAX_SHOW_TYPES];
};

/* Parameters for command show methods */
union Combat_show_parms {
	struct {
		UNSHORT Nr_moves;
		UNSHORT Positions[MAX_COMBAT_MOVES][2];
	} Move_parms;
	struct {
		UNSHORT Target_X, Target_Y;
		UNSHORT Weapon_slot_index;
	} Attack_parms;
	struct {
		UNSHORT Damage;
	} Damage_parms;
};

/* Projectile data */
struct Projectile_data {
	UNSHORT To_gfx_nr;
	UNSHORT Fro_gfx_nr;
	UNSHORT Display_width;						/* Display size in % */
	UNSHORT Display_height;
	UNSHORT Speed;
};

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

void Combat_show(struct Combat_participant *Part, UNSHORT Show_type,
 union Combat_show_parms *P);

void Party_show_close_range(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Party_show_long_range(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Party_show_damage(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Party_show_deflect(struct Combat_participant *Part,
 union Combat_show_parms *P);

void Standard_monster_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_exit(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_move(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_close_range(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_long_range(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_flee(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_cast_spell(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_damage(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_death(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_deflect(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_give_up(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Standard_monster_show_appear(struct Combat_participant *Part,
 union Combat_show_parms *P);

void Demon_hand_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P);

void Show_moving_projectile(struct Combat_participant *Attacker_part,
 UNSHORT Target_X, UNSHORT Target_Y, UNSHORT Current_weapon_slot_index);

UNSHORT Prepare_COMOB_movement(struct Combat_participant *Source_part,
 struct COMOB *COMOB, UNSHORT Target_X, UNSHORT Target_Y,
 UNSHORT Movement_speed);

UNSHORT Set_COMOB_vector(struct COMOB *COMOB, SILONG Target_X_3D,
 SILONG Target_Y_3D, SILONG Target_Z_3D, UNSHORT Movement_speed);

BOOLEAN Determine_COMOB_flip(struct COMOB *COMOB, SILONG Target_X_3D,
 SILONG Target_Y_3D, SILONG Target_Z_3D);

void Get_part_rectangle(struct Combat_participant *Part, SILONG *Left_3D_ptr,
 SILONG *Top_3D_ptr, SILONG *Right_3D_ptr, SILONG *Bottom_3D_ptr);

void Get_3D_part_coordinates(struct Combat_participant *Part,
 SILONG *X_3D_ptr, SILONG *Y_3D_ptr, SILONG *Z_3D_ptr);

void Convert_tactical_to_3D_coordinates(SISHORT Tactical_X,
 SISHORT Tactical_Y, SILONG *X_3D_ptr, SILONG *Z_3D_ptr);

void Lock_part_damage_frame(struct Combat_participant *Part);
void Unlock_part_damage_frame(struct Combat_participant *Part);

#endif

