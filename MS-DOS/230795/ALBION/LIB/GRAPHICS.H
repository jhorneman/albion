/************
 * NAME     : GRAPHICS.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 1-9-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <BBDEF.H>
#include <BBOPM.H>

/*
 ** Defines ****************************************************************
 */

#define DEFAULT_MPTR			(0)
#define FORWARD_MPTR			(1)
#define BACKWARD_MPTR		(2)
#define LEFT3D_MPTR			(3)
#define RIGHT3D_MPTR			(4)
#define TURNLEFT_MPTR		(5)
#define TURNRIGHT_MPTR		(6)
#define TURN180LEFT_MPTR	(7)
#define TURN180RIGHT_MPTR	(8)
#define UP2D_MPTR				(9)
#define DOWN2D_MPTR			(10)
#define LEFT2D_MPTR			(11)
#define RIGHT2D_MPTR			(12)
#define UPLEFT_MPTR			(13)
#define UPRIGHT_MPTR			(14)
#define DOWNRIGHT_MPTR		(15)
#define DOWNLEFT_MPTR		(16)
#define POP_UP_MPTR			(17)
#define LOADING_MPTR			(18)
#define WAIT_MPTR				(19)
#define CLICK_MPTR			(20)
#define PICK_MPTR				(21)
#define ACTION_3D_MPTR		(22)
#define NO_ACTION_3D_MPTR	(23)
#define RAM_MPTR				(24)

/*
 ** Global variables *******************************************************
 */

extern struct Mouse_pointer Mouse_pointers[];

extern UNBYTE Pop_up_bg[2][1024];
extern UNBYTE Window_vertical[4][48];
extern UNBYTE Window_horizontal[4][48];
extern UNBYTE Window_TL[];
extern UNBYTE Window_TR[];
extern UNBYTE Window_BL[];
extern UNBYTE Window_BR[];

extern UNBYTE Exit_button[];

extern UNBYTE Damage_symbol[];
extern UNBYTE Protection_symbol[];

extern UNBYTE Gold_symbol[];
extern UNBYTE Food_symbol[];

extern UNBYTE Cross_symbol[];
extern UNBYTE Broken_symbol[];

extern UNBYTE Full_spell_strength_bar[];
extern UNBYTE Empty_spell_strength_bar[];

extern UNBYTE Combat_action_icons[6][256];

extern UNBYTE Eye_symbols[];

extern UNBYTE Clock_symbol[];
extern UNBYTE Time_font[];

extern UNBYTE Compass_symbol[];
extern UNBYTE Compass_bit[];

extern UNBYTE Select_2D_cursor[];

extern UNBYTE Lock_symbol[];

#endif

