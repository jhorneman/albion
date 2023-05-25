; General objects
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 28-2-1994

	XDEF	Earth_class
	XDEF	HBox_class
	XDEF	DBox_class
	XDEF	Symbol_class
	XDEF	TSymbol_class
	XDEF	Text_class
	XDEF	Window_class
	XDEF	Text_area_class

	SECTION	Program,code
;***************************************************************************	
; [ Standard rectangle initialization ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to standard OID (.l)
; All registers are restored
;***************************************************************************
Set_rectangle:
	movem.l	d0/d1,-(sp)
	move.w	OID_X(a1),d0
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
; [ Initialize method for box object ]
;   IN : a0 - Pointer to box object (.l)
;        a1 - Pointer to box OID (.l)
; All registers are restored
;***************************************************************************
Init_box:
	bset	#Object_norect,Object_flags(a0)
	jmp	Set_rectangle

;***************************************************************************	
; [ Draw method for HBox object ]
;   IN : a0 - Pointer to HBox object (.l)
; All registers are restored
;***************************************************************************
Draw_HBox_object:
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
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Feedback method for HBox object ]
;   IN : a0 - Pointer to HBox object (.l)
; All registers are restored
;***************************************************************************
Feedback_HBox_object:
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
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Draw method for DBox object ]
;   IN : a0 - Pointer to DBox object (.l)
; All registers are restored
;***************************************************************************
Draw_DBox_object:
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
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Initialize method for symbol objects ]
;   IN : a0 - Pointer to symbol object (.l)
;        a1 - Pointer to symbol OID (.l)
; All registers are restored
;***************************************************************************
Init_symbol:
	movem.l	d0/a0/a1,-(sp)
; ---------- Initialize object --------------------
	move.w	OID_X(a1),d0		; Set coordinates
	add.w	d0,X1(a0)
	move.w	OID_Y(a1),d0
	add.w	d0,Y1(a0)
	bset	#Object_norect,Object_flags(a0)
	lea.l	Object_symbol(a0),a0	; Copy
	lea.l	OID_Symbol(a1),a1
	moveq.l	#Symbol_data_size/2-1,d0
.Loop:	move.w	(a1)+,(a0)+
	dbra	d0,.Loop
	movem.l	(sp)+,d0/a0/a1
	rts

;***************************************************************************	
; [ Draw method for symbol objects ]
;   IN : a0 - Pointer to symbol object (.l)
; All registers are restored
;***************************************************************************
Draw_symbol_object:
	movem.l	d0/d1/a0,-(sp)
	move.w	X1(a0),d0			; Get data
	move.w	Y1(a0),d1
	lea.l	Object_symbol(a0),a0
	jsr	Draw_symbol		; Draw symbol
	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Feedback method for symbol objects ]
;   IN : a0 - Pointer to symbol object (.l)
; All registers are restored
;***************************************************************************
Feedback_symbol_object:
	movem.l	d0/d1/a0,-(sp)
	move.w	X1(a0),d0			; Get data
	move.w	Y1(a0),d1
	addq.w	#1,d1
	lea.l	Object_symbol(a0),a0
	jsr	Draw_symbol		; Draw symbol
	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Feedback method for Tsymbol objects ]
;   IN : a0 - Pointer to symbol object (.l)
; All registers are restored
;***************************************************************************
Feedback_Tsymbol_object:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0			; Get data
	move.w	Y1(a0),d1
	lea.l	Object_symbol(a0),a0
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
; [ Initialize method for text objects ]
;   IN : a0 - Pointer to text object (.l)
;        a1 - Pointer to text OID (.l)
; All registers are restored
;***************************************************************************
Init_text:
	move.l	d0,-(sp)
