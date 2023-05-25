; User-interface Modules
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
