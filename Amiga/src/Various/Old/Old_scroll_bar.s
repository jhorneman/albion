
;*****************************************************************************
; [ Push a scroll bar on the stack ]
;   IN : a0 - Pointer to scroll bar data (.l)
; All registers are	restored
;*****************************************************************************
Push_Scroll_bar:
	move.l	a1,-(sp)
	movea.l	Scroll_bar_Sp,a1
	addq.l	#4,a1
	cmpa.l	#Scroll_bar_Stack_end,a1	; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	move.l	a1,Scroll_bar_Sp
	jsr	Init_scroll_bar		; Initialize new scroll bar
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a scroll bar from the stack ]
; All registers are	restored
; Notes :
;   - Because [ Close_window ] calls [ Pop_Tree ], which resets variables
;     ([ Offset_X ] and [ Offset_Y]) which are used by [ Init_scroll_bar ],
;     it is wise to close a window BEFORE popping a scroll bar.
;*****************************************************************************
Pop_Scroll_bar:
	move.l	a0,-(sp)
	movea.l	Scroll_bar_Sp,a0
	cmpa.l	#Scroll_bar_Stack_start,a0	; Possible ?
	beq.s	.Exit
	subq.l	#4,a0			; Pop
	move.l	a0,Scroll_bar_Sp
	move.l	(a0),a0			; Get previous scroll bar
	cmp.l	#0,a0			; Is stack base ?
	beq.s	.Exit
	jsr	Init_scroll_bar		; No -> Initialize
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the scroll bar stack ]
; All registers are	restored
;*****************************************************************************
Reset_scroll_bar_stack:
	move.l	a0,-(sp)
	lea.l	Scroll_bar_Stack_start,a0	; Reset stack
	clr.l	(a0)
	move.l	a0,Scroll_bar_Sp
	move.l	(sp)+,a0
	rts


;***************************************************************************	
; [ Move the slider up all the way ]
; No registers are restored
;***************************************************************************
Scroll_bar_all_up:
	jsr	Light_slider
	move.l	Scroll_bar_data_ptr,a0
	moveq.l	#0,d0			; All the way up
	jsr	Update_slider
	jmp	Update_screen

;***************************************************************************	
; [ Move the slider down all the way ]
; No registers are restored
;***************************************************************************
Scroll_bar_all_down:
	jsr	Light_slider
	move.l	Scroll_bar_data_ptr,a0
	move.w	Scroll_bar_max_height(a0),d0	; All the way down
	jsr	Update_slider
	jmp	Update_screen
