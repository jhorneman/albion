/************
 * NAME     : SPECITEM.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef SPECITEM_H
#define SPECITEM_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** External global variables **********************************************
 */

extern MEM_HANDLE Compass_OPM_handle;
extern MEM_HANDLE Clock_OPM_handle;

/*
 ** Prototypes *************************************************************
 */

void Init_special_items(void);
void Exit_special_items(void);

void Show_special_items(void);
void Hide_special_items(void);

void Prepare_2D_special_items_display(void);
void Display_special_items_2D(void);

void Display_special_items_3D(void);

#endif

