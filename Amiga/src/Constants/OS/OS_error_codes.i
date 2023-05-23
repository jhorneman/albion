; OS contants : error codes
; Written by J.Horneman (In Tune With The Universe)
; Start : 27-1-1994


; OpenScreen error codes, which are returned in the (optional) LONG
; pointed to by ti_Data for the SA_ErrorCode tag item

OSERR_NOMONITOR	EQU 1	; Named monitor spec not available
OSERR_NOCHIPS	EQU 2	; You need newer custom chips
OSERR_NOMEM	EQU 3	; Couldn't get normal memory
OSERR_NOCHIPMEM	EQU 4	; Couldn't get chipmem
OSERR_PUBNOTUNIQUE	EQU 5	; Public screen name already used
OSERR_UNKNOWNMODE	EQU 6	; Don't recognize mode asked for
OSERR_TOODEEP	EQU 7	; Screen deeper than hardware supports
OSERR_ATTACHFAIL	EQU 8	; Failed to attach screens
OSERR_NOTAVAILABLE	EQU 9	; Mode not available for other reason
