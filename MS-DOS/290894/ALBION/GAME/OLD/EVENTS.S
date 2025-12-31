; Event handling
; Written by J.Horneman (In Tune With The Universe)
; Start : 3-5-1994

; - Success & break flag setting must be handled IN events
; - Don't forget to clear the success flag at the start of certain events!
; - Init_display call has been removed

	SECTION	Program,code
;*****************************************************************************
; [ Perform an action ]
;   IN : d0 - Action type (.w)
;        d1 - Action value (.w)
;        d2 - Action extra value (.b)
;        d3 - Actor index (.b)
;        d4 - List modes (.b)
;        a0 - Pointer to action routine (.l)
; All registers are restored
; Notes :
;   - It is assumed that this action will execute by default.
;*****************************************************************************
Perform_action:
	movem.l	a0/a1,-(sp)
	move.l	a0,a1
	jsr	Push_Set_list		; Push
	move.b	d0,Action_type(a0)		; Initialize
	move.w	d1,Action_value(a0)
	move.b	d2,Action_extra(a0)
	move.b	d3,Actor_index(a0)
	move.b	d4,Set_list_modes(a0)
	move.l	a1,Action_ptr(a0)
	move.l	#Standard_set_list,Action_list_ptr
	jsr	Execute_set_list		; Execute
	jsr	Pop_Set_list		; Pop
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Execute the current set list ]
; All registers are restored
; Notes :
;   - This routine must be re-entrant.
;   - The Execute and Signal events will call this routine.
;   - This routine assumes that the Set list structure has been initialized
;      properly.
;*****************************************************************************
Execute_set_list:
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	Set_list_Sp,a6
; ---------- Handle the set list ----------------
.Next:	move.l	Set_list_ptr(a6),a0		; Get list pointer
	cmp.w	#-1,(a0)			; End of list ?
	beq	.Done
	moveq.l	#0,d1			; No -> Get set
	move.b	(a0)+,d1
	moveq.l	#0,d0
	move.b	(a0)+,d0
	move.l	a0,Set_list_ptr(a6)		; Store list pointer
	move.l	a6,-(sp)			; Execute
	move.l	.Ptrs(pc,d1.w*4),a1
	jsr	(a1)
	move.l	(sp)+,a6
	btst	#Have_executed,Set_list_flags(a6)	; Executed ?
	beq.s	.Next
	bra.s	.Exit			; Yes -> Exit
; ---------- Execute the action -------------------
.Done:	move.b	Set_list_flags(a6),d0	; Get flags
	btst	#Have_executed,d0		; Already executed ?
	bne.s	.Exit
	btst	#Forbidden,d0		; No -> Forbidden ?
	bne.s	.Exit
.Do:	move.l	Action_ptr(a6),a0		; Execute
	jsr	(a0)
	bset	#Have_executed,Set_list_flags(a6)	; Set flag
.Exit:	movem.l	(sp)+,d0-d7/a0-a6
	rts

.Ptrs:	dc.l Handle_map_set
	dc.l Handle_default_set
	dc.l Handle_item_set
	dc.l Handle_magic_set
	dc.l Handle_dialogue_set
	dc.l Handle_acting_member_set
	dc.l Handle_non_acting_member_set
	dc.l Handle_acting_part_set
	dc.l Handle_friend_part_set
	dc.l Handle_enemy_part_set

;*****************************************************************************
; [ Handle the map event set ]
; All registers are restored
;*****************************************************************************
Handle_map_set:
; Check local events
; Check global events
	rts

;*****************************************************************************
; [ Handle the default event set ]
; All registers are restored
;*****************************************************************************
Handle_default_set:
	movem.l	d0/d1/d6/a0/a1,-(sp)
	move.b	Default_event_set_handle,d0	; Search
	moveq.l	#-1,d6
	jsr	Search_event_set	
	cmp.w	#-1,d1			; Found anything ?
	beq	.Exit
	jsr	Push_event_context		; Yes -> Make new context
	sub.l	a1,a1
	jsr	Prepare_event_set_context
	move.b	Default_event_text_handle,Event_text_handle(a0)
	jsr	Execute_event_chain		; Execute event chain