; ---------- Initialize object --------------------
	move.w	OID_Text_object_width(a1),Text_object_width(a0)	; Copy
	move.l	OID_Text_ptr(a1),Text_ptr(a0)
	move.w	OID_Text_colour(a1),Text_colour(a0)
	move.w	OID_Text_feedback_colour(a1),Text_feedback_colour(a0)
	move.w	OID_X(a1),d0		; Set coordinates
	add.w	d0,X1(a0)
	move.w	OID_Y(a1),d0
	add.w	d0,Y1(a0)
	bset	#Object_norect,Object_flags(a0)
	move.l	(sp)+,d0
	rts

;***************************************************************************	
; [ Draw method for text objects ]
;   IN : a0 - Pointer to text object (.l)
; All registers are restored
;***************************************************************************
Draw_text_object:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0			; Get data
	move.w	Y1(a0),d1
	move.w	Text_object_width(a0),d2
	move.w	Text_colour(a0),Ink_colour
	move.l	Text_ptr(a0),a0		; Print text
	jsr	Put_centered_text_line
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Feedback method for text objects ]
;   IN : a0 - Pointer to text object (.l)
; All registers are restored
;***************************************************************************
Feedback_text_object:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0			; Get data
	move.w	Y1(a0),d1
	addq.w	#1,d1
	move.w	Text_object_width(a0),d2
	move.w	Text_feedback_colour(a0),Ink_colour
	move.l	Text_ptr(a0),a0		; Print text
	jsr	Put_centered_text_line
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Highlight method for text objects ]
;   IN : a0 - Pointer to text object (.l)
; All registers are restored
;***************************************************************************
Highlight_text_object:
	movem.l	d0-d2/a0,-(sp)
	move.w	X1(a0),d0			; Get data
	move.w	Y1(a0),d1
	move.w	Text_object_width(a0),d2
	move.w	#Highlight_colour,Ink_colour
	move.l	Text_ptr(a0),a0		; Print text
	jsr	Put_centered_text_line
	movem.l	(sp)+,d0-d2/a0
	rts

;***************************************************************************	
; [ Initialize method for window objects ]
;   IN : a0 - Pointer to window object (.l)
;        a1 - Pointer to window OID (.l)
; All registers are restored
;***************************************************************************
Init_window:
	movem.l	d0-d7/a0/a1,-(sp)
	jsr	Set_rectangle
	jsr	Push_MA
	move.l	a0,a1
	move.w	X1(a1),d0			; Calculate width & height
	move.w	Y1(a1),d1
	move.w	X2(a1),d6
	move.w	Y2(a1),d7
	add.w	#16,d6
	and.w	#$fff0,d0
	and.w	#$fff0,d6
	sub.w	d0,d6
	sub.w	d1,d7
	lsr.w	#4,d6
	addq.w	#1,d7
	move.w	#Screen_depth*2,d0		; Calculate buffer size
	mulu.w	d6,d0
	mulu.w	d7,d0
	jsr	Allocate_CHIP		; Allocate
	move.b	d0,Window_bg_handle(a1)
	jsr	Claim_pointer		; Save background
	move.l	d0,a0
	move.w	X1(a1),d0
	and.w	#$fff0,d0
	moveq.l	#Screen_depth,d5
	jsr	Get_block
	Free	Window_bg_handle(a1)
	movem.l	(sp)+,d0-d7/a0/a1
	rts

;***************************************************************************	
; [ Exit method for window objects ]
;   IN : a0 - Pointer to window object (.l)
; All registers are restored
;***************************************************************************
Exit_window:
	movem.l	d0/d1/d5-d7/a0/a1,-(sp)
	move.l	a0,a1
	move.w	X1(a1),d0			; Calculate width & height
	move.w	Y1(a1),d1
	move.w	X2(a1),d6
	move.w	Y2(a1),d7
	add.w	#16,d6
	and.w	#$fff0,d0
	and.w	#$fff0,d6
	sub.w	d0,d6
	sub.w	d1,d7
	lsr.w	#4,d6
	addq.w	#1,d7
	Get	Window_bg_handle(a1),a0	; Restore background
	moveq.l	#Screen_depth,d5
	jsr	Put_unmasked_block
	move.b	Window_bg_handle(a1),d0
	jsr	Free_pointer
	jsr	Free_memory
	jsr	Pop_MA
	movem.l	(sp)+,d0/d1/d5-d7/a0/a1
	rts

