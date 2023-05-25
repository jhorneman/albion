
; --------- Check handles -------------------------
	lea.l	Real_handles,a0		; All handles
	moveq.l	#1,d0
	move.w	#Max_handles-2,d7
.Loop3:	lea.l	Memory_lists,a1		; Count handle occurrence
	moveq.l	#0,d1
	move.w	#Max_blocks-1,d6
.Loop4:	cmp.b	Block_handle(a1),d0		; Is this	the one ?
	bne.s	.Next4
	addq.w	#1,d1			; Count up
.Next4:	lea.l	Block_data_size(a1),a1	; Next entry
	dbra	d6,.Loop4
	tst.l	(a0)+			; Handle occupied ?
	beq.s	.Free
	cmp.w	#1,d1			; Used only once ?
	bne	.Error
	bra.s	.Next3
.Free:	tst.w	d1			; Used ?
	bne	.Error
.Next3:	addq.w	#1,d0			; Next handle
	dbra	d7,.Loop3
	bra.s	.Exit
.Error:	move.l	#HANDLES_CORRUPTED,Return_value	; Error !
	jmp	Exit_program
.Exit:
