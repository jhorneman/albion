; File types
; Written by J.Horneman (In Tune With The Universe)
; Start : 27-1-1994

Max_saves	EQU 10
Save_nr_offset	EQU 6
Nr_save_files	EQU 5

	rsreset
;*****************************************************************************
; These are the Omnifile types
	rsreset
	rs.b 1			; Memory block (NOT A FILE)
Saves_file:	rs.b 1
Party_data_file:	rs.b 1
Party_char_file:	rs.b 1
NPC_char_file:	rs.b 1
Monster_char_file:	rs.b 1
Small_portraits_file:	rs.b 1
Full_body_pix_file:	rs.b 1
Palette_file:	rs.b 1
Music_file:	rs.b 1
Map_data_file:	rs.b 1
Map_text_file:	rs.b 1
Icon_data_file:	rs.b 1
Icon_gfx_file:	rs.b 1
Trans_gfx_file:	rs.b 1
Person_gfx_file:	rs.b 1
Lab_data_file:	rs.b 1
Lab_background_file:	rs.b 1
Wall_3D_file:	rs.b 1
Object_3D_file:	rs.b 1
Overlay_3D_file:	rs.b 1
Floor_3D_file:	rs.b 1
Combat_background_file:	rs.b 1
Combat_gfx_file:	rs.b 1
Monster_group_file:	rs.b 1
Monster_gfx_file:	rs.b 1
Chest_data_file:	rs.b 1
Merchant_data_file:	rs.b 1
Automap_file:	rs.b 1
Automap_gfx_file:	rs.b 1
Dictionary_file:	rs.b 1
Place_data_file:	rs.b 1
Riddlemouth_gfx_file:	rs.b 1
Object_gfx_file:	rs.b 1
Pictures_file:	rs.b 1
Party_text_file:	rs.b 1
NPC_text_file:	rs.b 1
Monster_text_file:	rs.b 1
Object_text_file:	rs.b 1
Script_file:	rs.b 1
Floor_3D_buffer_file:	rs.b 1
Max_file_type:	rs.b 0

;*****************************************************************************
; This is the File info structure
	rsreset
File_start_priority:	rs.b 1
File_memory_type:	rs.b 1
File_unpack_offset:	rs.w 1
File_relocator:	rs.l 1
File_info_data_size:	rs.b 0	; Beware when changing this !
