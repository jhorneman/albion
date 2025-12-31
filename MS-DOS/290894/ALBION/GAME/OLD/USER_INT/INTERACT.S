; Normal interactions
; Written by J.Horneman (In Tune With The Universe)
; Start : 12-4-1994

	XDEF	Normal_touched
	XDEF	Normal_clicked
	XDEF	Normal_rightclicked

	SECTION	Program,code
;***************************************************************************
; [ Normal touch interaction ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Normal_touched:
	movem.l	d0-d2,-(sp)
	move.w	Object_self(a0),d0
	move.w	Current_highlighted_object,d2	; Still the same object ?
	cmp.w	d0,d2
	beq.s	.Exit
	move.w	d0,Current_highlighted_object	; No -> Change
	exg.l	d0,d2
	moveq.l	#Draw_method,d1		; Redraw old object
	jsr	Execute_method
	move.w	d2,d0			; Highlight new object
	moveq.l	#Highlight_method,d1
	jsr	Execute_method
	moveq.l	#Help_method,d1		; Has on-line help text ?
	jsr	Has_method
	beq.s	.No_help
	jsr	Execute_method		; Yes -> Get it
	move.w	Help_line_handle,d0		; Print it
	moveq.l	#Print_method,d1
	jsr	Execute_method
	move.w	Object_self(a0),d0
.No_help:	move.l	a0,-(sp)
	moveq.l	#Pop_up_method,d1		; Has pop-up menu ?
	jsr	Has_method
	beq.s	.No_popup
	lea.l	Pop_up_Mptr,a0		; Yes -> Special pointer
	bra.s	.Go_on
.No_popup:	lea.l	Default_Mptr,a0		; No -> Normal pointer
.Go_on:	jsr	Change_Mptr
	move.l	(sp)+,a0
	jsr	Update_screen
.Exit:	movem.l	(sp)+,d0-d2
	rts

;***************************************************************************
; [ Normal left-clicked interaction ]
;   IN : a0 - Pointer to object (.l)
;  OUT : eq - Not clicked
;        ne - Clicked
; All registers are restored
;***************************************************************************
Normal_clicked:
	movem.l	d0/d1/d7,-(sp)
	move.w	Object_self(a0),d0
	moveq.l	#-1,d7			; Object is down
	bra.s	.Down
.Again:	move.b	Button_state,d1		; Mouse button pressed ?
	btst	#Left_pressed,d1
	beq.s	.Done
	jsr	Is_over_object		; Yes -> Over object ?
	sne	d1
	cmp.b	d1,d7			; Any change ?
	bne.s	.Change
	jsr	Switch_screens		; No
	bra.s	.Again
.Change:	move.b	d1,d7			; Yes -> Up or down ?
	bne.s	.Down
	moveq.l	#Draw_method,d1		; Draw object up
	jsr	Execute_method
	jsr	Update_screen
	bra.s	.Again
.Down:	moveq.l	#Feedback_method,d1		; Draw object down
	jsr	Execute_method
	jsr	Update_screen
	bra.s	.Again
.Done:	tst.b	d7			; Is down ?
	beq.s	.Skip
	moveq.l	#Draw_method,d1		; Restore object
	jsr	Execute_method
	jsr	Update_screen
.Skip:	clr.w	Current_highlighted_object	; Clear
	tst.b	d7			; Well ?
	movem.l	(sp)+,d0/d1/d7
	rts

;***************************************************************************
; [ Normal right-clicked interaction ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Normal_rightclicked:
	movem.l	d0/d1,-(sp)
	jsr	Wait_4_rightunclick		; Wait
	move.w	Object_self(a0),d0		; Over object ?
	jsr	Is_over_object
	sne	d1
	clr.w	Current_highlighted_object	; Clear
	tst.b	d1			; Well ?
	beq.s	.Exit
	moveq.l	#Pop_up_method,d1		; Execute
	jsr	Execute_method
.Exit:	movem.l	(sp)+,d0/d1
	rts
