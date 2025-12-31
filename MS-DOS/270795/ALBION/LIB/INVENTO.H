/************
 * NAME     : INVENTO.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 21-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef INVENTORY_H
#define INVENTORY_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

#define INVENTORY_MIDDLE	(135)

#define BODY_AREA_WIDTH		(128)
#define BODY_AREA_HEIGHT	(168)

/* Left inventory types */
#define INVLEFT_STATUS_1	(1)
#define INVLEFT_STATUS_2	(2)
#define INVLEFT_STATUS_3	(3)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern UNSHORT Inventory_member;
extern UNSHORT Inventory_mode;
extern MEM_HANDLE Inventory_char_handle;

extern MEM_HANDLE Full_body_pic_handle;

extern BOOLEAN In_Inventory;
extern BOOLEAN Left_Inventory;

extern UNSHORT InvLeft_object;
extern UNSHORT InvRight_object;

extern UNSHORT Body_item_list_object;
extern UNSHORT Backpack_item_list_object;

extern struct Object_class InvRight_Class;

extern struct Button_OID Exit_inventory_button_OID;

/*
 ** Prototypes *************************************************************
 */

/* Inventory management functions */
void Go_Inventory(UNSHORT Member_nr, UNSHORT Mode_nr);
BOOLEAN Do_go_inventory(struct Event_action *Action);
void Exit_Inventory(struct Button_object *Button);

void Go_inventory_mode(UNSHORT Mode_nr);
void Switch_inventory_mode(struct Button_object *Button);

void Inventory_ModExit(void);

/* Inventory support functions */
BOOLEAN Init_inventory_data(void);
void Exit_inventory_data(void);

#endif

