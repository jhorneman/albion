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
;   IN : a0 - Pointer to button object (.l)
;        a1 - Pointer to button OID (.l)
; All registers are restored
;***************************************************************************
Init_button:
	movem.l	d0/d1,-(sp)
; ---------- Initialize object --------------------
	move.l	OID_Button_data(a1),Button_data(a0)	; Copy
	move.l	OID_Button_function(a1),Button_function(a0)
	move.w	OID_X(a1),d0		; Set rectangle
	move.w	OID_Y(a1),d1
	add.w	X1(a0),d0
	add.w	Y1(a0),d1
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	add.w	OID_width(a1),d0
	add.w	OID_height(a1),d1
	subq.w	#1,d0
	subq.w	#1,d1
	move.w	d0,X2(a0)
	move.w	d1,Y2(a0)
	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************	
; [ Draw method for switch objects ]
;   IN : a0 - Pointer to switch object (.l)
; All registers are restored
;***************************************************************************
Draw_switch:
	move.l	d1,-(sp)
	tst.b	Switch_state(a0)		; Down ?
	beq.s	.Do
	moveq.l	#Feedback_method,d1		; Yes
.Do:	jsr	Execute_child_methods	; Do children
	move.l	(sp)+,d1
	rts

;***************************************************************************	
; [ Highlight method for switch objects ]
;   IN : a0 - Pointer to switch object (.l)
; All registers are restored
;***************************************************************************
Highlight_switch:
	tst.b	Switch_state(a0)		; Down ?
	bne.s	.Exit
	jsr	Execute_child_methods	; No -> Do children
.Exit:	rts

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
;   IN : a0 - Pointer to radio button object (.l)
;        a1 - Pointer to radio button OID (.l)
; All registers are restored
;***************************************************************************
Init_radio_button:
	move.w	OID_Radio_button_nr(a1),Radio_button_nr(a0)	; Copy
	move.w	OID_Radio_handle(a1),Radio_handle(a0)
	rts

;***************************************************************************	
; [ Init method for BT buttons ]
;   IN : a0 - Pointer to BT button object (.l)
;        a1 - Pointer to BT button OID (.l)
; All registers are restored
;***************************************************************************
Init_BT_button:
	movem.l	d0-d2/a0-a2,-(sp)
	move.l	a1,-(sp)
	Make_OID	HBox,a1
	move.l	a0,a2
	clr.w	OID_X(a1)			; Make border
	clr.w	OID_Y(a1)
	move.w	Y2(a2),d1
	move.w	X2(a2),d2
	sub.w	Y1(a2),d1
	sub.w	X1(a2),d2
	addq.w	#1,d1
	addq.w	#1,d2
	move.w	d1,OID_height(a1)
	move.w	d2,OID_width(a1)
	lea.l	HBox_class,a0		; Add
	move.w	Object_self(a2),d0
	jsr	Add_object
	Free_OID
	move.l	(sp)+,a0			; Make text
	Make_OID	Text,a1
	move.w	OID_Button_text_colour(a0),OID_Text_colour(a1)
	move.w	OID_Button_text_feedback_colour(a0),OID_Text_feedback_colour(a1)
	clr.w	OID_X(a1)
	sub.w	#Standard_text_height,d1
	lsr.w	#1,d1
	move.w	d1,OID_Y(a1)
	move.w	d2,OID_Text_object_width(a1)
	move.l	Button_data(a2),OID_Text_ptr(a1)
	lea.l	Text_class,a0		; Add
	move.w	Object_self(a2),d0
	jsr	Add_object
	Free_OID
	movem.l	(sp)+,d0-d2/a0-a2
	rts

;***************************************************************************	
; [ Init method for BS buttons ]
;   IN : a0 - Pointer to BS button object (.l)
; All registers are restored
;***************************************************************************
Init_BS_button:
	movem.l	d0-d3/a0-a3,-(sp)
	move.l	a0,a2
	Make_OID	HBox,a1
	clr.w	OID_X(a1)			; Make border
	clr.w	OID_Y(a1)
	move.w	X2(a2),d2
	move.w	Y2(a2),d3
	sub.w	X1(a2),d2
	sub.w	Y1(a2),d3
	addq.w	#1,d2
	addq.w	#1,d3
	move.w	d2,OID_width(a1)
	move.w	d3,OID_height(a1)
	lea.l	HBox_class,a0		; Add
	move.w	Object_self(a2),d0
	jsr	Add_object
	Free_OID
	move.l	Button_data(a2),a0		; Make symbol
	Make_OID	Symbol,a1
	lea.l	OID_Symbol(a1),a3
	moveq.l	#Symbol_data_size/2-1,d0
.Loop:	move.w	(a0)+,(a3)+
	dbra	d0,.Loop
	move.l	Button_data(a2),a0
	move.w	Symbol_width(a0),d0		; Centre
	lsl.w	#4,d0
	sub.w	d0,d2
	lsr.w	#1,d2
	move.w	d2,OID_X(a1)
	sub.w	Symbol_height(a0),d3
	lsr.w	#1,d3
	move.w	d3,OID_Y(a1)
	lea.l	Symbol_class,a0		; Add
	move.w	Object_self(a2),d0
	jsr	Add_object
	Free_OID
	movem.l	(sp)+,d0-d3/a0-a3
	rts

