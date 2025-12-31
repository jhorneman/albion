/************
 * NAME     : ICCHANGE.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef ICCHANGE_H
#define ICCHANGE_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** Defines ****************************************************************
 */

/* Icon change / modification types */
#define UNDERLAY_2D_CHANGE		(0)
#define OVERLAY_2D_CHANGE		(1)
#define NORMAL_3D_CHANGE		(2)
#define FLOOR_3D_CHANGE			(3)
#define CEILING_3D_CHANGE		(4)
#define NPC_MOVE_CHANGE			(5)
#define NPC_GFX_CHANGE			(6)
#define EVENT_ENTRY_CHANGE		(7)
#define BLOCK_2D_REPLACE		(8)
#define BLOCK_2D_MIX				(9)
#define TRIGGER_MODE_CHANGE	(10)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern MEM_HANDLE Permanent_modification_list;
extern UNSHORT Nr_permanent_modifications;

extern MEM_HANDLE Temporary_modification_list;
extern UNSHORT Nr_temporary_modifications;

/*
 ** Prototypes *************************************************************
 */

/* Modification list management */
UNSHORT Init_modifications(UNBYTE *Data, MEM_HANDLE *First_list_handle_ptr);

void Exit_modifications(MEM_HANDLE First_list_handle);

MEM_HANDLE Prepare_modifications_for_saving(MEM_HANDLE First_list_handle,
 UNSHORT Nr_modifications);

void Make_modifications(UNSHORT Map_nr, MEM_HANDLE First_list_handle,
 UNSHORT Nr_modifications);

void Clone_modifications(UNSHORT Source_map_nr, UNSHORT Target_map_nr);

void Add_permanent_modification(UNSHORT X, UNSHORT Y, UNSHORT Type,
 UNSHORT Sub_type, UNSHORT Value, UNSHORT Map_nr);

void Add_temporary_modification(UNSHORT X, UNSHORT Y, UNSHORT Type,
 UNSHORT Sub_type, UNSHORT Value, UNSHORT Map_nr);

void Do_change_icon(SISHORT X, SISHORT Y, UNSHORT Type, UNSHORT Sub_type,
 UNSHORT Value);

#endif

