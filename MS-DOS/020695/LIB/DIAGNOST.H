/************
 * NAME     : DIAGNOST.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

struct Record {
	UNBYTE Event_type;
	UNBYTE Counter;
	union {
		struct {
			UNLONG X, Y;
			UNLONG Graphics_offset;
		} _2D;
		struct {
			SISHORT dX, dY;
			UNLONG Camera_angle;
			UNSHORT Camera_height;
			UNSHORT Horizon_Y;
		} _3D;
		struct {
			UNSHORT X, Y;
			UNSHORT Map_nr;
		} Map;
	} Data;
};

/*
 ** External global variables **********************************************
 */

extern BOOLEAN Recording_flic;
extern struct OPM Diag_OPM;
extern struct Textstyle Diagnostic_text_style;

/*
 ** Prototypes *************************************************************
 */

void Init_dprint(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);
void Exit_dprint(void);
void Clrhome_dprint(void);
void CR_dprint(void);
void dprintf(UNCHAR *Format, ...);

void Start_recording(UNCHAR *Filename);
void Record_2D(UNLONG X, UNLONG Y, UNLONG Graphics_offset);
void Stop_recording(void);
void Start_playback(UNCHAR *Filename);
void Playback_2D(struct Record *Record);
void Stop_playback(void);

void MEM_List(void);

BOOLEAN Check_diagnostic_keys(struct BLEV_Event_struct *Event);
void Save_screen(UNCHAR *Filename);

UNBYTE *Write_Motorola_word(UNBYTE *Address, UNSHORT Word);
UNBYTE *Write_Motorola_long(UNBYTE *Address, UNLONG Long);

void Start_flic_recording(UNBYTE *Filename);
void Stop_flic_recording(void);
void Record_flic_frame(void);

#endif

