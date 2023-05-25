; File types
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 10-3-1994

	MODULE	DDT_File_types

	OUTPUT	DDT:Objects/File_types.o

	XREF	Fatal_error
	XREF	Load_file
	XREF	Save_file
	XREF	Save_encrypted_file
	XREF	Claim_pointer
	XREF	Free_pointer
	XREF	Kill_memory
	XREF	Get_memory_length
	XREF	Copy_memory
	XREF	DecR_convert
	XREF	Dictionary_filename
	XREF	Relocate_music
	XREF	Relocate_3D_floor_buffer
	XREF	Relocate_3D_wall

	XDEF	Get_Omnifile_name
	XDEF	Get_file_safety_factor
	XDEF	Get_file_memory_type
	XDEF	Get_file_priority
	XDEF	Get_file_relocator
	XDEF	Default_memory_type
	XDEF	Default_safety_factor
	XDEF	Get_saved_game
	XDEF	Put_saved_game

	incdir	DDT:Constants/
	include	Global.i
	include	OS/OS.i
	include	Core.i

	SECTION	Program,code
;*****************************************************************************
; [ Get Omnifile name ]
;   IN : d0 - 0 / Omnifile number {1...} (.w)
;        a0 - Filename (when d0 = 0) (.l)
;  OUT : a0 - Pointer to Omnifile name (.l)
; Changed registers : a0
;*****************************************************************************
Get_Omnifile_name:
	tst.w	d0			; Normal file type ?
	beq.s	.Exit
	cmpi.w	#Max_file_type,d0		; Yes -> Legal file type ?
	bmi.s	.Ok
	move.l	#ILLEGAL_FILE_TYPE,d0	; No
	jmp	Fatal_error
.Ok:	lea.l	Omnifilenames,a0		; Yes -> Get pointer to filename
	move.l	-4(a0,d0.w*4),a0
.Exit:	rts

;*****************************************************************************
; [ Get file memory type ]
;   IN : d1 - File type (.b)
;  OUT : d1 - Memory type (.b)
; Changed registers : d1
;*****************************************************************************
Get_file_memory_type:
	move.l	a0,-(sp)
	lea.l	File_info,a0
	and.w	#$00ff,d1
	move.b	File_memory_type(a0,d1.w*8),d1
	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Get file memory type ]
;   IN : d1 - File type (.b)
;  OUT : d1 - File priority (.b)
; Changed registers : d1
;*****************************************************************************
Get_file_priority:
	move.l	a0,-(sp)
	lea.l	File_info,a0
	and.w	#$00ff,d1
	move.b	File_start_priority(a0,d1.w*8),d1
	and.b	#$7f,d1
	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Get file unpack safety factor ]
;   IN : d0 - File type (.b)
;  OUT : d0 - Unpack safety factor (.l)
; Changed registers : d0
;*****************************************************************************
Get_file_safety_factor:
	move.l	a0,-(sp)
	lea.l	File_info,a0
	and.w	#$00ff,d0
	move.w	File_unpack_offset(a0,d0.w*8),d0
	ext.l	d0
	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Get file unpack safety factor ]
;   IN : d0 - File type (.b)
;  OUT : a2 - Pointer to file relocator (.l)
; Changed registers : a2
; Notes :
;   - [File_relocators] entry :
;     	Block types which may be moved at will : [Copy_memory]
;     	Block types which may not be moved : 0
;     	Block types which must be relocated : Custom routine
;   - Relocator routines receive the same parameters as [Copy_memory].
;*****************************************************************************
Get_file_relocator:
	move.l	d0,-(sp)
	lea.l	File_info,a2
	and.w	#$00ff,d0
	move.l	File_relocator(a2,d0.w*8),a2
	move.l	(sp)+,d0
	rts

;*****************************************************************************
; [ Copy a saved game from a save folder to the work files ]
;   IN : d0 - Save number / 0 for restart (.w)
; All registers are restored
;*****************************************************************************
Get_saved_game:
	movem.l	d0/d1/d6/d7/a0-a3,-(sp)
	jsr	Select_save		; Select folder
	moveq.l	#0,d1			; (For Save_file)
	lea.l	Save_filenames,a2
	lea.l	Omnifilenames,a3
	moveq.l	#Nr_save_files-1,d7
.Loop:	move.l	2(a2),a0			; Get source filename
	moveq.l	#0,d0			; Load file
	jsr	Load_file
	move.b	d0,d6
	jsr	Claim_pointer		; Get file address
	move.l	d0,a0
	move.w	(a2),d0			; Get file type
	lsl.w	#2,d0			; Get destination filename
	move.l	-4(a3,d0.w),a1
	move.b	d6,d0			; Get file length
	jsr	Get_memory_length
	jsr	Save_file			; Save file
	move.b	d6,d0			; Destroy file
	jsr	Free_pointer
	jsr	Kill_memory
	addq.l	#6,a2			; Next
	dbra	d7,.Loop
	movem.l	(sp)+,d0/d1/d6/d7/a0-a3
	rts

