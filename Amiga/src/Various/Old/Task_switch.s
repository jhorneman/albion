
	move.l	My_task_ptr,a0		; Prepare task
	move.l	#Leave_task,tc_Switch(a0)
	move.l	#Return_to_task,tc_Launch(a0)

;***************************************************************************	
; [ Prepare to leave task ]
;***************************************************************************
Prepare_to_leave_task:
	movem.l	d0/a0,-(sp)
	move.l	My_task_ptr,a0		; Enable
	move.b	tc_Flags(a0),d0
	bset	#TB_SWITCH,d0
	bset	#TB_LAUNCH,d0
	move.b	d0,tc_Flags(a0)
	movem.l	(sp)+,d0/a0
	rts

;***************************************************************************	
; [ Leave task ]
;***************************************************************************
Leave_task:
	movem.l	d0-d7/a0-a6,-(sp)
	Wait_4_blitter
	jsr	Exit_mouse		; Mouse off
	st	Task_off
	move.l	My_task_ptr,a0		; Disable
	move.b	tc_Flags(a0),d0
	bclr	#TB_LAUNCH,d0
	move.b	d0,tc_Flags(a0)
	movem.l	(sp)+,d0-d7/a0-a6
.Exit:	rts

;***************************************************************************	
; [ OS returns to task ]
;***************************************************************************
Return_to_task:
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	My_task_ptr,a0		; Waiting ?
	move.b	tc_State(a0),d0
	cmp.b	#TS_RUN,d0
	bne.s	.Exit
	move.b	tc_Flags(a0),d0		; No -> Disable
	bclr	#TB_SWITCH,d0
	bclr	#TB_LAUNCH,d0
	move.b	d0,tc_Flags(a0)
	jsr	Init_mouse		; Mouse on
	sf	Task_off
.Exit:	movem.l	(sp)+,d0-d7/a0-a6
	rts
