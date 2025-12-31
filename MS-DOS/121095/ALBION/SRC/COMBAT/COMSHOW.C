/************
 * NAME     : COMSHOW.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 10-5-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMSHOW.H
 ************/

/* includes */

#include <stdlib.h>
#include <math.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <GFXFUNC.H>
#include <TEXT.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <COMBAT.H>
#include <COMSHOW.H>
#include <COMOBS.H>
#include <TACTICAL.H>
#include <PRTLOGIC.H>
#include <MAGIC.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <ITMLOGIC.H>
#include <GAMETEXT.H>
#include <COMMSUB.H>

/* defines */

#define MAX_COMBAT_SHOW_SETS		(5)

#define MAX_PROJECTILES				(3)

/* structure definitions */

/* Projectile data */
struct Projectile_data {
	UNSHORT To_gfx_nr;
	UNSHORT Fro_gfx_nr;
	UNSHORT Display_width;						/* Display size in % */
	UNSHORT Display_height;
	UNSHORT Speed;
};

/* prototypes */

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
void Standard_monster_show_appear(struct Combat_participant *Part,
 union Combat_show_parms *P);

void Fear_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P);

void Warniak_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P);
void Warniak_show_death(struct Combat_participant *Part,
 union Combat_show_parms *P);

void Storm_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P);

void Show_moving_projectile(struct Combat_participant *Attacker_part,
 UNSHORT Target_X, UNSHORT Target_Y, UNSHORT Weapon_slot_index);

/* global variables */

static struct Combat_show_set Combat_show_sets[MAX_COMBAT_SHOW_SETS] = {
	/* Party member show set */
	{
		NULL,
		NULL,
		NULL,
		NULL,
		Party_show_close_range,
		Party_show_long_range,
		NULL,
		NULL,
		Party_show_damage,
		NULL,
		Party_show_deflect,
		NULL
	},
	/* Standard monster show set */
	{
		Standard_monster_show_init,
		Standard_monster_show_exit,
		NULL,
		Standard_monster_show_move,
		Standard_monster_show_close_range,
		Standard_monster_show_long_range,
		Standard_monster_show_flee,
		Standard_monster_show_cast_spell,
		Standard_monster_show_damage,
		Standard_monster_show_death,
		NULL,
		Standard_monster_show_appear
	},
	/* Fear show set */
	{
		Fear_show_init,
		Standard_monster_show_exit,
		NULL,
		Standard_monster_show_move,
		Standard_monster_show_close_range,
		Standard_monster_show_long_range,
		Standard_monster_show_flee,
		Standard_monster_show_cast_spell,
		Standard_monster_show_damage,
		Standard_monster_show_death,
		Standard_monster_show_deflect,
		Standard_monster_show_appear
	},
	/* Warniak show set */
	{
		Warniak_show_init,
		Standard_monster_show_exit,
		NULL,
		Standard_monster_show_move,
		Standard_monster_show_close_range,
		Standard_monster_show_long_range,
		Standard_monster_show_flee,
		Standard_monster_show_cast_spell,
		Standard_monster_show_damage,
		Warniak_show_death,
		Standard_monster_show_deflect,
		Standard_monster_show_appear
	},
	/* Storm show set */
	{
		Storm_show_init,
		Standard_monster_show_exit,
		NULL,
		Standard_monster_show_move,
		Standard_monster_show_close_range,
		Standard_monster_show_long_range,
		Standard_monster_show_flee,
		Standard_monster_show_cast_spell,
		Standard_monster_show_damage,
		Warniak_show_death,
		Standard_monster_show_deflect,
		Standard_monster_show_appear
	}
};

