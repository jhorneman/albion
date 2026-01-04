; Modules
; Written by J.Horneman (In Tune With The Universe)
; Start : 17-2-1994

Active_keys	macro
	dc.l $00ff0000,$00010000,Select_1	; Active character select
	dc.l $00ff0000,$00020000,Select_2
	dc.l $00ff0000,$00030000,Select_3
	dc.l $00ff0000,$00040000,Select_4
	dc.l $00ff0000,$00050000,Select_5
	dc.l $00ff0000,$00060000,Select_6
	endm
Inv_keys	macro
	dc.l $00ff0000,$00500000,GoInv_1	; Inventory select
	dc.l $00ff0000,$00510000,GoInv_2
	dc.l $00ff0000,$00520000,GoInv_3
	dc.l $00ff0000,$00530000,GoInv_4
	dc.l $00ff0000,$00540000,GoInv_5
	dc.l $00ff0000,$00550000,GoInv_6
	endm


	SECTION	Program,code
Basic_abort:
	jsr	Wait_4_unclick
	jmp	Pop_Module


	SECTION	Fast_DATA,data
Wait_Mev:
	dc.w $0101
	dc.l Basic_abort
	dc.w $1010
	dc.l Basic_abort
	dc.w -1
Wait_Kev:
	dc.l $000000ff," ",Basic_abort
Empty_Kev:
;	dc.l $0000ff00,0,Schnism_check
	dc.l 0

; ********** Default ******************************
Default_module:
	dc.b Global_mod,0
	dc.l 0,0,0
	dc.l 0,0,0,0
	dc.l 0

; ********** Waiting for user *********************
Wait_4_user_Mod:
	dc.b Local_mod,0
	dc.l -1,-1,-1
	dc.l 0,0,0,0
	dc.l -1

Map2D_Mod:

; ********** 3D map *******************************
Map3D_Mod:
	dc.b Global_mod,0
	dc.l M3_DisUpd,.VblQ,.ScrQ
	dc.l M3_ModInit,M3_ModExit,M3_DisInit,0
	dc.l 0

.VblQ:	dc.l Time_3D
	dc.l 0
.ScrQ:
;	dc.l Do_map_colour_cycling
	dc.l 0

;	dc.b Global_mod,M3Map_ID
;	dc.l M3_DisUpd
;	dc.l Map3D_VblQ,Map_ScrQ,Map3D_Mev,Map3D_Kev
;	dc.l M3_ModInit,M3_ModExit,M3_DisInit,Map_DisExit
;	dc.l 0

