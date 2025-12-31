/************
 * NAME     : SCROLL.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 25-7-1994
 * PROJECT  : Scrolling playfield system
 * NOTES    :
 * SEE ALSO : SCROLL.C
 ************/

#ifndef SCROLL_H
#define SCROLL_H

/*
 ** Macro definitions ******************************************************
 */

#define sgn(A) (((A)==0) ? (0) : (((A)>0) ? (1) : (-1)))

/*
 ** Structure definitions **************************************************
 */

struct Scroll_data {
	UNSHORT Flags;

	/* The unit update function */
	void (*Update_unit)(struct Scroll_data *, UNSHORT, UNSHORT, UNSHORT,
	 UNSHORT);

	/* Dimensions of one scroll unit (= tile) */
	UNSHORT Unit_width, Unit_height;

	/* Dimensions of viewport (= visible window) */
	UNSHORT Viewport_width, Viewport_height;

	/* Dimensions of scroll buffer (= base OPM) in units */
	UNSHORT sbWidth, sbHeight;

	/* Dimensions of entire playfield */
	UNLONG Playfield_width, Playfield_height;

	/* Coordinates within a unit */
	UNSHORT Unit_X, Unit_Y;

	/* Coordinates within the viewport (actually within base OPM) */
	UNSHORT Viewport_X, Viewport_Y;

	/* Coordinates within playfield */
	SILONG Playfield_X, Playfield_Y;

	/* Most recent scroll vector */
	SISHORT Vector_X, Vector_Y;

	/* The base OPM (where the graphics are) */
	MEM_HANDLE Base_OPM_handle;
	struct OPM Base_OPM;
};

/* Bitmasks for Scroll_data.Flags */

/*
 ** Global variables *******************************************************
 */

/*
 ** Prototypes *************************************************************
 */

void Init_scroll(struct Scroll_data *Scroll, UNLONG X, UNLONG Y);
void Exit_scroll(struct Scroll_data *Scroll);

void Set_scroll_position(struct Scroll_data *Scroll, UNLONG X, UNLONG Y);
void Fill_scroll_buffer(struct Scroll_data *Scroll);

void Do_scroll(struct Scroll_data *Scroll, SISHORT dX, SISHORT dY);
void Update_scroll_rectangle(struct Scroll_data *Scroll, UNSHORT X, UNSHORT Y,
 UNSHORT Width, UNSHORT Height);

void Scroll_display(struct Scroll_data *Scroll, struct SCREENPORT *Screen,
 SISHORT X, SISHORT Y);
void Do_scroll_display(struct Scroll_data *Scroll, struct SCREENPORT *Screen,
 SISHORT Screen_X, SISHORT Screen_Y, UNSHORT Buffer_X, UNSHORT Buffer_Y,
 UNSHORT Width, UNSHORT Height);

void Scroll_display_OPM(struct Scroll_data *Scroll, struct OPM *OPM,
 SISHORT X, SISHORT Y);
void Do_scroll_display_OPM(struct Scroll_data *Scroll, struct OPM *OPM,
 SISHORT Screen_X, SISHORT Screen_Y, UNSHORT Buffer_X, UNSHORT Buffer_Y,
 UNSHORT Width, UNSHORT Height);

void Convert_scroll_coordinates(struct Scroll_data *Scroll, SILONG *X_coord,
 SILONG *Y_coord);

#endif

