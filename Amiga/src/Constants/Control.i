; Program control constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 23-2-1994

Method	macro
	dc.w \1_method
	dc.l \2
	endm

Make_OID	macro
	lea.l	-\1_OID_size(sp),sp
	move.l	sp,\2
	move.l	#\1_OID_size,-(sp)
	endm

Free_OID	macro
	add.l	(sp)+,sp
	endm

Max_modules	EQU 8

Max_roots	EQU 8
Max_objects	EQU 100
Object_pool_size	EQU Max_objects*50

;*****************************************************************************
; This is the Module structure
	rsreset
	rs.b 1			; Local/global flag
Module_ID:	rs.b 1			; Module ID
DisUpd_ptr:	rs.l 1		; Pointer	to display update routine
VblQ_ptr:	rs.l 1			; Pointer	to Vbl queue
ScrQ_ptr:	rs.l 1			; Pointer	to Screen	queue
ModInit_ptr:	rs.l 1		; Pointer	to Module	Init routine
ModExit_ptr:	rs.l 1		; Pointer	to Module	Exit routine
DisInit_ptr:	rs.l 1		; Pointer	to Display Init routine
DisExit_ptr:	rs.l 1		; Pointer	to Display Exit routine
Palette_ptr:	rs.l 1		; Pointer to palette list
Module_data_size:	rs.b 1

;*****************************************************************************
; These are the module types
; Note : these are actually states of bit 0 !!!
	rsreset
Global_mod:	rs.b 1
Local_mod:	rs.b 1

;***************************************************************************	
; This is the Root structure
	rsreset
	rs.w 1			; Number of first object / 0
Root_data_size:	rs.b 0

;***************************************************************************	
; This is the Object header structure
	rsreset
	rs.b Recta_size		; MUST be at the start
Object_self:	rs.w 1
Object_next:	rs.w 1
Object_parent:	rs.w 1
Object_child:	rs.w 1
Object_flags:	rs.b 1
	rseven
Object_class:	rs.l 1
Object_header_size:	rs.b 0

;***************************************************************************	
; These are the Object flags
	rsreset
Object_warn_when_deleted:	rs.b 1	; Used by [ Wait_4_object ].
Object_control:	rs.b 1		; Object is used for grouping other
				;  objects and has no rectangle of
				;  it's own.
Object_disable:	rs.b 1		; Interaction with this object is
				;  disabled.

;***************************************************************************	
; This is the Class definition structure
	rsreset
Class_parent_class:	rs.l 1
Class_object_size:	rs.w 1
Class_methods:	rs.b 0

;***************************************************************************	
; These are the methods
	rsreset
	rs.b 1
Init_method:	rs.b 1
Exit_method:	rs.b 1
Close_method:	rs.b 1
Touched_method:	rs.b 1
Left_method:	rs.b 1
Right_method:	rs.b 1
DLeft_method:	rs.b 1
DRight_method:	rs.b 1
Custommouse_method:	rs.b 1
Customkey_method:	rs.b 1
Highlight_method:	rs.b 1
Help_method:	rs.b 1
Feedback_method:	rs.b 1
Pop_up_method:	rs.b 1
Focus_method:	rs.b 1
Move_method:	rs.b 1
Cycle_method:	rs.b 1
Draw_method:	rs.b 1
Update_method:	rs.b 1
Erase_method:	rs.b 1
Get_method:	rs.b 1
Set_method:	rs.b 1
Print_method:	rs.b 1
