; Object-Oriented User Interface management
; Written by J.Horneman (In Tune With The Universe)
; Start : 22-2-1994

	XDEF	Add_object
	XDEF	Delete_object
	XDEF	Get_object_data
	XDEF	Execute_method
	XDEF	Execute_child_methods
	XDEF	Push_Root
	XDEF	Pop_Root
	XDEF	Reset_root_stack
	XDEF	Is_over_object
	XDEF	Normal_clicked
	XDEF	Normal_right_clicked
	XDEF	Normal_highlighted
	XDEF	Dehighlight
	XDEF	Delete_self
	XDEF	Wait_4_object

; Notes :
;   - There is no real base class since there seems to be no use for this,
;     i.e. there are no global functions or attributes which all objects
;     should have and which aren't already in the object definition.

	SECTION	Program,code
;*****************************************************************************
; [ Create an instance of an object and bind it into the current tree ]
;   IN : d0 - Handle of parent object / 0 (Layer 1) / -1 (New root) (.w)
;        a0 - Pointer to object class definition (.l)
;        a1 - Pointer to OID (Object Initialization Data) (.l)
;  OUT : d0 - Handle of object / 0 (ERROR) (.w)
; Changed registers : d0
; Notes :
;   - This routine MUST be re-entrant because an object's Initialize method
;     can add new objects.
;   - The Init method will receive a pointer to the object in a0 and a
;     pointer to the OID in a1.
;   - The Init method MUST save ALL registers.
;   - The Init method should initialize the object's area, although the
;     top-left corner will be set to the parent object's top-left corner.
;*****************************************************************************
Add_object:
	movem.l	d1/d2/d4/d7/a0/a2/a3,-(sp)
	move.w	d0,d4			; Protect
	jsr	Add_object_to_pool		; Add object to pool
	tst.w	d7			; Success ?
	bne.s	.Ok
	moveq.l	#0,d0			; No -> Error
	bra	.Exit
.Ok:	move.l	a0,a3
; ---------- Bind object in tree ------------------
	cmp.w	#-1,d4			; Make new root ?
	bne.s	.Not_new
	clr.w	Object_parent(a3)		; Yes
	jsr	Push_Root
	move.l	Root_Sp,a0		; Bind (first object)
	move.w	d7,(a0)
	bra.s	.Done
.Not_new:	move.w	d4,Object_parent(a3)	; Insert parent handle
	move.l	Root_Sp,a0		; Get first object
	move.w	(a0),d0			; Is the tree empty ?
	bne.s	.Not_MT
	move.w	d7,(a0)			; Yes -> Bind (first object)
	bra.s	.Done
.Not_MT:	lea.l	Object_ptrs,a2		; No
	tst.w	d4			; Add at level 1 ?
	bne.s	.Not_L1
.Again1:	move.l	-4(a2,d0.w*4),a0		; Yes -> Find last object
	move.w	Object_next(a0),d0
	bne.s	.Again1
	move.w	d7,Object_next(a0)		; Bind
	bra.s	.Done
.Not_L1:	jsr	Search_object		; Search parent object
	cmp.l	#0,a0			; Found ?
	bne.s	.Yes
	move.l	(sp)+,a0			; No -> Error
	move.w	d7,d0			; Delete object
	jsr	Remove_object_from_pool
	moveq.l	#0,d0
	bra.s	.Exit
.Yes:
xx Check for no_container HERE !
	move.w	X1(a0),X1(a3)		; Set parent coordinates
	move.w	Y1(a0),Y1(a3)
	move.w	Object_child(a0),d2		; Already has a child ?
	bne.s	.Again2
	move.w	d7,Object_child(a0)		; No -> Bind
	bra.s	.Done
.Again2:	move.l	-4(a2,d2.w*4),a0		; Yes -> Find last object
	move.w	Object_next(a0),d2
	bne.s	.Again2
	move.w	d7,Object_next(a0)		; Bind
; ---------- Initialize object --------------------
.Done:	move.l	a3,a0
	move.w	d7,d0			; Initialize object
	moveq.l	#Init_method,d1
	jsr	Execute_method

	ifne	FALSE
	movem.l	d0-d7/a0-a6,-(sp)
	btst	#Object_control,Object_flags(a0)
	bne.s	.Skip
	btst	#Object_norect,Object_flags(a0)
	bne.s	.Skip
	jsr	Random
	move.w	d0,d4
	move.w	X1(a0),d0
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	jsr	Draw_box
	jsr	Update_screen
	jsr	Wait_4_click
