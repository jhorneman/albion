#ifndef BBEVENT_H
#define BBEVENT_H

#include <bbdef.h>

/* Die folgenden Konstanten definieren Eventtypen */
#define BLEV_NOEVENT	0L
#define BLEV_KEYDOWN	1L
#define BLEV_KEYUP		2L
#define BLEV_MOUSELDOWN	4L
#define BLEV_MOUSELUP	8L
#define BLEV_MOUSELDBL	16L
#define BLEV_MOUSERDOWN	32L
#define BLEV_MOUSERUP	64L
#define BLEV_MOUSERDBL	128L
#define BLEV_MOUSELSDOWN	256L
#define BLEV_MOUSELSUP	512L
#define BLEV_MOUSERSDOWN	1024L
#define BLEV_MOUSERSUP	2048L
#define BLEV_MOUSE_EVENT	(BLEV_MOUSELSDOWN|BLEV_MOUSELSUP|BLEV_MOUSERSDOWN|BLEV_MOUSERSUP| BLEV_MOUSELDOWN | BLEV_MOUSELUP | BLEV_MOUSELDBL | BLEV_MOUSERDOWN | BLEV_MOUSERUP | BLEV_MOUSERDBL)

/* Die folgenden Konstanten beschreiben den Maustasten und Keyboardstatus */
#define BLEV_NOKEY			0L
#define BLEV_MOUSELPRESSED	1L
#define BLEV_MOUSERPRESSED	2L
#define BLEV_ASCII			4L
#define BLEV_SHIFT			8L
#define BLEV_CTRL			16L
#define BLEV_ALT			32L
#define BLEV_KEYPRESSED		(BLEV_NOKEY | BLEV_MOUSELPRESSED | BLEV_MOUSERPRESSED | BLEV_ASCII | BLEV_SHIFT | BLEV_CTRL | BLEV_ALT)

/* Die folgenden Konstanten definieren Sondertasten */
#define BLEV_ESC			256

#define BLEV_F1				257
#define BLEV_F2				258
#define BLEV_F3				259
#define BLEV_F4				260
#define BLEV_F5				261
#define BLEV_F6				262
#define BLEV_F7				263
#define BLEV_F8				264
#define BLEV_F9				265
#define BLEV_F10			266
#define BLEV_F11			267
#define BLEV_F12			268

#define BLEV_TAB			269
#define BLEV_INS			270
#define BLEV_DEL			271
#define BLEV_BS				272
#define BLEV_HOME			273
#define BLEV_END			274
#define BLEV_PGUP			275
#define BLEV_PGDN			276
#define BLEV_LEFT			277
#define BLEV_UP				278
#define BLEV_RIGHT			279
#define BLEV_DOWN			280
#define BLEV_RETURN			281


/* Der folgende Typ dient zur Ein- und Ausgabe von Events */
typedef struct BLEV_Event_struct{
	SILONG				sl_eventtype;
	struct	SCREENPORT	*p_screenport;
	UNLONG				ul_pressed_keys;
	SILONG				sl_key_code;
	SILONG				sl_mouse_x;
	SILONG				sl_mouse_y;
} BLEV_Event_struct;

/* Funktionsprototypen */
SILONG	BLEV_Init( void );
void		BLEV_Exit( void );
SILONG	BLEV_GetEvent( struct BLEV_Event_struct	*p_event );
SILONG	BLEV_IsEventWaiting( void );
void		BLEV_ClearAllEvents( void );
SILONG	BLEV_PutEvent( struct BLEV_Event_struct		*p_event );
void 		BLEV_GetMousePos(struct BBPOINT *pnt);

#endif
