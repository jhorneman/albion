//------------------------------------------------------------------------------
//Event.c - Eventhandling-funktionen von Jurie; ich hab nur die Kommentare
//          geÑndert.
//------------------------------------------------------------------------------
#include "setup.h"

#define BLEV_MOUSELSDOWN	256L
#define BLEV_MOUSELSUP		512L
#define BLEV_MOUSERSDOWN	1024L
#define BLEV_MOUSERSUP		2048L

//Tja, was bedeutete jetzt static noch mal fÅr globale Variablen, eh ?
static SILONG EventGetchBufferSize=0;
static SILONG EventGetchBuffer[100];

//------------------------------------------------------------------------------
// NAME      : Get_event
// FUNCTION  : Get an input event and handle input recording / playback.
// FILE      : CONTROL.C
// AUTHOR    : Jurie Horneman
// FIRST     : 19.09.94 12:24
// LAST      : 19.09.94 12:24
// INPUTS    : struct BLEV_Event_struct *Event - Event.
// RESULT    : None.
// BUGS      : No known.
// SEE ALSO  :
//------------------------------------------------------------------------------
void Get_event(struct BLEV_Event_struct *Event)
{
	UNSHORT Old, New;

	//Do thang
	SYSTEM_SystemTask();

	//Get event
	BLEV_GetEvent(Event);

	//Get mouse coordinates
	Mouse_X = Event->sl_mouse_x;
	Mouse_Y = Event->sl_mouse_y;

	//Get old and new button state
	Old = Button_state;
	New = Button_state & 0x0011;

	//Check changes in the left and right mouse button states
	if ((Event->sl_eventtype == BLEV_MOUSELDOWN)
	 || (Event->sl_eventtype == BLEV_MOUSELSDOWN))
		New |= 0x01;

	if ((Event->sl_eventtype == BLEV_MOUSELUP)
	 || (Event->sl_eventtype == BLEV_MOUSELSUP))
		New &= ~0x01;

	if ((Event->sl_eventtype == BLEV_MOUSERDOWN)
	 || (Event->sl_eventtype == BLEV_MOUSERSDOWN))
		New |= 0x10;

	if ((Event->sl_eventtype == BLEV_MOUSERUP)
	 || (Event->sl_eventtype == BLEV_MOUSERSUP))
		New &= ~0x10;

	//Handles the Keyboard:
	if (Event->sl_eventtype == BLEV_KEYDOWN)
	{
		EventUngetch (Event->sl_key_code);
	}

	//Calculate the complete new button state
	Button_state = New | (((~Old & 0x0011) & New) << 1)
	 | (((~New & 0x0011) & Old) << 2);

	//Is this a second left-click that should be ignored ?
	if ((Event->sl_eventtype == BLEV_MOUSELDOWN) && (Ignore_second_left_click))
	{
		//Yes -> Clear flag
		Ignore_second_left_click = FALSE;

		//Get next event
		Get_event(Event);
	}

	//Is this a second right-click that should be ignored ?
	if ((Event->sl_eventtype == BLEV_MOUSERDOWN) && (Ignore_second_right_click))
	{
		//Yes -> Clear flag
		Ignore_second_right_click = FALSE;

		//Get next event
		Get_event(Event);
	}
}

//------------------------------------------------------------------------------
// NAME      : Update_input
// FUNCTION  : Update input.
// FILE      : CONTROL.C
// AUTHOR    : Jurie Horneman
// FIRST     : 05.09.94 13:32
// LAST      : 05.09.94 13:32
// INPUTS    : None.
// RESULT    : None.
// BUGS      : No known.
// NOTES     : - This function makes sure that the events will be read and
//              that the current mouse coordinates and button state remain
//              up to date.
// SEE ALSO  :
//------------------------------------------------------------------------------
void Update_input(void)
{
	struct BLEV_Event_struct Event;

	do
	{
		//Get event
		Get_event(&Event);
	}
	//Until there are no more events
	while (Event.sl_eventtype != BLEV_NOEVENT);
}

//------------------------------------------------------------------------------
//Geil! Ich wollte schon immer mal wegen 'ner BB-internen Message queue meine
//eigenen Routinen schreiben, damit die von der Message-Queue abgefangenen
//Keyboard kommandos erst einmal wieder in die globale Queue kommen...
//------------------------------------------------------------------------------
SILONG EventGetch (void)
{
	SILONG Key;

	if (EventGetchBufferSize)
	{
		Key = EventGetchBuffer[0];
		memmove (EventGetchBuffer, EventGetchBuffer+1, sizeof(EventGetchBuffer[0])*99);
		EventGetchBufferSize--;
	}

	return (Key);
}

//------------------------------------------------------------------------------
//Schiebt Zeichen in MEINE Event-Queue:
//------------------------------------------------------------------------------
void EventUngetch (SILONG Key)
{
	//Cool! Weil BB's rc bei Spezialzeichen willkÅrlich remappt wird und mein
	//Prg fÅr die Standart-PC codes gedacht ist remappen wir es wieder zurÅck..
	switch (Key)
	{
		case BLEV_LEFT:
			EventUngetch (0);
			EventUngetch (75);
			break;

		case BLEV_UP:
			EventUngetch (0);
			EventUngetch (72);
			break;

		case BLEV_RIGHT:
			EventUngetch (0);
			EventUngetch (77);
			break;

		case BLEV_DOWN:
			EventUngetch (0);
			EventUngetch (80);
			break;

		case BLEV_RETURN:
			EventUngetch (13);
			break;

		case BLEV_ESC:
			EventUngetch (27);
			break;

		case BLEV_BS:
			EventUngetch (8);
			break;

		default:
			if (EventGetchBufferSize<100)
			{
				EventGetchBuffer[EventGetchBufferSize]=Key;
				EventGetchBufferSize++;
			}
			break;
	}
}

//------------------------------------------------------------------------------
//Sagt an, ob noch was in der Queue ist:
//------------------------------------------------------------------------------
SILONG EventKbhit(void)
{
	return (EventGetchBufferSize);
}