.Skip:	movem.l	(sp)+,d0-d7/a0-a6
	endc

.Exit:	movem.l	(sp)+,d1/d2/d4/d7/a0/a2/a3
	rts

;*****************************************************************************
; [ Search an object ]
;   IN : d0 - First object handle (.w)
;        d4 - Target object handle (.w)
;        a2 - Pointer to [ Object_ptrs ] (.l)
;  OUT : a0 - Pointer to target object / 0 (.l)
; Changed registers : a0
;*****************************************************************************
Search_object:
	move.l	d0,-(sp)
.Again:	move.l	-4(a2,d0.w*4),a0		; Get object address
	cmp.w	d0,d4			; Is this the one ?
	beq.s	.Exit
	move.w	Object_child(a0),d0		; Has children ?
	beq.s	.Next
	move.l	a0,-(sp)			; Yes -> Search
	jsr	Search_object
	cmp.w	Object_self(a0),d4		; Found anything ?
	bne.s	.No
	addq.l	#4,sp			; Yes -> Exit
	bra.s	.Exit
.No:	move.l	(sp)+,a0
.Next:	move.w	Object_next(a0),d0		; Next object
	bne.s	.Again
	sub.l	a0,a0			; Found nothing
.Exit:	move.l	(sp)+,d0
	rts

;*****************************************************************************
; [ Remove an object from the current tree and destroy it ]
;   IN : d0 - Handle of object (.w)
; All registers are restored
; Notes :
;   - All child objects will be deleted as well.
;   - Because of this, the routine MUST be re-entrant.
;   - Exit methods will receive a pointer to the object in a0 and MUST
;     save all registers.
;*****************************************************************************
Delete_object:
	movem.l	d0/d1/d7/a0-a2,-(sp)
	tst.w	d0			; Exit if handle is zero
	beq	.Exit
	move.w	d0,d7			; Protect
; ---------- Handle deletion warnings -------------
	jsr	Get_object_data		; Object marked ?
	btst	#Object_warn_when_deleted,Object_flags(a0)
	beq.s	.No
	subq.w	#1,Warn_when_deleted_counter	; Count down
; ---------- Search object and referring object ---
.No:	move.l	Root_Sp,a0		; Get first object
	move.w	(a0),d0
	sub.l	a0,a0			; Default
	tst.w	d0			; Any objects in tree ?
	beq.s	.Go_on
	sub.l	a1,a1			; Yes -> Search
	lea.l	Object_ptrs,a2
	move.l	-4(a2,d7.w*4),a0
	move.w	Object_parent(a0),d0	; Has parent ?
	beq.s	.Found
	cmp.w	#-1,d0
	beq.s	.Found
	move.l	-4(a2,d0.w*4),a1		; Yes -> Search referring object
	move.w	Object_child(a1),d0
.Again1:	cmp.w	d0,d7
	beq.s	.Found
	move.l	-4(a2,d0.w*4),a1
	move.w	Object_next(a1),d0
	bne.s	.Again1
	sub.l	a1,a1			; None
; ---------- Kill child objects -------------------
.Found:	move.w	Object_child(a0),d0		; Has children ?
	beq.s	.Do
	move.l	a0,-(sp)			; Yes
.Again2:	move.l	-4(a2,d0.w*4),a0
	move.w	Object_next(a0),d1
	jsr	Delete_object		;    (Re-enter)
	move.w	d1,d0
	bne.s	.Again2
	move.l	(sp)+,a0
; ---------- Unlink object ------------------------
.Do:	cmp.l	#0,a1			; Any referring object ?
	bne.s	.Yes
	move.l	Root_Sp,a1		; No -> At the root
	move.w	Object_next(a0),(a1)
	bra.s	.Go_on
.Yes:	move.w	Object_next(a0),d0		; Yes
	cmp.w	Object_next(a1),d7		; Is parent or brother ?
	bne.s	.Parent
	move.w	d0,Object_next(a1)		; Brother
	bra.s	.Go_on
.Parent:	move.w	d0,Object_child(a1)		; Parent
; ---------- Exit object --------------------------
.Go_on:	move.w	d7,d0			; Exit object
	moveq.l	#Exit_method,d1
	jsr	Execute_method
	move.w	d7,d0			; Remove from pool
	jsr	Remove_object_from_pool
