/************
 * NAME     : COMMAG4.C
 * AUTOR    : Rainer Reber, BlueByte
 * START    : 6-10-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMMAGIC.H
 ************/

/* includes */

#include <stdlib.h>
#include <math.h>

#include <BBDEF.H>
#include <BBMEM.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <MAGIC.H>
#include <COMBVAR.H>
#include <COMACTS.H>
#include <COMOBS.H>
#include <COMMAGIC.H>
#include <COMMSUB.H>
#include <ITMLOGIC.H>
#include <TACTICAL.H>
#include <XFTYPES.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C4_Spell_1_handler
 * FUNCTION  : Spell handler (Cause panic).
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     :
 * LAST      :
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C4_Spell_1_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C4_Spell_1);
}

void
Do_C4_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *COMOB;
	MEM_HANDLE cloud_handle = NULL;
	SILONG sx,sy,sz,zx,zy,zz,mx,my,mz,steps;

	/* Load graphics */
	cloud_handle = Load_subfile(COMBAT_GFX, 68);
	if (!cloud_handle)
		return;

	/* Koordinaten des Starts (Monster Mitglied) */
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&sx,&sy,&sz);

	/* Koordinaten des Ziels (Party Mitglied) */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* Schritte bis zum Ziel */
	steps=calc_delta_xyz(sx,sy,sz, zx,zy,zz, &mx,&my,&mz, 300);

	/* Wolke erstellen */
	if((COMOB=Gen_COMOB(sx,sy,sz, steps, 100, cloud_handle, GC_TRANS))==NULL){
		ERROR_PopError();
		return;
	}

	/* UserFlags */
	COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

	/* Bewegung */
	COMOB->dX_3D=mx;
	COMOB->dY_3D=my;
	COMOB->dZ_3D=mz;

	/* steps mal updaten */
	update_n_times(steps);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Set condition */
		Set_condition(Victim_part->Char_handle, PANICKED);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C4_Spell_2_handler
 * FUNCTION  : Spell handler (Cause poisoning).
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     :
 * LAST      :
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C4_Spell_2_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C4_Spell_2);
}

void
Do_C4_Spell_2(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *COMOB;
	MEM_HANDLE cloud_handle = NULL;
	SILONG sx,sy,sz,zx,zy,zz,mx,my,mz,steps;

	/* Load graphics */
	cloud_handle = Load_subfile(COMBAT_GFX, 66);
	if (!cloud_handle)
		return;

	/* Koordinaten des Starts (Monster Mitglied) */
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&sx,&sy,&sz);

	/* Koordinaten des Ziels (Party Mitglied) */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* Schritte bis zum Ziel */
	steps=calc_delta_xyz(sx,sy,sz, zx,zy,zz, &mx,&my,&mz, 300);

	/* Wolke erstellen */
	if((COMOB=Gen_COMOB(sx,sy,sz, steps, 100, cloud_handle, GC_TRANS))==NULL){
		ERROR_PopError();
		return;
	}

	/* UserFlags */
	COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

	/* Bewegung */
	COMOB->dX_3D=mx;
	COMOB->dY_3D=my;
	COMOB->dZ_3D=mz;

	/* steps mal updaten */
	update_n_times(steps);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Set condition */
		Set_condition(Victim_part->Char_handle, POISONED);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C4_Spell_3_handler
 * FUNCTION  : Spell handler (Cause irritation).
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     :
 * LAST      :
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C4_Spell_3_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C4_Spell_3);
}

void
Do_C4_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *COMOB;
	MEM_HANDLE cloud_handle = NULL;
	SILONG sx,sy,sz,zx,zy,zz,mx,my,mz,steps;

	/* Load graphics */
	cloud_handle = Load_subfile(COMBAT_GFX, 67);
	if (!cloud_handle)
		return;

	/* Koordinaten des Starts (Monster Mitglied) */
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&sx,&sy,&sz);

	/* Koordinaten des Ziels (Party Mitglied) */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* Schritte bis zum Ziel */
	steps=calc_delta_xyz(sx,sy,sz, zx,zy,zz, &mx,&my,&mz, 300);

	/* Wolke erstellen */
	if((COMOB=Gen_COMOB(sx,sy,sz, steps, 100, cloud_handle, GC_TRANS))==NULL){
		ERROR_PopError();
		return;
	}

	/* UserFlags */
	COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

	/* Bewegung */
	COMOB->dX_3D=mx;
	COMOB->dY_3D=my;
	COMOB->dZ_3D=mz;

	/* steps mal updaten */
	update_n_times(steps);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Set condition */
		Set_condition(Victim_part->Char_handle, IRRITATED);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C4_Spell_4_handler
 * FUNCTION  : Spell handler (Cause disease).
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     :
 * LAST      :
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C4_Spell_4_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C4_Spell_4);
}

void
Do_C4_Spell_4(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *COMOB;
	MEM_HANDLE cloud_handle = NULL;
	SILONG sx,sy,sz,zx,zy,zz,mx,my,mz,steps;

	/* Load graphics */
	cloud_handle = Load_subfile(COMBAT_GFX, 65);
	if (!cloud_handle)
		return;

	/* Koordinaten des Starts (Monster Mitglied) */
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&sx,&sy,&sz);

	/* Koordinaten des Ziels (Party Mitglied) */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* Schritte bis zum Ziel */
	steps=calc_delta_xyz(sx,sy,sz, zx,zy,zz, &mx,&my,&mz, 300);

	/* Wolke erstellen */
	if((COMOB=Gen_COMOB(sx,sy,sz, steps, 100, cloud_handle, GC_TRANS))==NULL){
		ERROR_PopError();
		return;
	}

	/* UserFlags */
	COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

	/* Bewegung */
	COMOB->dX_3D=mx;
	COMOB->dY_3D=my;
	COMOB->dZ_3D=mz;

	/* steps mal updaten */
	update_n_times(steps);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Set condition */
		Set_condition(Victim_part->Char_handle, DISEASED);
	}
}
