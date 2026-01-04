#ifndef BBSYSTEM_H
#define BBSYSTEM_H


/************
 * NAME     :  BBSYSTEM.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */

#include <BBDEF.h>
#include <BBDSA.h>

/* Defines */
#define BB32_DOSAIL // Flag gibt an ob Interrupts in SYSTEM_Init Åber Ail gehÑndelt werden

/* Anzahl Ticks pro Sekunde (1/60 Sekunde) */
#define TICKS_PER_SECOND 60

#define SYSTEM_FLAG_NOMOUSEINT 1L /* SYSTEMinit mit SYSTEMTASK Maussteuerung aktivieren */
#define SYSTEM_FLAG_NOTIMERINT 2L /* SYSTEMinit mit SYSTEMTASK Timerinterrupt deaktivieren */
#define SYSTEM_FLAG_NOKEYINT 4L /* SYSTEMinit mit SYSTEMTASK Tastaturinterrupt deaktivieren */

#define SYSTEM_FLAG_MOUSESYSTEMTASK 8L /* SYSTEMinit mit SYSTEMTASK Maussteuerung aktivieren */
#define SYSTEM_FLAG_TIMERSYSTEMTASK 16L /* SYSTEMinit mit SYSTEMTASK Timerinterrupt deaktivieren */
#define SYSTEM_FLAG_KEYSYSTEMTASK 32L /* SYSTEMinit mit SYSTEMTASK Tastaturinterrupt deaktivieren */

/* Mouse pointer defines */

#define SYSTEM_MousePointerNormal	0			/* Normal mouse pointer */
#define SYSTEM_MousePointerSleep	1			/* Sleeping mouse pointer */
#define SYSTEM_MousePointerOld	2			/* Alten Mauszeiger wieder verwenden */

/* Global variables */
extern BOOLEAN	SYSTEM_QuitProgramFlag;
extern struct SCREENPORT *SYSTEM_ActiveScreenPort;		/* Pointer to active screen port */
extern UNLONG SYSTEM_Init_Flags; /* Initialisierungsflags fÅr SYSTEM_Init */

/* MausKoordinaaten */
extern SILONG SYSTEMVAR_mousex;
extern SILONG SYSTEMVAR_mousey;
extern BOOLEAN SYSTEMVAR_lbut;
extern BOOLEAN SYSTEMVAR_rbut;
/* Mouseclipping wird von DSA_openScreen gesetzt */
extern SILONG SYSTEMVAR_mouseclipx0; /* Mausclipping */
extern SILONG SYSTEMVAR_mouseclipy0;
extern SILONG SYSTEMVAR_mouseclipx1;
extern SILONG SYSTEMVAR_mouseclipy1;
/* Maus aktiv ? */
extern BOOLEAN SYSTEMVAR_ShowMouse;
extern BOOLEAN SYSTEMVAR_noMouseInterrupt;

/* Prototypes of global functions */
extern BOOLEAN SYSTEM_Init( void );
extern void SYSTEM_Exit( void );
extern void SYSTEM_SystemTask( void );
extern void SYSTEM_SystemAlert( SISHORT dialogId );
extern UNLONG SYSTEM_GetBLEVStatusLong( void );
extern void SYSTEM_MouseRestore( void );
extern void SYSTEM_MouseDisplay(void);
extern BOOLEAN SYSTEM_ShowMousePtr(struct GTO *gto);
extern void SYSTEM_HideMousePtr( void );
extern UNLONG SYSTEM_GetTicks( void );
extern void SYSTEM_WaitTicks( UNLONG waitticks );
extern void SYSTEM_SetMouseArea(struct BBRECT *Area);
extern void SYSTEM_SetMousePtr(SISHORT X, SISHORT Y);
//extern int lock_region (void *address, unsigned length);

extern void SYSTEM_Timer_function(void);

#endif