.Exit:	movem.l	(sp)+,d0/d1/d7/a0-a2
	rts

;*****************************************************************************
; [ Get an object's address ]
;   IN : d0 - Object handle (.w)
;  OUT : a0 - Pointer to object (.l)
; Changed registers : a0
;*****************************************************************************
Get_object_data:
	lea.l	Object_ptrs,a0
	move.l	-4(a0,d0.w*4),a0
	rts

;*****************************************************************************
; [ Execute an object's method ]
;   IN : d0 - Object handle (.w)
;        d1 - Method number (.w)
; All registers are restored
; Notes :
;   - First the methods of the parent classes will be executed.
;   - The method will receive a pointer to the object in a0. All registers
;     are left intact by this routine APART FROM a0 AND a6. 
;*****************************************************************************
Execute_method:
	movem.l	a0/a6,-(sp)
	tst.w	d0			; Exit if handle is zero
	beq.s	.Exit
	jsr	Get_object_data		; Get object data
	move.l	Object_class(a0),a6		; Get class definition
	jsr	.Do			; Do
.Exit:	movem.l	(sp)+,a0/a6
	rts

.Do:	tst.l	Class_parent_class(a6)	; Has a parent class ?
	beq.s	.No
	move.l	a6,-(sp)			; Yes -> Do first
	move.l	Class_parent_class(a6),a6
	jsr	.Do
	move.l	(sp)+,a6
.No:	lea.l	Class_methods(a6),a6	; Search for method
.Again:	cmp.w	#-1,(a6)			; End ?
	beq.s	.Exit2
	cmp.w	(a6),d1			; Is this the one ?
	bne.s	.Next
	move.l	2(a6),a6			; Yes -> Execute
	cmp.l	#0,a6
	beq.s	.Exit2
	jsr	(a6)
	bra.s	.Exit2
.Next:	addq.l	#6,a6			; No -> Next method
	bra.s	.Again
.Exit2:	rts

;*****************************************************************************
; [ Execute an object's childrens' method ]
;   IN : d0 - Object handle (.w)
;        d1 - Method number (.w)
; All registers are restored
; Notes :
;   - [ Execute_method ] is called for each child object.
;   - The method will receive a pointer to the object in a0. All registers
;     are left intact by this routine, APART FROM a0 AND a6, which is
;     destroyed by [ Execute_method ].
;*****************************************************************************
Execute_child_methods:
	movem.l	d0/a0,-(sp)
	jsr	Get_object_data		; Get object data
	move.w	Object_child(a0),d0		; Any children ?
	beq.s	.Exit
.Again:	jsr	Execute_method		; Execute method
	jsr	Get_object_data		; Any brothers ?
	move.w	Object_next(a0),d0
	bne.s	.Again
.Exit:	movem.l	(sp)+,d0/a0
	rts

;*****************************************************************************
; [ Add an object to the object pool ]
;   IN : a0 - Pointer to object class definition (.l)
;  OUT : d7 - Object handle / 0 (ERROR) (.w)
;        a0 - Pointer to new object (.l)
; Changed registers : d7,a0
;*****************************************************************************
Add_object_to_pool:
	movem.l	d0-d2/a1/a2,-(sp)
	moveq.l	#0,d7			; Default is error
; ---------- Find space in pool -------------------
	move.w	Nr_objects,d0		; Maximum object ?
	cmp.w	#Max_objects,d0
	beq	.Exit
	move.w	Last_object_offset,d1	; Will it fit ?
	move.w	d1,d2
	add.w	Class_object_size(a0),d2
	cmp.w	#Object_pool_size,d2
	bpl	.Exit
	move.w	d2,Last_object_offset	; Yes
	addq.w	#1,d0			; Increase counter
	move.w	d0,Nr_objects
	lea.l	Object_pool,a1		; Make pointer
	add.w	d1,a1
	lea.l	Object_ptrs,a2		; Find free entry in list
	moveq.l	#1,d7
	moveq.l	#Max_objects-1,d0
.Loop:	tst.l	(a2)
	beq.s	.Found
	addq.w	#1,d7
	addq.l	#4,a2
	dbra	d0,.Loop
	bra	.Exit
.Found:	move.l	a1,(a2)			; Insert
; ---------- Create instance ----------------------
	exg.l	a0,a1
	moveq.l	#0,d0			; Clear object data
	move.w	Class_object_size(a1),d0
	moveq.l	#0,d1
	jsr	Fill_memory
	move.w	d7,Object_self(a0)		; Store variables
	move.l	a1,Object_class(a0)
	move.w	#-1,Y2(a0)
.Exit:	movem.l	(sp)+,d0-d2/a1/a2
	rts

;*****************************************************************************
; [ Remove an object from the object pool ]
;   IN : d0 - Object handle (.w)
; All registers are restored
;*****************************************************************************
Remove_object_from_pool:
	movem.l	d0/d6/d7/a0/a1,-(sp)
	lea.l	Object_ptrs-4,a0		; Get pointer
	lsl.w	#2,d0
	add.w	d0,a0
	move.l	(a0),a1
	clr.l	(a0)			; Clear
	move.l	Object_class(a1),a0		; Get object size
	moveq.l	#0,d7
	move.w	Class_object_size(a0),d7
	lea.l	0(a1,d7.w),a0		; Copy other objects down
	move.l	#Object_pool_end,d0
	sub.l	a0,d0
	jsr	Copy_memory
	lea.l	Object_ptrs,a0		; Adjust pointers
	moveq.l	#Max_objects-1,d6
.Loop:	move.l	(a0),d0			; Anything there ?
	beq.s	.Next
	cmp.l	a1,d0			; Above deleted object ?
	bmi.s	.Next
	sub.l	d7,d0			; Yes -> Move down
	move.l	d0,(a0)
.Next:	addq.l	#4,a0			; Next
	dbra	d6,.Loop
	sub.w	d7,Last_object_offset	; Count down
	subq.w	#1,Nr_objects
	movem.l	(sp)+,d0/d6/d7/a0/a1
	rts

;*****************************************************************************
; [ Push a new tree root on the object stack ]
; All registers are restored
;*****************************************************************************
Push_Root:
	move.l	a0,-(sp)
	movea.l	Root_Sp,a0
	cmpa.l	#RootStack_end,a0		; Possible ?
	beq.s	.Exit
	addq.l	#Root_data_size,a0		; Yes
	clr.w	(a0)
	move.l	a0,Root_Sp		; Store new Sp
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Pop a tree root from the object stack ]
; All registers are restored
; Notes :
;   - This routine removes all objects from the tree.
;*****************************************************************************
Pop_Root:
	move.l	a0,-(sp)
	movea.l	Root_Sp,a0
	cmpa.l	#RootStack_start,a0		; Possible ?
	beq.s	.Exit
.Again:	move.w	(a0),d0			; Delete first layer
	beq.s	.Empty
	jsr	Delete_object
	bra.s	.Again
.Empty:	subq.l	#Root_data_size,a0		; Pop
	move.l	a0,Root_Sp
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the root stack ]
; All registers are	restored
;*****************************************************************************
Reset_root_stack:
	movem.l	d0/a0,-(sp)
	movea.l	Root_Sp,a0
.Again:	move.w	(a0),d0			; Delete first layer
	beq.s	.Empty
	jsr	Delete_object
	bra.s	.Again
.Empty:	cmpa.l	#RootStack_start,a0		; Reached beginning ?
	beq.s	.Done
	subq.l	#Root_data_size,a0		; No -> Pop
	bra.s	.Again
.Done:	move.l	a0,Root_Sp		; Store new Sp
	movem.l	(sp)+,d0/a0
	rts

;*****************************************************************************
; [ Handle mouse interaction with objects ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Button state (.b
;  OUT : ne - Action
;        eq - No action
; All registers are restored
;*****************************************************************************
Handle_object_Mevs:
	movem.l	d3/a6,-(sp)
	move.l	Root_Sp,a6		; Is the root empty ?
	move.w	(a6),d3
	beq.s	.Exit1
	lea.l	Object_ptrs,a6		; No -> Do
	jsr	.Do
.Exit1:	movem.l	(sp)+,d3/a6
	rts

;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Click state (.w)
;        d3 - First object handle (.w)
;        a6 - Pointer to [ Object_ptrs ] (.l)
;  OUT : ne - Action
;        eq - No action
; All registers are restored
.Do:
	movem.l	d3/d7/a0-a2,-(sp)
	moveq.l	#-1,d7			; Default is action
.Again:	move.l	-4(a6,d3.w*4),a0		; Get object address
	btst	#Object_norect,Object_flags(a0)
	bne.s	.Next
	btst	#Object_control,Object_flags(a0)
	bne.s	.In
	cmp.w	X1(a0),d0			; No -> In rectangle ?
	blt.s	.Out
	cmp.w	X2(a0),d0
	bgt.s	.Out
	cmp.w	Y1(a0),d1
	blt.s	.Out
	cmp.w	Y2(a0),d1
	bgt.s	.Out
.In:	move.w	Object_child(a0),d3		; In -> Has children ?
	beq.s	.No
	jsr	.Do			; Yes -> Search
	bne.s	.Exit2
.No:	move.l	a0,a1			; No -> Search Mev
	move.l	Object_class(a0),a2
	jsr	Search_Mev
	bne.s	.Exit2
	btst	#Object_control,Object_flags(a0)
	bne.s	.Next
	moveq.l	#0,d7
	bra.s	.Exit2
.Out:	btst	#Object_no_container,Object_flags(a0)	; Out -> Container ?
	beq.s	.Next
	move.w	Object_child(a0),d3		; No -> Has children ?
	beq.s	.Next
	jsr	.Do			; Yes -> Search
	bne.s	.Exit2
.Next:	move.w	Object_next(a0),d3		; Next object
	bne.s	.Again
	moveq.l	#0,d7			; No action
.Exit2:	tst.w	d7			; Any luck ?
	movem.l	(sp)+,d3/d7/a0-a2
	rts

; [ Search & handle Mev ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Button state (.b)
;        a1 - Pointer to object (.l)
;        a2 - Pointer to object class definition (.l)
;  OUT : ne - Something was done
;        eq - Nothing was done
; All registers are restored
Search_Mev:
	movem.l	d5/d6/a0/a2,-(sp)
	moveq.l	#0,d6			; Default is no luck
	lea.l	Class_methods(a2),a0	; Search for Mev method
.Again1:	cmp.w	#-1,(a0)			; End ?
	beq.s	.Done
	cmp.w	#Mev_method,(a0)		; Found ?
	bne.s	.Next1
	move.l	2(a0),a0			; Yes -> Handle list
.Again2:	cmp.w	#-1,(a0)			; End of list ?
	beq.s	.Done
	move.b	(a0)+,d5			; Mask
	and.b	d2,d5
	cmp.b	(a0)+,d5			; Compare
	bne.s	.Next2
	movem.l	d0-d7/a0-a6,-(sp)		; Execute
	movea.l	(a0),a2
	move.l	a1,a0
	jsr	(a2)
	movem.l	(sp)+,d0-d7/a0-a6
	moveq.l	#-1,d6			; Yay!
	bra.s	.Exit
.Next2:	addq.l	#4,a0			; Next event
	bra.s	.Again2
.Next1:	addq.l	#6,a0			; No -> Next method
	bra.s	.Again1
.Done:	tst.l	Class_parent_class(a2)	; Has a parent class ?
	beq.s	.Exit
	move.l	Class_parent_class(a2),a2	; Yes -> Do
	jsr	Search_Mev
	sne	d6
.Exit:	tst.w	d6			; Any luck ?
	movem.l	(sp)+,d5/d6/a0/a2
	rts

;*****************************************************************************
; [ Check if the mouse is over an object ]
;   IN : d0 - Object handle (.w)
;  OUT : eq - Not
;        ne - Is
; All registers are restored
;*****************************************************************************
Is_over_object:
	movem.l	d0/d1/d7/a0,-(sp)
	moveq.l	#0,d7			; Default is Not
	tst.w	d0			; Exit if handle is zero
	beq.s	.Exit
	jsr	Get_object_data		; Get object data
	move.w	Mouse_X,d0		; Get mouse coordinates
	move.w	Mouse_Y,d1
xx	cmp.w	#-1,Y2(a0)		; Interactionless ?
xx	beq.s	.Exit
	cmp.w	X1(a0),d0			; In rectangle ?
	bmi.s	.Exit
	cmp.w	X2(a0),d0
	bgt.s	.Exit
	cmp.w	Y1(a0),d1
	bmi.s	.Exit
	cmp.w	Y2(a0),d1
	bgt.s	.Exit
	moveq.l	#-1,d7			; Yes!
.Exit:	tst.w	d7			; Any luck ?
	movem.l	(sp)+,d0/d1/d7/a0
	rts

;***************************************************************************
; [ Normal clicked mouse event ]
;   IN : a0 - Pointer to object (.l)
;  OUT : eq - Not selected
;        ne - Selected
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
.Done:	moveq.l	#Draw_method,d1		; Restore object
	jsr	Execute_method
	jsr	Update_screen
	jsr	Dehighlight		; Clear
	tst.b	d7			; Well ?
	movem.l	(sp)+,d0/d1/d7
	rts

;***************************************************************************
; [ Normal right clicked mouse event ]
;   IN : a0 - Pointer to object (.l)
;  OUT : eq - Not selected
;        ne - Selected
; All registers are restored
;***************************************************************************
Normal_right_clicked:
	movem.l	d0/d1/d7,-(sp)
	move.w	Object_self(a0),d0
	moveq.l	#-1,d7			; Object is down
	bra.s	.Down
.Again:	move.b	Button_state,d1		; Mouse button pressed ?
	btst	#Right_pressed,d1
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
.Done:	moveq.l	#Draw_method,d1		; Restore object
	jsr	Execute_method
	jsr	Update_screen
	jsr	Dehighlight		; Clear
	tst.b	d7			; Well ?
	movem.l	(sp)+,d0/d1/d7
	rts

;***************************************************************************
; [ Normal highlighted mouse event ]
;   IN : a0 - Pointer to object (.l)
;  OUT : eq - Not selected
;        ne - Selected
; All registers are restored
;***************************************************************************
Normal_highlighted:
	movem.l	d0-d2,-(sp)
	move.w	Object_self(a0),d0
	move.w	Current_highlighted_object,d2	; Still the same ?
	cmp.w	d0,d2
	beq.s	.Exit
	move.w	d0,Current_highlighted_object	; Yes
	exg.l	d0,d2
	moveq.l	#Draw_method,d1		; Redraw previous object
	jsr	Execute_method
	move.w	d2,d0
	moveq.l	#Highlight_method,d1	; Highlight new object
	jsr	Execute_method
	jsr	Update_screen
.Exit:	movem.l	(sp)+,d0-d2
	rts

;***************************************************************************
; [ Remove current highlighting ]
; All registers are restored
;***************************************************************************
Dehighlight:
	movem.l	d0/d1,-(sp)
	move.w	Current_highlighted_object,d0	; Anything highlighted ?
	beq.s	.Exit
	clr.w	Current_highlighted_object	; Yes -> Clear
	moveq.l	#Draw_method,d1		; Redraw object
	jsr	Execute_method
	jsr	Update_screen
.Exit:	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************
; [ Delete an object ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Delete_self:
	move.l	d0,-(sp)
	move.w	Object_self(a0),d0
	jsr	Delete_object
	move.l	(sp)+,d0
	rts

;*****************************************************************************
; [ Wait until an object is deleted ]
;   IN : d0 - Object handle (.w)
; All registers are restored
;*****************************************************************************
Wait_4_object:
	movem.l	d0-d2/d7/a0,-(sp)
	jsr	Get_object_data		; Mark object
	bset	#Object_warn_when_deleted,Object_flags(a0)
	move.w	Warn_when_deleted_counter,d7	; Increase counter
	addq.w	#1,d7
	move.w	d7,Warn_when_deleted_counter
	move.l	d0,-(sp)
.Loop:	move.w	Mouse_X,d0		; Main loop
	move.w	Mouse_Y,d1
	move.b	Button_state,d2
	jsr	Handle_object_Mevs
	jsr	Switch_screens
	cmp.w	Warn_when_deleted_counter,d7	; Deleted ?
	beq.s	.Loop
	move.l	(sp)+,d0
	jsr	Get_object_data		; Unmark object
	bclr	#Object_warn_when_deleted,Object_flags(a0)
	movem.l	(sp)+,d0-d2/d7/a0
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Root_Sp:	dc.l RootStack_start

	SECTION	Fast_BSS,bss
RootStack_start:
	ds.b Max_roots*Root_data_size
RootStack_end:

Warn_when_deleted_counter:	ds.w 1
Current_highlighted_object:	ds.w 1
Nr_objects:	ds.w 1
Last_object_offset:	ds.w 1
	ds.l 1				; To cause error
Object_ptrs:	ds.l Max_objects
Object_pool:	ds.w Object_pool_size
Object_pool_end:
