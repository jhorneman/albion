; Map constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 24-3-1994

NPCs_per_map	EQU 32

;*****************************************************************************
; These are the offsets for the NPC data
	rsreset
NPC_char_nr:	rs.b 1		; Triples as short text number
				;  & monstergroup number
NPC_travel_mode:	rs.b 1
NPC_event_nr:	rs.b 1
	rseven
NPC_graphics_nr:	rs.w 1
NPC_status_bits:	rs.w 1
NPC_trigger:	rs.w 1
NPC_data_size:      rs.b 0

;*****************************************************************************
; These are the NPC status bits
	rsreset
NPC_type:	rs.b 2
NPC_movement_type:	rs.b 2
Short_dialogue:	rs.b 1
Icon_priority:	rs.b 1
NPC_wave_anim:	rs.b 1
NPC_async_anim:	rs.b 1
NPC_random_anim:	rs.b 1
NPC_map_graphics:	rs.b 1
NPC_icon_height:	rs.b 2

;*****************************************************************************
; These are the NPC types
	rsreset
Party_NPC:	rs.b 1
NPC_NPC:	rs.b 1
Monster_NPC:	rs.b 1
Object_NPC:	rs.b 1

;*****************************************************************************
; These are the NPC movement types
	rsreset
Path_movement:	rs.b 1
Random_movement:	rs.b 1
Waiting_movement:	rs.b 1
Chasing_movement:	rs.b 1

;*****************************************************************************
; These are the offsets for the virtual character data
	rsreset
	rs.b NPC_data_size
VMap_X:	rs.w 1			; Map coordinates
VMap_Y:	rs.w 1
VSource_X:	rs.l 1			; Dungeon coordinates
VSource_Y:	rs.l 1
VTarget_X:	rs.l 1			; Map coordinates
VTarget_Y:	rs.l 1
VFlags:	rs.b 1			; Internal flags
	rseven
VPath_ptr:	rs.l 1			; Offset to path (if necessary)
VDir:	rs.w 1			; Current direction
VPathlen:	rs.w 1			; Remaining length of random path
VNPC_data_size:       rs.b 0

	rsreset
Movement_ended:	rs.b 1
NPC_collided:	rs.b 1
Moving_randomly:	rs.b 1
NPC_sleeping:	rs.b 1
NPC_blocks:	rs.b 1

;*****************************************************************************
; This is the event block structure
	rsreset
	rs.b 1			; Event type
Event_b1:	rs.b 1			; Byte data
Event_b2:	rs.b 1
Event_b3:	rs.b 1
Event_b4:	rs.b 1
Event_b5:	rs.b 1
Event_w6:	rs.w 1			; Word data
Event_w8:	rs.w 1
Next_event_nr:	rs.w 1		; Number of next event in chain
Event_data_size:    rs.b 0

;*****************************************************************************
; This is the Goto-point structure
	rsreset
Goto_X:	rs.b 1			; X-coordinate
Goto_Y:	rs.b 1			; Y-coordinate
Goto_viewdir:	rs.b 1		; View direction
Goto_bit_nr:	rs.b 1		; Save bit number
Goto_pnt_data_size: rs.b 0

;*****************************************************************************
; This is the map data structure
	rsreset
Map_special:	rs.w 1		; Map status bits
Map_display_type:	rs.b 1		; = 1 (3D) or 2 (2D)
Map_music:	rs.b 1
Map_width:	rs.b 1
Map_height:	rs.b 1
Lab_data_nr:	rs.b 0
Icon_data_nr:	rs.b 1
Palette_nr:	rs.b 1
	rseven
NPC_data:	rs.b NPCs_per_map*NPC_data_size
Map_data:	rs.b 0

; *** Map layers ***

; Repeated [width x height] times :

; (2D)	0 3 - Underlay (12 bits), Overlay (12 bits)

; (3D)	0 b - Map

; EVEN

; *** Event entry list ***

; 0 w - Number of event entries
; Repeated {Height of map + 1} times :
;	0 w - Number of event entries for this line x 6
;	Repeated {Number of event entries for this line} times :
;		0 w - X-coordinate (irrelevant for Y = 0)
;		2 w - Event trigger modes
;		4 w - Number of first event block in chain (0-65535)

; *** Event data ***

; 0 w - Number of event blocks
; Repeated {Number of event blocks} times :
;	0 ? - Event data block

; *** NPC path/position data ***

; Repeated {NPCs_per_map} times :
; 	If the NPC exists :
;		If the NPC's movement type is Path :
;			Repeated {Steps_per_day} times :
;				0 w - X-coordinate
;				2 w - Y-coordinate
;		Else :
;			0 w - X-coordinate
;			2 w - Y-coordinate

; *** Goto-point data ***

; 0 w - Number of Goto-points
; Repeated {Number of Goto-points} times :
;	0 ? - Goto-point data

; *** 3D event automapper icons ***

; Repeated {Number of event entries} times :
;	0 b - Number of automapper icon

;*****************************************************************************
; These are the map status bits
	rsreset
Light_status:	rs.b 2
Map_type:	rs.b 2
Planet_spaceship_bit:	rs.b 1

;*****************************************************************************
; These are the light status
	rsreset
Always_light:	rs.b 1
Always_dark:	rs.b 1
Light_changes:	rs.b 1

;*****************************************************************************
; These are the map types
	rsreset
City_map_type:	rs.b 1
Dungeon_map_type:	rs.b 1
Wilderniss_map_type:	rs.b 1
Interior_map_type:	rs.b 1
