/************
 * NAME     : CONTROL.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-8-1994
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

#define ROOTS_MAX					(4)
#define UI_OBJECTS_MAX 			(150)
#define UI_OBJECT_POOL_SIZE	(UI_OBJECTS_MAX * 50)

#define UPDATE_OPMS_MAX		(8)

#define MODULES_MAX			(8)
#define MOUSE_POINTERS_MAX	(8)
#define MA_MAX					(8)

/* Module types */
#define SCREEN_MOD			(0)
#define WINDOW_MOD			(1)
#define MODE_MOD				(2)

/* Screens */
#define NO_SCREEN				(0)
#define MAP_2D_SCREEN		(1)
#define MAP_3D_SCREEN		(2)
#define COMBAT_SCREEN		(3)
#define INVENTORY_SCREEN	(4)
#define AUTOMAP_SCREEN		(5)
#define DOOR_SCREEN			(5)
#define CHEST_SCREEN			(6)
#define DIALOGUE_SCREEN		(7)

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
#define DROP_METHOD			(24)
#define RESTORE_METHOD		(25)
#define INQUIRE_DROP_METHOD	(26)

/* Error codes */
#define ERROR_OBJECT_TOO_SMALL		(1)
#define ERROR_TOO_MANY_OBJECTS		(2)
#define ERROR_OBJECT_POOL_IS_FULL	(3)
#define ERROR_OBJECT_HANDLE_ZERO		(4)
#define ERROR_OBJECT_DOESNT_EXIST	(5)
#define ERROR_ILLEGAL_OBJECT_HANDLE	(6)

/* Drag & drop data ID's */
#define BODY_ITEM_DD_DATA_ID			(1)
#define BACKPACK_ITEM_DD_DATA_ID		(2)
#define CHEST_ITEM_DD_DATA_ID			(3)
#define GOLD_DD_DATA_ID					(4)
#define FOOD_DD_DATA_ID					(5)

/*
 ** Type definitions **************************************************
 */

typedef UNLONG (*Method_function) (struct Object *, union Method_parms *);
typedef void (*Drag_drop_abort_handler) (struct Drag_drop_data *Drag_drop_data_ptr);

/*
 ** Structure definitions **************************************************
 */

/* Modules */
struct Module {
	UNSHORT Flags;
	UNSHORT Type;
	UNSHORT Screen_type;
	void (*MainLoop_function)(void);
	void (*ModInit_function)(void);
	void (*ModExit_function)(void);
	void (*DisInit_function)(void);
	void (*DisExit_function)(void);
	void (*DisUpd_function)(void);
};

/* Bitmasks for Module.Flags */
#define LOCAL_MOD				(1 << 0)
#define WAS_GLOBAL_MOD		(1 << 1)
#define NO_INPUT_HANDLING	(1 << 2)

/* Roots / trees */
struct Root {
	UNSHORT First_object;
	struct OPM *OPM;
};

/* Methods */
struct Method {
	UNSHORT ID;
	Method_function Function;
};

/* Object classes */
struct Object_class {
	UNSHORT Flags;
	UNSHORT Size;
	struct Method *Method_list;
};

/* User-interface objects */
struct Object {
	struct BBRECT Rect;		/* Absolute interaction rectangle */
	SISHORT X, Y;				/* Display coordinates (relative to OPM) */
	UNSHORT Self;				/* This object's handle */
	UNSHORT Next;				/* Brother's handle */
	UNSHORT Parent;			/* Parent's handle */
	UNSHORT Child;				/* First child's handle */
	UNSHORT Flags;				/* Flags */
	struct Object_class *Class;	/* Pointer to class description */
};

/* Bitmasks for Object_class.Flags and Object.Flags */

/* Used by [ Wait_4_object ]. Do NOT set in object class ! */
#define OBJECT_WARN_WHEN_DELETED		(1 << 0)

/* Object is used for grouping other objects and has no rectangle of
   it's own. */
#define OBJECT_CONTROL					(1 << 1)

/* Interaction with this object is disabled. */
#define OBJECT_DISABLED					(1 << 3)

/* This object forms a new display base : it's top-left coordinates become
   the display origin for all child objects. */
#define OBJECT_DISPLAY_BASE			(1 << 4)

/* Mouse pointer */
struct Mouse_pointer {
	SISHORT X, Y;					/* Hot-spot coordinates */
	UNSHORT Width, Height;
	UNBYTE *Graphics;
};

/* Drag & drop data */
struct Drag_drop_data {
	UNSHORT Data_ID;
	Drag_drop_abort_handler Abort_handler;
	struct Object *Source_object;
	UNBYTE *Data;
};

/* Method parameters */
union Method_parms {
	struct BBRECT *Rect;					/* Restore method */
	UNLONG Direction;						/* Move method */
	UNSHORT Button_state;				/* Custommouse method */
	struct BLEV_Event_struct *Event;	/* Customkey method */
	UNLONG Value;							/* Set method */
	UNCHAR *String;
	struct Drag_drop_data *Drag_drop_data_ptr;	/* Drop / inquire drop method */
};

/*
 ** External global variables **********************************************
 */

extern struct OPM *Current_OPM;

extern UNLONG Update_duration;

extern BOOLEAN Drag_drop_mode;
extern struct Drag_drop_data Current_drag_drop_data;

extern BOOLEAN Palette_has_changed;

extern UNSHORT Mouse_X;
extern UNSHORT Mouse_Y;
extern UNSHORT Button_state;

extern UNSHORT Feedback_object;
extern UNSHORT Highlighted_object;
extern UNSHORT Focussed_object;

extern struct BBRECT Default_MA;
extern struct Module Default_module;

/*
 ** Prototypes *************************************************************
 */

/* Drag & drop mode functions */
void Enter_drag_drop_mode(UNSHORT Data_ID, Drag_drop_abort_handler Handler,
 struct Object *Source_object, UNBYTE *Data);
void Leave_drag_drop_mode(void);
void Abort_drag_drop_mode(void);

/* Central get-event function */
void Get_event(struct BLEV_Event_struct *Event);

/* Wait for user functions */
void Wait_4_user(void);
void Wait_4_click(void);
void Wait_4_unclick(void);
void Wait_4_right_unclick(void);

/* Input handling functions */
void Update_input(void);
void Handle_input(void);
void Clear_input_buffer(void);

/* Mouse input handling support functions */
void Handle_mouse_event(struct BLEV_Event_struct *Event);
BOOLEAN Search_mouse_event_object(UNSHORT Method, UNSHORT Handle);
BOOLEAN Search_mouse_method(UNSHORT Method, struct Object *Object);
BOOLEAN Predict_method_capability(UNSHORT Method, UNSHORT Handle);

/* Key input handling support functions */
void Handle_key_event(struct BLEV_Event_struct *Event);

/* Object management functions */
UNSHORT Add_object(UNSHORT Parent_handle, struct Object_class *Class,
 UNBYTE *OID, SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);
struct Object *Search_object(UNSHORT Handle, UNSHORT Target);
void Delete_object(UNSHORT Handle);

/* Object pool management functions */
struct Object *Add_object_to_pool(struct Object_class *Class);
void Remove_object_from_pool(UNSHORT Handle);

/* Object method functions */
UNLONG Execute_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P);
void Execute_child_methods(UNSHORT Handle, UNSHORT Method, union Method_parms *P);
void Execute_broadcast_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P);
void Do_execute_broadcast_method(UNSHORT Handle, UNSHORT Method, union Method_parms *P);

/* Object rectangle access functions */
void Change_object_size(UNSHORT Handle, UNSHORT Width, UNSHORT Height);
void Change_object_position(UNSHORT Handle, SISHORT X, SISHORT Y);

/* Additional object functions */
struct Object *Get_object_data(UNSHORT Handle);
BOOLEAN Is_object_present(UNSHORT Handle);

void Wait_4_object(UNSHORT Handle);

BOOLEAN Has_method(UNSHORT Handle, UNSHORT Method);

BOOLEAN Is_over_object(UNSHORT Object);

UNLONG Delete_self(struct Object *Object, union Method_parms *P);

UNSHORT Object_at(SISHORT mX, SISHORT mY);

UNSHORT Find_object_at(SISHORT mX, SISHORT mY, UNSHORT Handle);

//void Set_appropriate_mouse(void);

void Disable_object(UNSHORT Handle);
void Enable_object(UNSHORT Handle);

/* Object tree management functions */
void Push_root(struct OPM *OPM);
void Pop_root(void);
void Reset_root_stack(void);

/* Object tree support functions */
void Set_root_OPM(struct OPM *OPM);
UNSHORT Get_root_object_handle(void);

/* Module stack management functions */
void Push_module(struct Module *Module);
void Pop_module(void);
void Reset_module_stack(void);

/* Module stack support functions */
void Init_module(struct Module *Module);
void Exit_module(struct Module *Module);

/* Module element execution functions */
void Main_loop(void);
void Init_display(void);
void Exit_display(void);
void Update_display(void);

/* Module data access functions */
UNSHORT Current_screen_type(UNSHORT Levels);

/* Mouse pointer stack management functions */
void Push_mouse(struct Mouse_pointer *New);
void Pop_mouse(void);
void Reset_mouse_stack(void);
void Change_mouse(struct Mouse_pointer *New);

/* Mouse pointer support functions */
void Init_mouse_pointer(struct Mouse_pointer *New);
UNBYTE *Write_chunk_name(UNBYTE *Address, UNCHAR Chunk_name[]);

/* Mouse pointer display functions */
void Mouse_on(void);
void Mouse_off(void);

/* Mouse area stack management functions */
void Push_MA(struct BBRECT *New);
void Pop_MA(void);
void Reset_MA_stack(void);

/* Mouse area support functions */
void Init_MA(struct BBRECT *New);

/* Screen update functions */
void Update_screen(void);
void Switch_screens(void);

void Add_update_OPM(struct OPM *OPM);
void Remove_update_OPM(struct OPM *OPM);

void Show_OPMs(void);
void Show_changed_OPMs(void);

//void Update_all_OPMs(void);

/* Error report function */
void Control_error(UNSHORT Error_code);

#endif