;***************************************************************************	
; [ Init method for TS buttons ]
;   IN : a0 - Pointer to TS button object (.l)
; All registers are restored
;***************************************************************************
Init_TS_button:
	movem.l	d0-d3/a0-a3,-(sp)
	move.l	a0,a2
	Make_OID	Symbol,a1
	move.l	Button_data(a2),a0		; Make two-symbol
	lea.l	OID_Symbol(a1),a3
	moveq.l	#Symbol_data_size/2-1,d0
.Loop:	move.w	(a0)+,(a3)+
	dbra	d0,.Loop
	move.l	Button_data(a2),a0
	move.w	X2(a2),d2			; Centre
	sub.w	X1(a2),d2
	addq.w	#1,d2
	move.w	Symbol_width(a0),d0
	lsl.w	#4,d0
	sub.w	d0,d2
	lsr.w	#1,d2
	move.w	d2,OID_X(a1)
	move.w	Y2(a2),d3
	sub.w	Y1(a2),d3
	addq.w	#1,d3
	sub.w	Symbol_height(a0),d3
	lsr.w	#1,d3
	move.w	d3,OID_Y(a1)
	lea.l	TSymbol_class,a0		; Add
	move.w	Object_self(a2),d0
	jsr	Add_object
	Free_OID
	movem.l	(sp)+,d0-d3/a0-a3
	rts

;***************************************************************************
; [ Button selected mouse event ]
;   IN : a0 - Pointer to button object (.l)
; No registers are restored
;***************************************************************************
Button_selected:
	jsr	Normal_clicked		; Do normal feedback
	beq.s	.Exit
	move.l	Button_function(a0),a1	; Execute function
	jsr	(a1)
.Exit:	rts

;***************************************************************************
; [ Switch selected mouse event ]
;   IN : a0 - Pointer to switch object (.l)
; No registers are restored
;***************************************************************************
Switch_selected:
	not.b	Switch_state(a0)		; Switch
	move.w	Object_self(a0),d0		; Redraw
	moveq.l	#Draw_method,d1
	jsr	Execute_method
	jsr	Update_screen
	jsr	Wait_4_unclick		; Wait
	jsr	Dehighlight
	move.l	Button_function(a0),a1	; Execute function
	jmp	(a1)

;***************************************************************************
; [ Radio button selected mouse event ]
;   IN : a0 - Pointer to radio button object (.l)
; No registers are restored
;***************************************************************************
Radio_button_selected:
	move.w	Radio_handle(a0),d0		; Change buttons
	moveq.l	#Set_method,d1
	move.w	Radio_button_nr(a0),d2
	jsr	Execute_method
	jsr	Wait_4_unclick		; Wait
	jmp	Dehighlight

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Button_class:
	dc.l 0
	dc.w Button_object_size
	Method Init,Init_button
	Method Draw,Execute_child_methods
	Method Feedback,Execute_child_methods
	Method Highlight,Execute_child_methods
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $0202
	dc.l Button_selected
	dc.w $3300
	dc.l Normal_highlighted
	dc.w -1

BT_Button_class:
	dc.l Button_class
	dc.w Button_object_size
	Method Init,Init_BT_button
	dc.w -1

BS_Button_class:
	dc.l Button_class
	dc.w Button_object_size
	Method Init,Init_BS_button
	dc.w -1

TS_Button_class:
	dc.l Button_class
	dc.w Button_object_size
	Method Init,Init_TS_button
	dc.w -1

Switch_class:
	dc.l 0
	dc.w Switch_object_size
	Method Init,Init_button
	Method Draw,Draw_switch
	Method Highlight,Highlight_switch
	Method Get,Get_switch
	Method Set,Set_switch
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $0202
	dc.l Switch_selected
	dc.w $3300
	dc.l Normal_highlighted
	dc.w -1

BT_Switch_class:
	dc.l Switch_class
	dc.w Switch_object_size
	Method Init,Init_BT_button
	dc.w -1

BS_Switch_class:
	dc.l Switch_class
	dc.w Switch_object_size
	Method Init,Init_BS_button
	dc.w -1

TS_Switch_class:
	dc.l Switch_class
	dc.w Switch_object_size
	Method Init,Init_TS_button
	dc.w -1

Radio_class:
	dc.l 0
	dc.w Radio_object_size
	Method Init,Init_radio
	Method Draw,Execute_child_methods
	Method Get,Get_radio
	Method Set,Set_radio
	dc.w -1

Radio_button_class:
	dc.l Switch_class
	dc.w Radio_button_object_size
	Method Init,Init_radio_button
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $0202
	dc.l Radio_button_selected
	dc.w $3300
	dc.l Normal_highlighted
	dc.w -1

BT_Radio_button_class:
	dc.l Radio_button_class
	dc.w Radio_button_object_size
	Method Init,Init_BT_button
	dc.w -1

BS_Radio_button_class:
	dc.l Radio_button_class
	dc.w Radio_button_object_size
	Method Init,Init_BS_button
	dc.w -1

TS_Radio_button_class:
	dc.l Radio_button_class
	dc.w Radio_button_object_size
	Method Init,Init_TS_button
	dc.w -1
