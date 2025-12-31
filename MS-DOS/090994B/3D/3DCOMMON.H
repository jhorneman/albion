/************
 * NAME     : 3DCOMMON.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 16-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMMON3D_H
#define COMMON3D_H

#include <BBDEF.H>
#include <BBOPM.H>
#include "3D_MAP.H"

/*
 ** Defines ****************************************************************
 */

#define SQUARE_SIZE_3D           1024           /* In 3D units */
#define ANGLE_STEPS             16384           /* Equals 360 degrees */

/*
 ** Structure definitions **************************************************
 */

typedef struct

        {       UNLONG          Flags;                  /* See below */
                UNSHORT         Player_X[3];            /* 32:16 bit player coordinates in 3D units */
                UNSHORT         Player_Z[3];
                UNLONG          Camera_angle;           /* 16:16 bit */

                UNSHORT         Camera_height;          /* In cm */
                SISHORT         Horizon_Y;              /* In pixels measured from the middle of the 3D-window */
                SISHORT         Horizon_offset;         /* Abstand vom oberen Bildrand zum oberen Rand des Grafikstreifens in Pixeln */
                UNSHORT         Horizon_angle_factor;   /* Gibt an, wie oft die Hintergrundgrafik im Sichtkreis (360') wiederholt wird */

                UNBYTE          Sky_colour;             /* Farbe des Himmel-Bereichs (ber dem Horizont) */
                UNBYTE          Floor_colour;           /* Farbe des Boden-Bereichs (unter dem Horizont) */

                UNSHORT         Window_3D_width;        /* In pixels */
                UNSHORT         Window_3D_height;

                UNSHORT         Wall_height;            /* In cm */
                UNSHORT         Square_size_cm;
                UNSHORT         Cm_to_Units_log2;       /* Umrechnungs-Shifts cm -> Units */

                Object_group_3D  *Object_groups;
                 Floor_3D        *Floors;               /* Probably not needed */
                Object_3D        *Objects;
                  Wall_3D       **Wall_pointers;        /* Pointer to list of pointers to wall data */

                NPC_3D          *NPCs;                  /* Pointer to NPC list */
                UNSHORT         Nr_NPCs;                /* Number of NPCs in list (max. 32) */

                Square_3D       *Map_pointer;

                UNBYTE          *Shade_table;
                UNSHORT          Shade_factor;

                UNBYTE          *Background_graphics;   /* NULL means no background */
                UNSHORT          Background_width;      /* In pixels */
                UNSHORT          Background_height;

                struct OPM      *Target_OPM;

                UNBYTE          Shade_function[1024];   /* Variable Abdunklungs-Charakteristik */
        }

        Interface_3DM;




/* Bitmasks for Interface_3DM.Flags */

#define DUNGEON_OR_CITY         (1<<0)

/*
 ** External global variables **********************************************
 */

extern Interface_3DM I3DM;

#endif

