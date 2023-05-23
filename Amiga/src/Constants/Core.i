; Core constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 27-1-1994

Max_Mptr	EQU 12
Max_MA	EQU 12
Max_CA	EQU 12

Max_keys	EQU 20			; Maximum input events
Max_Mevs	EQU 10

Dclick_interval	EQU 10
Busy_interval	EQU 20

Mask_buffer_size	EQU 2*2*48

;*****************************************************************************
; Screen dimensions
Screen_width	EQU 320+32
Screen_height	EQU 200+40
Screen_depth	EQU 8

Trunced_width	EQU (Screen_width+15)/16

Bytes_per_plane	EQU ((Screen_width+63)/64)*8
Bytes_per_line	EQU Screen_depth*Bytes_per_plane
Screen_size	EQU Screen_height*Bytes_per_line

Pal_size	EQU 1<<Screen_depth

;*****************************************************************************
; This is the Mouse PoinTeR structure
	rsreset
Mptr_hotspot_X:	rs.w 1
Mptr_hotspot_Y:	rs.w 1
Mptr_colourbase:	rs.w 1
Mptr_graphics:	rs.b 0

;*****************************************************************************
; Mouse pointer parameters

Mptr_width	EQU 32
Mptr_height	EQU 32

	rsreset
Sprite_bitmap:	rs.l 1
Sprite_width:	rs.w 1
Sprite_1_nr:	rs.w 1
Sprite_2_nr:	rs.w 1
Sprite_1:	rs.l 1
Sprite_2:	rs.l 1
Sprite_data_size:	rs.b 0

;*****************************************************************************
; File parameters

File_header_size	EQU 12
Max_subfiles	EQU 500
Max_batch	EQU 64

Packed_ID	EQU (1<<24)!("L"<<16)!("O"<<8)!("B")
Crypt_ID	EQU ("J"<<8)!("H")

	rsreset
Encrypted:	rs.b 1
Packed:	rs.b 1

;*****************************************************************************
; These are the mouse button status bits
	rsreset
Left_pressed:	rs.b 1
Left_clicked:	rs.b 1
Left_released:	rs.b 1
Left_double:	rs.b 1
Right_pressed:	rs.b 1
Right_clicked:	rs.b 1
Right_released:	rs.b 1
Right_double:	rs.b 1

;*****************************************************************************
; This is the Mouse Area structure
	rsreset
	rs.b Recta_size
MA_data_size:	rs.b 0

;*****************************************************************************
; This is the Clip Area structure
	rsreset
	rs.b Recta_size
CA_data_size:	rs.b 0

;*****************************************************************************
; These are the Key Event bits
	rsreset
	rs.b 8				; ASCII code
	rs.b 8				; Not used
	rs.b 8				; Scan-code
Shift_key:	rs.b 1
Control_key:	rs.b 1
Alternate_key:	rs.b 1
CapsLock_key:	rs.b 1
Amiga_key:	rs.b 1
Diskinserted_key:	rs.b 1

;*****************************************************************************
; These are other Core constants

	incdir	DDT:
	include	Constants/Memory.i
	include	Constants/File_types.i