;*****************************************************************************
; [ Copy a saved game from the work files to a save folder ]
;   IN : d0 - Save number (.w)
; All registers are restored
;*****************************************************************************
Put_saved_game:
	movem.l	d0/d1/d6/d7/a0-a3,-(sp)
	tst.w	d0			; Restart game ?
	beq	.Exit
	jsr	Select_save		; Select folder
	moveq.l	#0,d1			; (For Save_file)
	lea.l	Save_filenames,a2
	lea.l	Omnifilenames,a3
	moveq.l	#Nr_save_files-1,d7
.Loop:	move.w	(a2),d0			; Get file type
	lsl.w	#2,d0			; Get source filename
	move.l	-4(a3,d0.w),a0
	moveq.l	#0,d0			; Load file
	jsr	Load_file
	move.b	d0,d6
	jsr	Claim_pointer		; Get file address
	move.l	d0,a0
	move.l	2(a2),a1			; Get destination filename
	move.b	d6,d0			; Get file length
	jsr	Get_memory_length
	jsr	Save_encrypted_file		; Save file
	move.b	d6,d0			; Destroy file
	jsr	Free_pointer
	jsr	Kill_memory
	addq.l	#6,a2			; Next
	dbra	d7,.Loop
.Exit:	movem.l	(sp)+,d0/d1/d6/d7/a0-a3
	rts

;*****************************************************************************
; [ Select a save folder ]
;   IN : d0 - Save number (.w)
; All registers are restored
; Notes :
;   - 0 is the restart folder.
;*****************************************************************************
Select_save:
	movem.l	d1/d6/d7/a0/a1,-(sp)
	cmp.w	#Max_saves+1,d0		; Too high ?
	bpl.s	.Exit
	move.b	#"0",d6			; No -> do
	moveq.l	#2,d7
	lea.l	Save_filenames,a1
	moveq.l	#Nr_save_files-1,d1
.Loop:	addq.l	#2,a1			; Get filename
	move.l	(a1)+,a0
	lea.l	Save_nr_offset(a0),a0
	jsr	DecR_convert
	move.b	#"/",(a0)			; Repair
	dbra	d1,.Loop
.Exit:	movem.l	(sp)+,d1/d6/d7/a0/a1
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
; (starts at type 0 !!!)
File_info:
	dc.b 100			; Memory block
Default_memory_type:	dc.b DONTCARE
Default_safety_factor:	dc.w 0
	dc.l Copy_memory
	dc.b 0,DONTCARE		; Saves file
	dc.w 0
	dc.l Copy_memory
	dc.b 100,DONTCARE		; Party data file
	dc.w 0
	dc.l Copy_memory
	dc.b 50,DONTCARE		; Party char file
	dc.w 0
	dc.l Copy_memory
	dc.b 100,DONTCARE		; NPC char file
	dc.w 1024
	dc.l Copy_memory
	dc.b 110,DONTCARE		; Monster char file
	dc.w 1024
	dc.l Copy_memory
	dc.b 120,CHIP		; Small portraits file
	dc.w 1024
	dc.l Copy_memory
	dc.b 120,CHIP		; Full body pix file
	dc.w 1024
	dc.l Copy_memory
	dc.b 101,DONTCARE		; Palette file
	dc.w 1024
	dc.l Copy_memory
	dc.b 100,DONTCARE		; Music file
	dc.w 0
	dc.l Relocate_music
	dc.b 99,DONTCARE		; Map data file
	dc.w 4096
	dc.l Copy_memory
	dc.b 50,DONTCARE		; Map text file
	dc.w 2048
	dc.l Copy_memory
	dc.b 120,DONTCARE		; Icon data file
	dc.w 1024
	dc.l Copy_memory
	dc.b 120,CHIP		; Icon gfx file
	dc.w 1024
	dc.l Copy_memory
	dc.b 121,CHIP		; Trans gfx file
	dc.w 1024
	dc.l Copy_memory
	dc.b 120,CHIP		; Person gfx file
	dc.w 1024
	dc.l Copy_memory
	dc.b 110,DONTCARE		; Lab data file
	dc.w 1024
	dc.l Copy_memory
	dc.b 90,FAST		; Lab background file
	dc.w 1024
	dc.l Copy_memory
	dc.b 99,FAST		; Wall 3D file
	dc.w 1024
	dc.l Relocate_3D_wall
	dc.b 90,FAST		; Object 3D file
	dc.w 1024
	dc.l Copy_memory
	dc.b 80,FAST		; Overlay 3D file
	dc.w 1024
	dc.l Copy_memory
	dc.b 0,DONTCARE		; Floor 3D file
	dc.w 0
	dc.l Copy_memory
	dc.b 100,CHIP		; Combat background file
	dc.w 4096
	dc.l Copy_memory
	dc.b 120,CHIP		; Combat gfx file
	dc.w 1024
	dc.l Copy_memory
	dc.b 90,DONTCARE		; Monster group file
	dc.w 1024
	dc.l Copy_memory
	dc.b 0,CHIP		; Monster gfx file
	dc.w 1024
	dc.l Copy_memory
	dc.b 80,DONTCARE		; Chest data file
	dc.w 0
	dc.l Copy_memory
	dc.b 90,DONTCARE		; Merchant data file
	dc.w 0
	dc.l Copy_memory
	dc.b 0,DONTCARE		; Automap file
	dc.w 0
	dc.l Copy_memory
	dc.b 120,CHIP		; Automap gfx file
	dc.w 1024
	dc.l Copy_memory
	dc.b 0,DONTCARE		; Dictionary file
	dc.w 1024
	dc.l Copy_memory
	dc.b 0,DONTCARE		; Place data file
	dc.w 1024
	dc.l Copy_memory
	dc.b 0,CHIP		; Riddlemouth gfx file
	dc.w 1024
	dc.l Copy_memory
	dc.b 127,CHIP		; Object gfx file
	dc.w 1024
	dc.l Copy_memory
	dc.b 80,DONTCARE		; Pictures file
	dc.w 1024
	dc.l Copy_memory
	dc.b 100,DONTCARE		; Party text file
	dc.w 1024
	dc.l Copy_memory
	dc.b 70,DONTCARE		; NPC text file
	dc.w 2048
	dc.l Copy_memory
	dc.b 60,DONTCARE		; Monster text file
	dc.w 2048
	dc.l Copy_memory
	dc.b 0,DONTCARE		; Object text file
	dc.w 2048
	dc.l Copy_memory
	dc.b 40,DONTCARE		; Script file
	dc.w 2048
	dc.l Copy_memory
	dc.b 0,FAST		; 3D floor buffer
	dc.w 0
	dc.l Relocate_3D_floor_buffer

Save_filenames:
	dc.w Party_data_file
	dc.l .SFile01
	dc.w Party_char_file
	dc.l .SFile02
	dc.w Automap_file
	dc.l .SFile03
	dc.w Chest_data_file
	dc.l .SFile04
	dc.w Merchant_data_file
	dc.l .SFile05

.SFile01:	dc.b "Save.00/Party_data",0
.SFile02:	dc.b "Save.00/Party_char.omni",0
.SFile03:	dc.b "Save.00/Automap.omni",0
.SFile04:	dc.b "Save.00/Chest_data.omni",0
.SFile05:	dc.b "Save.00/Merchant_data.omni",0
	even

Omnifilenames:
	dc.l .File01,.File02,.File03,.File04
	dc.l .File05,.File06,.File07,.File08
	dc.l .File09,.File10,.File11,.File12
	dc.l .File13,.File14,.File15,.File16
	dc.l .File17,.File18,.File19,.File20
	dc.l .File21,.File22,.File23,.File24
	dc.l .File25,.File26,.File27,.File28
	dc.l .File29,Dictionary_filename,.File31,.File32
	dc.l .File33,.File34,.File35,.File36
	dc.l .File37,.File38,.File39,0

.File01:	dc.b "Saves",0
.File02:	dc.b "Party_data",0
.File03:	dc.b "Party_char_data.omni",0
.File04:	dc.b "NPC_char_data.omni",0
.File05:	dc.b "Monster_char_data.omni",0
.File06:	dc.b "Small_portraits.omni",0
.File07:	dc.b "Full_body_pix.omni",0
.File08:	dc.b "Palettes.omni",0
.File09:	dc.b "Music.omni",0
.File10:	dc.b "Map_data.omni",0
.File11:	dc.b "Map_texts.omni",0
.File12:	dc.b "Icon_data.omni",0
.File13:	dc.b "Icon_graphics.omni",0
.File14:	dc.b "Trans_graphics.omni",0
.File15:	dc.b "Person_graphics.omni",0
.File16:	dc.b "Lab_data.omni",0
.File17:	dc.b "Lab_backgrounds.omni",0
.File18:	dc.b "3D_walls.omni",0
.File19:	dc.b "3D_objects.omni",0
.File20:	dc.b "3D_overlays.omni",0
.File21:	dc.b "3D_floors.omni",0
.File22:	dc.b "Combat_backgrounds.omni",0
.File23:	dc.b "Combat_graphics",0
.File24:	dc.b "Monster_groups.omni",0
.File25:	dc.b "Monster_graphics.omni",0
.File26:	dc.b "Chest_data.omni",0
.File27:	dc.b "Merchant_data.omni",0
.File28:	dc.b "Automaps.omni",0
.File29:	dc.b "Automap_graphics",0
.File31:	dc.b "Place_data",0
.File32:	dc.b "Riddlemouth_graphics",0
.File33:	dc.b "Object_graphics",0
.File34:	dc.b "Pictures.omni",0
.File35:	dc.b "Party_texts.omni",0
.File36:	dc.b "NPC_texts.omni",0
.File37:	dc.b "Monster_texts.omni",0
.File38:	dc.b "Object_texts.omni",0
.File39:	dc.b "Scripts.omni",0
	even
