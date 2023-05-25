; 3D map constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 24-3-1994

Objects_per_group	EQU 8

Anim_speed_3D	EQU 6
Max_3D_VBLs	EQU 10

Square_size	EQU 512		; Square size in cm. DO NOT CHANGE!!!
NPC_stat_frame	EQU 2

Max_3D_objects	EQU 64
Max_3D_walls	EQU 32
Max_3D_floors	EQU 40		; Must be multiple of 4
; NOTE! This variable is also the maximum number of floor FRAMES.

Max_visible_walls	EQU 30		; Add 1 entry for safety
Max_visible_objects	EQU 150
Max_visible_floors	EQU 200		; Add 1 entry for safety, must be even

Max_visible_elements	EQU Max_visible_walls+1+Max_visible_objects+Max_visible_floors+1

First_wall	EQU 101

; List			Object	Wall	Floor
; Dungeon_sort_list		8	8	8
; Dungeon_wall_list		-	18	46
; Dungeon_object_list	12	-	-
; Dungeon_draw_list		2/22	4	-
; Dungeon_clipping_table	-	4	-

Map3D_X	EQU 48
Map3D_Y	EQU 20

; ---------- Dungeon stuff ------------------------
slang	EQU 2048			; anzahl winkelschritte
proj_faktor	EQU 256
proj_log	EQU 8
Map3D_width	EQU 256
Map3D_height	EQU 134

patt_size	EQU 1024			; Square size in 3D units
psize_log	EQU 10
pvec_mulval EQU patt_size*10/286

list_size	EQU 256			; anzahl zu verwaltender elemente

boden_maxz	EQU 22000

	rsreset
dpk_x1:		rs.w 1
dpk_x2:		rs.w 1
dpk_y1:		rs.w 1
dpk_y2:		rs.w 1
dpk_z1:		rs.w 1
dpk_z2:		rs.w 1

;*****************************************************************************
; This is the 3D LAByrinth data structure
	rsreset
Wall_height_in_cm:	rs.w 1
Horizon_position:	rs.w 1
Shade_table_nr:	rs.w 1
Background_nr:	rs.w 1
Lab_data:	rs.w 0
;	0 w - Number of object groups
;	? ? - Object group data
;          ? w - Number of floors & ceilings
;          ? ? - Floor & ceiling data
;          ? w - Number of objects
;	? ? - Object data
;          ? w - Number of walls
;	? ? - Wall data

;*****************************************************************************
; These are the offsets for the 3D objects in a group
	rsreset
Object_X:	rs.w 1
Object_Y:	rs.w 1
Object_Z:	rs.w 1
Object_nr:	rs.w 1
Objectingroup_data_size:	rs.b 0

;*****************************************************************************
; These are the offsets for the 3D object group data
	rsreset
Object_automapper_icon:	rs.w 1
	rs.b Objects_per_group*Objectingroup_data_size
Objectgroup3D_data_size:	rs.b 0

;*****************************************************************************
; These are the offsets for the 3D floor data
	rsreset
Floor_bits:	rs.l 1
Floor_nr_frames:	rs.b 1
	rs.b 1
Floor_graphics_nr:	rs.w 1
Floor3D_data_size:	rs.b 0
; Data size must be 8 bytes because of scaled addressing modes !!!

;*****************************************************************************
; These are the offsets for the 3D object data
	rsreset
Object_bits:	rs.l 1
Object_graphics_nr:	rs.w 1
Object_nr_frames:	rs.b 1
	rs.b 1
Object_width:	rs.w 1
Object_height:	rs.w 1
Object_dungeon_width:	rs.w 1
Object_dungeon_height:	rs.w 1
Object3D_data_size:	rs.b 0

;*****************************************************************************
; These are the 3D status bits
	rsreset
	rs.b 1
Vision_blocked_bit:	rs.b 1
Distort_bit:	rs.b 1
	rs.b 1
	rs.b 1
Masked_wall_bit:	rs.b 1
Transparent_wall_bit:	rs.b 1

;*****************************************************************************
; These are the offsets for the 3D wall data
	rsreset
Wall_bits:	rs.l 1
Wall_graphics_nr:	rs.w 1
Wall_nr_frames:	rs.b 1
Wall_automapper_icon:	rs.b 1
Wall_transparent_colour:	rs.w 1
Wall_width:	rs.w 1
Wall_height:	rs.w 1
Wall_nr_overlays:	rs.w 1
Wall_data_size:	rs.b 0
; Overlay data

;*****************************************************************************
; These are the offsets for the 3D wall overlay data
	rsreset
Overlay_graphics_nr:	rs.w 1
Overlay_nr_frames:	rs.b 1
Overlay_mask_flag:	rs.b 1		; 0 EQU Block, 1 EQU Mask
Overlay_X:	rs.w 1
Overlay_Y:	rs.w 1
Overlay_width:	rs.w 1
Overlay_height:	rs.w 1
Overlay_data_size:	rs.b 0
