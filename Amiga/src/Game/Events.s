; Event handling
; Written by J.Horneman (In Tune With The Universe)
; Start : 3-5-1994

; - Success & break flag setting and clearing (!) must be handled IN events
; - Don't forget to clear the success flag at the start of certain events!
; - Init_display call has been removed


; The final execute...
; a0 contains the pointer to the "executee"

	tst.b	Have_executed		; Already executed ?
	bne.s	.Done
	tst.b	Execute_ordered		; No -> Execute ordered ?
	bne.s	.Do
	tst.b	Execute_forbidden		; No -> Execute forbidden ?
	bne.s	.Done
	tst.b	Execute_is_default		; No -> Execute by default ?
	beq.s	.Done
.Do:	jsr	(a0)			; Execute
	st	Have_executed		; Set flag
.Done:
	rts



Find event : DEPENDS ON CONTEXT TYPE, among others

Map:	Set Event handle, Event text handle, Event X & Y,
	   Event entry number and Event map number
	Set Event base



General_set_list:
	dc.l Search_map_set
	dc.l Search_active_member_sets
	rept 5
	dc.l Search_other_member_sets
	endr
	dc.l Search_default_set
	dc.l 0



	jsr	Search_event_set		; Search event set
	cmp.w	#-1,d1			; Found anything ?
	beq.s	.No
	jsr	Push_event_context		; Yes -> Push new context
	move.b	d0,Event_handle(a0)		; Insert context data

	jsr	Execute_event_chain



;*****************************************************************************
; [ Search an event set for a certain action event ]
;   IN : d0 - Event set memory handle (.b)
;        d3 - Value (.w)
;        d4 - Extra value (.b)
;        d5 - Action type (.b)
;        d6 - Actor type (.b)
;  OUT : d1 - Chain number / -1 (not found) (.w)
; Changed registers : d1
; Notes :
;   - This routine will search the entire event set for the most specific
;      event chain matching the input values.
;*****************************************************************************
Search_event_set:
	movem.l	d0/d2/d7/a0-a3,-(sp)
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
;  d6 - Actor type (.b)
;  d7 - Counter (.w)
;  a0 - Pointer to entry list (.l)
;  a1 - Pointer to event blocks (.l)
;  a2 - Pointer to start of current chain (.l)
;  a3 - Pointer to found chain (.l)

	bra	.Entry
.Loop:	move.w	(a0)+,d0			; Get start of chain
	mulu.w	#Event_data_size,d0
	lea.l	0(a1,d0.l),a2
	cmp.b	#Action_type,(a2)		; Is Action event ?
	bne.s	.Next
	btst	d6,Event_b2(a2)		; Yes -> Right actor type ?
	beq.s	.Next
	cmp.b	Event_b1(a2),d5		; Yes -> Right action ?
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
.Done:	movem.l	(sp)+,d0/d2/d7/a0-a3
	jmp	Free_pointer

;*****************************************************************************
; [ Execute the current event chain (the one on the event context stack) ]
; All registers are	restored
; Notes :
;   - This routine assumes that the following entries of the current context
;      have been set :
;      Event_handle, Event_base, Event_block_nr and Context_validator.
;   - The Event_data entry will be set by this routine.
;   - The event context will automatically be popped once the chain has been
;      executed. Pop_event_context requires the Context_validator entry.
;*****************************************************************************
Execute_event_chain:
	movem.l	d0-d7/a0-a6,-(sp)
	movea.l	Event_context_Sp,a6		; Get current context
	move.w	Event_block_nr(a6),d0	; Get first block number
	cmpi.w	#-1,d0			; End of the chain ?
	beq	.Exit
	mulu.w	#Event_data_size,d0		; No -> Get data offset
	add.l	Event_base(a6),d0
	Get	Event_handle(a6),a0		; Copy event block
	add.l	d0,a0
	lea.l	Event_data(a6),a1
	moveq.l	#(Event_data_size/2)-1,d7
.Loop:	move.w	(a0)+,(a1)+
	dbra	d7,.Loop
	Free	Event_handle(a6)
	sf	Event_context_flags(a6)	; Clear flags
.Again:	bclr	#Break_event_chain,Event_context_flags(a6)
	moveq.l	#0,d0			; Get current event type
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
	dc.l Dummy
	dc.l Signal_event
	dc.l Dummy
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
	move.l	#ENDLESS_EVENT,Return_value	; Yes -> Exit
	jmp	Exit_program
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
	move.l	Context_validator(a0),a1	; Is the old context still
	jsr	(a1)			;  valid ?
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


;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data

	SECTION	Fast_BSS,bss
Event_context_Sp:	ds.l 1		; Event context stack
Event_contextStack_start:
	ds.w Max_Event_contexts*(Event_context_size/2)
Event_contextStack_end:















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
