
Method	macro
	dc.w \1_method
	dc.l \2
	endm

;***************************************************************************	
; Scroll bar object
; OID :
	rsreset
OID_Total_units:	rs.w 1		; Total amount of units
OID_Units_width:	rs.w 1		; Number of units per row
OID_Units_height:	rs.w 1		; Number of rows on screen
OID_Scroll_bar_X:	rs.w 1		; Coordinates of scroll bar
OID_Scroll_bar_Y:	rs.w 1
OID_Scroll_bar_height:	rs.w 1	; Height of scroll bar in pixels
Scroll_bar_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size
Total_units:	rs.w 1		; Total amount of units
Units_width:	rs.w 1		; Number of units per row
Units_height:	rs.w 1		; Number of rows on screen
Slider_height:	rs.w 1		; Height of slider in pixels
Slider_Y:	rs.w 1			; Y-coordinate of slider
Row_height:	rs.w 1		; Height of row in pixels
Current_row:	rs.w 1		; Current row
Scroll_bar_height:	rs.w 1		; Height of scroll bar in pixels
Scroll_bar_max_height:	rs.w 1	; Maximum Y-coordinate of slider
Scroll_bar_result:	rs.w 1		; Current unit selected by scroll bar {0...}
Scroll_bar_object_size:	rs.b 0

;***************************************************************************	
; Item list object
; OID :
	rsreset

Item_list_OID_size:	rs.b 0

; Object :
	rsreset
	rs.b Object_header_size

Item_list_object_size:	rs.b 0



;***************************************************************************	
; [ Initialize method for item list objects ]
;   IN : a0 - Pointer to item list object (.l)
;        a1 - Pointer to item list OID (.l)
; All registers are restored
;***************************************************************************
Init_item_list:

	move.w	Object_self(a0),d0

	Add Scroll bar

	d0 = d0 !

	Add Item slots

	rts

;***************************************************************************	
; [ Initialize method for scroll bar objects ]
;   IN : a0 - Pointer to scroll bar object (.l)
;        a1 - Pointer to scroll bar OID (.l)
; All registers are restored
;***************************************************************************
Init_scroll_bar:
	movem.l	d0/d1,-(sp)
; ---------- Initialize object --------------------
	move.w	OID_Total_units(a1),Total_units(a0)	; Copy
	move.w	OID_Units_width(a1),Units_width(a0)
	move.w	OID_Units_height(a1),Units_height(a0)
	move.w	OID_Scroll_bar_height(a1),Scroll_bar_height(a0)
	move.w	OID_Scroll_bar_X(a1),d0	; Set rectangle
	move.w	OID_Scroll_bar_Y(a1),d1
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	addq.w	#Scroll_bar_width-1,d0
	add.w	Scroll_bar_height(a1),d1
	subq.w	#1,d1
	move.w	d0,X2(a0)
	move.w	d1,Y2(a0)
; ---------- Scroll at all ? ----------------------
	moveq.l	#0,d0			; Total units less or same
	move.w	Total_units(a0),d0		;  as visible units ?
	move.w	Units_width(a0),d1
	mulu.w	Units_height(a0),d1
	cmp.w	d0,d1
	bgt.s	.Yes
	move.w	Scroll_bar_height(a0),d0	; Yes -> Set data
	move.w	d0,Slider_height(a0)
	clr.w	Scroll_bar_max_height(a0)
	move.w	#1,Row_height(a0)
	bra.s	.Exit
; ---------- Prepare scroll bar variables ---------
.Yes:	cmp.w	#1,Units_width(a0)		; More than 1 unit per row ?
	beq.s	.One
	divu.w	Units_width(a0),d0		; Yes
.One:	move.w	d0,d1
	moveq.l	#0,d0			; Calculate row height
	move.w	Scroll_bar_height(a0),d0
	divu.w	d1,d0
	move.w	d0,Row_height(a0)
	moveq.l	#0,d0			; Calculate maximum height
	move.w	Total_units(a0),d0
	divu.w	Units_width(a0),d0
	sub.w	Units_height(a0),d0
	mulu.w	Row_height(a0),d0
	move.w	d0,Scroll_bar_max_height(a0)
	sub.w	Scroll_bar_height(a0),d0	; Calculate slider height
	neg.w	d0
	move.w	d0,Slider_height(a0)
.Exit:	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************	
; [ Draw AND Update method for scroll bar objects ]
;   IN : a0 - Pointer to scroll bar object (.l)
; All registers are restored
;***************************************************************************
Draw_scroll_bar:
	movem.l	d0/d1/d7/a0,-(sp)
	sf	Slider_selected
	jsr	Do_draw_scroll_bar		; Re-draw scroll bar
	move.w	Object_self(a0),d0		; Draw all elements
	move.w	#Draw_method,d1
	move.w 	Scroll_bar_result(a0),d7
	jsr	Execute_child_methods
	movem.l	(sp)+,d0/d1/d7/a0
	rts

