
Update_display:
	ifne	Cheat
	clr.l	Update_timer		; Start the clock
	endc
	ifne	Cheat
	move.l	Update_timer,d0		; Stop the clock
	addq.l	#1,d0
	move.w	d0,Update_time_value
	jsr	Print_update_time		; Print
	endc

VblQ_handler:
	ifne	Cheat
	addq.l	#1,Update_timer		; Count
	endc

	ifne	Cheat
;*****************************************************************************
; [ Print display update time ]
; All registers are restored
;*****************************************************************************
Print_update_time:
	movem.l	d0-d4/d6/d7/a0,-(sp)
	tst.b	Show_update_time		; Show ?
	beq.s	.Exit
	move.w	#300,d0			; Yes -> Erase area
	moveq.l	#0,d1
	move.w	#319,d2
	moveq.l	#7,d3
	moveq.l	#0,d4
	jsr	Draw_box
	lea.l	Number,a0			; Convert number
	move.w	Update_time_value,d0
	moveq.l	#" ",d6
	moveq.l	#2,d7
	jsr	DecR_convert
	lea.l	Number,a0			; Display number
	move.w	#300,d0
	moveq.l	#0,d1
	jsr	Put_text_line
.Exit:	movem.l	(sp)+,d0-d4/d6/d7/a0
	rts
	endc

Number:	ds.b 11

	ifne	Cheat
Show_update_time:	ds.b 1
	even
Update_timer:	ds.l 1
Update_time_value:	ds.w 1
	endc
