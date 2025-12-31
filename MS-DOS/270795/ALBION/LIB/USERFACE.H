/************
 * NAME     : USERFACE.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 13-9-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef USERFACE_H
#define USERFACE_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>

/*
 ** Defines ****************************************************************
 */

#define Draw_high_box Draw_high_border

/*
 ** Structure definitions **************************************************
 */

/* Window object */
struct Window_object {
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;
};

/*
 ** External global variables **********************************************
 */

extern struct Object_class Earth_Class;
extern struct Object_class Group_Class;

extern struct Module Window_Mod;

/*
 ** Prototypes *************************************************************
 */

/* Restore functions */
void Restore_background(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);

/* Standard object methods */
UNLONG Draw_children(struct Object *Object, union Method_parms *P);
UNLONG Restore_slab(struct Object *Object, union Method_parms *P);
UNLONG Restore_window(struct Object *Object, union Method_parms *P);

/* Window module functions */
void Window_ModInit(void);
void Window_ModExit(void);

/* Generic window object methods */
UNLONG Exit_Window_object(struct Object *Object, union Method_parms *P);
UNLONG Close_Window_object(struct Object *Object, union Method_parms *P);

/* Window support functions */
void Prepare_window(struct Window_object *Window);
void Destroy_window(struct Window_object *Window);

/* Window display functions */
void Draw_window_border(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height);
void Draw_window_inside(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height);

/* User-interface display support functions */
void Draw_high_border(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height);
void Draw_deep_border(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height);
void Draw_deep_box(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height);
void Draw_light_box(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height);

/* Normal user-interface object methods */
UNLONG Normal_touched(struct Object *Object, union Method_parms *P);
UNLONG Dehighlight(struct Object *Object, union Method_parms *P);
BOOLEAN Normal_clicked(struct Object *Object);
UNLONG Normal_rightclicked(struct Object *Object, union Method_parms *P);

/* Wipe functions */
void Wipe_out(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);

#endif