;***************************************************************************	
; [ Scroll bar object mouse event ]
;   IN : a0 - Pointer to scroll bar object (.l)
; No registers are restored
;***************************************************************************
Scroll_bar_pressed:
; ---------- Determine action ---------------------
	move.w	Mouse_Y,d0		; Still in bar area ?
	move.w	Scroll_bar_Y(a0),d1
	cmp.w	d1,d0
	bmi	.Exit
	move.w	d1,d2
	add.w	Scroll_bar_height(a0),d1
	cmp.w	d1,d0
	bpl	.Exit
	add.w	Slider_Y(a0),d2		; Above slider ?
	cmp.w	d2,d0
	bpl	.Not_above
	jsr	Scroll_bar_page_up		; Yes -> Page up
	bra	.Exit
.Not_above:
	add.w	Slider_height(a0),d2	; Below slider ?
	cmp.w	d2,d0
	bmi.s	.Drag
	jsr	Scroll_bar_page_down	; Yes -> Page down
	bra	.Exit
; ---------- Drag slider --------------------------
.Drag:	st	Slider_selected		; Selected !
	jsr	Hide_HDOBs
	jsr	Update_screen
	lea.l	Scroll_bar_MA,a1		; Create MA
	move.w	Scroll_bar_X(a0),d0
	move.w	d0,MA_X1(a1)
	addq.w	#Scroll_bar_width-1,d0
	move.w	d0,MA_X2(a1)
	move.w	Scroll_bar_Y(a0),d0
	move.w	d0,MA_Y1(a1)
	add.w	Scroll_bar_max_height(a0),d0
	move.w	d0,MA_Y2(a1)
	move.l	a0,-(sp)			; Set mouse
	move.l	a1,a0
	jsr	Push_MA
	move.l	(sp)+,a0
	jsr	Mouse_off
	move.w	Slider_Y(a0),d0
	add.w	Scroll_bar_Y(a0),d0
	move.w	d0,Mouse_Y
	move.w	Slider_Y(a0),d0		; Get Y-coordinate
.Again:	ext.l	d0			; Update needed ?
	divu.w	Row_height(a0),d0
	cmp.w	Current_row(a0),d0
	beq.s	.No
	move.w	Slider_Y(a0),d0		; Yes -> Update
	jsr	Update_slider
	jsr	Update_screen
	bra.s	.Go_on
.No:	jsr	Do_draw_scroll_bar		; Show bar
	jsr	Switch_screens
.Go_on:	move.b	Button_state,d0		; Still pressed ?
	btst	#Left_pressed,d0
	beq.s	.Done
	move.w	Mouse_Y,d0		; Get position
	sub.w	Scroll_bar_Y(a0),d0
	move.w	d0,Slider_Y(a0)		; Store
	bra.s	.Again
.Done:	Pop	MA			; Restore mouse
	jsr	Mouse_on
	sf	Slider_selected		; Deselect
	jsr	Update_slider
	jsr	Update_screen
	jsr	Show_HDOBs
.Exit:	rts

;***************************************************************************	
; [ Move the slider up one page ]
;   IN : a0 - Pointer to scroll bar object (.l)
; No registers are restored
;***************************************************************************
Scroll_bar_page_up:
	jsr	Light_slider		; Select
	move.w	Units_height(a0),d0		; Half-page up
	lsr.w	#1,d0
	bne.s	.Notzero
	moveq.l	#1,d0
.Notzero:	neg.w	d0
	add.w	Current_row(a0),d0
	bpl.s	.Ok
	moveq.l	#0,d0
.Ok:	mulu.w	Row_height(a0),d0
	jsr	Wait_4_unclick		; Wait
	jsr	Update_slider		; Deselect
	jmp	Update_screen

;***************************************************************************	
; [ Move the slider down one page ]
;   IN : a0 - Pointer to scroll bar object (.l)
; No registers are restored
;***************************************************************************
Scroll_bar_page_down:
	jsr	Light_slider		; Select
	move.w	Units_height(a0),d0		; Half-page down
	lsr.w	#1,d0
	bne.s	.Notzero
	moveq.l	#1,d0
.Notzero:	add.w	Current_row(a0),d0
	mulu.w	Row_height(a0),d0
	cmp.w	Scroll_bar_max_height(a0),d0
	bmi.s	.Ok
	move.w	Scroll_bar_max_height(a0),d0
.Ok:	jsr	Wait_4_unclick		; Wait
	jsr	Update_slider		; Deselect
	jmp	Update_screen

;***************************************************************************	
; [ Light slider ]
;   IN : a0 - Pointer to scroll bar object (.l)
; All registers are restored
;***************************************************************************	
Light_slider:
	st	Slider_selected
	jsr	Do_draw_scroll_bar
	jsr	Update_screen
	sf	Slider_selected
	rts

;***************************************************************************	
; [ Update slider ]
;   IN : d0 - New Y-coordinate (.w)
;        a0 - Pointer to scroll bar object (.l)
; All registers are restored
;***************************************************************************	
Update_slider:
	movem.l	d0/d1/a0,-(sp)
	cmp.w	Scroll_bar_max_height(a0),d0	; Too far down ?
	bmi.s	.Ok
	move.w	Scroll_bar_max_height(a0),d0	; Yes
