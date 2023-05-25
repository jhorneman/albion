
;*****************************************************************************
; These are the offsets for the LAByrinth data file
	rsreset
Wall_in_cm:	rs.w 1
Default_combat_bg_nr:	rs.w 1
Sky_colour_nr:	rs.b 1
Floor_colour_nr:	rs.b 1
Ceiling_graphics_nr:	rs.b 1
Floor_graphics_nr:	rs.b 1
Lab_data:	rs.w 0
; Number of object groups	rs.w 1

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
; These are the offsets for the 3D object data
	rsreset
Object_bits:	rs.l 1
Object_graphics_nr:	rs.w 1
Object_nr_frames:	rs.b 1
	rs.b 1
Object_width:	rs.b 1
Object_height:	rs.b 1
Object_dungeon_width:	rs.w 1
Object_dungeon_height:	rs.w 1
Object3D_data_size:	rs.b 0

;*****************************************************************************
; These are the offsets for the 3D wall data
	rsreset
Wall_bits:	rs.l 1
Wall_width:	rs.w 1
Wall_height:	rs.w 1
Wall_nr_frames:	rs.b 1
	rs.b 1
Wall_graphics_nr:	rs.b 1
Wall_automapper_icon:	rs.b 1
Wall_minimap_colour:	rs.b 1
Wall_nr_overlays:	rs.b 1
Wall_data_size:	rs.b 0

;*****************************************************************************
; These are the offsets for the 3D wall overlay data
	rsreset
Overlay_mask_flag:	rs.b 1			; 0 = Block, 1 = Mask
Overlay_graphics_nr:	rs.b 1
Overlay_X:	rs.b 1
Overlay_Y:	rs.b 1
Overlay_width:	rs.b 1
Overlay_height:	rs.b 1
Overlay_data_size:	rs.b 0
