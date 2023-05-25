
;***************************************************************************
; [ Do feedback ]
;   IN : a0 - Pointer to feedback string (.l)
; All registers are restored
; Notes :
;   - The feedback string will be duplicated so the original can be
;     destroyed.
;***************************************************************************
Do_feedback:
