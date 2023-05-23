; Game constants
; Written by J.Horneman (In Tune With The Universe)
; Start : 10-3-1994

	incdir	DDT:
	include	Constants/Global.i

Max_steps	EQU 288

	rsreset
North:	rs.b 1
East:	rs.b 1
South:	rs.b 1
West:	rs.b 1

	rsreset
North8:	rs.b 1
North_east:	rs.b 1
East8:	rs.b 1
South_east:	rs.b 1
South8:	rs.b 1
South_west:	rs.b 1
West8:	rs.b 1
North_west:	rs.b 1

;*****************************************************************************
; These are other game constants

	include	Game/Map.i
	include	Game/2D_map.i
	include	Game/3D_map.i
