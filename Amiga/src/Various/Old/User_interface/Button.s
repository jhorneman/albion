; Button handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 19-1-1994

	MODULE	DDT_Buttons

	OUTPUT	DDT:Objects/User_interface/Buttons.o


	XDEF	Do_button
	XDEF	Button_up
	XDEF	Button_down

	XREF	Push_Module
	XREF	Print_centered_string
	XREF	Draw_high_box
	XREF	Draw_deep_box
	XREF	Put_masked_block
	XREF	Update_screen
	XREF	Find_object
	XREF	Button_Mod
	XREF	Mouse_X
	XREF	Mouse_Y


	incdir	DDT:
	include	Constants/Global.i
	include	Constants/Core.i
	include	Constants/Hull.i
	include	Constants/User_interface.i


	SECTION	Program,code
;************************************************************************************
; [ Handle button ]
;   IN : a0 - Pointer to button data (.l)
;  OUT : eq - Button was not pressed
;        ne - Button was pressed
; All registers are restored
; Notes :
;   - This routine should be called by the routine which is called when a button
; is clicked.
;************************************************************************************
Do_button:
	move.l	a0,-(sp)
	move.l	a0,Current_button		; Store button data
	sf	Button_is_down		; Button is up
	Push	Module,Button_Mod		; Handle it
	tst.b	Button_is_down		; Button down ?
	beq.s	.Exit
	jsr	Button_up			; Yes -> Display button up
	jsr	Update_screen
.Exit:	move.l	(sp)+,a0
	tst.b	Button_is_down		; Get result
	rts

Button_DisUpd:
	move.l	Current_button,a0		; Get current button data
	move.w	Mouse_X,d0		; Get object under mouse-pointer
	move.w	Mouse_Y,d1
	jsr	Find_object
	cmp.l	Button_ID(a0),d2		; Over button ?
	seq	d0
	cmp.b	Button_is_down,d0		; State changed ?
	beq	.Exit
	move.b	d0,Button_is_down		; Yes
	tst.b	d0			; Up or down ?
	bne.s	.Down
	jsr	Button_up			; Up
	bra.s	.Go_on
.Down:	jsr	Button_down		; Down
.Go_on:	jsr	Update_screen
.Exit:	rts

;************************************************************************************
; [ Display the current button up ]
; No registers are restored
;************************************************************************************
Button_up:
	move.l	Current_button,a1		; Get current button data
	move.w	Button_X(a1),d0		; Get button coordinates
	move.w	Button_Y(a1),d1
	cmp.b	#TS_button,Button_type(a1)	; Two symbols ?
	beq	.TS
	move.w	Border_width(a1),d2		; No -> Draw border
	move.w	Border_height(a1),d3
	jsr	Draw_high_box
	cmp.b	#BT_button,Button_type(a1)	; Border + text or Border + Symbol ?
	beq	.BT
	move.l	Button_ptr(a1),a2		; Border + symbol -> Get symbol data
	move.w	Symbol_base_colour(a2),d4
	move.w	Symbol_depth(a2),d5
	move.w	Symbol_width(a2),d6
	move.w	Symbol_height(a2),d7
	move.l	Symbol_ptr(a2),a0
	sub.w	d6,d2			; Centre symbol
	sub.w	d7,d3
	lsr.w	#1,d2
	lsr.w	#1,d3
	add.w	d2,d0
	add.w	d3,d1
	add.w	#15,d6			; Width in truncs
	lsr.w	#4,d6
	jsr	Put_masked_block		; Display symbol
	bra	.Exit
.BT:	move.l	Button_ptr(a1),a0		; Border + text -> Get text
	sub.w	#Standard_text_height,d3	; Centre text
	lsr.w	#1,d3
	add.w	d3,d1
	jsr	Print_centered_string	; Print
	bra	.Exit
.TS:	move.l	Button_ptr(a1),a2		; Two symbols -> Get symbol data
	move.w	Symbol_base_colour(a2),d4
	move.w	Symbol_depth(a2),d5
	move.w	Symbol_width(a2),d6
	move.w	Symbol_height(a2),d7
	move.l	Symbol_ptr(a2),a0
	add.w	#15,d6			; Width in truncs
	lsr.w	#4,d6
	jsr	Put_masked_block		; Display symbol
.Exit:	rts

;************************************************************************************
; [ Display the current button down ]
; No registers are restored
;************************************************************************************
Button_down:
	move.l	Current_button,a1		; Get current button data
	move.w	Button_X(a1),d0		; Get button coordinates
	move.w	Button_Y(a1),d1
	cmp.b	#TS_button,Button_type(a1)	; Two symbols ?
	beq	.TS
	move.w	Border_width(a1),d2		; No -> Draw border
	move.w	Border_height(a1),d3
	jsr	Draw_deep_box
	cmp.b	#BT_button,Button_type(a1)	; Border + text or Border + Symbol ?
	beq	.BT
	move.l	Button_ptr(a1),a2		; Border + symbol -> Get symbol data
	move.w	Symbol_base_colour(a2),d4
	move.w	Symbol_depth(a2),d5
	move.w	Symbol_width(a2),d6
	move.w	Symbol_height(a2),d7
	move.l	Symbol_ptr(a2),a0
	sub.w	d6,d2			; Centre symbol
	sub.w	d7,d3
	lsr.w	#1,d2
	lsr.w	#1,d3
	add.w	d2,d0
	add.w	d3,d1
	add.w	#15,d6			; Width in truncs
	lsr.w	#4,d6
	jsr	Put_masked_block		; Display symbol
	bra	.Exit
.BT:	move.l	Button_ptr(a1),a0		; Border + text -> Get text
	sub.w	#Standard_text_height,d3	; Centre text
	lsr.w	#1,d3
	add.w	d3,d1
	jsr	Print_centered_string	; Print
	bra	.Exit
.TS:	move.l	Button_ptr(a1),a2		; Two symbols -> Get symbol data
	move.w	Symbol_base_colour(a2),d4
	move.w	Symbol_depth(a2),d5
	move.w	Symbol_width(a2),d6
	move.w	Symbol_height(a2),d7
	move.l	Symbol_ptr(a2),a0
	add.w	#15,d6			; Width in truncs
	lsr.w	#4,d6
	move.w	d5,d2			; Skip first symbol
	add.w	d2,d2
	mulu.w	d6,d2
	mulu.w	d7,d2
	add.w	d2,a0
	jsr	Put_masked_block		; Display symbol
.Exit:	rts

;************************************************************************************
; [ The DATA & BSS segments ]
;************************************************************************************
	SECTION	Fast_BSS,bss
Current_button:	ds.l 1			; Address of current button data
Button_is_down:	ds.b 1			; Current button state
	even
