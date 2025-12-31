; Button handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 19-1-1994

	XDEF	BT_Button_class
	XDEF	BS_Button_class
	XDEF	TS_Button_class
	XDEF	BT_Switch_class
	XDEF	BS_Switch_class
	XDEF	TS_Switch_class
	XDEF	Radio_class
	XDEF	BT_Radio_button_class
	XDEF	BS_Radio_button_class
	XDEF	TS_Radio_button_class

	SECTION	Program,code
;***************************************************************************	
; [ Initialize method for button objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to OID (.l)
; All registers are restored
;***************************************************************************
Init_button:
	move.l	OID_Button_data(a1),Button_data(a0)
	move.l	OID_Button_function(a1),Button_function(a0)
	jmp	Set_rectangle

;***************************************************************************	
; [ Draw method for button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_button:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_normal_box
	subq.w	#1,d0
	subq.w	#1,d1
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Feedback method for button, switch or radio button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Feedback_button:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_deep_box
	subq.w	#1,d0
	subq.w	#1,d1
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Highlight method for button, switch or radio button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Highlight_button:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_high_box
	subq.w	#1,d0
	subq.w	#1,d1
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Left method for button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Button_selected:
	move.l	a1,-(sp)
	jsr	Normal_clicked
	beq.s	.Exit
	move.l	Button_function(a0),a1	; Execute function
	jsr	(a1)
.Exit:	move.l	(sp)+,a1
	rts

;***************************************************************************	
; [ Draw method for BT button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_BT_button:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	sub.w	d0,d2
	addq.w	#1,d2
	move.l	Button_data(a0),a0
	move.w	#Draw_colour,Ink_colour
	jsr	Put_centered_text_line
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Feedback method for BT button, switch or radio button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Feedback_BT_button:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	addq.w	#1,d1
	move.w	X2(a0),d2
	sub.w	d0,d2
	addq.w	#1,d2
	move.l	Button_data(a0),a0
	move.w	#Feedback_colour,Ink_colour
	jsr	Put_centered_text_line
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Highlight method for BT button, switch or radio button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Highlight_BT_button:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	addq.w	#1,d1
	move.w	X2(a0),d2
	sub.w	d0,d2
	addq.w	#1,d2
	move.l	Button_data(a0),a0
	move.w	#Highlight_colour,Ink_colour
	jsr	Put_centered_text_line
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Draw method for BS button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_BS_button:
	movem.l	d0/d1/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.l	Button_data(a0),a0
	jsr	Draw_symbol
	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Feedback method for BS button, switch or radio button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Feedback_BS_button:
	movem.l	d0/d1/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	addq.w	#1,d1
	move.l	Button_data(a0),a0
	jsr	Draw_symbol
	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Feedback method for TS button objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Feedback_TS_button:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.l	Button_data(a0),a0
	moveq.l	#0,d2			; Calculate symbol size
	move.b	Symbol_depth(a0),d2
	add.w	d2,d2
	mulu.w	Symbol_width(a0),d2
	mulu.w	Symbol_height(a0),d2
	add.l	d2,Symbol_offset(a0)	; Select second frame
	jsr	Draw_symbol		; Draw symbol
	sub.l	d2,Symbol_offset(a0)	; Restore
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Draw method for switch objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_switch:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	tst.b	Switch_state(a0)		; Down ?
	bne.s	.Down
	sub.w	d0,d2			; No
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_high_box
	subq.w	#1,d0
	subq.w	#1,d1
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
	bra.s	.Exit
.Down:	addq.w	#1,d0			; Yes
	addq.w	#1,d1
	subq.w	#1,d2
	subq.w	#1,d3
	jsr	Marmor_box
	subq.w	#1,d0
	subq.w	#1,d1
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
	subq.w	#1,d0
	subq.w	#1,d1
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
.Exit:	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Highlight method for switch objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Highlight_switch:
	movem.l	d0-d3,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	tst.b	Switch_state(a0)		; Down ?
	bne.s	.Down
	sub.w	d0,d2			; No
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_high_box
	subq.w	#1,d0
	subq.w	#1,d1
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
	bra.s	.Exit
.Down:	addq.w	#1,d0			; Yes
	addq.w	#1,d1
	subq.w	#1,d2
	subq.w	#1,d3
	jsr	Brighter_box
	subq.w	#1,d0
	subq.w	#1,d1
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
	subq.w	#1,d0
	subq.w	#1,d1
	addq.w	#2,d2
	addq.w	#2,d3
	jsr	Draw_deep_border
.Exit:	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Get method for switch objects ]
;   IN : a0 - Pointer to switch object (.l)
;  OUT : d2 - Current switch state (.w)
; Changed registers : d2
;***************************************************************************
Get_switch:
	moveq.l	#0,d2
	move.b	Switch_state(a0),d2
	rts

;***************************************************************************	
; [ Set method for switch objects ]
;   IN : a0 - Pointer to switch object (.l)
;        d2 - New switch state (.w)
; All registers are restored
;***************************************************************************
Set_switch:
	movem.l	d0-d2,-(sp)
	tst.w	d2			; Change to 0 or -1
	sne	d2
	cmp.b	Switch_state(a0),d2		; Changed ?
	beq.s	.Exit
	move.b	d2,Switch_state(a0)		; Yes -> Store
	move.w	Object_self(a0),d0		; Redraw
	moveq.l	#Draw_method,d1
	jsr	Execute_method
	jsr	Update_screen
.Exit:	movem.l	(sp)+,d0-d2
	rts

;***************************************************************************	
; [ Draw method for BT switch objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_BT_switch:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	sub.w	d0,d2
	addq.w	#1,d2
	move.l	Button_data(a0),a0
	tst.b	Switch_state(a0)		; Down ?
	beq.s	.No
	addq.w	#1,d1			; Yes
.No:	move.w	#Draw_colour,Ink_colour	; No
	jsr	Put_centered_text_line
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Draw method for BS switch objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_BS_switch:
	movem.l	d0/d1/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.l	Button_data(a0),a0
	tst.b	Switch_state(a0)		; Down ?
	beq.s	.No
	addq.w	#1,d1			; Yes
.No:	jsr	Draw_symbol
	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Draw method for TS switch objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_TS_switch:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.l	Button_data(a0),a0
	moveq.l	#0,d2			; Calculate symbol size
	move.b	Symbol_depth(a0),d2
	add.w	d2,d2
	mulu.w	Symbol_width(a0),d2
	mulu.w	Symbol_height(a0),d2
	add.l	d2,Symbol_offset(a0)	; Select second frame
	jsr	Draw_symbol		; Draw symbol
	sub.l	d2,Symbol_offset(a0)	; Restore
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Initialize method for radio objects ]
;   IN : a0 - Pointer to radio object (.l)
;        a1 - Pointer to radio OID (.l)
; All registers are restored
;***************************************************************************
Init_radio:
	bset	#Object_control,Object_flags(a0)
	rts

;***************************************************************************	
; [ Get method for radio objects ]
;   IN : a0 - Pointer to radio object (.l)
;  OUT : d2 - Current button number (.w)
; Changed registers : d2
;***************************************************************************
Get_radio:
	move.w	Current_button(a0),d2
	rts

;***************************************************************************	
; [ Set method for radio objects ]
;   IN : a0 - Pointer to radio object (.l)
;        d2 - New button number (.w)
; All registers are restored
;***************************************************************************
Set_radio:
	movem.l	d0-d2/d5-d7/a0/a1,-(sp)
	move.w	Current_button(a0),d1	; Changed ?
	cmp.w	d1,d2
	beq.s	.Exit
	move.w	d1,d7			; Yes -> Find old button
	jsr	Find_radio_button
	move.w	d0,d5
	move.w	d2,d7			; Find new button
	jsr	Find_radio_button
	move.w	d0,d6
	beq.s	.Exit			; Found ?
	move.w	d2,Current_button(a0)	; Yes -> Store new button
	tst.w	d1			; Any old button ?
	beq.s	.No
	move.w	d5,d0			; Yes -> Switch off
	moveq.l	#Set_method,d1
	moveq.l	#0,d2
	jsr	Execute_method
.No:	move.w	d6,d0			; Switch new button on
	moveq.l	#Set_method,d1
	moveq.l	#-1,d2
	jsr	Execute_method
	jsr	Get_object_data		; Execute function
	move.l	Button_function(a0),a1
	jsr	(a1)
.Exit:	movem.l	(sp)+,d0-d2/d5-d7/a0/a1
	rts

Find_radio_button:
	move.l	a0,-(sp)
	move.w	Object_child(a0),d0		; Start with first child
.Again:	jsr	Get_object_data		; Get object data
	cmp.w	Radio_button_nr(a0),d7	; Is this the one ?
	bne.s	.No
	move.w	Object_self(a0),d0		; Yes
	bra.s	.Exit
.No:	move.w	Object_next(a0),d0		; No -> Next brother
	bne.s	.Again
	moveq.l	#0,d0			; Nothing found
.Exit:	move.l	(sp)+,a0
	rts

;***************************************************************************	
; [ Initialize method for radio button objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to OID (.l)
; All registers are restored
;***************************************************************************
Init_radio_button:
	move.w	OID_Radio_button_nr(a1),Radio_button_nr(a0)	; Copy
	rts

;***************************************************************************
; [ Left method for switch object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Switch_selected:
	movem.l	d0/d1/a1,-(sp)
	jsr	Normal_clicked
	beq.s	.Exit
	not.b	Switch_state(a0)		; Switch
	move.w	Object_self(a0),d0		; Redraw
	moveq.l	#Draw_method,d1
	jsr	Execute_method
	jsr	Update_screen
	clr.w	Current_highlighted_object	; Clear
	move.l	Button_function(a0),a1	; Execute function
	jsr	(a1)
.Exit:	movem.l	(sp)+,d0/d1/a1
	rts

;***************************************************************************
; [ Left method for radio button object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Radio_button_selected:
	movem.l	d0-d2,-(sp)
	jsr	Normal_clicked
	beq.s	.Exit
	move.w	Object_parent(a0),d0	; Change buttons
	moveq.l	#Set_method,d1
	moveq.l	#0,d2
	move.b	Radio_button_nr(a0),d2
	jsr	Execute_method
	clr.w	Current_highlighted_object	; Clear
.Exit:	movem.l	(sp)+,d0-d2
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Button_class:
	dc.l 0
	dc.w Button_object_size
	Method Init,Init_button
	Method Draw,Draw_button
	Method Feedback,Feedback_button
	Method Highlight,Highlight_button
	Method Left,Button_selected
	Method Touched,Normal_touched
	dc.w -1

BT_Button_class:
	dc.l Button_class
	dc.w Button_object_size
	Method Draw,Draw_BT_button
	Method Feedback,Feedback_BT_button
	Method Highlight,Highlight_BT_button
	dc.w -1

BS_Button_class:
	dc.l Button_class
	dc.w Button_object_size
	Method Draw,Draw_BS_button
	Method Feedback,Feedback_BS_button
	dc.w -1

TS_Button_class:
	dc.l Button_class
	dc.w Button_object_size
	Method Draw,Draw_BS_button
	Method Feedback,Feedback_TS_button
	dc.w -1

Switch_class:
	dc.l 0
	dc.w Switch_object_size
	Method Init,Init_button
	Method Draw,Draw_switch
	Method Feedback,Feedback_button
	Method Highlight,Highlight_switch
	Method Get,Get_switch
	Method Set,Set_switch
	Method Left,Switch_selected
	Method Touched,Normal_touched
	dc.w -1

BT_Switch_class:
	dc.l Switch_class
	dc.w Switch_object_size
	Method Draw,Draw_BT_switch
	Method Feedback,Feedback_BT_button
	Method Highlight,Highlight_BT_button
	dc.w -1

BS_Switch_class:
	dc.l Switch_class
	dc.w Switch_object_size
	Method Draw,Draw_BS_switch
	Method Feedback,Feedback_BS_button
	dc.w -1

TS_Switch_class:
	dc.l Switch_class
	dc.w Switch_object_size
	Method Draw,Draw_TS_switch
	Method Feedback,Feedback_TS_button
	dc.w -1

Radio_class:
	dc.l 0
	dc.w Radio_object_size
	Method Init,Init_radio
	Method Draw,Execute_child_methods
	Method Move,Move_highlight
	Method Get,Get_radio
	Method Set,Set_radio
	dc.w -1

Radio_button_class:
	dc.l 0
	dc.w Radio_button_object_size
	Method Init,Init_button
	Method Draw,Draw_switch
	Method Feedback,Feedback_button
	Method Highlight,Highlight_switch
	Method Get,Get_switch
	Method Set,Set_switch
	Method Left,Radio_button_selected
	Method Touched,Normal_touched
	dc.w -1

BT_Radio_button_class:
	dc.l Radio_button_class
	dc.w Radio_button_object_size
	Method Draw,Draw_BT_switch
	Method Feedback,Feedback_BT_button
	Method Highlight,Highlight_BT_button
	dc.w -1

BS_Radio_button_class:
	dc.l BS_Switch_class
	dc.w Radio_button_object_size
	Method Draw,Draw_BS_switch
	Method Feedback,Feedback_BS_button
	dc.w -1

TS_Radio_button_class:
	dc.l TS_Switch_class
	dc.w Radio_button_object_size
	Method Draw,Draw_TS_switch
	Method Feedback,Feedback_TS_button
	dc.w -1