;***************************************************************************	
; [ Initialize method for text area objects ]
;   IN : a0 - Pointer to text area object (.l)
;        a1 - Pointer to text area OID (.l)
; All registers are restored
;***************************************************************************
Init_text_area:
	move.l	a2,-(sp)
	jsr	Set_rectangle
	lea.l	Text_area_PA(a0),a2		; Copy to PA
	move.w	X1(a0),X1(a2)
	move.w	Y1(a0),Y1(a2)
	move.w	X2(a0),X2(a2)
	move.w	Y2(a0),Y2(a2)
	move.w	OID_Ink(a1),PA_Ink(a2)	; Set rest of PA
	move.w	OID_Shadow(a1),PA_Shadow(a2)
	move.w	OID_Paper(a1),PA_Paper(a2)
	move.l	(sp)+,a2
	rts

;***************************************************************************	
; [ Print method for text area objects ]
;   IN : a0 - Pointer to text area object (.l)
;        a1 - Pointer to text (.l)
; All registers are restored
;***************************************************************************
Print_text_area:
	movem.l	a0/a1,-(sp)
	lea.l	Text_area_PA(a0),a0
	jsr	Push_PA
	move.l	a1,a0
	jsr	Display_text
	Pop	PA
	movem.l	(sp)+,a0/a1
	rts

;***************************************************************************	
; [ Erase method for text objects ]
;   IN : a0 - Pointer to text area object (.l)
; All registers are restored
;***************************************************************************
Erase_text_area:
	movem.l	a0/a1,-(sp)
	lea.l	Text_area_PA(a0),a0
	jsr	Push_PA
	jsr	Erase_PA
	Pop	PA
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Earth_class:
	dc.l 0
	dc.w Earth_object_size
	Method Init,Set_rectangle
	Method Mev,.Mev
	dc.w -1

.Mev:	dc.w $3300
	dc.l Dehighlight
	dc.w -1

Box_class:
	dc.l 0
	dc.w Box_object_size
	Method Init,Init_box
	dc.w -1
HBox_class:
	dc.l Box_class
	dc.w HBox_object_size
	Method Draw,Draw_HBox_object
	Method Feedback,Feedback_HBox_object
	dc.w -1
DBox_class:
	dc.l Box_class
	dc.w DBox_object_size
	Method Draw,Draw_DBox_object
	dc.w -1
Symbol_class:
	dc.l 0
	dc.w Symbol_object_size
	Method Init,Init_symbol
	Method Draw,Draw_symbol_object
	Method Feedback,Feedback_symbol_object
	dc.w -1
TSymbol_class:
	dc.l 0
	dc.w Symbol_object_size
	Method Init,Init_symbol
	Method Draw,Draw_symbol_object
	Method Feedback,Feedback_Tsymbol_object
	dc.w -1
Text_class:
	dc.l 0
	dc.w Text_object_size
	Method Init,Init_text
	Method Draw,Draw_text_object
	Method Feedback,Feedback_text_object
	Method Highlight,Highlight_text_object
	dc.w -1
Window_class:
	dc.l 0
	dc.w Window_object_size
	Method Init,Init_window
	Method Exit,Exit_window
	Method Draw,Execute_child_methods
	Method Update,Execute_child_methods
	Method Feedback,Execute_child_methods
	Method Highlight,Execute_child_methods
	dc.w -1
Text_area_class:
	dc.l 0
	dc.w Text_area_object_size
	Method Init,Init_text_area
	Method Print,Print_text_area
	Method Erase,Erase_text_area
	dc.w -1
