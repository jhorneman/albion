     
  .   &    ����@��      TRUE�   @        @��      X1�   Z       @�       X2    t       @�       Y1�   �       @��      Y2�   �        @��      FALSE   �    ����@�       Cheat   �       @��      
Recta_sizeT  
       )@�       
DISK_ERROR�  0       
@��      KICKSTART_ERROR  T       @��      MODE_ID_ERROR  x       '@��      BAD_OMNIFILE   �       !@��      AREA_CORRUPTED   �       %@��      INCOMPLETE_READ  �       @�       PROCESSOR_ERROR         @��      DOS_LIBRARY_ERROR  :       @�       ASL_LIBRARY_ERROR  b       @��      GET_SPRITE_ERROR  �       @��      INTERLEAVE_ERROR�  �       &@�       INCOMPLETE_WRITE   �       "@��      NOT_ENOUGH_FILES~         @�       CREATE_PORT_ERROR  0       @��      GRAPHICS_LIBRARY_ERROR�  Z       @��      ALLOC_SPRITE_ERROR   �       @��      ALLOC_BITMAP_ERROR   �       @��      OPEN_SCREEN_ERROR  �       @�U      OPEN_DEVICE_ERROR  �       @��      NOT_ENOUGH_BLOCKS  $        @�       HANDLES_CORRUPTED  L       $@��      ILLEGAL_FILE_TYPE  t       @�      OPEN_WINDOW_ERROR  �       @��      CREATE_IOREQ_ERROR"  �       @��      NOT_ENOUGH_HANDLES�  �       #@�U      DATA_DIR_NOT_FOUNDU         (@�       SUBFILE_NOT_LOADED   F       *@��      TEXT_TOO_LONG_ERROR  t       @�      INTUITION_LIBRARY_ERROR  �       @��      ADD_INPUT_HANDLER_ERROR  �       @�       NOT_ENOUGH_CHIP_MEMORY�  �       @��      NOT_ENOUGH_FAST_MEMORY�           @�      ALLOC_SCREEN_BUFFER_ERROR  b    �6� ��      Work:DDT/Constants/Global.i�  �      ~@��      LOCAL  �  �    ute_Local\@:
	endm
r  �      �@��      Push�  �  �        	lea.l	\2,a0
	jsr	Push_\1
	endm
  &      @�       Pop    &    ut_p	jsr	Pop_\1
	endm
  �      @@�       Get  P  �    Weig	move.l	d0,-(sp)
	move.b	\1,d0
	jsr	Claim_pointer
	move.l	d0,\2
	move.l	(sp)+,d0
	endm
T        �@��      Free   �      le T	move.l	d0,-(sp)
	move.b	\1,d0
	jsr	Free_pointer
	move.l	(sp)+,d0
	endm
  ~      8@��      XGet   H  }    st P	move.b	\1,d0
	jsr	Claim_pointer
	move.l	d0,\2
	endm
b  �      �@�T      rseven�  �  �    Obje	rs.w 0
	endm
  	      �@��      XFree  �  	    t HD	move.b	\1,d0
	jsr	Free_pointer
	endm
        �8@ ���1x  ERROR_CODES.I