.Ok:	move.w	d0,Slider_Y(a0)		; Set variables
	ext.l	d0
	divu.w	Row_height(a0),d0
	move.w	d0,Current_row(a0)
	mulu.w	Units_width(a0),d0
	move.w	d0,Scroll_bar_result(a0)
	jsr	Do_draw_scroll_bar		; Re-draw scroll bar
	move.w	Object_self(a0),d0		; Draw all elements
	move.w	#Draw_method,d1
	move.w 	Scroll_bar_result(a0),d7
	jsr	Execute_child_methods
.None:	movem.l	(sp)+,d0/d1/a0
	rts

;***************************************************************************	
; [ Draw scroll bar ]
;   IN : a0 - Pointer to scroll bar object (.l)
; All registers are restored
;***************************************************************************
Do_draw_scroll_bar:
	movem.l	d0-d4/a1,-(sp)
	tst.b	Slider_selected		; Slider selected ?
	bne.s	.Yes
	lea.l	Unselected_slider_colours,a1
	bra.s	.Do
.Yes:	lea.l	Selected_slider_colours,a1
; ---------- Draw scroll bar ----------------------
.Do:	move.w	X1(a0),d0			; Get coordinates
	move.w	Y1(a0),d1
	subq.w	#1,d0			; Surrounding box
	subq.w	#1,d1
	moveq.l	#Scroll_bar_width+2,d2
	move.w	Scroll_bar_height(a0),d3
	addq.w	#2,d3
	jsr	Draw_deep_box
	addq.w	#1,d0			; Top line
	addq.w	#1,d1
	move.w	d0,d2
	addq.w	#Scroll_bar_width-1,d2
	moveq.l	#Scroll_bar_shadow_colour,d4
	jsr	Draw_hline
	move.w	d1,d3			; Left line
	addq.w	#1,d1
	add.w	Scroll_bar_height(a0),d3
	subq.w	#1,d3
	jsr	Draw_vline
; ---------- Draw slider --------------------------
	move.w	X1(a0),d0			; Get coordinates
	move.w	Y1(a0),d1
	add.w	Slider_Y(a0),d1
	move.w	(a1)+,d4			; Top-left pixel
	jsr	Plot_pixel
	addq.w	#1,d0 			; Top line
	move.w	d0,d2
	addq.w	#Scroll_bar_width-3,d2
	move.w	(a1)+,d4
	jsr	Draw_hline
	move.w	d2,d0			; Top-right pixel
	addq.w	#1,d0
	move.w	(a1)+,d4
	jsr	Plot_pixel
	subq.w	#Scroll_bar_width-1,d0	; Left line
	addq.w	#1,d1
	move.w	d1,d3
	add.w	Slider_height(a0),d3
	subq.w	#3,d3
	move.w	(a1)+,d4
	jsr	Draw_vline
	addq.w	#1,d0			; Main box
	move.w	(a1)+,d4
	jsr	Draw_box
	addq.w	#Scroll_bar_width-2,d0	; Right line
	addq.w	#1,d0
	move.w	(a1)+,d4
	jsr	Draw_hline
	subq.w	#Scroll_bar_width-1,d0	; Bottom-left pixel
	add.w	Slider_height(a0),d1
	subq.w	#2,d1
	move.w	(a1)+,d4
	jsr	Plot_pixel
	addq.w	#1,d0 			; Bottom line
	move.w	(a1)+,d4
	jsr	Draw_hline
	addq.w	#Scroll_bar_width-2,d0	; Bottom-right pixel
	move.w	(a1)+,d4
	jsr	Plot_pixel
	move.w	Scroll_bar_Y(a0),d3		; Slider at the bottom ?
	add.w	Scroll_bar_height(a0),d3
	subq.w	#1,d3
	cmp.w	d1,d3
	bmi.s	.Exit
	subq.w	#Scroll_bar_width-1,d0	; No -> Draw shadow
	addq.w	#1,d1
	move.w	d0,d2
	addq.w	#Scroll_bar_width-1,d2
	moveq.l	#Scroll_bar_shadow_colour,d4
	jsr	Draw_hline
.Exit:	movem.l	(sp)+,d0-d4/a1
	rts










Scroll_bar_class:
	dc.l 0,Scroll_bar_object_size,5
	Method Init,Init_scroll_bar
	Method Draw,Draw_scroll_bar
	Method Update,Draw_scroll_bar
	Method Get,Get_scroll_bar
	Method Mev,.Mev

.Mev:	dc.b $ff,$02
	dc.l Scroll_bar_pressed
	dc.w $00ff

Item_list_class:
	dc.l 0,Item_list_object_size,4
	Method Init,Init_item_list
	Method Draw,Draw_item_list
	Method Update,Update_item_list
	Method Mev,.Mev

.Mev:	dc.w $00ff
