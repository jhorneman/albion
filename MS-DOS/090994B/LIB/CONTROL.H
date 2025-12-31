/************
 * NAME     : CONTROL.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 04.08.94 10:51
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef CONTROL_H
#define CONTROL_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBDSA.H>
#include <BBEVENT.H>

/*
 ** Defines ****************************************************************
 */

#define ROOTS_MAX				(4)
#define OBJECTS_MAX			(100)
#define OBJECT_POOL_SIZE	(OBJECTS_MAX * 50)

#define UPDATE_OPMS_MAX		(16)

#define MODULES_MAX			(8)
#define MOUSE_POINTERS_MAX	(8)

/* Module types */
#define SCREEN_MOD			(0)
#define WINDOW_MOD			(1)
#define MODE_MOD				(2)

/* Method ID's */
#define INIT_METHOD			(1)
#define EXIT_METHOD			(2)
#define CLOSE_METHOD			(3)
#define TOUCHED_METHOD		(4)
#define LEFT_METHOD			(5)
#define RIGHT_METHOD			(6)
#define DLEFT_METHOD			(7)
#define DRIGHT_METHOD		(8)
#define CUSTOMMOUSE_METHOD	(9)
#define CUSTOMKEY_METHOD	(10)
#define HIGHLIGHT_METHOD	(11)
#define HELP_METHOD			(12)
#define FEEDBACK_METHOD		(13)
#define POP_UP_METHOD		(14)
#define FOCUS_METHOD			(15)
#define MOVE_METHOD			(16)
#define CYCLE_METHOD			(17)
#define DRAW_METHOD			(18)
#define UPDATE_METHOD		(19)
#define ERASE_METHOD			(20)
#define GET_METHOD			(21)
#define SET_METHOD			(22)
#define PRINT_METHOD			(23)

/*
 ** Structure definitions **************************************************
 */

/* Modules */
struct Module {
	UNBYTE Flags;
	UNBYTE Type;
	void (*MainLoop_function)(void);
	void (*ModInit_function)(void);
	void (*ModExit_function)(void);
	void (*DisInit_function)(void);
	void (*DisExit_function)(void);
	void (*DisUpd_function)(void);
};

/* Bitmasks for Module.Flags */
#define LOCAL_MOD			(1<<0)
#define WAS_GLOBAL_MOD	(1<<1)

/* Roots / trees */
struct Root {
	UNSHORT First_object;
	UNSHORT Flags;
	struct OPM *OPM;
};

/* Method parameters */
union Method_parms {
	struct BBRECT Rect;
	UNLONG Direction;						/* Move method */
	UNSHORT Button_state;				/* Custommouse method */
	struct BLEV_Event_struct *Event;	/* Customkey method */
};

/* Methods */
struct Method {
	UNSHORT ID;
	UNLONG (*Function)(struct Object *, union Method_parms *);
};

/* Object classes */
struct Object_class {
	UNSHORT Size;
	struct Method *Method_list;
};

/* User-interface objects */
struct Object {
	struct BBRECT Rect;
	UNSHORT Self, Next, Parent, Child;
	UNSHORT Flags;
	struct Object_class *Class;
};

/* Bitmasks for Object.Flags */
#define OBJECT_WARN_WHEN_DELETED		(1<<0) // Used by [ Wait_4_object ].
#define OBJECT_CONTROL					(1<<1)
// Object is used for grouping other objects and has no rectangle of
//  it's own.
#define OBJECT_NO_CONTAINER			(1<<2)
// Object's children lie outside it's own rectangle.
#define OBJECT_DISABLE					(1<<3)
// Interaction with this object is disabled.

/* Mouse pointer */
struct Mouse_pointer {
	SISHORT X, Y;					/* Hot-spot coordinates */
	UNSHORT Width, Height;
	UNBYTE *Graphics;
};

/*
 ** External global variables **********************************************
 */

/* The module stack */
extern struct Module Module_stack[MODULES_MAX];
extern UNSHORT Module_stack_index;

extern UNLONG Update_duration;

extern UNSHORT Mouse_X;
extern UNSHORT Mouse_Y;
extern UNSHORT Button_state;

/*
 ** Prototypes *************************************************************
 */

void Update_input(void);
void Handle_input(void);
void Clear_input_buffer(void);
void Update_button_state(struct BLEV_Event_struct *Event);
void Handle_mouse_event(struct BLEV_Event_struct *Event);
BOOLEAN Search_mouse_event_object(UNSHORT Method, UNSHORT Handle);
BOOLEAN Search_mouse_method(UNSHORT Method, struct Object *Object);
BOOLEAN Check_Mev_list(struct Mev *Mev_list);

void Handle_key_event(struct BLEV_Event_struct *Event);
BOOLEAN Check_Kev_list(struct BLEV_Event_struct *Event, struct Kev *Kev_list);

UNSHORT Add_object(UNSHORT Parent_handle, struct Object_class *Class,
 UNBYTE *OID);
struct Object *Search_object(UNSHORT Handle, UNSHORT Target);
void Delete_object(UNSHORT Handle);
struct Object *Add_object_to_pool(struct Object_class *Class);
void Remove_object_from_pool(UNSHORT Handle);
UNLONG Execute_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P);
void Execute_child_methods(UNSHORT Handle, UNSHORT Method, union Method_parms *P);
void Execute_broadcast_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P);
void Do_execute_broadcast_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P);
void Wait_4_object(UNSHORT Handle);
BOOLEAN Has_method(UNSHORT Handle, UNSHORT Method);

void Push_root(struct OPM *OPM);
void Pop_root(void);
void Reset_root_stack(void);

void Push_module(struct Module *Module);
void Init_module(struct Module *Module);
void Pop_module(void);
void Exit_module(struct Module *Module);
void Reset_module_stack(void);

void Main_loop(void);
void Init_display(void);
void Exit_display(void);
void Update_display(void);

void Reset_mouse_stack(void);
void Push_mouse(struct Mouse_pointer *New);
void Pop_mouse(void);
void Change_mouse(struct Mouse_pointer *New);
void Init_mouse_pointer(struct Mouse_pointer *New);
void Write_chunk_name(UNBYTE *Address, UNCHAR Chunk_name[]);

void Switch_screens(void);
void Add_update_OPM(struct OPM *OPM);
void Remove_update_OPM(struct OPM *OPM);
void Show_OPMs(void);

#endif

