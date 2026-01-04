
/* Text command variables */
struct Combat_participant *Combat_text_acting_part;
struct Combat_participant *Combat_text_victim_part;
UNSHORT Combat_text_damage;
UNSHORT Combat_text_weapon_item_index;


/*
; ---------- Find defender ------------------------
	lea.l	Combat_matrix,a1		; Get defender data
	move.w	Part_target(a0),d0
	lsl.w	#2,d0
	add.w	d0,a1
	move.l	(a1),d0			; Still there ?
	bne.s	.Yes
	clr.b	Part_action(a0)		; No -> Reset action
	move.w	#-1,Part_target(a0)
	Get	Part_handle(a0),a2		; Remove ammunition
	jsr	Remove_ammo
	Free	Part_handle(a0)
	tst.b	Party_or_monster		; Monster ?
	beq	.Party0
	move.l	Part_COMOB_ptr(a0),a1	; Yes -> Move forward
	subq.w	#1,COMOB_3D_Z(a1)
	Set_anim	Longrange_anim,a0		; Animate
	jsr	Wait_4_animation
	addq.w	#1,COMOB_3D_Z(a1)		; Move back
.Party0:	move.w	#211,d0			; "Missed !"
	jsr	Do_part_prompt
	bra	.Exit2
.Yes:	move.l	d0,a1
	Get	Part_handle(a0),a2		; Get character data
	Get	Part_handle(a1),a3

; *** R E G I S T E R   C O N T E N T S ***
;  a0 - Pointer to attacker's participant data
;  a1 - Pointer to defender's participant data
;  a2 - Pointer to attacker's character data
;  a3 - Pointer to defender's character data

; ---------- Check undamageability ----------------
	move.w	Body_conditions(a3),d0	; Undamageable ?
	and.w	#Damage_mask,d0
	beq.s	.Ok0
	jsr	Show_longrange_attack	; Yes
	move.w	#262,d0			; "Cannot damage !"
	jsr	Do_prompt
	bra	.Exit
; ---------- Check magical protection -------------
.Ok0:	move.w	Magic_bonus_armour(a3),d0	; Magical aura too strong ?
	cmp.w	Magic_bonus_weapon(a2),d0
	bls.s	.Ok1
	jsr	Show_longrange_attack	; Yes
	move.w	#212,d0			; "Magic too strong !"
	jsr	Do_part_prompt
	bra	.Exit
; ---------- Get number of attacks ----------------
.Ok1:	moveq.l	#0,d6			; Get
	move.b	Attacks_per_round(a2),d6
	bclr	#Part_hurried,Part_flags(a0)	; Hurried ?
	beq.s	.No
	add.w	d6,d6			; Yes -> Double
.No:	subq.w	#1,d6
.Loop:	sf	Abort_attack		; Clear
	jsr	Show_longrange_attack	; Show
; ---------- Probe attacker's attack skill --------
	move.w	SAttack(a2),d0		; Get attack skill
	add.w	SAttack+Magic(a2),d0
	jsr	Probe_100			; Monster -> Probe
	bpl.s	.Ok2
	move.w	#213,d0			; "Attack failed !"
	jsr	Do_part_prompt
	bra	.Done
; ---------- Probe defender's parade skill --------
.Ok2:	cmp.b	#Parade_action,Part_action(a1)	; Defending ?
	bne	.Ok3
	move.w	SParade(a2),d0		; Get parade skill
	add.w	SParade+Magic(a2),d0
	jsr	Probe_100			; Probe
	bpl.s	.Ok3
	move.w	#214,d0			; "Attack was deflected !"
	jsr	Do_part_prompt
	jsr	Break_attacker
	jsr	Break_parader
	bra	.Done
; ---------- Probe attacker's critical hit skill --
.Ok3:	btst	#End_monster_type,Monster_type_status(a3)	; Endmonster ?
	bne.s	.Ok4
	move.w	SCritical_hit(a2),d0	; Get critical hit skill
	add.w	SCritical_hit+Magic(a2),d0
	jsr	Probe_100			; Probe
	bmi.s	.Ok4
	move.w	#216,d0			; "Critical hit !"
	jsr	Do_part_prompt
	jsr	Break_attacker
	jsr	Break_defender
	move.w	Life_points(a3),d1		; Kill
	bra	.Damage
; ---------- Calculate afflicted damage -----------
.Ok4:	jsr	Calculate_damage_and_protection
	jsr	Break_attacker		; Break NOW!
	jsr	Break_defender
	sub.w	d2,d1			; Calculate difference
	bgt.s	.Do
	move.w	#215,d0			; "No damage was done !"
	jsr	Do_part_prompt
	bra	.Done
; ---------- Do damage ----------------------------
.Do:	jsr	Print_hitting		; Tell 'em
.Damage:	move.w	d1,d0			; For the records
	jsr	Process_damage
	cmp.b	#1,Part_type(a1)		; Party or monster ?
	bne.s	.Monster4
	move.l	a0,-(sp)			; Do damage
	move.l	a1,a0
	jsr	Do_combat_damage
	move.l	(sp)+,a0
	bra	.Done
.Monster4:	move.l	a0,-(sp)			; Do damage
	move.l	a1,a0
	jsr	Do_damage_to_monster
	move.l	(sp)+,a0
.Done:	tst.b	Abort_attack		; Abort ?
	beq.s	.Next
	clr.b	Part_action(a0)		; Reset action
	move.w	#-1,Part_target(a0)
	bra.s	.Exit
.Next:	dbra	d6,.Loop			; Next attack
.Exit:	Free	Part_handle(a0)		; Exit
	Free	Part_handle(a1)
.Exit2:	rts
*/

BOOLEAN Abort_attack;

void
Do_long_range_attack(struct Combat_participant *Attacker_part)
{
	struct Combat_participant *Defender_part;
	struct Character_data *Char;
	UNSHORT Nr_attacks;
	UNSHORT i;

	/* Find defender */
	Defender_part = Get_part_from_tactical(Attacker_part->Target_X,
	 Attacker_part->Target_Y);

	/* Get number of attacks per round */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Attacker_part->Char_handle);
	Nr_attacks = (UNSHORT) Char->Attacks_per_round;
	MEM_Free_pointer(Attacker_part->Char_handle);

	/* Is the attacker hurried ? */
	if (Attacker_part->Flags & PART_HURRIED)
	{
		/* Yes -> Double the number of attacks */
		Nr_attacks *= 2;
	}

	/* For each attack */
	for (i=0;<Nr_attacks;i++)
	{
		/* Perform action : Long range attack */

		/* Continue attack ? */
		if (Abort_attack)
		{
			/* No -> Abort attack */
			break;
		}
	}
}

/* Data :
 Attacker_part
 Defender_part
*/

{
	struct Character_data *Attacker_char;
	struct Character_data *Defender_char;
	UNSHORT Afflicted_damage;
	UNSHORT Current_weapon_slot_index;

	/* Clear abort attack flag */
	Abort_attack = FALSE;

	/* Get attacker's character data */
	Attacker_char = (struct Character_data *)
	 MEM_Claim_pointer(Attacker_part->Char_handle);

	/* Select (next) weapon */

	/* Remove ammunition */

	/* Show long-range attack */

	/* Show flying projectile */

	/* Is there a defender ? */
	if (Defender_part)
	{
		/* Yes -> Get defender's character data */
		Defender_char = (struct Character_data *)
		 MEM_Claim_pointer(Defender_part->Char_handle);

		/* Print "<attacker> attacks <defender> with <weapon>." */

		/* Attacker's attack skill successfully probed ? */
		if (Probe_skill(Attacker_part->Char_handle, LONG_RANGE_ATTACK))
		{
			/* Yes -> Can the defender be damaged ? */
			if (Get_conditions(Defender_part->Char_handle) & DAMAGE_MASK)
			{
				/* No -> Print "<defender> cannot be damaged." */

				/* Abort attack */
				Abort_attack = TRUE;
			}
			else
			{
				/* Yes -> Is the defender doing nothing /
				 is the defender's dexterity attribute successfully probed ? */
				if ((Defender_part->Current_action == NO_COMACT) &&
					(Probe_attribute(Defender_part->Char_handle, DEXTERITY))
				{
					/* Yes -> Print "Attack was deflected!" */

					/* No -> Try to break the attacker's weapon */
					Break_combat_item(Attacker_part, Current_weapon_slot_index);

					/* Try to break the defender's weapon & shield */
					Break_combat_item(Defender_part,
				}
				else
				{
					/* No -> Try to break the attacker's weapon */

					/* Try to break the defender's armour & helmet */

					/* Is the defender NOT an END MONSTER /
					 was the attacker's critical hit skill successfully probed ? */
					if (!(Defender_char->Flags & END_MONSTER) &&
						(Probe_skill(Attacker_part->Char_handle, CRITICAL_HIT)))
					{
						/* Yes -> Print "<attacker> made a critical hit!" */

						/* Set afflicted damage to maximum */
						Afflicted_damage = Defender->Life_points;
					}
					else
					{
						/* No -> Calculate afflicted damage */
						Calculate_afflicted_damage( ... );
					}

					/* Any damage done ? */
					if (Afflicted_damage)
					{
						/* Yes -> Print "<attacker> does <damage> damage! */

						/* Process damage for danger evaluation */

						/* Show damage */

						/* Do damage */

					}
					else
					{
						/* No -> Print "No damage done!" */

					}
				}
			}
		}
		else
		{
			/* No -> Print "<attacker> failed!" */

		}
	}
	else
	{
		/* No -> Print "<attacker> missed!" */

		/* Reset action */

		/* Abort attack */
		Abort_attack = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_ammo_and_reload
 * FUNCTION  : Remove the current weapon's ammunition and try to reload.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.03.95 18:08
 * LAST      : 10.03.95 18:08
 * INPUTS    : struct Combat_participant *Attacker_part - Pointer to the
 *              attacker's combat participant data.
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
Remove_ammo_and_reload(struct Combat_participant *Attacker_part,
 UNSHORT Current_weapon_slot_index)
{
	struct Character_data *Attacker_char;
	struct Item_data *Item_data;
	struct Item_data *Weapon_item_data;
	struct Item_data *Ammo_item_data;
	BOOLEAN Found;
	UNSHORT Ammo_item_index;
	UNSHORT i;

	/* Any weapon ? */
	if (Current_weapon_slot_index != NO_BODY_PLACE)
	{
		/* Yes ->*/
		Item_data = (struct Item_data *) MEM_Claim_pointer(Item_data_handle);

		/* Get weapon item data */
		Weapon_item_data = Item_data +
		 Attacker_char->Body_items[Current_weapon_slot_index - 1].Index - 1;

		/* Does this weapon require ammunition ? */
		if (Weapon_item_data->Ammo_ID)
		{
			/* Yes -> Get attacker character data */
			Attacker_char = (struct Character_data *)
			 MEM_Claim_pointer(Attacker_part->Char_handle);

			/* Get ammunition item index */
			Ammo_item_index = Attacker_char->Body_items[LEFT_HAND - 1];

			/* Get ammunition item data */
			Ammo_item_data = Item_data + Ammo_item_index - 1;

			/* Is this item ammunition / is it the RIGHT ammunition ? */
			if ((Ammo_item_data->Type == AMMO_IT) &&
			 (Weapon_item_data->Ammo_ID == Ammo_item_data->Ammo_ID))
			{
				/* Yes -> Search backpack for item with same index */
				Found = FALSE;
				for (i=0;i<ITEMS_PER_CHAR;i++)
				{
					/* Does the item have the same index ? */
					if (Attacker_char->Backpack_items[i].Index == Ammo_item_index)
					{
						/* Yes -> Remove the item */
						Remove_item(Attacker_part->Char_handle, i + ITEMS_ON_BODY + 1,
						 1);

						/* Found it! */
						Found = TRUE
						break;
					}

					/* Found something ? */
					if (!Found)
					{
						/* No -> Remove item in left hand */
						Remove_item(Attacker_part->Char_handle, LEFT_HAND, 1);

						/* Print "<attacker> used last ammo!" */
						Print_combat_message(Attacker_part, );

						/* Reset participant action */
						Attacker_part->Current_action = NO_COMACT;

						/* Abort attack */
						Abort_attack = TRUE;
					}
				}
			}
			MEM_Free_pointer(Attacker_part->Char_handle);
		}
		else
		{
			/* No -> Should this weapon be destroyed after use ? */
			if (Weapon_item_data->Flags & DESTROY_AFTER_USE)
			{
				/* Yes -> Destroy weapon */
				Remove_item(Attacker->Char_handle, Current_weapon_slot_index, 1);

				/* Reset participant action */
				Attacker_part->Current_action = NO_COMACT;

				/* Abort attack */
				Abort_attack = TRUE;
			}
		}
		MEM_Free_pointer(Item_data_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_combat_message
 * FUNCTION  : Print a combat message.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.03.95 16:20
 * LAST      : 12.03.95 16:20
 * INPUTS    : struct Combat_participant *Acting_part - Pointer to acting
 *              participant's data.
 *             struct Combat_participant *Victim_part - Pointer to victim
 *              participant data.
 *             UNSHORT Damage - Amount of damage.
 *             UNSHORT Weapon_item_index - Weapon item index.
 *             UNSHORT Message_nr - Message number.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This central function handles all text output during combat.
 *              It sets the global variables which are read by the {DAMG}-,
 *              {WEAP}-, {COMB}- and {VICT}-text commands. Any information
 *              not appropriate to / needed by the current message should be
 *              set to NULL or 0.
 *              By centralizing the access to these global variables it is
 *              guaranteed that they always contain a correct value.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_combat_message(struct Combat_participant *Acting_part, struct
 Combat_participant *Victim_part, UNSHORT Damage, UNSHORT Weapon_item_index,
 UNSHORT Message_nr)
{
	/* Set global combat text parameters */
	Combat_text_acting_part = Acting_part;
	Combat_text_victim_part = Victim_part;
	Combat_text_damage = Damage;
	Combat_text_weapon_item_index = Weapon_item_index;

	/* Print message */
	Execute_method(Permanent_text_object, SET_METHOD,
	 (void *) System_text_ptrs[Message_nr]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Break_combat_items
 * FUNCTION  : Try to break items used in combat.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 13:32
 * LAST      : 13.03.95 13:32
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Body_item_slot_mask - Bit-list indicating which
 *              body item slots should be checked.
 *             UNLONG Item_type_mask - Bit-list indicating which item types
 *              can break.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Break_combat_items(MEM_HANDLE Char_handle, UNSHORT Body_item_slot_mask,
 UNLONG Item_type_mask)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	struct Item_data *Body_item_data;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);
	Item_data = (struct Item_data *) MEM_Claim_pointer(Item_data_handle);

	/* Check all body slots */
	for (i=1;i<=ITEMS_ON_BODY;i++)
	{
		/* Check this slot ? */
		if (Body_item_slot_mask & (1 << i))
		{
			/* Yes -> Anything in this slot ? */
			if (Char->Body_items[i - 1].Index)
			{
				/* Yes -> Get item data */
				Body_item_data = Item_data[Char->Body_items[i - 1].Index - 1];

				/* Is it of the right type / not already broken ? */
				if ((Item_type_mask & (1 << Body_item_data->Type)) ||
				 !(Char->Body_items[i - 1].Flags & BROKEN_ITEM))
				{
					/* Yes -> Try to break it */
					if (Probe((UNSHORT) Body_item_data->Break_chance, 1000))
					{
						/* Broken -> Print message */

						/* Break item */
						Char->Body_items[i - 1].Flags |= BROKEN_ITEM;

						/* Put away */
						Put_item_in_apres_pool(&(Char->Body_items[i - 1]));

						/* Remove item from character */
						Remove_item(Char_handle, i, 1);
					}
				}
			}
		}
	}
	MEM_Free_pointer(Item_data_handle);
	MEM_Free_pointer(Char_handle);
}


/*
;*****************************************************************************
; [ Show moving projectile ]
;   IN : a0 - Pointer to attacker's participant data (.l)
;        a1 - Pointer to defender's participant data (.l)
;        a2 - Pointer to attacker's character data (.l)
;        a3 - Pointer to defender's character data (.l)
; All registers are restored
;*****************************************************************************
Show_moving_projectile:
	movem.l	d0/d1/d7/a0/a3,-(sp)
	lea.l	Object_data+4,a3		; Get weapon's ammo ID
	move.w	Char_inventory+Right_hand_slot+Object_index(a2),d0
	subq.w	#1,d0
	mulu.w	#Item_data_size,d0
	move.b	Ammo_use_ID(a3,d0.w),d0
	ext.w	d0
	lea.l	Projectile_table,a3		; Get projectile info
	mulu.w	#Proj_data_size,d0
	add.w	d0,a3
	jsr	Determine_COMOB_flip	; Determine flip
	smi	d1
	move.w	Proj_speed(a3),d0		; Prepare movement
	jsr	Prepare_COMOB_movement
	tst.b	d1			; Left or right ?
	beq.s	.Right
	bset	#0,COMOB_Mirror_flags(a0)	; Flip
.Right:	tst.w	COMOB_3D_vector_Z(a0)	; To or fro ?
	bpl.s	.To
	move.l	Proj_fro_gfx_base(a3),COMOB_Gfx_base(a0)
	bra.s	.Go_on
.To:	move.l	Proj_to_gfx_base(a3),COMOB_Gfx_base(a0)
.Go_on:	move.w	Proj_source_width(a3),COMOB_Source_width(a0)	; Initialize COMOB
	move.w	Proj_source_height(a3),COMOB_Source_height(a0)
	move.w	Proj_target_width(a3),COMOB_Display_width(a0)
	move.w	Proj_target_height(a3),COMOB_Display_height(a0)
	move.w	Proj_nr_frames(a3),d0
	move.b	FXGFX_handle,COMOB_Gfx_handle(a0)
	bra.s	.Entry
.Loop:	jsr	Update_combat_screen	; Show
	jsr	Circle_COMOB		; Animate
.Entry:	dbra	d7,.Loop
	jsr	Delete_COMOB		; Delete
.Exit:	movem.l	(sp)+,d0/d1/d7/a0/a3
	rts
*/

void
Show_moving_projectile(UNSHORT Source_X, UNSHORT Source_Y