.Exit:	movem.l	(sp)+,d0/d1/d6/a0/a1
	rts

;*****************************************************************************
; [ Handle the item event set ]
; All registers are restored
;*****************************************************************************
Handle_item_set:
	movem.l	d0/d1/d6/a0/a1,-(sp)
	move.l	Set_list_Sp,a0		; Using an item ?
	btst	#Item_set_mode,Set_list_modes(a0)
	beq.s	.Exit
	move.b	Item_event_set_handle,d0	; Yes -> Search
	moveq.l	#-1,d6
	jsr	Search_event_set	
	cmp.w	#-1,d1			; Found anything ?
	beq	.Exit
	jsr	Push_event_context		; Yes -> Make new context
	sub.l	a1,a1
	jsr	Prepare_event_set_context
	move.b	Item_event_text_handle,Event_text_handle(a0)
	jsr	Execute_event_chain		; Execute event chain
.Exit:	movem.l	(sp)+,d0/d1/d6/a0/a1
	rts

;*****************************************************************************
; [ Handle the magic event set ]
; All registers are restored
;*****************************************************************************
Handle_magic_set:
	movem.l	d0/d1/d6/a0/a1,-(sp)
	move.l	Set_list_Sp,a0		; Using magic ?
	btst	#Magic_set_mode,Set_list_modes(a0)
	beq.s	.Exit
	move.b	Magic_event_set_handle,d0	; Yes -> Search
	moveq.l	#-1,d6
	jsr	Search_event_set	
	cmp.w	#-1,d1			; Found anything ?
	beq	.Exit
	jsr	Push_event_context		; Yes -> Make new context
	sub.l	a1,a1
	jsr	Prepare_event_set_context
	move.b	Magic_event_text_handle,Event_text_handle(a0)
	jsr	Execute_event_chain		; Execute event chain
.Exit:	movem.l	(sp)+,d0/d1/d6/a0/a1
	rts

;*****************************************************************************
; [ Handle the dialogue partner's event sets ]
; All registers are restored
;*****************************************************************************
Handle_dialogue_set:
	ifne	FALSE
	movem.l	d0/d6/d7/a0-a2,-(sp)
	move.l	Set_list_Sp,a0		; In dialogue ?
	btst	#Dialogue_set_mode,Set_list_modes(a0)
	beq.s	.Exit
	move.b	Dialogue_handle,d0		; Yes -> Check

; Check conditions...

;	beq.s	.Exit

	moveq.l	#Me_actor_type,d6		; Do
	lea.l	_context_validator,a1
	lea.l	Dialogue_event_set_handles,a2
	moveq.l	#0,d7			; Try first event set
	move.b	0(a2,d7.w),d0
	jsr	Search_event_set		; Search
	cmp.w	#-1,d1			; Found anything ?
	bne.s	.Found
	addq.w	#1,d7			; No -> Try second set
	move.b	0(a2,d7.w),d0
	beq	.Exit			; If any
	jsr	Search_event_set		; Search
	cmp.w	#-1,d1			; Found anything ?
	beq	.Exit
.Found:	jsr	Push_event_context		; Yes -> Make new context
	jsr	Prepare_event_set_context
	lea.l	Dialogue_event_text_handles,a2
	move.b	0(a2,d7.w),Event_text_handle(a0)
	jsr	Execute_event_chain		; Execute event chain
.Exit:	movem.l	(sp)+,d0/d6/d7/a0-a2
	endc
	rts

;*****************************************************************************
; [ Handle the acting member's event sets ]
; All registers are restored
;*****************************************************************************
Handle_acting_member_set:
	movem.l	d0/d6/a0/a1,-(sp)
	move.l	Set_list_Sp,a0		; In combat ?
	btst	#Combat_set_mode,Set_list_modes(a0)
	bne.s	.Exit
	move.w	Active_member,d0		; No
	jsr	Check_member_capability	; Check
	beq.s	.Exit
	moveq.l	#Me_actor_type,d6		; Do
	lea.l	_context_validator,a1
	jsr	Search_member_sets
.Exit:	movem.l	(sp)+,d0/d6/a0/a1
	rts

;*****************************************************************************
; [ Handle a non-acting member's event sets ]
;   IN : d0 - Parameter (.w)
; All registers are restored
;*****************************************************************************
Handle_non_acting_member_set:
	movem.l	d0/d6/a1,-(sp)
	move.l	Set_list_Sp,a0		; In combat ?
	btst	#Combat_set_mode,Set_list_modes(a0)
	bne.s	.Exit
	cmp.w	Active_member,d0		; No -> Is this the active
	beq.s	.Exit			;        member ?
	jsr	Check_member_capability	; No -> Check
	beq.s	.Exit
	moveq.l	#Friend_set_type,d6		; Do
	lea.l	_context_validator,a1
	jsr	Search_member_sets
.Exit:	movem.l	(sp)+,d0/d6/a1
	rts

;*****************************************************************************
; [ Handle the acting participant's event sets ]
; All registers are restored
;*****************************************************************************
Handle_acting_part_set:
	ifne	FALSE

	move.l	Set_list_Sp,a0		; In combat ?
	btst	#Combat_set_mode,Set_list_modes(a0)
	beq	.Exit
	tst.b	Party_or_monster		; Yes -> Party or monster ?
	bne	.Monster
	move.w	Active_participant,d0	; Party
	jsr	Check_member_capability	; Check
	beq.s	.Exit
	moveq.l	#Me_actor_type,d6		; Do
	lea.l	_context_validator,a1
	jsr	Search_member_sets
	bra.s	.Exit
.Monster:	move.w	Active_participant,d0	; Party
	jsr	Check_monster_capability	; Check
	beq.s	.Exit
	moveq.l	#Me_actor_type,d6		; Do
	lea.l	_context_validator,a1
	jsr	Search_monster_sets

.Exit:
	endc
	rts

;*****************************************************************************
; [ Handle a friendly participant's event sets ]
;   IN : d0 - Parameter (.w)
; All registers are restored
;*****************************************************************************
Handle_friend_part_set:
	ifne	FALSE
	btst	Combat
	beq	.Exit
	tst.b	Party_or_monster		; Party or monster ?
	bne	.Monster

.Exit:
	endc
	rts

;*****************************************************************************
; [ Handle an enemy participant's event sets ]
;   IN : d0 - Parameter (.w)
; All registers are restored
;*****************************************************************************
Handle_enemy_part_set:
	ifne	FALSE
	btst	Combat
	beq	.Exit
	tst.b	Party_or_monster		; Party or monster ?
	bne	.Monster
	rts
	endc

;*****************************************************************************
; [ Search a party member's event sets ]
;   IN : d0 - Member index {1...6} (.w)
;        d6 - Actor type (.b)
;        a1 - Pointer to context validator (.l)
; All registers are restored
;*****************************************************************************
Search_member_sets:
	movem.l	d0/d7/a0/a2,-(sp)
	move.w	d0,d7
	add.w	d7,d7
	lea.l	Event_set_handles,a2	; Try first event set
	move.b	-2(a2,d7.w),d0
	jsr	Search_event_set		; Search
	cmp.w	#-1,d1			; Found anything ?
	bne.s	.Found
	addq.w	#1,d7			; No -> Try second set
	move.b	-2(a2,d7.w),d0
	beq	.Exit			; If any
	jsr	Search_event_set		; Search
	cmp.w	#-1,d1			; Found anything ?
	beq	.Exit
.Found:	jsr	Push_event_context		; Yes -> Make new context
	jsr	Prepare_event_set_context
	lea.l	Event_text_handles,a2
	move.b	-2(a2,d7.w),Event_text_handle(a0)
	jsr	Execute_event_chain		; Execute event chain
.Exit:	movem.l	(sp)+,d0/d7/a0/a2
	rts

;*****************************************************************************
; [ Prepare an event set context ]
;   IN : d0 - Event set memory handle (.b)
;        d1 - Chain number (.w)
;        a0 - Pointer to new event context (.l)
;        a1 - Pointer to context validator (.l)
; All registers are restored
;*****************************************************************************
Prepare_event_set_context:
	movem.l	d0/a2,-(sp)
	move.b	d0,Event_handle(a0)		; Store event data handle
	move.b	d1,Event_chain_nr(a0)	; Store chain number
	move.l	a1,Context_validator(a0)	; Store context validator
	jsr	Claim_pointer		; Get event set data
	move.l	d0,a2
	move.w	(a2)+,d0			; Get number of chains
	add.w	d0,d0			; Calculate event base
	addq.w	#2,d0
	move.w	d0,Event_base(a0)		; Store
	move.w	0(a2,d1.w*2),Event_block_nr(a0)	; Get first block number
	movem.l	(sp)+,d0/a2
	jmp	Free_pointer

;*****************************************************************************
; [ Search an event set for a certain action event ]
;   IN : d0 - Event set memory handle (.b)
;        d6 - Actor type / -1 (doesn't matter) (.b)
;  OUT : d1 - Chain number / -1 (not found) (.w)
; Changed registers : d1
; Notes :
;   - This routine will search the entire event set for the most specific
;      event chain matching the input values.
;   - It will use the action parameters from the current set list.
;*****************************************************************************
Search_event_set:
	movem.l	d0/d2-d7/a0-a3/a6,-(sp)
	move.l	Set_list_Sp,a6
	move.w	Action_value(a6),d3		; Get action parameters
	move.b	Action_extra(a6),d4
	move.b	Action_type(a6),d5
	jsr	Claim_pointer		; Get event set data
	move.l	d0,a0
	move.w	(a0)+,d7			; Get number of chains
	lea.l	0(a0,d7.w*2),a1		; Get start of event blocks
	moveq.l	#-1,d1			; Default is not found
	moveq.l	#0,d2			; Clear

; REGISTER CONTENTS :
;  d1 - Found index (.w)
;  d2 - Current chain index (.w)
;  d3 - Value (.w)
;  d4 - Extra value (.b)
;  d5 - Action type (.b)
;  d6 - Set type (.b)
;  d7 - Counter (.w)
;  a0 - Pointer to entry list (.l)
;  a1 - Pointer to event blocks (.l)
;  a2 - Pointer to start of current chain (.l)
;  a3 - Pointer to found chain (.l)

	bra	.Entry
.Loop:	move.w	(a0)+,d0			; Get start of chain
	mulu.w	#Event_data_size,d0
	lea.l	0(a1,d0.l),a2
	cmp.b	#Action_event,(a2)		; Is Action event ?
	bne.s	.Next
	cmp.w	#-1,d6			; Yes -> Personal ?
	beq.s	.Skip
	btst	d6,Event_b2(a2)		; Yes -> Right set type ?
	beq.s	.Next
.Skip:	cmp.b	Event_b1(a2),d5		; Yes -> Right action ?
	bne.s	.Next
	cmp.b	Event_b3(a2),d4		; Yes -> Right extra value ?
	beq.s	.Yes
	cmp.b	#-1,Event_b3(a2)		; No -> Any ?
	bne.s	.Next
	cmp.w	Event_w6(a2),d3		; Yes -> Right value ?
	beq.s	.Store
	cmp.w	#-1,Event_w6(a2)		; No -> Any ?
	bne.s	.Next
	cmp.w	#-1,d1			; Yes -> Already found one ?
	beq.s	.Store
	cmp.b	#-1,Event_b3(a3)		; Yes -> Is that one more
	bne.s	.Next			;         specific ?
	cmp.w	#-1,Event_w6(a3)
	bne.s	.Next
.Store:	move.w	d2,d1			; Store, but search onward
	move.l	a2,a3
	bra.s	.Next
.Yes:	cmp.w	Event_w6(a2),d3		; Yes -> Right value ?
	beq.s	.Found
	cmp.w	#-1,Event_w6(a2)		; No -> Any ?
	beq.s	.Store
	bra.s	.Next			; No -> Next
.Found:	move.w	d2,d1			; Store and end search
	bra.s	.Done
.Next:	addq.w	#1,d2			; Next chain
.Entry:	dbra	d7,.Loop
.Done:	movem.l	(sp)+,d0/d2-d7/a0-a3/a6
	jmp	Free_pointer

;*****************************************************************************
; [ Execute the current event chain (the one on the event context stack) ]
; All registers are	restored
; Notes :
;   - This routine assumes that the following entries of the current context
;      have been set :
;      Event_handle, Event_base and Event_block_nr.
;   - The Event_data entry will be set by this routine.
;   - The event context will automatically be popped once the chain has been
;      executed. Pop_event_context requires the Context_validator entry for
;      the previous event context.
;*****************************************************************************
Execute_event_chain:
	movem.l	d0-d7/a0-a6,-(sp)
	movea.l	Event_context_Sp,a6		; Get current context
	move.w	Event_block_nr(a6),d0	; Get first block number
	cmpi.w	#-1,d0			; End of the chain ?
	beq	.Exit
	mulu.w	#Event_data_size,d0		; No -> Get data offset
	add.l	Event_base(a6),d0
	Get	Event_handle(a6),a0		; Copy first event block
	add.l	d0,a0
	lea.l	Event_data(a6),a1
	moveq.l	#(Event_data_size/2)-1,d7
.Loop:	move.w	(a0)+,(a1)+
	dbra	d7,.Loop
	Free	Event_handle(a6)
	sf	Event_context_flags(a6)	; Clear flags
.Again:	moveq.l	#0,d0			; Get current event type
	move.b	Event_data(a6),d0
	tst.w	d0			; Legal ?
	beq.s	.Next
	cmpi.w	#Max_event_types+1,d0
	bpl.s	.Next
	ifne	Cheat
	jsr	Print_current_event		; Yes -> Display event
	endc
	movea.l	-4(.Ptrs,d0.w*4),a0		; Execute event
	move.l	a6,-(sp)
	jsr	(a0)
	move.l	(sp)+,a6
.Next:	btst	#Break_event_chain,Event_context_flags(a6)	; Break ?
	bne.s	.Exit
.Entry:	jsr	Chain_to_next_event		; Next event
	bne.s	.Again			; Last ?
.Exit:	jsr	Pop_Event_context		; Yes -> Pop context
	movem.l	(sp)+,d0-d7/a0-a6
	rts

    .Ptrs:	dc.l Map_exit_event
	dc.l Door_event
	dc.l Chest_event
	dc.l Text_event
	dc.l Spinner_event
	dc.l Trap_event
	dc.l Dummy
	dc.l Datachange_event
	dc.l Change_icon_event
	dc.l Encounter_event
	dc.l Place_action_event
	dc.l Query_event
	dc.l Modify_event
	dc.l Forbid_event
	dc.l Signal_event
	dc.l Allow_event
	dc.l Sound_event
	dc.l Start_dialogue_event
	dc.l Create_trans_event
	dc.l Execute_event
	dc.l Remove_member_event
	dc.l End_dialogue_event
	dc.l Wipe_event
	dc.l Play_animation_event
	dc.l Dummy
	dc.l Pause_event
	dc.l Simple_chest_event
	dc.l Ask_surrender_event
	dc.l Do_script_event

;*****************************************************************************
; [ Chain to next event ]
;  OUT : ne - There is a next event
;        eq - There is no next event
; All registers are restored
; Notes :
;  - This routine re-sets [ Event_block_nr ] and [ Event_data ].
;*****************************************************************************
Chain_to_next_event:
	movem.l	d0/d6/d7/a0/a1/a6,-(sp)
	moveq.l	#-1,d6			; Default is no next event
	move.l	Event_context_Sp,a6		; Get current event context
	move.w	Event_data+Next_event_nr(a6),d0	; Get next block number
	move.w	Event_block_nr(a6),d7	; Get current block number
	move.w	d0,Event_block_nr(a6)	; Set new block number
	cmpi.w	#-1,d0			; End of the chain ?
	beq.s	.Exit
	cmp.w	d0,d7			; No -> Endless loop ?
	bne.s	.No
	move.l	#ENDLESS_EVENT,d0		; Yes -> Exit
	jmp	Fatal_error
.No:	mulu.w	#Event_data_size,d0		; No -> Get data offset
	add.l	Event_base(a6),d0
	Get	Event_handle(a6),a0		; Copy event block
	add.l	d0,a0
	lea.l	Event_data(a6),a1
	moveq.l	#(Event_data_size/2)-1,d7
.Loop:	move.w	(a0)+,(a1)+
	dbra	d7,.Loop
	Free	Event_handle(a6)
	moveq.l	#0,d6			; Yay!
.Exit:	tst.w	d6			; Well ?
	movem.l	(sp)+,d0/d6/d7/a0/a1/a6
	rts

;*****************************************************************************
; [ Push an event context on the stack ]
;  OUT : a0 - Pointer to new event context (.l)
; Changed registers : a0
; Notes :
;   - The new context will be cleared.
;*****************************************************************************
Push_Event_context:
	movem.l	d0/d7/a1,-(sp)
	move.l	Event_context_Sp,d0		; Stack empty ?
	bne.s	.No
	lea.l	Event_contextStack_start,a0	; Yes -> Start
	bra.s	.Go_on
.No:	move.l	d0,a0			; No -> Next entry
	lea.l	Event_context_size(a0),a0
	cmpa.l	#Event_contextStack_end,a0	; Stack full ?
	beq.s	.Exit
.Go_on:	move.l	a0,Event_context_Sp		; Store new Sp
	move.l	a0,a1			; Clear context
	moveq.l	#Event_context_size/2-1,d7
.Loop:	clr.w	(a1)+
	dbra	d7,.Loop
.Exit:	movem.l	(sp)+,d0/d7/a1
	rts

;*****************************************************************************
; [ Pop an event context from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_Event_context:
	movem.l	d0/a0/a1,-(sp)
	move.l	Event_context_Sp,d0		; Stack empty ?
	beq.s	.Exit
	movea.l	d0,a0			; No
	cmpa.l	#Event_contextStack_start,a0	; Only one entry left ?
	bne.s	.No
	clr.l	Event_context_Sp		; Yes -> Stack is now empty
	bra.s	.Exit
.No:	lea.l	-Event_context_size(a0),a0	; No -> Pop
	move.l	a0,Event_context_Sp
	move.l	Context_validator(a0),d0	; Must the old context be
	beq.s	.Exit			;  validated ?
	move.l	d0,a1
	jsr	(a1)			; Yes -> Is it still valid ?
	bne.s	.Exit
	bset	#Break_event_chain,Event_context_flags(a0)	; No -> Break
.Exit:	movem.l	(sp)+,d0/a0/a1
	rts

;*****************************************************************************
; [ Reset the event context stack ]
; All registers are	restored
;*****************************************************************************
Reset_Event_context_stack:
	clr.l	Event_context_Sp
	rts

;*****************************************************************************
; [ Push an set list on the stack ]
;  OUT : a0 - Pointer to new set list structure (.l)
; Changed registers : a0
; Notes :
;   - The new set list structure will be cleared.
;*****************************************************************************
Push_Set_list:
	movem.l	d0/d7/a1,-(sp)
	move.l	Set_list_Sp,d0		; Stack empty ?
	bne.s	.No
	lea.l	Set_listStack_start,a0	; Yes -> Start
	bra.s	.Go_on
.No:	move.l	d0,a0			; No -> Next entry
	lea.l	Set_list_size(a0),a0
	cmpa.l	#Set_listStack_end,a0	; Stack full ?
	beq.s	.Exit
.Go_on:	move.l	a0,Set_list_Sp		; Store new Sp
	move.l	a0,a1			; Clear structure
	moveq.l	#Set_list_size/2-1,d7
.Loop:	clr.w	(a1)+
	dbra	d7,.Loop
.Exit:	movem.l	(sp)+,d0/d7/a1
	rts

;*****************************************************************************
; [ Pop an set list from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_Set_list:
	movem.l	d0/a0/a1,-(sp)
	move.l	Set_list_Sp,d0		; Stack empty ?
	beq.s	.Exit
	movea.l	d0,a0			; No
	cmpa.l	#Set_listStack_start,a0	; Only one entry left ?
	bne.s	.No
	clr.l	Set_list_Sp		; Yes -> Stack is now empty
	bra.s	.Exit
.No:	lea.l	-Set_list_size(a0),a0	; No -> Pop
	move.l	a0,Set_list_Sp

.Exit:	movem.l	(sp)+,d0/a0/a1
	rts

;*****************************************************************************
; [ Reset the set list stack ]
; All registers are	restored
;*****************************************************************************
Reset_Set_list_stack:
	clr.l	Set_list_Sp
	rts

;*****************************************************************************
; [ Execute event ]
; No registers are restored
;*****************************************************************************
Execute_event:
	jmp	Execute_set_list		; Execute

;*****************************************************************************
; [ Forbid event ]
; No registers are restored
;*****************************************************************************
Forbid_event:
	move.l	Set_list_sp,a0
	bset	#Forbidden,Set_list_flags(a0)
	rts

;*****************************************************************************
; [ Allow event ]
; No registers are restored
;*****************************************************************************
Allow_event:
	move.l	Set_list_sp,a0
	bclr	#Forbidden,Set_list_flags(a0)
	rts

;*****************************************************************************
; [ Signal event ]
; No registers are restored
;*****************************************************************************
Signal_event:
	move.l	Set_list_Sp,a0		; Get current list modes
	move.b	Set_list_modes(a0),d4
	jsr	Push_Set_list		; Push new set list
	move.b	#Signal_action,Action_type(a0)	; Initialize
	move.l	Event_context_Sp,a1
	moveq.l	#0,d0
	move.b	Event_data+Event_b1(a1),d0
	move.w	d0,Action_value(a0)

xxx	move.b	d3,Actor_index(a0)

	move.b	d4,Set_list_modes(a0)
	move.l	#Dummy,Action_ptr(a0)
	move.l	#Standard_set_list,Action_list_ptr
	jsr	Execute_set_list		; Execute
	jmp	Pop_Set_list		; Pop

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
Standard_set_list:
	dc.b Map_set,0
	dc.b Acting_member_set,0		; Except in combat
	dc.b Non_acting_member_set,1		; Except in combat
	dc.b Non_acting_member_set,2		; Except in combat
	dc.b Non_acting_member_set,3		; Except in combat
	dc.b Non_acting_member_set,4		; Except in combat
	dc.b Non_acting_member_set,5		; Except in combat
	dc.b Non_acting_member_set,6		; Except in combat
	dc.b Acting_part_set,0		; Only in combat
	dc.b Friend_part_set,1		; Only in combat
	dc.b Friend_part_set,2		; Only in combat
	dc.b Friend_part_set,3		; Only in combat
	dc.b Friend_part_set,4		; Only in combat
	dc.b Friend_part_set,5		; Only in combat
	dc.b Friend_part_set,6		; Only in combat
	dc.b Friend_part_set,7		; Only in combat
	dc.b Friend_part_set,8		; Only in combat
	dc.b Friend_part_set,9		; Only in combat
	dc.b Friend_part_set,10		; Only in combat
	dc.b Friend_part_set,11		; Only in combat
	dc.b Friend_part_set,12		; Only in combat
	dc.b Friend_part_set,13		; Only in combat
	dc.b Friend_part_set,14		; Only in combat
	dc.b Friend_part_set,15		; Only in combat
	dc.b Friend_part_set,16		; Only in combat
	dc.b Friend_part_set,17		; Only in combat
	dc.b Friend_part_set,18		; Only in combat
	dc.b Enemy_part_set,1		; Only in combat
	dc.b Enemy_part_set,2		; Only in combat
	dc.b Enemy_part_set,3		; Only in combat
	dc.b Enemy_part_set,4		; Only in combat
	dc.b Enemy_part_set,5		; Only in combat
	dc.b Enemy_part_set,6		; Only in combat
	dc.b Enemy_part_set,7		; Only in combat
	dc.b Enemy_part_set,8		; Only in combat
	dc.b Enemy_part_set,9		; Only in combat
	dc.b Enemy_part_set,10		; Only in combat
	dc.b Enemy_part_set,11		; Only in combat
	dc.b Enemy_part_set,12		; Only in combat
	dc.b Enemy_part_set,13		; Only in combat
	dc.b Enemy_part_set,14		; Only in combat
	dc.b Enemy_part_set,15		; Only in combat
	dc.b Enemy_part_set,16		; Only in combat
	dc.b Enemy_part_set,17		; Only in combat
	dc.b Enemy_part_set,18		; Only in combat
	dc.b Dialogue_set,0		; Only in dialogue
	dc.b Item_set,0			; Only with items
	dc.b Magic_set,0			; Only with magic
	dc.b Default_set,0
	dc.w -1


	SECTION	Fast_BSS,bss
Event_context_Sp:	ds.l 1		; Event context stack
Event_contextStack_start:
	ds.w Max_Event_contexts*(Event_context_size/2)
Event_contextStack_end:

Set_list_Sp:	ds.l 1		; Set list stack
Set_listStack_start:
	ds.w Max_Set_lists*(Set_list_size/2)
Set_listStack_end:












;*****************************************************************************
; [ Check if a party member can act ]
;   IN : d0 - Member index {1...6} (.w)
;  OUT : ne - Yes
;        eq - No
; All registers are restored
; Notes :
;   - This routine will check whether there actually is a party member with
;      this number and if this party member is not unconscious or lamed.
;*****************************************************************************
Check_member_capability:
	movem.l	d7/a0,-(sp)
	moveq.l	#0,d7			; Default is no
	lea.l	Member_nrs,a0		; Does this member exist ?
	tst.w	-2(a0,d0.w*2)
	beq.s	.Exit

; Check conditions...
	moveq.l	#-1,d7			; Yay!

.Exit:	tst.w	d7			; Well ?
	movem.l	(sp)+,d7/a0
	rts

; DEFAULT EVENTS
Combat_chain:
	dc.b Encounter_type
	dc.b 0
	dc.b 0
	dc.b 0
	dc.b 0
	dc.b 0
	dc.w 0			; INSERT Monster group number
	dc.w 0			; Take background from icon data
	dc.w -1
Short_dialogue_chain:
	dc.b Text_type
	dc.b 3			; Spoken by NPC
	dc.b 0
	dc.b 0
	dc.b 0			; INSERT NPC index
	dc.b 0			; INSERT Text number
	dc.w 0
	dc.w 0
	dc.w -1
Long_dialogue_chain:
	dc.b Start_dialogue_type
	dc.b 0			; INSERT Character type
	dc.b 0
	dc.b 0
	dc.b 0
	dc.b 0
	dc.w 0			; INSERT Character number
	dc.w 0
	dc.w -1
