/************
 * NAME     : MONDISP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 9-3-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MONDISP.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <SORT.H>
#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <COMBAT.H>
#include <COMACTS.H>
#include <COMOBS.H>
#include <TACTICAL.H>
#include <EVELOGIC.H>
#include <MAGIC.H>
#include <APRES.H>
#include <XFTYPES.H>
#include <MUSIC.H>

/* global variables */

static Monster_display_handler
 Monster_display_handler_table[MAX_MONSTER_TYPES][MAX_MONDISPS] = {
	{Default_appear_mondisp_handler,
	 NULL,
	 Default_move_mondisp_handler,
	 NULL,
	 Default_short_range_mondisp_handler,
	 Default_long_range_mondisp_handler,
	 Default_magic_mondisp_handler,
	 Default_deflect_mondisp_handler,
	 Default_flee_mondisp_handler,
	 Default_damage_mondisp_handler,
	 Default_give_up_mondisp_handler,
	 Default_die_mondisp_handler }
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_monster_display
 * FUNCTION  : Do a certain display behaviour of a monster.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.03.95 13:57
 * LAST      : 09.03.95 13:57
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Mondisp_type - Monster display type.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_monster_display(struct Combat_participant *Part, UNSHORT Mondisp_type)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_appear_mondisp_handler
 * FUNCTION  : Default appear monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 15:40
 * LAST      : 13.03.95 15:40
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_appear_mondisp_handler(struct Combat_participant *Monster_part)
{
	/* Does this monster have an appear animation ? */
	if (Monster_has_animation(Monster_part, APPEAR_COMANIM)
	{
		/* Yes -> Set appear animation */
		Set_combat_animation(Monster_part, APPEAR_COMANIM);

		/* Wait for end of animation */
		Wait_4_combat_animation(Monster_part);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_move_mondisp_handler
 * FUNCTION  : Default move monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_move_mondisp_handler(struct Combat_participant *Monster_part)
{
}

/*
	cmp.b	#Stand_anim,Part_anim(a0)	; Already animating ?
	beq.s	.No
	jsr	Wait_4_animation		; Yes -> Wait
.No:	moveq.l	#0,d0			; Get destination Y
	move.w	Part_target(a0),d0
	add.w	d0,d0
	move.w	Part_target(a0,d0.w),d0
	divu.w	#6,d0
	cmp.w	Part_Y(a0),d0		; Move or retreat ?
	bmi.s	.Retreat
	move.w	#228,d0			; " moves!"
	jsr	Do_part_prompt
	bra.s	.Go_on
.Retreat:	move.w	#232,d0			; " retreats!"
	jsr	Do_part_prompt
.Go_on:	Get	Part_original_handle(a0),a1
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
.Exit:
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_short_range_mondisp_handler
 * FUNCTION  : Default short-range attack monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 15:43
 * LAST      : 13.03.95 15:43
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_short_range_mondisp_handler(struct Combat_participant *Monster_part)
{
	/* Move the monster forward */
	Monster_part->COMOB->Z_3D -= 1;

	/* Set short-range attack animation */
	Set_combat_animation(Monster_part, SHORT_RANGE_COMANIM);

	/* Wait for end of animation */
	Wait_4_combat_animation(Monster_part);

	/* Move the monster backward */
	Monster_part->COMOB->Z_3D += 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_long_range_mondisp_handler
 * FUNCTION  : Default long-range attack monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 15:44
 * LAST      : 13.03.95 15:44
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_long_range_mondisp_handler(struct Combat_participant *Monster_part)
{
	/* Move the monster forward */
	Monster_part->COMOB->Z_3D -= 1;

	/* Set long-range attack animation */
	Set_combat_animation(Monster_part, LONG_RANGE_COMANIM);

	/* Show moving projectile */
	Show_moving_projectile(Monster_part);

	/* Wait for end of animation */
	Wait_4_combat_animation(Monster_part);

/*
	jsr	Remove_ammo		; Remove ammunition
*/

	/* Move the monster backward */
	Monster_part->COMOB->Z_3D += 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_magic_mondisp_handler
 * FUNCTION  : Default magical attack monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 15:45
 * LAST      : 13.03.95 15:45
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_magic_mondisp_handler(struct Combat_participant *Monster_part)
{
	/* Set magical attack attack animation */
	Set_combat_animation(Monster_part, MAGIC_COMANIM);

	/* Wait for end of animation */
	Wait_4_combat_animation(Monster_part);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_deflect_mondisp_handler
 * FUNCTION  : Default magic deflect monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_deflect_mondisp_handler(struct Combat_participant *Monster_part)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_flee_mondisp_handler
 * FUNCTION  : Default flee monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_flee_mondisp_handler(struct Combat_participant *Monster_part)
{
}

/*
;*****************************************************************************
; [ Show flight ]
;   IN : a0 - Pointer to participant data (.l)
; All registers are restored
;*****************************************************************************
Show_flight:
	movem.l	d7/a0,-(sp)
	Set_anim	Stand_anim,a0		; Move back...
	move.l	Part_COMOB_ptr(a0),a0
	move.w	#-2,COMOB_3D_vector_Y(a0)	; ...and down
	move.w	#50,COMOB_3D_vector_Z(a0)
	moveq.l	#20-1,d7
.Loop:	jsr	Update_combat_screen
	dbra	d7,.Loop
	jsr	Delete_COMOB		; Delete
	movem.l	(sp)+,d7/a0
	rts
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_damage_mondisp_handler
 * FUNCTION  : Default receive damage monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_damage_mondisp_handler(struct Combat_participant *Monster_part)
{
}

/*
	movem.l	d0/a0,-(sp)
	Set_anim	Hit_anim,a0		; Hit animation
	move.l	a0,a1			; Create blood explosion
	move.l	Part_COMOB_ptr(a1),a2
	jsr	Add_COMOB
	move.w	COMOB_3D_X(a2),COMOB_3D_X(a0)	; Set position
	move.w	COMOB_Display_height(a2),d0
	lsr.w	#1,d0
	sub.w	#16,d0
	bpl.s	.Pos
	moveq.l	#0,d0
.Pos:	move.w	d0,COMOB_3D_Y(a0)
	move.w	COMOB_3D_Z(a2),d0
	subq.w	#1,d0
	move.w	d0,COMOB_3D_Z(a0)
	move.w	Part_damage(a1),d0		; Set size
	lsr.w	#2,d0
	cmp.w	#50,d0
	bmi.s	.Ok1
	moveq.l	#50,d0
.Ok1:	add.w	#32,d0
	move.w	d0,COMOB_Display_width(a0)
	move.w	d0,COMOB_Display_height(a0)
	move.w	#32,COMOB_Source_width(a0)	; Set other data
	move.w	#32,COMOB_Source_height(a0)
	move.b	FXGFX_handle,COMOB_Gfx_handle(a0)
	move.l	#Blood_FXGFX,COMOB_Gfx_base(a0)
	moveq.l	#4,d0			; Show
	moveq.l	#4-1,d1
.Loop:	jsr	Update_combat_screen
	jsr	Update_combat_screen
	jsr	Circle_COMOB
	dbra	d1,.Loop
	jsr	Delete_COMOB		; Delete
	move.l	a1,a0			; Wait
	jsr	Wait_4_animation
	movem.l	(sp)+,d0/a0
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_give_up_mondisp_handler
 * FUNCTION  : Default give up monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_give_up_mondisp_handler(struct Combat_participant *Monster_part)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_die_mondisp_handler
 * FUNCTION  : Default die monster display behaviour.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Default_die_mondisp_handler(struct Combat_participant *Monster_part)
{
}

/*
;*****************************************************************************
; [ Show monster death ]
;   IN : a0 - Pointer to monster's participant data (.l)
; All registers are	restored
; Notes :
;   - This routine shows a death by damage (i.e. non-magical).
;*****************************************************************************
Show_monster_death:
	movem.l	d7/a0/a1,-(sp)
	Get	Part_original_handle(a0),a1	; Die animation ?
	move.b	Anim_frame+Die_anim-1(a1),d0
	Free	Part_original_handle(a0)
	tst.b	d0
	beq	.Skip
	Set_anim	Die_anim,a0		; Yes -> Die !
	jsr	Wait_4_animation
.Skip:	move.l	Part_COMOB_ptr(a0),a0	; Remove COMOB
	move.l	a0,a1
	jsr	Delete_COMOB
	jsr	Add_COMOB			; Create new one
	move.w	COMOB_3D_X(a1),COMOB_3D_X(a0)	; Initialize
	move.w	COMOB_3D_Y(a1),COMOB_3D_Y(a0)
	move.w	COMOB_3D_Z(a1),COMOB_3D_Z(a0)
	move.w	COMOB_Display_width(a1),d0
	move.w	d0,d1
	lsr.w	#2,d1
	add.w	d1,d0
	move.w	d0,COMOB_Display_width(a0)
	move.w	COMOB_Display_height(a1),d0
	move.w	d0,d1
	lsr.w	#2,d1
	add.w	d1,d0
	move.w	d0,COMOB_Display_height(a0)
	move.w	#48,COMOB_Source_width(a0)
	move.w	#59,COMOB_Source_height(a0)
	move.b	FXGFX_handle,COMOB_Gfx_handle(a0)
	move.l	#Explosion_FXGFX,COMOB_Gfx_base(a0)
	moveq.l	#14-1,d7			; Show & animate
.Loop:	jsr	Update_combat_screen
	addq.w	#1,COMOB_Frame(a0)
	bset	#COMOB_update,COMOB_Flags(a0)
	dbra	d7,.Loop
	jsr	Delete_COMOB		; Delete
	movem.l	(sp)+,d7/a0/a1
	rts
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Monster_has_animation
 * FUNCTION  : Check if a monster has a certain animation.
 * FILE      : MONDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 15:35
 * LAST      : 13.03.95 15:35
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 *             UNSHORT Anim_type - Animation type.
 * RESULT    : BOOLEAN : Monster has animation.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Monster_has_animation(struct Combat_participant *Monster_part,
 UNSHORT Anim_type)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Get monster character data */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Monster_part->Char_handle);

	/* Does this monsters have the animation ? */
	Result = (Char->Anim_lengths[Anim_type] > 0);

	MEM_Free_pointer(Monster_part->Char_handle);

	return(Result);
}

