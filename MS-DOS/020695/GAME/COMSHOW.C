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
#include <SCROLBAR.H>
#include <COMBAT.H>
#include <COMSHOW.H>
#include <COMOBS.H>
#include <TACTICAL.H>
#include <EVELOGIC.H>
#include <MAGIC.H>
#include <APRES.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <INVITEMS.H>
#include <GAMETEXT.H>

/* global variables */

static struct Combat_show_set Combat_show_sets[] = {
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
		NULL,
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
		Standard_monster_show_deflect,
		Standard_monster_show_give_up,
		Standard_monster_show_appear
	},
	/* Demon hand show set */
	{
		Demon_hand_show_init,
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
		Standard_monster_show_give_up,
		Standard_monster_show_appear
	}
};

static struct Projectile_data Projectile_table[MAX_PROJECTILES] = {
	/* Arrow */
	{
		49, 50,
		150, 150,
		20
	},
	/* Bolt */
	{
		51, 52,
		150, 150,
		25
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
 * LAST      : 13.05.95 13:55
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

	/* Legal show type ? */
	if (Show_type < MAX_SHOW_TYPES)
	{
		/* Yes -> Get address of show handler */
		Handler = Combat_show_sets[Part->Show_set_index].Handlers[Show_type];

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
 * LAST      : 15.05.95 18:13
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
	COMOB = Add_COMOB();

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
	Show_moving_projectile(Part, P->Attack_parms.Target_X,
	 P->Attack_parms.Target_Y, P->Attack_parms.Weapon_slot_index);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_show_damage
 * FUNCTION  : Show party receiving damage behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.05.95 18:13
 * LAST      : 15.05.95 18:13
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

	/* Add scratches COMOB to participant */
	COMOB = Add_COMOB();
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

	/* Destroy scratches memory */
	MEM_Free_memory(Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_show_deflect
 * FUNCTION  : Show party deflecting magic behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_init
 * FUNCTION  : Initialize standard monster show.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 11:43
 * LAST      : 10.05.95 11:43
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
	struct COMOB *Shadow_COMOB;
	struct Gfx_header *Gfx;
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

		return;
	}

	/* Calculate and store tactical icon number */
	Part->Tactical_icon_nr = PARTY_CHARS_MAX + Char->Monster_graphics_nr;

	/* Add main and shadow COMOBs */
	Main_COMOB = Add_COMOB();
	Shadow_COMOB = Add_COMOB();

	/* Attach COMOBs to participant data */
	Part->Main_COMOB = Main_COMOB;
	Part->Shadow_COMOB = Shadow_COMOB;

	/* Calculate 3D coordinates for main COMOB */
	Convert_tactical_to_3D_coordinates(Part->Tactical_X,
	 Part->Tactical_Y, &(Main_COMOB->X_3D), &(Main_COMOB->Z_3D));
	Main_COMOB->Y_3D = (0 - (SILONG) Char->Baseline) * COMOB_DEC_FACTOR;

	/* Copy to shadow COMOB */
	Shadow_COMOB->X_3D = Main_COMOB->X_3D;
	Shadow_COMOB->Z_3D = Main_COMOB->Z_3D;
	Shadow_COMOB->Y_3D = 0;

	/* Copy display dimensions to main COMOB */
	Main_COMOB->Display_width = Char->X_factor;
	Main_COMOB->Display_height = Char->Y_factor;

	/* Set data for shadow COMOB */
	Shadow_COMOB->Display_width = Main_COMOB->Display_width;
	Shadow_COMOB->Display_height = Main_COMOB->Display_height;

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

	/* Set current frames for COMOBs */
	Main_COMOB->Frame = Frame * 2;
	Shadow_COMOB->Frame = Frame * 2 + 1;

	/* Set monster's default frame */
	Part->Default_frame = Frame;

	/* Set shadow priority */
	Shadow_COMOB->Priority = 90;

	/* Set COMOB hot-spots */
	Main_COMOB->Hotspot_X_offset = 50;
	Main_COMOB->Hotspot_Y_offset = 100;

	Shadow_COMOB->Hotspot_X_offset = 50;
	Shadow_COMOB->Hotspot_Y_offset = 50;

	/* Set main COMOB draw mode */
	Main_COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

	/* Set shadow COMOB draw mode */
	Shadow_COMOB->User_flags |= COMOB_SHADOW;
	Shadow_COMOB->Draw_mode = COLOURING_COMOB_DRAWMODE;
	Shadow_COMOB->Special_handle = NULL;
	Shadow_COMOB->Special_offset = (UNLONG) &(Recolour_tables[0][0]);

	/* Set COMOB graphics handles and offsets */
	Main_COMOB->Graphics_handle = Part->Graphics_handle;
	Main_COMOB->Graphics_offset = 0;

	Shadow_COMOB->Graphics_handle = Part->Graphics_handle;
	Shadow_COMOB->Graphics_offset = 0;

	/* Set approximated graphics dimensions */
	Gfx = (struct Gfx_header *) MEM_Claim_pointer(Part->Graphics_handle);
	Part->Gfx_width = Gfx->Width;
	Part->Gfx_height = Gfx->Height;
	MEM_Free_pointer(Part->Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_exit
 * FUNCTION  : Exit standard monster show.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 18:02
 * LAST      : 10.05.95 18:02
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
	MEM_Free_memory(Part->Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_move
 * FUNCTION  : Show standard monster movement behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
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
}

/*
 	Get	Part_original_handle(a0),a1
	moveq.l	#0,d5			; Get number of frames
	move.b	Anim_frame(a1),d5
;	btst	#0,Anim_motion(a1)		; Wave or circle ?
;	beq.s	.Circle
;	move.w	d5,d0			; Wave
;	subq.w	#1,d0
;	add.w	d0,d5
.Circle:	Free	Part_original_handle(a0)
	tst.w	d5			; At least one
	bne.s	.Some
	moveq.l	#1,d5
.Some:	bset	#Part_handanimated,Part_flags(a0)	; Prepare animation
	lea.l	Combat_matrix,a1		; Do moves
	lea.l	Part_target(a0),a2
	move.l	Part_COMOB_ptr(a0),a3
	move.w	(a2)+,d7			; Get number of moves
	bra	.Entry1
.Loop1:	move.w	(a2)+,d6			; Get destination
	move.w	d6,d0
	lsl.w	#2,d0
	tst.l	0(a1,d0.w)		; Empty ?
	bne	.Done
	move.l	a0,0(a1,d0.w)		; Copy to destination
	Set_anim	Stand_anim,a0
	move.w	Part_Y(a0),d0		; Clear source
	mulu.w	#6,d0
	add.w	Part_X(a0),d0
	lsl.w	#2,d0
	clr.l	0(a1,d0.w)
	moveq.l	#0,d0			; Calculate new coordinates
	move.w	d6,d0
	divu.w	#6,d0
	move.w	d0,d1
	swap	d0
	move.w	d0,Part_X(a0)		; Store
	move.w	d1,Part_Y(a0)
	jsr	Calculate_3D_coordinates	; Calculate target coordinates
	sub.w	COMOB_3D_X(a3),d0		; Calculate total vector
	sub.w	COMOB_3D_Z(a3),d1
	ext.l	d0
	ext.l	d1
	divs.w	d5,d0			; Calculate movement vector
	divs.w	d5,d1
	move.w	d0,COMOB_3D_vector_X(a3)	; Set vector
	move.w	d1,COMOB_3D_vector_Z(a3)
	move.w	d5,d6			; Show movement
	subq.w	#1,d6
.Loop2:	jsr	Do_animation_update
	jsr	Update_combat_screen
	dbra	d6,.Loop2
	clr.w	COMOB_3D_vector_X(a3)	; Clear vector
	clr.w	COMOB_3D_vector_Z(a3)
.Entry1:	dbra	d7,.Loop1			; Next move
.Done:	bclr	#Part_handanimated,Part_flags(a0)
	jsr	Wait_4_animation		; Wait
*/

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
	Part->Shadow_COMOB->Z_3D -= 1;

	/* Set close-range attack animation */
	Set_combat_animation(Part, CLOSE_RANGE_COMANIM);

	/* Wait for end of animation */
	Wait_4_combat_animation(Part);

	/* Move monster back again */
	Part->Main_COMOB->Z_3D += 1;
	Part->Shadow_COMOB->Z_3D += 1;
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
	Part->Shadow_COMOB->Z_3D -= 1;

	/* Set long-range attack animation */
	Set_combat_animation(Part, LONG_RANGE_COMANIM);

	/* Show moving projectile */
	Show_moving_projectile(Part, P->Attack_parms.Target_X,
	 P->Attack_parms.Target_Y, P->Attack_parms.Weapon_slot_index);

	/* Wait for end of animation */
	Wait_4_combat_animation(Part);

	/* Move monster back again */
	Part->Main_COMOB->Z_3D += 1;
	Part->Shadow_COMOB->Z_3D += 1;
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
	struct COMOB *Shadow_COMOB;
	UNSHORT i;

	/* Set move animation */
	Set_combat_animation(Part, MOVE_COMANIM);

	/* Get COMOBs */
	Main_COMOB = Part->Main_COMOB;
	Shadow_COMOB = Part->Shadow_COMOB;

	for(i=0;i<20;i++)
	{
		/* Move monster back */
		Main_COMOB->Z_3D += 50 * COMOB_DEC_FACTOR;
		Shadow_COMOB->Z_3D += 50 * COMOB_DEC_FACTOR;

		Update_screen();
	}

	/* Delete combat objects */
	Delete_COMOB(Main_COMOB);
	Delete_COMOB(Shadow_COMOB);

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
 * LAST      : 12.05.95 11:06
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

	/* Set hit animation */
	Set_combat_animation(Part, HIT_COMANIM);

	/* Load blood */
	Graphics_handle = Load_subfile(COMBAT_GFX, 46);

	/* Add blood COMOB to participant */
	COMOB = Add_COMOB();
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

	/* Destroy blood memory */
	MEM_Free_memory(Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_death
 * FUNCTION  : Show standard monster dying behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.05.95 11:06
 * LAST      : 12.05.95 11:06
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

	struct COMOB *COMOB;
	MEM_HANDLE Graphics_handle;
	SILONG Y_3D;
	UNSHORT Nr_frames;
	UNSHORT i;

	/* Has death animation ? */
	if (Has_combat_animation(Part, DIE_COMANIM))
	{
		/* Yes -> Set death animation */
		Set_combat_animation(Part, DIE_COMANIM);

		/* Wait */
		Wait_4_combat_animation(Part);

		/* Delete monster COMOBs */
		Delete_COMOB(Part->Main_COMOB);
		Delete_COMOB(Part->Shadow_COMOB);
	}
	else
	{
		/* No -> Load explosion */
	 	Graphics_handle = Load_subfile(COMBAT_GFX, 43);

		/* Add explosion COMOB */
		COMOB = Add_COMOB();

		/* Copy position from monster COMOB */
		COMOB->X_3D = Part->Main_COMOB->X_3D;
		COMOB->Y_3D = Part->Main_COMOB->Y_3D;
		COMOB->Z_3D = Part->Main_COMOB->Z_3D;

		/* Copy size from monster COMOB */
		COMOB->Display_width =
		 Part->Main_COMOB->Display_width * 2;
		COMOB->Display_height =
		 Part->Main_COMOB->Display_height * 2;

		/* Set COMOB parameters */
		COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 100;

		COMOB->Graphics_handle = Graphics_handle;

		COMOB->Special_handle = Luminance_table_handle;

		/* Delete monster COMOBs */
		Delete_COMOB(Part->Main_COMOB);
		Delete_COMOB(Part->Shadow_COMOB);

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

		/* Destroy explosion memory */
		MEM_Free_memory(Graphics_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_deflect
 * FUNCTION  : Show standard monster deflecting magic behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Standard_monster_show_give_up
 * FUNCTION  : Show standard monster giving up behaviour.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Standard_monster_show_give_up(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
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
		Part->Shadow_COMOB->Frame = Frame * 2 + 1;
	}

	MEM_Free_pointer(Part->Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Demon_hand_show_init
 * FUNCTION  : Initialize demon hand show.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.05.95 15:04
 * LAST      : 17.05.95 15:04
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             union Combat_show_parms *P - Pointer to parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Demon_hand_show_init(struct Combat_participant *Part,
 union Combat_show_parms *P)
{
	union COMOB_behaviour_data *Behaviour_data;
	SILONG Amplitude;
	UNSHORT Period, Value;

	/* Do standard show init */
	Standard_monster_show_init(Part, P);

	/* Set main COMOB draw mode */
	Part->Main_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	Part->Main_COMOB->Special_handle = Transluminance_table_handle;

	/* Initialize oscillation of main COMOB along the Y-axis */
	Period = 30 + (rand() % 20);
	Value = rand() % Period;
	Amplitude = (4 * COMOB_DEC_FACTOR) + (rand() % (2 * COMOB_DEC_FACTOR));

	Behaviour_data = Add_COMOB_behaviour(Part->Main_COMOB, NULL,
	 Oscillate_handler);

	Behaviour_data->Oscillate_data.Type = OSCILLATE_Y;
	Behaviour_data->Oscillate_data.Period = Period;
	Behaviour_data->Oscillate_data.Value = Value;
	Behaviour_data->Oscillate_data.Amplitude = Amplitude;

	/* Initialize oscillation of main COMOB along the X-axis */
	Period = 30 + (rand() % 20);
	Value = rand() % Period;
	Amplitude = (4 * COMOB_DEC_FACTOR) + (rand() % (2 * COMOB_DEC_FACTOR));

	Behaviour_data = Add_COMOB_behaviour(Part->Main_COMOB, NULL,
	 Oscillate_handler);

	Behaviour_data->Oscillate_data.Type = OSCILLATE_X;
	Behaviour_data->Oscillate_data.Period = Period;
	Behaviour_data->Oscillate_data.Value = Value;
	Behaviour_data->Oscillate_data.Amplitude = Amplitude;

	/* Copy oscillation along the X-axis for shadow COMOB */
	Behaviour_data = Add_COMOB_behaviour(Part->Shadow_COMOB, NULL,
	 Oscillate_handler);

	Behaviour_data->Oscillate_data.Type = OSCILLATE_X;
	Behaviour_data->Oscillate_data.Period = Period;
	Behaviour_data->Oscillate_data.Value = Value;
	Behaviour_data->Oscillate_data.Amplitude = Amplitude;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_moving_projectile
 * FUNCTION  : Show a moving projectile.
 * FILE      : COMSHOW.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.05.95 18:42
 * LAST      : 15.05.95 18:42
 * INPUTS    : struct Combat_participant *Attacker_part - Pointer to the
 *              attacker's combat participant data.
 *             UNSHORT Target_X - X-coordinate of target in tactical grid.
 *             UNSHORT Target_Y - Y-coordinate of target in tactical grid.
 *             UNSHORT Current_weapon_slot_index - Index of slot containing
 *              currently used weapon (1...9 / 0 for no weapon).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If a monster uses a long-range weapon without ammunition,
 *              this function can still be called safely by setting the
 *              Current_weapon_slot_index to 0.
 *             - No matter in *which* slot the weapon is, the ammunition is
 *              always assumed to be in the left hand. However, the
 *              function will check if this is the case.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_moving_projectile(struct Combat_participant *Attacker_part,
 UNSHORT Target_X, UNSHORT Target_Y, UNSHORT Current_weapon_slot_index)
{
	MEM_HANDLE Projectile_gfx_handle;
	struct Character_data *Attacker_char;
	struct Item_data *Weapon_item_data;
	struct COMOB *COMOB;
	UNSHORT Projectile_type = 0;
	UNSHORT Nr_steps;
	UNSHORT i;

	/* Get attacker's character data */
	Attacker_char = (struct Character_data *)
	 MEM_Claim_pointer(Attacker_part->Char_handle);

	/* Any weapon ? */
	if (Current_weapon_slot_index != NO_BODY_PLACE)
	{
		/* Yes -> Get weapon item data */
		Weapon_item_data =
		 Get_item_data(&(Attacker_char->Body_items[Current_weapon_slot_index -
		 1]));

		/* Get projectile type */
		Projectile_type = (UNSHORT) Weapon_item_data->Misc[1];

		Free_item_data();
	}

	MEM_Free_pointer(Attacker_part->Char_handle);

	/* Legal projectile type ? */
	if (Projectile_type >= MAX_PROJECTILES)
	{
		/* No -> Error */
		Error(ERROR_ILLEGAL_PROJECTILE_TYPE);
		return;
	}

	/* Make COMOB */
	COMOB = Add_COMOB();

	/* Prepare movement */
	Nr_steps = Prepare_COMOB_movement(Attacker_part, COMOB,
	 Target_X, Target_Y, Projectile_table[Projectile_type].Speed);

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
	for (i=0;i<Nr_steps;i++)
	{
		Update_screen();
	}

	/* Destroy COMOB */
	Delete_COMOB(COMOB);
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

	return(Nr_steps);
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

	return(Nr_steps);
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
		return(TRUE);
	else
		return(FALSE);
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

		/* Calculate final width and height of COMOB */
		Target_width = (Part->Gfx_width *
		 Part->Main_COMOB->Display_width) / 100;
		Target_height = (Part->Gfx_height *
		 Part->Main_COMOB->Display_height) / 100;

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

