; Memory manager constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 27-1-1994

Max_areas		EQU 10
Max_blocks	EQU 500
Max_handles	EQU 200
Max_files		EQU 255

Small_fish	EQU 5*1024		; Too small to manage
OS_CHIP	EQU 20*1024		; Returned to OS
OS_FAST	EQU 20*1024
Minimum_CHIP	EQU 280000	; Minimum needed
Minimum_FAST	EQU 75000

Check_frequency	EQU 10000		; Memory self-check frequency
Age_all_frequency	EQU 20		; Age all blocks frequency

;*****************************************************************************
; These are the BLOCK memory types
	rsreset
	rs.b 1
CHIP:	rs.b 1				; 1
FAST:	rs.b 1				; 2
DONTCARE:	rs.b 1				; 3

;*****************************************************************************
; These are the AREA memory types
; Note : These are bit numbers referring to the BLOCK memory types
	rsreset
CHIP_area:	rs.b 1
FAST_area:	rs.b 1

;*****************************************************************************
; This is the memory area structure
	rsreset
Area_start:	rs.l 1		; Area start & size
Area_size:	rs.l 1
	rs.l 1			; Previous : always zero !
	rs.l 1			;     Next : zero / start of list
	rs.b 1			; Area number
Memory_type:	rs.b 1		; 0 = CHIP, 1 = FAST
Area_data_size:	rs.b 0

;*****************************************************************************
; This is the memory block structure
	rsreset
Block_start:	rs.l 1		; Block start & size
Block_size:	rs.l 1
Block_previous:	rs.l 1		; Links
Block_next:	rs.l 1
Block_info:	rs.b 0		; Block info :
Block_flags:	rs.b 1		;	Flags
Block_handle:	rs.b 1		;	Handle number
Block_claim:	rs.b 1		;	Claim counter
Block_file_index:	rs.b 1		;	File info index
Block_data_size:	rs.b 0

;*****************************************************************************
; These are the memory block flags
	rsreset
Allocated:	rs.b 1
Invalid:	rs.b 1
Checked:	rs.b 1

;*****************************************************************************
; This is the file info structure
	rsreset
File_type:	rs.b 1			; File type (0 = free)
File_priority:	rs.b 1		; Priority
File_load_counter:	rs.b 1		; Load counter
File_length_low:	rs.b 1		; Original length low byte
Subfile_nr:	rs.w 1		; Subfile number
File_data_size:	rs.b 0
