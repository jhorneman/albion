; 2D map constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 24-3-1994

Max_2D_objects	EQU 50

Map2D_X	EQU 0
Map2D_Y	EQU 16
Mapbuf_width	EQU 22
Mapbuf_height	EQU 12
Mapbuf_depth	EQU 4
Mapbuf_size	EQU Mapbuf_width*Mapbuf_height

;*****************************************************************************
; This is the 2D object structure
	rsreset
Object2D_index:	rs.w 1
Object2D_X:	rs.w 1
Object2D_Y:	rs.w 1
Object2D_width:	rs.w 1
Object2D_height:	rs.w 1
Object2D_gfx_handle:	rs.b 1
Object2D_level:	rs.b 1
Object2D_gfx_offset:	rs.l 1
Object2D_data_size:	rs.b 0

;*****************************************************************************
; This is the 2D icon data structure
	rsreset
Icon_bits:	rs.l 1
Icon_1st_frame:	rs.w 1
Icon_nr_frames:	rs.b 1
	rs.b 1
; Data size must be 8 bytes because of scaled addressing modes !!!

;*****************************************************************************
; These are the 2D icon status bits
	rsreset
Circle_wave_bit:	rs.b 1
Underlay_priority_bit:	rs.b 1
Overlay_bit:	rs.b 1
Async_anim_bit:	rs.b 1
Random_anim_bit:	rs.b 1
Icon_height:	rs.b 2
Blocked_direction:	rs.b 4
Blocked_travelmodes:	rs.b 10
Person_invisible_bit:	rs.b 1
Add_next_icon_bit:	rs.b 1
Sit_sleep_state:	rs.b 4
Combat_background_nr:	rs.b 5
