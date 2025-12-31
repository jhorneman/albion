#ifndef BB_INL_ASS_H
#define BB_INL_ASS_H

#include <bbdef.h>

/*********************************************************/
/* Assemblerroutinen fÅr Interruptroutinen eigener Stack */
/*********************************************************/
void beginn_int(UNBYTE *save,UNBYTE *stackptr);
#pragma aux beginn_int = \
 "mov	[ECX],ESP"    \
 "mov	ESP,EDX"			\
 "mov dx,ss"   			\
 "mov 4[ECX],dx"   	\
 "mov	dx,ds" 				\
 "mov	ss,dx" 				\
 "mov	es,dx" 				\
 parm [ECX] [EDX] 	\
 modify [];

void end_int(UNBYTE *save);
#pragma aux end_int = \
 "mov ESP,[ECX]"    \
 "mov dx,4[ECX]"   	\
 "mov ss,dx"   			\
 parm [ECX]					\
 modify [EDX];

/* KopierFunktion setzt und rettet das ES Register fÅr InterruptFunktiontn */
void intmemcopy(UNBYTE *from,UNBYTE *to,SILONG anz);
#pragma aux intmemcopy = \
 "mov ax,ds"   	\
 "mov es,ax"   	\
 "mov	eax,ecx"  \
 "shr	ecx,2"    \
 "rep movsd"    \
 "mov	cl,al"    \
 "and cl,3"     \
 "rep movsb"    \
 parm [ESI] [EDI] [ECX] \
 modify [EAX];

/* KopierFunktion setzt und rettet das ES Register fÅr InterruptFunktiontn */
void copydstoes( void );
#pragma aux copydstoes = \
 "mov ax,ds"   	\
 "mov es,ax"   	\
 parm [] \
 modify [AX];

#endif
