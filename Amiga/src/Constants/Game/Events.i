; Events constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 2-5-1994

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
; This is the event context structure :
	rsreset
Event_context_type:	rs.b 1
Event_context_flags:	rs.b 1
Event_handle:	rs.b 1		; Memory block containing event data
Event_text_handle:	rs.b 1		; Memory block containing texts
Event_chain_nr:	rs.b 1		; Current chain number
	rseven
Event_block_nr:	rs.w 1		; Current block number
Event_base:	rs.l 1		; Offset to event blocks
Event_data:	rs.b Event_data_size	; Current event block
Context_validator:	rs.l 1
Normal_event_data:	rs.b 0
	rs.l 2				; Extra data	
Event_context_size:	rs.b 0

; This is the set event context structure :
	rsreset
	rs.b Normal_event_data
Action_type:	rs.b 1		; Action type
Action_extra:	rs.b 1		; Extra value
Action_value:	rs.w 1		; Value
Actor:	rs.b 1

; This is the map event context structure :
	rsreset
	rs.b Normal_event_data
Event_map_nr:	rs.w 1		; Needed to save event
Event_trigger:	rs.b 1
Event_X:	rs.b 1
Event_Y:	rs.b 1

;*****************************************************************************
; These are the event context types :
	rsreset
Map_context:	rs.b 1
Set_context:	rs.b 1
Inline_context:	rs.b 1

;*****************************************************************************
; These are the event context flags :
	rsreset
Break_event_chain:	rs.b 1
Chain_contained_execute:	rs.b 1
Success_flag:	rs.b 1
Execute_is_default:	rs.b 1
Have_executed:	rs.b 1
Execute_ordered:	rs.b 1
Execute_forbidden:	rs.b 1

;*****************************************************************************
; These are the trigger modes
	rsreset
Normal_trigger:	rs.b 1
Examine_trigger:	rs.b 1
Touch_trigger:	rs.b 1
Speak_trigger:	rs.b 1
Use_item_trigger:	rs.b 1
Map_init_trigger:	rs.b 1
Every_step_trigger:	rs.b 1
Every_hour_trigger:	rs.b 1
Every_day_trigger:	rs.b 1
Default_trigger:	rs.b 1
Action_trigger:	rs.b 1

;*****************************************************************************
; These are the event types
	rsreset
	rs.b 1
Map_exit_type:	rs.b 1
Door_type:	rs.b 1
Chest_type:	rs.b 1
Text_type:	rs.b 1
Spinner_type:	rs.b 1
Trap_type:	rs.b 1
	rs.b 1
Datachange_type:	rs.b 1
Change_icon_type:	rs.b 1
Encounter_type:	rs.b 1
Place_action_type:	rs.b 1
Query_type:	rs.b 1
Modify_type:	rs.b 1
Action_type:	rs.b 1
Signal_type:	rs.b 1
	rs.b 1
Sound_type:	rs.b 1
Start_dialogue_type:	rs.b 1
Create_trans_type:	rs.b 1
Execute_type:	rs.b 1
Remove_member_type:	rs.b 1
End_dialogue_type:	rs.b 1
Wipe_type:	rs.b 1
Play_animation_type:	rs.b 1
	rs.b 1
Pause_type:	rs.b 1
Simple_chest_type:	rs.b 1
Ask_surrender_type:	rs.b 1
Do_script_type:	rs.b 1
Max_event_types:         rs.b 0

;*****************************************************************************
; These are the map exit event types
	rsreset
Normal_MX_type:	rs.b 1
Teleporter_MX_type:	rs.b 1
Trapdoor_up_MX_type:	rs.b 1
Jump_MX_type:	rs.b 1
End_sequence_MX_type:	rs.b 1
Trapdoor_down_MX_type:	rs.b 1
Shuttle_MX_type:	rs.b 1

;*****************************************************************************
; These are the text event types
	rsreset
Normal_text_type:	rs.b 1
Visual_text_type:	rs.b 1
Party_spoken_text_type:	rs.b 1
NPC_spoken_text_type:	rs.b 1
Multiple_choice_text_type:	rs.b 1
Place_title_text_type:	rs.b 1
Permanent_text_type:	rs.b 1
Combat_text_type:	rs.b 1
Slow_combat_text_type:	rs.b 1
