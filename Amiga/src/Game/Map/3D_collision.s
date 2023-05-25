
	SECTION	Program,code
;*****************************************************************************
; [ Initialize 3D collision map ]
; All registers are restored
;*****************************************************************************
Initialize_collision_map:
	movem.l	d0/d1,-(sp)
	move.w	Width_of_map,d0		; Allocate collision map
	move.w	d0,d1
	add.w	d0,d0
	add.w	d1,d0
	mulu.w	Height_of_map,d0
	jsr	Allocate_memory
	move.b	d0,Collision_map_handle
	jsr	Build_collision_map		; Build
	movem.l	(sp)+,d0/d1
	rts

;*****************************************************************************
; [ Build 3D collision map ]
; All registers are restored
; Notes :
;   - The easiest way to correctly update the collision map after one or
:      more icon changes is to re-build it.
;*****************************************************************************
Build_collision_map:
	movem.l	d0-d2/d5-d7/a0-a6,-(sp)
	move.b	Collision_map_handle,d0	; Clear
	jsr	Clear_memory
	jsr	Claim_pointer		; Build
	move.l	d0,a0
	move.w	Width_of_map,d0
	add.w	d0,a0
	add.w	d0,d0
	add.w	d0,a0
	Get	Mapdata_handle,a1
	lea.l	Map_data+1(a1),a1
	add.w	Width_of_map,a1
	Get	Labdata_handle,a2
	lea.l	Wall_data_offsets,a3
	move.w	Height_of_map,d7
	subq.w	#3,d7
.Loop_Y:	moveq.l	#5,d0
	move.l	a0,a5
	move.w	Width_of_map,d6
	subq.w	#3,d6
.Loop_X:	moveq.l	#0,d2			; Get map byte
	move.b	(a1)+,d2
	beq.s	.Next_X			; Anything ?
	cmp.b	#-1,d2			; Dummy wall ?
	beq.s	.Blocked
	cmp.w	#First_wall,d2		; Wall or object ?
	blt.s	.Next_X
	sub.w	#First_wall,d2		; Get wall status bits
	move.l	a2,a4
	add.l	0(a3,d2.w*4),a4
	move.l	Wall_bits(a4),d2
	btst.l	#Blocked_direction,d2	; Way blocked ?
	bne.s	.Blocked
	btst.l	#Blocked_travelmodes+On_foot,d2
	bne.s	.Next_X
.Blocked:


	move.w	d0,d1			; Blocked -> Set map
	move.l	a5,a6
	moveq.l	#5-1,d5
.Loop3:	bset	d1,-Bytes_per_collision_line(a6)
	bset	d1,(a6)
	bset	d1,Bytes_per_collision_line(a6)
	bset	d1,2*Bytes_per_collision_line(a6)
	bset	d1,3*Bytes_per_collision_line(a6)
	subq.w	#1,d1
	bpl.s	.Skip1
	moveq.l	#7,d1
	addq.l	#1,a6
.Skip1:	dbra	d5,.Loop3
.Next_X:	subq.w	#5,d0			; Next column
	bpl.s	.Skip2
	addq.w	#7,d0
	addq.l	#1,a5
.Skip2:	dbra	d6,.Loop_X
	lea.l	3*Bytes_per_collision_line(a0),a0	; Next line
	addq.l	#2,a1
	dbra	d7,.Loop_Y
	movem.l	(sp)+,d0-d2/d5-d7/a0-a6
	rts




; [ Test collision map ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Edge area number [0...8] (.w)
;  OUT : eq - No collision
;        ne - Collision
; All registers are restored

	move.w	d0,d2			; Get bit number & offset
	add.w	d0,d0
	add.w	d2,d0
	subq.w	#1,d0
	moveq.l	#7,d2
	and.w	d0,d2
	sub.w	d2,d0
	subq.w	#1,d1
	mulu.w	#3*Bytes_per_collision_line,d1
	add.w	d0,d1
	moveq.l	#5-1,d0			; Do
.Loop:	bset	d2,0(a0,d1.w)		; Set bits
	bset	d2,Bytes_per_collision_line(a0,d1.w)
	bset	d2,2*Bytes_per_collision_line(a0,d1.w)
	bset	d2,3*Bytes_per_collision_line(a0,d1.w)
	bset	d2,4*Bytes_per_collision_line(a0,d1.w)
	subq.w	#1,d2			; Next bit
	bpl.s	.Next
	addq.w	#1,d1
	moveq.l	#7,d2
.Next:	dbra	d0,.Loop

