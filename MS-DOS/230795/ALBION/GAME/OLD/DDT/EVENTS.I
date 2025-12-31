; Events constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 2-5-1994

Max_Event_contexts	EQU 40
Max_Set_lists	EQU 8

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
Success_flag:	rs.b 1

;*****************************************************************************
; This is the set list structure :
	rsreset
Set_list_flags:	rs.b 1
Set_list_modes:	rs.b 1
Actor_index:	rs.b 1
Action_type:	rs.b 1			; Action data
Action_extra:	rs.b 1
	rseven
Action_value:	rs.w 1
Action_ptr:	rs.l 1			; Pointer to action
Set_list_ptr:	rs.l 1			; Pointer to set list
Set_list_size:	rs.b 0

;*****************************************************************************
; These are the set list flags :
	rsreset
Have_executed:	rs.b 1
Forbidden:	rs.b 1

;*****************************************************************************
; These are the set list mode bits :
	rsreset
Dialogue_set_mode:	rs.b 1
Combat_set_mode:	rs.b 1
Item_set_mode:	rs.b 1
Magic_set_mode:	rs.b 1

;*****************************************************************************
; These are the set types :
	rsreset			; Parameter in set list :
Map_set:	rs.b 1
Default_set:	rs.b 1
Item_set:	rs.b 1
Magic_set:	rs.b 1
Dialogue_set:	rs.b 1
Acting_member_set:	rs.b 1
Non_acting_member_set:	rs.b 1	; Member number {1...6}
Acting_part_set:	rs.b 1
Friend_part_set:	rs.b 1		; Participant number {1...18}
Enemy_part_set:	rs.b 1		; Participant number {1...18}

;*****************************************************************************
; These are the action types :
	rsreset
Ask_about_action:	rs.b 1
Give_item_action:	rs.b 1
Give_gold_action:	rs.b 1
Give_food_action:	rs.b 1
Ask_to_join_action:	rs.b 1
Ask_to_leave_action:	rs.b 1
Dialogue_start_action:	rs.b 1
Dialogue_end_action:	rs.b 1
Select_answer_action:	rs.b 1
Unknown_word_action:	rs.b 1
Combat_start_action:	rs.b 1
Combat_end_action:	rs.b 1
Every_round_action:	rs.b 1
Part_is_hit_action:	rs.b 1
Part_dies_action:	rs.b 1
Part_hits_action:	rs.b 1
Part_kills_action:	rs.b 1
Part_is_asked_to_surrender_action:	rs.b 1
Part_flees_action:	rs.b 1
Part_attacks_short_action:	rs.b 1
Part_attacks_long_action:	rs.b 1
Part_is_attacked_short_action:	rs.b 1
Part_is_attacked_long_action:	rs.b 1
Part_attacks_magic_action:	rs.b 1
Part_is_attacked_magic_action:	rs.b 1
Part_deflects_attack_action:	rs.b 1
Part_attack_is_deflected_action:	rs.b 1
Part_makes_critical_hit_action:	rs.b 1
Train_a_skill_action:	rs.b 1
Heal_LP_action:	rs.b 1
Heal_a_condition_action:	rs.b 1
Remove_cursed_items_action:	rs.b 1
Scrutinize_item_action:	rs.b 1
Recharge_item_action:	rs.b 1
Rent_a_room_action:	rs.b 1
Buy_item_action:	rs.b 1
Sell_item_action:	rs.b 1
View_merchant_item_action:	rs.b 1
Buy_food_action:	rs.b 1
Buy_spell_scroll_action:	rs.b 1
Buy_transport_action:	rs.b 1
Learn_spell_action:	rs.b 1
Repair_item_action:	rs.b 1
Cast_spell_action:	rs.b 1
Use_item_action:	rs.b 1
Equip_item_action:	rs.b 1
Unequip_item_action:	rs.b 1
Drop_item_action:	rs.b 1
Take_item_from_chest_action:	rs.b 1
Take_item_from_junkpile_action:	rs.b 1
Store_item_in_chest_action:	rs.b 1
	rs.b 1
Take_item_from_backpack_action:	rs.b 1
Game_begins_action:	rs.b 1
Game_over_action:	rs.b 1
Signal_action:	rs.b 1
Level_made_action:	rs.b 1
Enter_automapper_action:	rs.b 1
Goto_point_used_action:	rs.b 1
Make_camp_action:	rs.b 1
Apres_combat_action:	rs.b 1
Condition_begins_action:	rs.b 1
Condition_effect_action:	rs.b 1

;*****************************************************************************
; These are the actor types :
	rsreset
Me_actor_type:	rs.b 1
Friend_actor_type:	rs.b 1
Enemy_actor_type:	rs.b 1

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
Map_exit_event:	rs.b 1
Door_event:	rs.b 1
Chest_event:	rs.b 1
Text_event:	rs.b 1
Spinner_event:	rs.b 1
Trap_event:	rs.b 1
	rs.b 1
Datachange_event:	rs.b 1
Change_icon_event:	rs.b 1
Encounter_event:	rs.b 1
Place_action_event:	rs.b 1
Query_event:	rs.b 1
Modify_event:	rs.b 1
Action_event:	rs.b 1
Signal_event:	rs.b 1
	rs.b 1
Sound_event:	rs.b 1
Start_dialogue_event:	rs.b 1
Create_trans_event:	rs.b 1
Execute_event:	rs.b 1
Remove_member_event:	rs.b 1
End_dialogue_event:	rs.b 1
Wipe_event:	rs.b 1
Play_animation_event:	rs.b 1
	rs.b 1
Pause_event:	rs.b 1
Simple_chest_event:	rs.b 1
Ask_surrender_event:	rs.b 1
Do_script_event:	rs.b 1
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
