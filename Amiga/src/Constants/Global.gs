     
  �   &    ����@�d      TRUE.   @        @�      X1    Z       @��      X2    t       @�@      Y1n   �       @�       Y2    �        @�Z      FALSE   �       @�n      
Recta_sizez   �       
@��      KICKSTART_ERROR         @�B      MODE_ID_ERROR  <       @�
      AREA_CORRUPTED
  b       @�.      PROCESSOR_ERROR  �       @�      DOS_LIBRARY_ERROR  �       @�       INTERLEAVE_ERROR  �       @�      CREATE_PORT_ERROR         @��      NOT_ENOUGH_FILES  0       @�       GRAPHICS_LIBRARY_ERROR   Z       @�      ALLOC_BITMAP_ERROR(  �       @�<      OPEN_SCREEN_ERROR  �       @�      OPEN_DEVICE_ERROR  �       @�      NOT_ENOUGH_BLOCKS  �       @�       HANDLES_CORRUPTED  "       @�      OPEN_WINDOW_ERROR  L       @�       CREATE_IOREQ_ERROR  v       @�r      NOT_ENOUGH_HANDLES   �        @�<      TEXT_TOO_LONG_ERROR  �       @�      INTUITION_LIBRARY_ERROR  �       @�      ADD_INPUT_HANDLER_ERROR  *       @�d      NOT_ENOUGH_CHIP_MEMORY   X       @�      NOT_ENOUGH_FAST_MEMORY           @��      ALLOC_SCREEN_BUFFER_ERROR  �    ا` ��      DDT:Constants/Global.i  �      �@�      LOCAL  �  �    loatLocal\@:
	endm
t  >      @�"      Push�    >    d MM	lea.l	\2,a0
	jsr	Push_\1
	endm
  z      X@�       Pop  h  z    dex 	jsr	Pop_\1
	endm
  �      �@�9      Get  �  �     siz	move.l	d0,-(sp)
	move.b	\1,d0
	jsr	Claim_pointer
	move.l	d0,\2
	move.l	(sp)+,d0
	endm
c  p      @�      Free�  (  p    floa	move.w	d0,-(sp)
	move.b	\1,d0
	jsr	Free_pointer
	move.w	(sp)+,d0
	endm
  �      �@�       rseven�  �  �    id p	rs.w 0
	endm
  �      �@��      qmulu  �  �    on I	mulu.w	\1,\2
	endm
        �� � �@�  ERROR_CODES.I_