static struct Projectile_data Projectile_table[MAX_PROJECTILES] = {
	/* Arrow */
	{
		49, 50,
		150, 150,
		10
	},
	/* Bolt */
	{
		51, 52,
		150, 150,
		15
	},
	/* Special */
	{
		53, 53,
		150, 150,
		10
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Combat_show
 * FUNCTION  : Show a certain kind of behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 11:00
 * LAST      : 10.10.95 11:08
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Show_type - Show type.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Combat_show(struct Combat_participant *Part, UNSHORT Show_type,
 union Combat_show_parms *P)
{
	Combat_show_handler Handler;
	UNSHORT Show_set_index;

	/* Legal show type ? */
	if (Show_type < MAX_SHOW_TYPES)
	{
		/* Yes -> Get participant's show set index */
		Show_set_index = Part->Show_set_index;

		/* Legal ? */
		if (Show_set_index >= MAX_COMBAT_SHOW_SETS)
		{
			/* No -> Is the participant a monster or a party member ? */
			if (Part->Type == PARTY_PART_TYPE)
			{
				/* Party -> Set show set to 0 */
				Show_set_index = 0;
			}
			else
			{
				/* Monster -> Set show set to 1 */
				Show_set_index = 1;
			}

			/* Store new show set index */
			Part->Show_set_index = Show_set_index;
		}

		/* Get address of show handler */
		Handler = Combat_show_sets[Show_set_index].Handlers[Show_type];

		/* Any handler ? */
		if (Handler)
		{
			/* Yes -> Execute */
			(Handler)(Part, P);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_show_close_range
 * FUNCTION  : Show party close-range attack behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.05.95 18:13
 * LAST      : 29.06.95 10:17
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Party_show_close_range(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct Combat_participant *Target_part;
	struct COMOB *COMOB;
	MEM_HANDLE Graphics_handle;
	UNSHORT Nr_frames;
	UNSHORT i;

	/* Load slash */
	Graphics_handle = Load_subfile(COMBAT_GFX, 48);

	/* Create slash COMOB */
	COMOB = Add_COMOB(100);
	if (COMOB)
	{
		/* Is there any participant at the target coordinates ? */
		Target_part =
		 Combat_matrix[P->Attack_parms.Target_Y][P->Attack_parms.Target_X].Part;
		if (Target_part)
		{
			/* Yes -> Use it's coordinates as target coordinates */
			Get_3D_part_coordinates(Target_part, &(COMOB->X_3D),
			 &(COMOB->Y_3D), &(COMOB->Z_3D));
		}
		else
		{
			/* No -> Convert tactical to 3D coordinates */
			Convert_tactical_to_3D_coordinates(P->Attack_parms.Target_X,
			 P->Attack_parms.Target_Y, &(COMOB->X_3D), &(COMOB->Z_3D));

			/* Set Y-coordinate */
			COMOB->Y_3D = 0;
		}

		/* Copy size from monster COMOB */
		COMOB->Display_width = 150;
		COMOB->Display_height = 150;

		/* Set COMOB parameters */
		COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		COMOB->Graphics_handle = Graphics_handle;

		/* Get number of animation frames */
		Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

		/* Show slash */
		for (i=0;i<Nr_frames;i++)
		{
			/* Set animation frame */
			COMOB->Frame = i;

			/* Show */
			Update_screen();
		}

		/* Delete COMOB */
		Delete_COMOB(COMOB);
	}

	/* Destroy slash memory */
	MEM_Free_memory(Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_show_long_range
 * FUNCTION  : Show party long-range attack / projectile behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.05.95 14:18
 * LAST      : 24.05.95 14:18
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Party_show_long_range(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	/* Show moving projectile */
	Show_moving_projectile
	(
		Part,
		P->Attack_parms.Target_X,
		P->Attack_parms.Target_Y,
		P->Attack_parms.Weapon_slot_index
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_show_damage
 * FUNCTION  : Show party receiving damage behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.05.95 18:13
 * LAST      : 29.06.95 10:19
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Party_show_damage(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct COMOB *COMOB;
	MEM_HANDLE Graphics_handle;
	UNSHORT Nr_frames;
	UNSHORT i;

	/* Load scratches */
	Graphics_handle = Load_subfile(COMBAT_GFX, 47);

	/* Create scratches COMOB */
	COMOB = Add_COMOB(100);
	if (COMOB)
	{
		/* Add scratches COMOB to participant */
		Get_3D_part_coordinates(Part, &(COMOB->X_3D), &(COMOB->Y_3D),
		 &(COMOB->Z_3D));

		/* Set size depending on damage */
		COMOB->Display_width = min((P->Damage_parms.Damage) / 2, 100) + 200;
		COMOB->Display_height = COMOB->Display_width;

		/* Set COMOB parameters */
		COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		COMOB->Graphics_handle = Graphics_handle;

		/* Get number of animation frames */
		Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

		/* Show scratches */
		for (i=0;i<Nr_frames;i++)
		{
			/* Set animation frame */
			COMOB->Frame = i;

			/* Show */
			Update_screen();
		}

		/* Delete COMOB */
		Delete_COMOB(COMOB);
	}

	/* Destroy scratches memory */
	MEM_Free_memory(Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_show_deflect
 * FUNCTION  : Show party deflecting behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.10.95 22:05
 * LAST      : 06.10.95 22:05
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Party_show_deflect(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	UNSHORT Strength;

	/* Get strength */
	Strength = P->Deflect_parms.Strength;

	/* Show deflect */
	Show_deflect(Part, Strength);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_init
 * FUNCTION  : Initialize standard monster show.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 11:43
 * LAST      : 15.07.95 19:33
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct Character_data *Char;
	struct COMOB *Main_COMOB;
	UNSHORT Tactical_icon_nr;
	UNSHORT Frame;

	/* Get monster character data */
	Char = (struct Character_data *)	MEM_Claim_pointer(Part->Char_handle);

	/* Load monster graphics */
	Part->Graphics_handle = Load_subfile(MONSTER_GFX, (UNSHORT)
	 Char->Monster_graphics_nr);
	if (!Part->Graphics_handle)
	{
		MEM_Free_pointer(Part->Char_handle);

		Error(ERROR_FILE_LOAD);
		Exit_program();

		return;
	}

	/* Calculate tactical icon number */
	Tactical_icon_nr = MAX_PARTY_CHARS + Char->Monster_graphics_nr;

	/* Load tactical icon graphics */
	Part->Tactical_icon_handle = Load_subfile(TACTICAL_ICONS, Tactical_icon_nr);
	if (!Part->Tactical_icon_handle)
	{
		MEM_Free_pointer(Part->Char_handle);

		Error(ERROR_FILE_LOAD);
		Exit_program();
	}

	/* Add main COMOB */
	Main_COMOB = Add_COMOB(100);

	/* Attach COMOB to participant data */
	Part->Main_COMOB = Main_COMOB;

	/* Calculate 3D coordinates for main COMOB */
	Convert_tactical_to_3D_coordinates
	(
		Part->Tactical_X,
		Part->Tactical_Y,
		&(Main_COMOB->X_3D),
		&(Main_COMOB->Z_3D)
	);

	Main_COMOB->Y_3D = (0 - (SILONG) Char->Baseline) * COMOB_DEC_FACTOR;

	/* Copy display dimensions to main COMOB */
	Main_COMOB->Display_width	= Char->X_factor;
	Main_COMOB->Display_height	= Char->Y_factor;

	/* Does this monster have an appear animation ? */
	if (Has_combat_animation(Part, APPEAR_COMANIM))
	{
		/* Yes -> Get first frame of appear animation */
		Frame = (UNSHORT) Char->Anim_sequences[APPEAR_COMANIM][0];
	}
	else
	{
		/* No -> Get first frame of move animation */
		Frame = (UNSHORT) Char->Anim_sequences[MOVE_COMANIM][0];
	}

	MEM_Free_pointer(Part->Char_handle);

	/* Set current animation frames */
	Main_COMOB->Frame = Frame * 2;

	/* Set default animation frame */
	Part->Default_frame = Frame;

	/* Set COMOB hot-spots */
	Main_COMOB->Hotspot_X_offset = 50;
	Main_COMOB->Hotspot_Y_offset = 100;

	/* Set main COMOB draw mode */
	Main_COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

	/* Set COMOB graphics handles and offsets */
	Main_COMOB->Graphics_handle = Part->Graphics_handle;
	Main_COMOB->Graphics_offset = 0;

	/* Add shadow to main COMOB */
	Add_shadow_to_COMOB(Main_COMOB);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_exit
 * FUNCTION  : Exit standard monster show.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 18:02
 * LAST      : 15.07.95 19:29
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_exit(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	/* Free graphics memory */
	MEM_Free_memory(Part->Tactical_icon_handle);
	MEM_Free_memory(Part->Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_move
 * FUNCTION  : Show standard monster movement behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.09.95 15:57
 * LAST      : 20.09.95 16:32
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_move(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct Character_data *Char;
	SILONG Source_X_3D, Source_Z_3D;
	SILONG Target_X_3D, Target_Z_3D;
	SILONG dX_3D, dZ_3D;
	UNSHORT X, Y;
	UNSHORT Nr_frames;
	UNSHORT i, j;

	/* Wait for animation to end (if any) */
	Wait_4_combat_animation(Part);

	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

	/* Get number of frames for movement animation */
	Nr_frames = (UNSHORT) Char->Anim_lengths[WALK_ANIM];

	/* Any animation / wave animation ? */
	if (Nr_frames && (Char->Anim_motion & (1 << WALK_ANIM)))
	{
		/* Yes -> More frames */
		Nr_frames += Nr_frames - 2;
	}

	MEM_Free_pointer(Part->Char_handle);

	/* At least one animation frame */
	Nr_frames = max(Nr_frames, 1);

	/* Get source 3D coordinates */
	Source_X_3D = Part->Main_COMOB->X_3D;
	Source_Z_3D = Part->Main_COMOB->Z_3D;

	/* The participant will be animated by hand */
	Part->Flags |= PART_HAND_ANIMATED;

	/* Do all moves */
	for (i=0;i<P->Move_parms.Nr_moves;i++)
	{
		/* Get target coordinates */
		X = P->Move_parms.Positions[i][0];
		Y = P->Move_parms.Positions[i][1];

		/* Calculate 3D coordinates */
		Convert_tactical_to_3D_coordinates
		(
			X,
			Y,
			&Target_X_3D,
			&Target_Z_3D
		);

		/* Calculate movement vector */
		dX_3D = Target_X_3D - Source_X_3D;
		dZ_3D = Target_Z_3D - Source_Z_3D;

		/* Interpolate */
		for (j=0;j<Nr_frames;j++)
		{
			/* Set coordinates */
			Part->Main_COMOB->X_3D = Source_X_3D + (dX_3D * j) / Nr_frames;
			Part->Main_COMOB->Z_3D = Source_Z_3D + (dZ_3D * j) / Nr_frames;

			/* Set next animation frame */
			Part->Anim_type = MOVE_COMANIM;
			Update_monster_animation(Part);

			/* Redraw */
			Update_screen();
		}

		/* Target coordinates become source coordinates */
		Source_X_3D = Target_X_3D;
		Source_Z_3D = Target_Z_3D;

		/* Set COMOB coordinates (just to be sure) */
		Part->Main_COMOB->X_3D = Source_X_3D;
		Part->Main_COMOB->Z_3D = Source_Z_3D;
	}

	/* No animation */
	Part->Anim_type = NO_COMANIM;

	/* The participant can be animated by the system again */
	Part->Flags &= ~PART_HAND_ANIMATED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_close_range
 * FUNCTION  : Show standard monster close-range attack behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.05.95 13:40
 * LAST      : 13.05.95 13:40
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_close_range(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	/* Move monster forward */
	Part->Main_COMOB->Z_3D -= 1;

	/* Set close-range attack animation */
	Set_combat_animation(Part, CLOSE_RANGE_COMANIM);

	/* Wait for end of animation */
	Wait_4_combat_animation(Part);

	/* Move monster back again */
	Part->Main_COMOB->Z_3D += 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_long_range
 * FUNCTION  : Show standard monster long-range attack / projectile behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.05.95 13:41
 * LAST      : 13.05.95 13:41
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_long_range(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	/* Move monster forward */
	Part->Main_COMOB->Z_3D -= 1;

	/* Set long-range attack animation */
	Set_combat_animation(Part, LONG_RANGE_COMANIM);

	/* Show moving projectile */
	Show_moving_projectile
	(
		Part,
		P->Attack_parms.Target_X,
		P->Attack_parms.Target_Y,
		P->Attack_parms.Weapon_slot_index
	);

	/* Wait for end of animation */
	Wait_4_combat_animation(Part);

	/* Move monster back again */
	Part->Main_COMOB->Z_3D += 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_flee
 * FUNCTION  : Show standard monster fleeing behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.05.95 13:46
 * LAST      : 13.05.95 13:46
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_flee(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct COMOB *Main_COMOB;
	UNSHORT i;

	/* Set move animation */
	Set_combat_animation(Part, MOVE_COMANIM);

	/* Get COMOBs */
	Main_COMOB = Part->Main_COMOB;

	for(i=0;i<20;i++)
	{
		/* Move monster back */
		Main_COMOB->Z_3D += 50 * COMOB_DEC_FACTOR;

		Update_screen();
	}

	/* Delete combat objects */
	Delete_COMOB(Main_COMOB);

	/* Clear pointer */
	Part->Main_COMOB = NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_cast_spell
 * FUNCTION  : Show standard monster spell-casting behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.05.95 11:05
 * LAST      : 12.05.95 11:05
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_cast_spell(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	/* Set use magic animation */
	Set_combat_animation(Part, MAGIC_COMANIM);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_damage
 * FUNCTION  : Show standard monster receiving damage behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.05.95 11:06
 * LAST      : 20.09.95 16:39
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_damage(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct COMOB *COMOB;
	MEM_HANDLE Graphics_handle;
	UNSHORT Nr_frames;
	UNSHORT i;

	/* Throw monster back */
	Part->Main_COMOB->Z_3D += 20 * COMOB_DEC_FACTOR;

	/* Set hit animation */
	Set_combat_animation(Part, HIT_COMANIM);

	/* Load blood */
	Graphics_handle = Load_subfile(COMBAT_GFX, 46);

	/* Create blood COMOB */
	COMOB = Add_COMOB(100);
	if (COMOB)
	{
		/* Add blood COMOB to participant */
		Get_3D_part_coordinates(Part, &(COMOB->X_3D), &(COMOB->Y_3D),
		 &(COMOB->Z_3D));

		/* Set size depending on damage */
		COMOB->Display_width = min((P->Damage_parms.Damage) / 2, 100) + 200;
		COMOB->Display_height = COMOB->Display_width;

		/* Set COMOB parameters */
		COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		COMOB->Graphics_handle = Graphics_handle;

		/* Get number of animation frames */
		Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

		/* Show blood */
		for (i=0;i<Nr_frames;i++)
		{
			/* Set animation frame */
			COMOB->Frame = i;

			/* Show */
			Update_screen();
			Update_screen();
		}

		/* Delete COMOB */
		Delete_COMOB(COMOB);
	}

	/* Destroy blood memory */
	MEM_Free_memory(Graphics_handle);

	/* Put monster back */
	Part->Main_COMOB->Z_3D -= 20 * COMOB_DEC_FACTOR;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_death
 * FUNCTION  : Show standard monster dying behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.05.95 11:06
 * LAST      : 09.10.95 20:20
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_death(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	static UNSHORT Explosion_Y_offsets[15] = {
		32, 28, 28, 0, 21, 33, 34, 39, 42, 47, 58, 61, 64, 67, 69
	};

	struct Character_data *Char;
	struct COMOB *COMOB;
	MEM_HANDLE Graphics_handle;
	SILONG Y_3D;
	UNSHORT Nr_frames;
	UNSHORT Char_bits;
	UNSHORT i;

	/* Throw monster back */
	Part->Main_COMOB->Z_3D += 20 * COMOB_DEC_FACTOR;

	/* Has death animation ? */
	if (Has_combat_animation(Part, DIE_COMANIM))
	{
		/* Yes -> Set death animation */
		Set_combat_animation(Part, DIE_COMANIM);

		/* Wait */
		Wait_4_combat_animation(Part);
	}

	/* Get victim character bits */
	Char_bits = Get_character_type(Part->Char_handle);

	/* Is supernatural ? */
	if (Char_bits & (SUPERNATURAL | GHOST))
	{
		/* Yes -> Load explosion */
	 	Graphics_handle = Load_subfile(COMBAT_GFX, 43);

		/* Add explosion COMOB */
		COMOB = Add_COMOB(100);
		if (COMOB)
		{
			/* Copy position from monster COMOB */
			COMOB->X_3D = Part->Main_COMOB->X_3D;
			COMOB->Y_3D = Part->Main_COMOB->Y_3D;
			COMOB->Z_3D = Part->Main_COMOB->Z_3D;

			/* Copy size from monster COMOB */
			COMOB->Display_width = Part->Main_COMOB->Display_width * 2;
			COMOB->Display_height = Part->Main_COMOB->Display_height * 2;

			/* Set COMOB parameters */
			COMOB->Draw_mode = LUMINANCE_COMOB_DRAWMODE;

			COMOB->Hotspot_X_offset = 50;
			COMOB->Hotspot_Y_offset = 100;

			COMOB->Graphics_handle = Graphics_handle;

			/* Delete monster COMOB */
			Delete_COMOB(Part->Main_COMOB);

			/* Get number of frames of explosion */
			Nr_frames = Get_nr_frames(Graphics_handle);

			/* Show explosion */
			Y_3D = COMOB->Y_3D;
			for (i=0;i<Nr_frames;i++)
			{
				/* Set vertical offset */
				COMOB->Y_3D = Y_3D + ((Explosion_Y_offsets[i] *
				 COMOB->Display_height) * COMOB_DEC_FACTOR) / 86;

				/* Set animation frame */
				COMOB->Frame = i;

				/* Show */
				Update_screen();
				Update_screen();
			}

			/* Delete COMOB */
			Delete_COMOB(COMOB);
		}

		/* Destroy explosion memory */
		MEM_Free_memory(Graphics_handle);
	}
	else
	{
		/* No -> Get monster character data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Part->Char_handle);

		/* Get number of frames for death animation */
		Nr_frames = (UNSHORT) Char->Anim_lengths[DIE_COMANIM];

		/* Set COMOB to last frame of death animation */
		Part->Main_COMOB->Frame =
		 (UNSHORT) Char->Anim_sequences[DIE_COMANIM][Nr_frames - 1] * 2;

		MEM_Free_pointer(Part->Char_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_deflect
 * FUNCTION  : Show standard monster deflecting behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.10.95 22:05
 * LAST      : 06.10.95 22:05
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_deflect(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	UNSHORT Strength;

	/* Get strength */
	Strength = P->Deflect_parms.Strength;

	/* Show deflect */
	Show_deflect(Part, Strength);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_appear
 * FUNCTION  : Show standard monster appearing behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 15:20
 * LAST      : 10.05.95 15:21
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_appear(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct Character_data *Char;
	UNSHORT Frame;

	/* Get monster character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

	/* Does this monster have an appear animation ? */
	if (Has_combat_animation(Part, APPEAR_COMANIM))
	{
		/* Yes -> Set appear animation */
		Set_combat_animation(Part, APPEAR_COMANIM);

		/* Wait for end of animation */
		Wait_4_combat_animation(Part);

		/* Get first frame of move animation */
		Frame = (UNSHORT) Char->Anim_sequences[MOVE_COMANIM][0];

		/* Set new default frame */
		Part->Default_frame = Frame;
		Part->Main_COMOB->Frame = Frame * 2;
	}

	MEM_Free_pointer(Part->Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fear_show_init
 * FUNCTION  : Initialize Fear show.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.05.95 15:04
 * LAST      : 29.06.95 10:20
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fear_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct COMOB_behaviour *Behaviour_data;
	SILONG Amplitude;
	UNSHORT Period, Value;

	/* Do standard show init */
	Standard_monster_show_init(Part, P);

	/* Set main COMOB draw mode */
	Part->Main_COMOB->Draw_mode = TRANSLUMINANCE_COMOB_DRAWMODE;

	/* Initialize oscillation of main COMOB along the Y-axis */
	Period = 60 + (rand() % 40);
	Value = rand() % Period;
	Amplitude = (4 * COMOB_DEC_FACTOR) + (rand() % (2 * COMOB_DEC_FACTOR));

	Behaviour_data = Add_COMOB_behaviour(Part->Main_COMOB, NULL,
	 Oscillate_handler);

	if (Behaviour_data)
	{
		Behaviour_data->Data.Oscillate_data.Type = OSCILLATE_Y;
		Behaviour_data->Data.Oscillate_data.Period = Period;
		Behaviour_data->Data.Oscillate_data.Value = Value;
		Behaviour_data->Data.Oscillate_data.Amplitude = Amplitude;
	}

	/* Initialize oscillation of main COMOB along the X-axis */
	Period = 60 + (rand() % 40);
	Value = rand() % Period;
	Amplitude = (4 * COMOB_DEC_FACTOR) + (rand() % (2 * COMOB_DEC_FACTOR));

	Behaviour_data = Add_COMOB_behaviour(Part->Main_COMOB, NULL,
	 Oscillate_handler);

	if (Behaviour_data)
	{
		Behaviour_data->Data.Oscillate_data.Type = OSCILLATE_X;
		Behaviour_data->Data.Oscillate_data.Period = Period;
		Behaviour_data->Data.Oscillate_data.Value = Value;
		Behaviour_data->Data.Oscillate_data.Amplitude = Amplitude;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Warniak_show_init
 * FUNCTION  : Initialize Warniak show.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.10.95 20:20
 * LAST      : 09.10.95 20:20
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Warniak_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct COMOB_behaviour *Behaviour_data;
	SILONG Amplitude;
	UNSHORT Period, Value;

	/* Do standard show init */
	Standard_monster_show_init(Part, P);

	/* Initialize oscillation of main COMOB along the Y-axis */
	Period = 60 + (rand() % 40);
	Value = rand() % Period;
	Amplitude = (4 * COMOB_DEC_FACTOR) + (rand() % (2 * COMOB_DEC_FACTOR));

	Behaviour_data = Add_COMOB_behaviour(Part->Main_COMOB, NULL,
	 Oscillate_handler);

	if (Behaviour_data)
	{
		Behaviour_data->Data.Oscillate_data.Type = OSCILLATE_Y;
		Behaviour_data->Data.Oscillate_data.Period = Period;
		Behaviour_data->Data.Oscillate_data.Value = Value;
		Behaviour_data->Data.Oscillate_data.Amplitude = Amplitude;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Warniak_show_death
 * FUNCTION  : Show Warniak death.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.10.95 20:33
 * LAST      : 09.10.95 20:33
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Warniak_show_death(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Storm_show_init
 * FUNCTION  : Initialize Storm show.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.10.95 12:08
 * LAST      : 11.10.95 12:08
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Storm_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	struct COMOB_behaviour *Behaviour_data;
	SILONG Amplitude;
	UNSHORT Period, Value;

	/* Do standard show init */
	Standard_monster_show_init(Part, P);

	/* Initialize oscillation of main COMOB along the Y-axis */
	Period = 60 + (rand() % 40);
	Value = rand() % Period;
	Amplitude = (4 * COMOB_DEC_FACTOR) + (rand() % (2 * COMOB_DEC_FACTOR));

	Behaviour_data = Add_COMOB_behaviour(Part->Main_COMOB, NULL,
	 Oscillate_handler);

	if (Behaviour_data)
	{
		Behaviour_data->Data.Oscillate_data.Type = OSCILLATE_Y;
		Behaviour_data->Data.Oscillate_data.Period = Period;
		Behaviour_data->Data.Oscillate_data.Value = Value;
		Behaviour_data->Data.Oscillate_data.Amplitude = Amplitude;
	}

	/* Initialize oscillation of main COMOB along the X-axis */
	Period = 60 + (rand() % 40);
	Value = rand() % Period;
	Amplitude = (4 * COMOB_DEC_FACTOR) + (rand() % (2 * COMOB_DEC_FACTOR));

	Behaviour_data = Add_COMOB_behaviour(Part->Main_COMOB, NULL,
	 Oscillate_handler);

	if (Behaviour_data)
	{
		Behaviour_data->Data.Oscillate_data.Type = OSCILLATE_X;
		Behaviour_data->Data.Oscillate_data.Period = Period;
		Behaviour_data->Data.Oscillate_data.Value = Value;
		Behaviour_data->Data.Oscillate_data.Amplitude = Amplitude;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_moving_projectile
 * FUNCTION  : Show a moving projectile.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.05.95 18:42
 * LAST      : 12.10.95 15:04
 * INPUTS    : struct Combat_participant *Attacker_part - Pointer to the
 *              attacker's combat participant data.
 *             UNSHORT Target_X - X-coordinate of target in tactical grid.
 *             UNSHORT Target_Y - Y-coordinate of target in tactical grid.
 *             UNSHORT Weapon_slot_index - Index of slot containing
 *              currently used weapon (1...9 / 0 for no weapon).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The projectile index is retrieved from the Misc_2 entry in
 *              the ammunition's item data, or the weapon's if no ammo is
 *              required.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_moving_projectile(struct Combat_participant *Attacker_part,
 UNSHORT Target_X, UNSHORT Target_Y, UNSHORT Weapon_slot_index)
{
	MEM_HANDLE Projectile_gfx_handle;
	struct Character_data *Attacker_char;
	struct Item_data *Weapon_item_data;
	struct Item_data *Ammo_item_data;
	struct COMOB *COMOB;
	SISHORT Nr_steps;
	UNSHORT Ammo_ID;
	UNSHORT Projectile_type = 0;

	/* Any weapon ? */
	if (Weapon_slot_index != NO_BODY_PLACE)
	{
		/* Yes -> Get attacker's character data */
		Attacker_char = (struct Character_data *)
		 MEM_Claim_pointer(Attacker_part->Char_handle);

		/* Get weapon item data */
		Weapon_item_data = Get_item_data(&(Attacker_char->Body_items[Weapon_slot_index - 1]));

		/* Get ammo ID from weapon */
		Ammo_ID = (UNSHORT) Weapon_item_data->Ammo_ID;

		/* Is ammunition required / anything in the left hand ? */
		if (Ammo_ID && !(Packet_empty(&(Attacker_char->Body_items[LEFT_HAND - 1]))))
		{
			/* Yes -> Get ammunition item data */
			Ammo_item_data = Get_item_data(&(Attacker_char->Body_items[LEFT_HAND - 1]));

			/* Is this item ammunition / is it the RIGHT ammunition ? */
			if ((Ammo_item_data->Type == AMMO_IT) &&
			 (Ammo_item_data->Ammo_ID == Ammo_ID))
			{
				/* Yes -> Get projectile type from ammo item data */
				Projectile_type = (UNSHORT) Ammo_item_data->Misc[1];
			}
			Free_item_data();
		}
		else
		{
			/* No -> Get projectile type from weapon item data */
			Projectile_type = (UNSHORT) Weapon_item_data->Misc[1];
		}
		Free_item_data();

		MEM_Free_pointer(Attacker_part->Char_handle);
	}

	/* Legal projectile type ? */
	if (Projectile_type >= MAX_PROJECTILES)
	{
		/* No -> Error */
		Error(ERROR_ILLEGAL_PROJECTILE_TYPE);
		return;
	}

	/* Make COMOB */
	COMOB = Add_COMOB(100);
	if (COMOB)
	{
		/* Prepare movement */
		Nr_steps = Prepare_COMOB_movement
		(
			Attacker_part,
			COMOB,
			Target_X,
			Target_Y,
			Projectile_table[Projectile_type].Speed
		);

		/* Going to or fro ? */
		if (COMOB->dZ_3D >= 0)
		{
			/* To -> Load projectile graphics */
			Projectile_gfx_handle = Load_subfile(COMBAT_GFX,
			 Projectile_table[Projectile_type].To_gfx_nr);
		}
		else
		{
			/* Fro -> Load projectile graphics */
			Projectile_gfx_handle = Load_subfile(COMBAT_GFX,
			 Projectile_table[Projectile_type].Fro_gfx_nr);
		}

		/* Initialize COMOB data */
		COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		COMOB->Display_width = Projectile_table[Projectile_type].Display_width;
		COMOB->Display_height = Projectile_table[Projectile_type].Display_height;

		COMOB->Graphics_handle = Projectile_gfx_handle;

		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

		/* Fiuwwww.... thud! */
		while (Nr_steps > 0)
		{
			Update_screen();
			Nr_steps -= Nr_combat_updates;
		}

		/* Destroy COMOB */
		Delete_COMOB(COMOB);

		/* Free graphics memory */
		MEM_Free_memory(Projectile_gfx_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_COMOB_movement
 * FUNCTION  : Prepare a COMOB for movement from a participant to a target
 *              square in the tactical grid.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.05.95 11:44
 * LAST      : 22.05.95 14:55
 * INPUTS    : struct Combat_participant *Source_part - Pointer to the
 *              data of the combat participant who is the source of the
 *              COMOB.
 *             struct COMOB *COMOB - Pointer to data of COMOB that is to be
 *              moved.
 *             UNSHORT Target_X - Target X-coordinate in tactical grid.
 *             UNSHORT Target_Y - Target Y-coordinate in tactical grid.
 *             UNSHORT Movement_speed - Movement speed.
 * RESULT    : UNSHORT : Number of steps to be taken.
 * BUGS      : No known.
 * NOTES     : - If the target coordinates contain a participant, it's
 *              coordinates will be used to determine the COMOB's movement
 *              vector.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Prepare_COMOB_movement(struct Combat_participant *Source_part,
 struct COMOB *COMOB, UNSHORT Target_X, UNSHORT Target_Y,
 UNSHORT Movement_speed)
{
	struct Combat_participant *Target_part;
	SILONG Target_X_3D;
	SILONG Target_Y_3D;
	SILONG Target_Z_3D;
	UNSHORT Nr_steps;

	/* Get source coordinates */
	Get_3D_part_coordinates(Source_part, &(COMOB->X_3D), &(COMOB->Y_3D),
	 &(COMOB->Z_3D));

	/* Is there any participant at the target coordinates ? */
	Target_part = Combat_matrix[Target_Y][Target_X].Part;
	if (Target_part)
	{
		/* Yes -> Use it's coordinates as target coordinates */
		Get_3D_part_coordinates(Target_part, &Target_X_3D, &Target_Y_3D,
		 &Target_Z_3D);
	}
	else
	{
		/* No -> Convert tactical to 3D coordinates */
		Convert_tactical_to_3D_coordinates(Target_X, Target_Y, &Target_X_3D,
		 &Target_Z_3D);

		/* Set Y-coordinate */
		Target_Y_3D = 0;
	}

	/* Flip ? */
	if (Determine_COMOB_flip(COMOB, Target_X_3D, Target_Y_3D, Target_Z_3D))
	{
		/* Yes */
		COMOB->User_flags |= COMOB_X_MIRROR;
	}

	/* Set movement vector */
	Nr_steps = Set_COMOB_vector(COMOB, Target_X_3D, Target_Y_3D, Target_Z_3D,
	 Movement_speed);

	return Nr_steps;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_COMOB_vector
 * FUNCTION  : Calculate the movement vector for a COMOB moving at a certain
 *              speed towards a certain target.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.05.95 12:17
 * LAST      : 16.05.95 12:17
 * INPUTS    : struct COMOB *COMOB - Pointer to data of COMOB that is to be
 *              moved.
 *             SILONG Target_X_3D - Target 3D X-coordinate.
 *             SILONG Target_Y_3D - Target 3D Y-coordinate.
 *             SILONG Target_Z_3D - Target 3D Z-coordinate.
 *             UNSHORT Movement_speed - Movement speed.
 * RESULT    : UNSHORT : Number of steps to be taken.
 * BUGS      : No known.
 * NOTES     : - The COMOB's current coordinates will be used as source
 *              coordinates.
 *             - The movement vector components will be stored in the
 *              COMOB's 3D vector data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Set_COMOB_vector(struct COMOB *COMOB, SILONG Target_X_3D, SILONG Target_Y_3D,
 SILONG Target_Z_3D, UNSHORT Movement_speed)
{
	SILONG dX_3D, dY_3D, dZ_3D;
	SILONG Length;
	UNSHORT Nr_steps;

	Movement_speed *= COMOB_DEC_FACTOR;

	/* Calculate vector */
	dX_3D = Target_X_3D - COMOB->X_3D;
	dY_3D = Target_Y_3D - COMOB->Y_3D;
	dZ_3D = Target_Z_3D - COMOB->Z_3D;

	/* Calculate vector length */
	Length = (dX_3D * dX_3D) + (dY_3D * dY_3D) + (dZ_3D * dZ_3D);
	if (Length)
	{
		Length = (SILONG) sqrt((double) Length);
//		Length /= COMOB_DEC_FACTOR;
	}

	/* Is length zero ? */
	if (Length)
	{
		/* No -> Calculate vector components */
		dX_3D *= (SILONG) Movement_speed;
		dY_3D *= (SILONG) Movement_speed;
		dZ_3D *= (SILONG) Movement_speed;

		dX_3D /= Length;
		dY_3D /= Length;
		dZ_3D /= Length;

		/* Calculate number of steps (at least one) */
		Nr_steps = max((Length / Movement_speed), 1);
	}
	else
	{
		/* Yes -> Clear vector */
		dX_3D = 0;
		dY_3D = 0;
		dZ_3D = 0;

		/* No steps */
		Nr_steps = 0;
	}

	/* Store vector components */
	COMOB->dX_3D = dX_3D;
	COMOB->dY_3D = dY_3D;
	COMOB->dZ_3D = dZ_3D;

	return Nr_steps;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Determine_COMOB_flip
 * FUNCTION  : Determine if a COMOB moving towards a certain target should
 *              be flipped.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.05.95 14:56
 * LAST      : 22.05.95 14:56
 * INPUTS    : struct COMOB *COMOB - Pointer to data of COMOB that is to be
 *              moved.
 *             SILONG Target_X_3D - Target 3D X-coordinate.
 *             SILONG Target_Y_3D - Target 3D Y-coordinate.
 *             SILONG Target_Z_3D - Target 3D Z-coordinate.
 * RESULT    : BOOLEAN : COMOB should be flipped.
 * BUGS      : No known.
 * NOTES     : - The COMOB's current coordinates will be used as source
 *              coordinates.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Determine_COMOB_flip(struct COMOB *COMOB, SILONG Target_X_3D,
 SILONG Target_Y_3D, SILONG Target_Z_3D)
{
	SILONG Source_X_3D;
	SILONG Source_Z_3D;
	SILONG Source_X_2D;
	SILONG Target_X_2D;
	SILONG Proj;

	/* Get source 3D coordinates */
	Source_X_3D = COMOB->X_3D / COMOB_DEC_FACTOR;
	Source_Z_3D = (COMOB->Z_3D / COMOB_DEC_FACTOR) + Combat_Z_offset;

	/* Calculate projection factor */
	Proj = Source_Z_3D + Combat_projection_factor;
	if (!Proj)
		Proj = 1;

	/* Project coordinates */
	Source_X_2D = (Source_X_3D * Combat_projection_factor) / Proj;

	/* Get target 3D coordinates */
	Target_X_3D = Target_X_3D / COMOB_DEC_FACTOR;
	Target_Z_3D = (Target_Z_3D / COMOB_DEC_FACTOR) + Combat_Z_offset;

	/* Calculate projection factor */
	Proj = Target_Z_3D + Combat_projection_factor;
	if (!Proj)
		Proj = 1;

	/* Project coordinates */
	Target_X_2D = (Target_X_3D * Combat_projection_factor) / Proj;

	/* Flip ? */
	if (Target_X_2D < Source_X_2D)
		return TRUE;
	else
		return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_part_rectangle
 * FUNCTION  : Get the 3D rectangle that covers a combat participant.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.06.95 18:47
 * LAST      : 25.07.95 18:47
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             SILONG *Left_3D_ptr - Pointer to left 3D X-coordinate.
 *             SILONG *Top_3D_ptr - Pointer to top 3D Y-coordinate.
 *             SILONG *Right_3D_ptr - Pointer to right 3D X-coordinate.
 *             SILONG *Bottom_3D_ptr - Pointer to bottom 3D Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The coordinates are multiplied with COMOB_DEC_FACTOR.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_part_rectangle(struct Combat_participant *Part, SILONG *Left_3D_ptr,
 SILONG *Top_3D_ptr, SILONG *Right_3D_ptr, SILONG *Bottom_3D_ptr)
{
	SILONG Left_3D, Top_3D;
	SILONG Dummy;

	/* Party or monster ? */
	if (Part->Type == PARTY_PART_TYPE)
	{
		/* Party -> Convert tactical coordinates */
		Convert_tactical_to_3D_coordinates(Part->Tactical_X, -1, &Left_3D,
		 &Dummy);

		/* Adjust X-coordinate */
		Left_3D -= 25 * COMOB_DEC_FACTOR;

		/* Set Y-coordinate */
		Top_3D = 105 * COMOB_DEC_FACTOR;

		/* Store coordinates */
		*Left_3D_ptr	= Left_3D;
		*Top_3D_ptr		= Top_3D;
		*Right_3D_ptr	= Left_3D + (50 * COMOB_DEC_FACTOR);
		*Bottom_3D_ptr	= Top_3D - (50 * COMOB_DEC_FACTOR);
	}
	else
	{
		/* Monster -> Get COMOB rectangle */
		Get_COMOB_rectangle
		(
			Part->Main_COMOB,
			Left_3D_ptr,
			Top_3D_ptr,
			Right_3D_ptr,
			Bottom_3D_ptr
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_3D_part_coordinates
 * FUNCTION  : Get the 3D coordinates of a combat participant.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.05.95 13:02
 * LAST      : 16.05.95 13:02
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             SILONG *X_3D_ptr - Pointer to 3D X-coordinate.
 *             SILONG *Y_3D_ptr - Pointer to 3D Y-coordinate.
 *             SILONG *Z_3D_ptr - Pointer to 3D Z-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_3D_part_coordinates(struct Combat_participant *Part, SILONG *X_3D_ptr,
 SILONG *Y_3D_ptr, SILONG *Z_3D_ptr)
{
	SILONG X_3D, Y_3D, Z_3D;
	UNSHORT Source_width, Source_height;
	UNSHORT Target_width, Target_height;

	/* Party or monster ? */
	if (Part->Type == PARTY_PART_TYPE)
	{
		/* Party -> Convert tactical coordinates */
		Convert_tactical_to_3D_coordinates(Part->Tactical_X, -1, &X_3D, &Z_3D);

		/* Set Y-coordinate */
		Y_3D = 80 * COMOB_DEC_FACTOR;

/*
	*Y_3D_ptr = Race_heights[Get_race(Part->Char_handle)];

	Get	Part_handle(a0),a1		; Get character's race
	moveq.l	#0,d1
	move.b	Char_race(a1),d1
	Free	Part_handle(a0)
	lea.l	Race_heights,a1		; Get height of race
	add.w	d1,d1
	move.w	0(a1,d1.w),d1
	mulu.w	#Projectile_height,d1	; Adjust
	divu.w	(a1),d1
*/

		/* COMOB must be in front */
		Z_3D += 1;
	}
	else
	{
		/* Monster -> Copy coordinates from main COMOB */
		X_3D = Part->Main_COMOB->X_3D;
		Y_3D = Part->Main_COMOB->Y_3D;
		Z_3D = Part->Main_COMOB->Z_3D;

		/* Get source dimensions of COMOB */
		Get_COMOB_source_size(Part->Main_COMOB, &Source_width, &Source_height);

		/* Calculate target dimensions of COMOB */
		Target_width = (Source_width * Part->Main_COMOB->Display_width) / 100;
		Target_height = (Source_height * Part->Main_COMOB->Display_height) /
		 100;

		/* Make adjustments for hot-spot */
		X_3D -= ((Target_width * Part->Main_COMOB->Hotspot_X_offset) / 100) *
		 COMOB_DEC_FACTOR;
		Y_3D += ((Target_height * Part->Main_COMOB->Hotspot_Y_offset) / 100) *
		 COMOB_DEC_FACTOR;

		/* Put the coordinates in the middle of the COMOB */
		X_3D += (Target_width / 2) * COMOB_DEC_FACTOR;
		Y_3D -= (Target_height / 2) * COMOB_DEC_FACTOR;

		/* Make adjustments for COMOB offsets */
/*		X_3D += Part->Main_COMOB->X_offset * COMOB_DEC_FACTOR;
		Y_3D += Part->Main_COMOB->Y_offset * COMOB_DEC_FACTOR; */

		/* COMOB must be in front */
		Z_3D -= 1;
	}

	/* Store coordinates */
	*X_3D_ptr = X_3D;
	*Y_3D_ptr = Y_3D;
	*Z_3D_ptr = Z_3D;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Convert_tactical_to_3D_coordinates
 * FUNCTION  : Convert tactical to 3D coordinates.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 19:08
 * LAST      : 16.05.95 13:04
 * INPUTS    : SISHORT Tactical_X - Tactical X-coordinate.
 *             SISHORT Tactical_Y - Tactical Y-coordinate.
 *             SILONG *X_3D_ptr - Pointer to output 3D X-coordinate.
 *             SILONG *Z_3D_ptr - Pointer to output 3D Z-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - An Y-coordinate of -1 indicates the party's position is
 *              meant.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Convert_tactical_to_3D_coordinates(SISHORT Tactical_X, SISHORT Tactical_Y,
 SILONG *X_3D_ptr, SILONG *Z_3D_ptr)
{
	/* Monster or party ? */
	if (Tactical_Y == -1)
	{
		/* Party -> Calculate X-coordinate */
		*X_3D_ptr = (((Tactical_X - 3) * 2) + 1) * 8 * COMOB_DEC_FACTOR;

		/* Set Z-coordinate */
		*Z_3D_ptr = PARTY_Z;
	}
	else
	{
		/* Monster -> Calculate X-coordinate */
		*X_3D_ptr = (((Tactical_X - 3) * 2) + 1) * Combat_square_width;

		/* Second row ? */
		if (Tactical_Y == 3)
		{
			/* Yes -> Set Z-coordinate */
//			*Z_3D_ptr = -232 * COMOB_DEC_FACTOR;
			*Z_3D_ptr = 0 - (Combat_square_depth / 3);
		}
		else
		{
			/* Calculate Z-coordinate */
			*Z_3D_ptr = (0 - (Tactical_Y - 2)) * Combat_square_depth;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Lock_part_damage_frame
 * FUNCTION  : Lock a combat participant in the damage frame.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 11:03
 * LAST      : 11.07.95 11:03
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Lock_part_damage_frame(struct Combat_participant *Part)
{
	struct Character_data *Char;
	UNSHORT Frame = 0xFFFF;
	UNSHORT Anim_length;

	/* Is a monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Clear current animation */
		Set_combat_animation(Part, NO_COMANIM);

		/* Get monster character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

		/* Has damage animation ? */
		Anim_length = Char->Anim_lengths[HIT_COMANIM];
		if (Anim_length)
		{
			/* Yes -> Get last frame */
			Frame = (UNSHORT)
			 Char->Anim_sequences[HIT_COMANIM][Anim_length - 1];
		}

		MEM_Free_pointer(Part->Char_handle);

		/* Any frame found ? */
		if (Frame != 0xFFFF)
		{
			/* Yes -> Set it */
			Part->Default_frame = Frame;
			Part->Main_COMOB->Frame = Frame * 2;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Unlock_part_damage_frame
 * FUNCTION  : Unlock a combat participant from the damage frame.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 11:04
 * LAST      : 11.07.95 11:05
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Unlock_part_damage_frame(struct Combat_participant *Part)
{
	struct Character_data *Char;
	UNSHORT Frame;

	/* Is a monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Clear current animation */
		Set_combat_animation(Part, NO_COMANIM);

		/* Get monster character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

		/* Get first frame of move animation */
		Frame = (UNSHORT) Char->Anim_sequences[MOVE_COMANIM][0];

		MEM_Free_pointer(Part->Char_handle);

		/* Set new default frame */
		Part->Default_frame = Frame;
		Part->Main_COMOB->Frame = Frame * 2;
	}
}

