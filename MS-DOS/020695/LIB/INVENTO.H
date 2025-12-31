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

#define INVENTORY_PAL_NR	(19)

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

/*
 ** Prototypes *************************************************************
 */

/* Inventory management functions */
void Go_Inventory(UNSHORT Member_nr, UNSHORT Mode_nr);
BOOLEAN Do_go_inventory(struct Event_action *Action);
void Exit_Inventory(struct Button_object *Button);

void Go_inventory_mode(UNSHORT Mode_nr);
void Switch_inventory_mode(struct Button_object *Button);

/* Inventory module functions */
void Inventory_ModInit(void);
void Inventory_ModExit(void);

void Init_Inventory_display(void);
void Exit_Inventory_display(void);

/* Left inventory methods */
void Init_InvLeft1_object(struct Object *Object, union Method_parms *P);
void Draw_InvLeft1_object(struct Object *Object, union Method_parms *P);

void Init_InvLeft2_object(struct Object *Object, union Method_parms *P);
void Draw_InvLeft2_object(struct Object *Object, union Method_parms *P);

void Init_InvLeft3_object(struct Object *Object, union Method_parms *P);
void Draw_InvLeft3_object(struct Object *Object, union Method_parms *P);

/* Right inventory methods */
void Init_InvRight_object(struct Object *Object, union Method_parms *P);
void Exit_InvRight_object(struct Object *Object, union Method_parms *P);
void Draw_InvRight_object(struct Object *Object, union Method_parms *P);

/* Damage display methods */
void Init_Damage_display_object(struct Object *Object, union Method_parms *P);
void Draw_Damage_display_object(struct Object *Object, union Method_parms *P);
void Update_Damage_display_object(struct Object *Object, union Method_parms *P);
void Highlight_Damage_display_object(struct Object *Object, union Method_parms *P);
void Help_Damage_display_object(struct Object *Object, union Method_parms *P);

/* Protection display methods */
void Init_Protection_display_object(struct Object *Object, union Method_parms *P);
void Draw_Protection_display_object(struct Object *Object, union Method_parms *P);
void Update_Protection_display_object(struct Object *Object, union Method_parms *P);
void Highlight_Protection_display_object(struct Object *Object, union Method_parms *P);
void Help_Protection_display_object(struct Object *Object, union Method_parms *P);

/* Weight display methods */
void Init_Weight_display_object(struct Object *Object, union Method_parms *P);
void Draw_Weight_display_object(struct Object *Object, union Method_parms *P);
void Update_Weight_display_object(struct Object *Object, union Method_parms *P);
void Highlight_Weight_display_object(struct Object *Object, union Method_parms *P);
void Help_Weight_display_object(struct Object *Object, union Method_parms *P);

/* Gold display methods */
void Init_Gold_display_object(struct Object *Object, union Method_parms *P);
void Draw_Gold_display_object(struct Object *Object, union Method_parms *P);
void Update_Gold_display_object(struct Object *Object, union Method_parms *P);
void Feedback_Gold_display_object(struct Object *Object, union Method_parms *P);
void Highlight_Gold_display_object(struct Object *Object, union Method_parms *P);
void Help_Gold_display_object(struct Object *Object, union Method_parms *P);
void Pop_up_Gold_display_object(struct Object *Object, union Method_parms *P);

/* Gold pop-up menu functions */
void Inventory_gold_PUM_evaluator(struct PUM *PUM);
void PUM_Drop_gold(UNLONG Data);

/* Food display methods */
void Init_Food_display_object(struct Object *Object, union Method_parms *P);
void Draw_Food_display_object(struct Object *Object, union Method_parms *P);
void Update_Food_display_object(struct Object *Object, union Method_parms *P);
void Feedback_Food_display_object(struct Object *Object, union Method_parms *P);
void Highlight_Food_display_object(struct Object *Object, union Method_parms *P);
void Help_Food_display_object(struct Object *Object, union Method_parms *P);
void Pop_up_Food_display_object(struct Object *Object, union Method_parms *P);

/* Food pop-up menu functions */
void Inventory_food_PUM_evaluator(struct PUM *PUM);
void PUM_Drop_food(UNLONG Data);

#endif

