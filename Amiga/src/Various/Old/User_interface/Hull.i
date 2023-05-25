; Hull constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 7-2-1994

Max_modules	EQU 8
Max_HDOBs	EQU 4
Max_trees EQU 8

;*****************************************************************************
; Text parameters

Capital_colour	EQU 19

Max_Fonts	EQU 3
Max_PA	EQU 12
Max_styles	EQU 4

Minimum_space	EQU 2

Standard_text_height	EQU 8
Max_font_height	EQU 20
Line_buffer_size	EQU Max_font_height*Bytes_per_plane+4

CR	EQU "^"			; Special text codes
Comstart_char	EQU "{"
Comend_char	EQU "}"
Solid_space	EQU "$"

Hyphen1	EQU "-"
Hyphen2	EQU "/"
Separator	EQU "|"
EOTPB	EQU "~"

Max_line_length	EQU 160
Max_TPBs	EQU 50
TPB_size	EQU 512

;*****************************************************************************
; These are the text styles
	rsreset
Normal_style:	rs.l 1
Fat_style:	rs.l 1
High_style:	rs.l 1
Fat_high_style:	rs.l 1

;*****************************************************************************
; This is the Line Info structure
	rsreset
Initial_font:	rs.l 1
Initial_ink:	rs.w 1
Initial_shadow:	rs.w 1
Line_width:	rs.w 1
Width_without_spaces:	rs.w 1
Line_height:	rs.b 1
Line_skip:	rs.b 1
Base_line:	rs.b 1
Nr_of_spaces:	rs.b 1
Print_method:	rs.b 1
Initial_style:	rs.b 1
String_length:	rs.b 1
Line_info_data_size:	rs.b 0

;*****************************************************************************
; These are the print methods
	rsreset
Print_left:	rs.b 1
Print_centered:	rs.b 1
Print_right:	rs.b 1
Print_justified:	rs.b 1

;*****************************************************************************
; This is the Font structure
	rsreset
Raw_char_height:	rs.w 1	; Height of characters in pixels
Raw_char_size:	rs.w 1	; Number of bytes per character in font
Between_width:	rs.w 1	; Number of pixels between characters
Between_height:	rs.w 1	; Number of pixels between lines
Font_base_line:	rs.w 1	; Y-position of base line
Width_of_space:	rs.w 1	; Number of pixels per space
Font_graphics:	rs.l 1	; Address of font graphics
Kerning_table:	rs.l 1	; Address of paired kerning table / 0
Font_translation:	rs.l 1	; Address of ASCII-to-character-index table
Width_table:	rs.l 1	; Address of character width table
Insert_char:	rs.l 1	; Address of character insertion routine   

;*****************************************************************************
; This is the Print Area structure
	rsreset
	rs.b Area_data_size
PA_Ink:	rs.w 1
PA_Shadow:	rs.w 1
PA_Paper:	rs.w 1
PA_bg_handle:	rs.b 1
	rseven
PA_data_size:	rs.b 0

;*****************************************************************************
; This is the Hand-Drawn OBject erase data structure
	rsreset
HDOB_oldX:	rs.w 1
HDOB_oldY:	rs.w 1
HDOB_bg_handle:	rs.b 1
	rseven
HDOB_erase_data_size:	rs.b 0

;*****************************************************************************
; This is the Hand-Drawn OBject structure
	rsreset
HDOB_drawX:	rs.w 1
HDOB_drawY:	rs.w 1
HDOB_width:	rs.w 1
HDOB_height:	rs.w 1
HDOB_depth:	rs.w 1
HDOB_base_colour:	rs.w 1
HDOB_flags:	rs.b 1
HDOB_gfx_handle:	rs.b 1
HDOB_offset:	rs.l 1
HDOB_screen1:	rs.b HDOB_erase_data_size
HDOB_screen2:	rs.b HDOB_erase_data_size
HDOB_data_size:	rs.b 0

;*****************************************************************************
; These are the Hand-Drawn OBject flags
	rsreset
HDOB_block:	rs.b 1
HDOB_silhouette:	rs.b 1
HDOB_remove:	rs.b 1

;*****************************************************************************
; This is the Module structure
	rsreset
	rs.b 1			; Local/global flag
Module_ID:	rs.b 1			; Module ID
DisUpd_ptr:	rs.l 1		; Pointer	to display update routine
VblQ_ptr:	rs.l 1			; Pointer	to Vbl queue
ScrQ_ptr:	rs.l 1			; Pointer	to Screen	queue
Mev_ptr:	rs.l 1			; Pointer	to Mouse Event list
Kev_ptr:	rs.l 1			; Pointer	to Key Event list
ModInit_ptr:	rs.l 1		; Pointer	to Module	Init routine
ModExit_ptr:	rs.l 1		; Pointer	to Module	Exit routine
DisInit_ptr:	rs.l 1		; Pointer	to Display Init routine
DisExit_ptr:	rs.l 1		; Pointer	to Display Exit routine
Raster_list_ptr:	rs.l 1		; Pointer to raster list
Mouse_ptr:	rs.l 1			; Pointer to mouse pointer
PA_ptr:		rs.l 1		; Pointer to print area
MA_ptr:		rs.l 1		; Pointer to mouse area
Module_data_size:	rs.b 1

;*****************************************************************************
; These are the module types
; Note : these are actually states of bit 0 !!!
	rsreset
Global_mod:	rs.b 1
Local_mod:	rs.b 1

;*****************************************************************************
; These are the Dynamic List structures
	rsreset
DL_first:	rs.w 1
DL_entries_per_block:	rs.w 1
DL_free_in_last:	rs.w 1
DL_entry_size:	rs.w 1
DL_data_size:	rs.b 0

	rsreset
DLB_next:	rs.w 1
DLB_header_size:	rs.b 0
