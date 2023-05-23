; Program error codes
; Written by J.Horneman (In Tune With The Universe)
; Start : 27-1-1994

; Notes :
;   - [ Fatal_error ] will make these negative.

	rsreset
	rs.b 10
KICKSTART_ERROR:	rs.b 1		; I don't like this Amiga
PROCESSOR_ERROR:	rs.b 1

DOS_LIBRARY_ERROR:	rs.b 1		; Hey, where are your libraries ?
GRAPHICS_LIBRARY_ERROR:	rs.b 1
INTUITION_LIBRARY_ERROR:	rs.b 1

MODE_ID_ERROR:	rs.b 1		; Your screen doesn't like me
ALLOC_BITMAP_ERROR:	rs.b 1
INTERLEAVE_ERROR:	rs.b 1
OPEN_SCREEN_ERROR:	rs.b 1
OPEN_WINDOW_ERROR:	rs.b 1
ALLOC_SCREEN_BUFFER_ERROR:	rs.b 1

CREATE_PORT_ERROR:	rs.b 1		; The OS says "Boo-hoo"
CREATE_IOREQ_ERROR:	rs.b 1
OPEN_DEVICE_ERROR:	rs.b 1
ADD_INPUT_HANDLER_ERROR:	rs.b 1

NOT_ENOUGH_CHIP_MEMORY:	rs.b 1	; Memory manager blues
NOT_ENOUGH_FAST_MEMORY:	rs.b 1
NOT_ENOUGH_BLOCKS:	rs.b 1
NOT_ENOUGH_HANDLES:	rs.b 1
HANDLES_CORRUPTED:	rs.b 1
AREA_CORRUPTED:	rs.b 1
NOT_ENOUGH_FILES:	rs.b 1

TEXT_TOO_LONG_ERROR:	rs.b 1	; "War and peace" or what ?
