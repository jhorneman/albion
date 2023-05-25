;*****************************************************************************
; These are the IDCMP classes

IDCMP_SIZEVERIFY	EQU $00000001
IDCMP_NEWSIZE	EQU $00000002
IDCMP_REFRESHWINDOW	EQU $00000004
IDCMP_MOUSEBUTTONS	EQU $00000008
IDCMP_MOUSEMOVE	EQU $00000010
IDCMP_GADGETDOWN	EQU $00000020
IDCMP_GADGETUP	EQU $00000040
IDCMP_REQSET	EQU $00000080
IDCMP_MENUPICK	EQU $00000100
IDCMP_CLOSEWINDOW	EQU $00000200
IDCMP_RAWKEY	EQU $00000400
IDCMP_REQVERIFY	EQU $00000800
IDCMP_REQCLEAR	EQU $00001000
IDCMP_MENUVERIFY	EQU $00002000
IDCMP_NEWPREFS	EQU $00004000
IDCMP_DISKINSERTED	EQU $00008000
IDCMP_DISKREMOVED	EQU $00010000
IDCMP_WBENCHMESSAGE	EQU $00020000	; System use only
IDCMP_ACTIVEWINDOW	EQU $00040000
IDCMP_INACTIVEWINDOW	EQU $00080000
IDCMP_DELTAMOVE	EQU $00100000
IDCMP_VANILLAKEY	EQU $00200000
IDCMP_INTUITICKS	EQU $00400000
;  for notifications from "boopsi" gadgets:
IDCMP_IDCMPUPDATE	EQU $00800000	; new for V36
; for getting help key report during menu session:
IDCMP_MENUHELP		EQU $01000000	; new for V36
; for notification of any move/size/zoom/change window:
IDCMP_CHANGEWINDOW	EQU $02000000	; new for V36
IDCMP_GADGETHELP	EQU $04000000	; new for V39

; NOTEZ-BIEN: $80000000 is reserved for internal use by IDCMP

; the IDCMP Flags do not use this special bit, which is cleared when
; Intuition sends its special message to the Task, and set when Intuition
; gets its Message back from the Task.	Therefore, I can check here to
; find out fast whether or not this Message is available for me to send
IDCMP_LONELYMESSAGE	EQU $80000000

; --- IDCMP Codes --------------------------------------------------------
; This group of codes is for the IDCMP_CHANGEWINDOW message
CWCODE_MOVESIZE	EQU $0000	; Window was moved and/or sized
CWCODE_DEPTH	EQU $0001	; Window was depth-arranged (new for V39)

; This group of codes is for the IDCMP_MENUVERIFY message
MENUHOT	EQU $0001	; IntuiWants verification or MENUCANCEL
MENUCANCEL	EQU $0002 	; HOT Reply of this cancels Menu operation
MENUWAITING	EQU $0003	; Intuition simply wants a ReplyMsg() ASAP

; These are internal tokens to represent state of verification attempts
; shown here as a clue.
OKOK	EQU MENUHOT		; guy didn't care
OKABORT	EQU $0004		; window rendered question moot
OKCANCEL 	EQU MENUCANCEL 		; window sent cancel reply

; This group of codes is for the IDCMP_WBENCHMESSAGE messages
WBENCHOPEN	EQU $0001
WBENCHCLOSE	EQU $0002
