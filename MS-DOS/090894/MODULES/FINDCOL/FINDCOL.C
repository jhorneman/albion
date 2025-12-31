/************
 * NAME     : FINDCOL.C
 * AUTHOR   : Christian Jungen, BlueByte
 * START    : 21-7-1994
 * PROJECT  : Fast colour finding
 * NOTES    :
 * SEE ALSO : FINDCOL.H
 ************/

/***************************************************************************\

Diese Routinen ersetzen die extrem rechenzeitaufwendigen find_closest_color-
Routine

Der Trick dahinter:

Zur Anwendung kommt eine Hashtechnik, die beim RGB-Wert erstmal nur die
oberensten Bit beachtet und dann quasi in einem pseudo-dreidimensionalen
Array die entsprechende Farbe sucht. Dabei mÅssen in der Regel keine bis
sehr wenige Farben miteinander verglichen werden.

Anmerkung:

Es besteht die Mîglichkeit Farbkorrektur fÅr bessere QualitÑt einzusetzen.
Der R-Werte werden mit 0.30, G-Wert 0.59 und der B-Wert mit 0.11 anteilig
multipliziert. DafÅr muss COLOR_CORRECTION auf ON gestellt werden.


Benutzung:

Initialisierung:
-> init_find_closest_color_looktable();

Jede Farbe einzeln eintragen RGB-Wertebereich=0-255, color_nr=Farbnummer
-> install_color_in_color_looktable(color_nr,r_val,g_val,b_val);

Jetzt mÅssen die leeren EintrÑge in der Tabelle berechnet werden
-> fill_up_color_looktable();

Unter Angabe von RGB-Werten (RGB-Wertebereich=0-255!) kann jetzt der Index
der Ñhnlichsten Farbe ermittelt werden!
-> UNLONG find_closest_color(r_val,g_val,b_val);

\***************************************************************************/

/* includes */

#include <BBDEF.H>
#include "findcol.h"

/*
 ** Defines ****************************************************************
 */

#define ON -1
#define OFF 0

#define COLOR_CORRECTION ON

#define R_COL_CORRECTION 3 /* die Werte sind gerundet und mit 10 erweitert */
#define G_COL_CORRECTION 6 /* die Erweiterung ist mîglich, da nur Farbdistanzen berechnet */
#define B_COL_CORRECTION 1 /* und miteinander verglichen werden */

#define R_COL_BITS 3
#define G_COL_BITS 3
#define B_COL_BITS 2

/* Werte um Bits auszumaskieren */
#define R_COL_BIT_MASK ((1<<R_COL_BITS)-1)
#define G_COL_BIT_MASK ((1<<G_COL_BITS)-1)
#define B_COL_BIT_MASK ((1<<B_COL_BITS)-1)

/* Grîsse der benîtigten Lookuptable */
#define ANZ_RGB_HASH_ENTRIES ( (1<<R_COL_BITS) * (1<<G_COL_BITS) * (1<<B_COL_BITS) )
/* max. Anzahl von Farben die eingetragen werden kînnen */
#define FINDCLOSEST_MAX_COLOR 256

#define HUGEST_UNSIGNED_VAL 0xffffffff

/*
 ** Structure definitions **************************************************
 */

/* Eintrag in Farb-Lookuptable */
struct SRGB_ENTRY{
	UNSHORT color_nr;
	unsigned char r_val,g_val,b_val;
	struct SRGB_ENTRY *nxt_rgb_entry;			/* Zeiger auf nÑchsten Farb-Eintrag */
};

/* global variable declarations */

struct SRGB_ENTRY *rgb_hash_pointer_map[ANZ_RGB_HASH_ENTRIES]; /* Zeiger auf die FarbeintrÑge */
struct SRGB_ENTRY rgb_lookup[FINDCLOSEST_MAX_COLOR];
UNLONG anz_installed_look_colors;					/* Anzahl installierte Farben */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : init_find_closest_color_looktable
 * FUNCTION  : initialisierung der Lookup-Tables fÅr find_closest_color();
 * FILE      : FINDCOL.C
 * AUTHOR    : Christian Jungen
 * FIRST     : 21.07.94 16:20
 * LAST      : 21.07.94 16:20
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
init_find_closest_color_looktable(void)
{
	UNLONG i;
	struct SRGB_ENTRY **pointer_1;
	struct SRGB_ENTRY *pointer_2;

	/* Anzahl der installierten Farbe auf Null setzten */
	anz_installed_look_colors=0;

	/* Zeigertabelle auf NULL setzten */
	pointer_1=&rgb_hash_pointer_map[0];
	for(i=0;i<ANZ_RGB_HASH_ENTRIES;i++)
		*pointer_1++=NULL;

	/* Verweis-Zeiger auf NULL setzten */
	pointer_2=&rgb_lookup[0];
	for(i=0;i<FINDCLOSEST_MAX_COLOR;i++)
		pointer_2++->nxt_rgb_entry=NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : install_color_in_color_looktable
 * FUNCTION  : installiert eine RGB-Farbe in der Lookup-Table
 * FILE      : FINDCOL.C
 * AUTHOR    : Christian Jungen
 * FIRST     : 21.07.94 16:20
 * LAST      : 21.07.94 16:20
 * INPUTS    : UNLONG color_nr - Colour number.
 *             UNLONG r_val - Red component (0-255).
 *             UNLONG g_val - Green component (0-255).
 *             UNLONG b_val - Blue component (0-255).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
install_color_in_color_looktable(UNLONG color_nr,UNLONG r_val,UNLONG g_val,
 UNLONG b_val)
{
	UNLONG rgb_code;
	struct SRGB_ENTRY *rgb_entry_ptr;

	if( color_nr<FINDCLOSEST_MAX_COLOR )
	{

		/* RGB-Code=Arrayindex in Loouptable erzeugen */
	  rgb_code=  ( (r_val>>(8-R_COL_BITS)) & R_COL_BIT_MASK ) << (G_COL_BITS+B_COL_BITS) |
               ( (g_val>>(8-G_COL_BITS)) & G_COL_BIT_MASK ) << B_COL_BITS |
		           ( (b_val>>(8-B_COL_BITS)) & B_COL_BIT_MASK );

		rgb_entry_ptr=rgb_hash_pointer_map[rgb_code];
		if( rgb_entry_ptr==NULL )	/* Pointer-Map bereits NULL? */
		{
			/* Zeiger auf Hash installieren */
			rgb_hash_pointer_map[rgb_code]=&rgb_lookup[anz_installed_look_colors];
		}
		else
		{
			/* sonst durchangeln bis Pointer=NULL gefunden wird */
			while( rgb_entry_ptr->nxt_rgb_entry!=NULL )
				rgb_entry_ptr=rgb_entry_ptr->nxt_rgb_entry;
			/* Zeiger auf Hash installieren */
			rgb_entry_ptr->nxt_rgb_entry=&rgb_lookup[anz_installed_look_colors];
		}

		/* Farbdaten einfÅgen */
		rgb_entry_ptr=&rgb_lookup[anz_installed_look_colors];
		rgb_entry_ptr->color_nr=color_nr;
		rgb_entry_ptr->r_val=r_val;
		rgb_entry_ptr->g_val=g_val;
		rgb_entry_ptr->b_val=b_val;

		/* Anzahl erhîhen */
		anz_installed_look_colors++;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : fill_up_color_looktable
 * FUNCTION  : nicht belegte Lookuptable-Entries berechnen.
 * FILE      : FINDCOL.C
 * AUTHOR    : Christian Jungen
 * FIRST     : 21.07.94 16:20
 * LAST      : 21.07.94 16:20
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
fill_up_color_looktable(void)
{
	UNLONG rgb_code;
	UNLONG r_val,g_val,b_val;
	UNLONG rgb_nr;
	struct SRGB_ENTRY *rgb_entry_ptr;
	struct SRGB_ENTRY *found_rgb_entry_ptr;
	unsigned long min_color_dist,color_dist;

	/* alle leeren Map-EintrÑge besetzten */
	for(rgb_code=0;rgb_code<ANZ_RGB_HASH_ENTRIES;rgb_code++)
	{
		if( rgb_hash_pointer_map[rgb_code]==NULL )		/* alle leeren EintrÑge suchen */
		{
			/* aus rgb_code r,g,b_val erzeugen (runterschiften und Bits ausmaskieren */
			r_val=( rgb_code>>(G_COL_BITS+B_COL_BITS) ) & R_COL_BIT_MASK;
			g_val=( rgb_code>>(B_COL_BITS) ) & G_COL_BIT_MASK;
			b_val=  rgb_code & B_COL_BIT_MASK;

			/* und in den Wertebereich 0-255 bringen, also echte RGB-Werte erzeugen */
			r_val<<=(8-R_COL_BITS);
			g_val<<=(8-G_COL_BITS);
			b_val<<=(8-B_COL_BITS);

			/* mit allen bereits installierten Farben vergleichen */
			rgb_entry_ptr=&rgb_lookup[0];
			min_color_dist=HUGEST_UNSIGNED_VAL;
			for(rgb_nr=0;rgb_nr<anz_installed_look_colors;rgb_nr++)
			{
				/* Farbdistanz berechnen */
			  #if COLOR_CORRECTION==ON
				color_dist= (r_val-rgb_entry_ptr->r_val) * (r_val-rgb_entry_ptr->r_val) * (R_COL_CORRECTION*R_COL_CORRECTION) +
	                  (g_val-rgb_entry_ptr->g_val) * (g_val-rgb_entry_ptr->g_val) * (G_COL_CORRECTION*G_COL_CORRECTION) +
	                  (b_val-rgb_entry_ptr->b_val) * (b_val-rgb_entry_ptr->b_val) * (B_COL_CORRECTION*B_COL_CORRECTION) ;
			  #else
				color_dist= (r_val-rgb_entry_ptr->r_val) * (r_val-rgb_entry_ptr->r_val)+
	                  (g_val-rgb_entry_ptr->g_val) * (g_val-rgb_entry_ptr->g_val)+
	                  (b_val-rgb_entry_ptr->b_val) * (b_val-rgb_entry_ptr->b_val);
			  #endif

				if( color_dist<min_color_dist )					/* Farbe Ñhnlicher? */
				{
					/* merke neuen kleinsten Abstand */
					min_color_dist=color_dist;
					/* merke den Index dieser Farbe */
					found_rgb_entry_ptr=rgb_entry_ptr;
				}
				rgb_entry_ptr++;												/* Zeiger auf nÑchste Farbe */
			}
			/* Zeiger auf gefunden RGB-Wert setzten */
			rgb_hash_pointer_map[rgb_code]=found_rgb_entry_ptr;
		} /* Endif */
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : find_closest_color
 * FUNCTION  : liefert fÅr RGB entsprechende Color-Nr zurÅck
 *              (RGB-Wertebereich jeweils 0-255!)
 * FILE      : FINDCOL.C
 * AUTHOR    : Christian Jungen
 * FIRST     : 21.07.94 16:20
 * LAST      : 21.07.94 16:20
 * INPUTS    : UNLONG r_val - Red component (0-255).
 *             UNLONG g_val - Green component (0-255).
 *             UNLONG b_val - Blue component (0-255).
 * RESULT    : UNLONG : Closest colour number.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
find_closest_color(UNLONG r_val,UNLONG g_val,UNLONG b_val)
{
	UNLONG rgb_code;
	struct SRGB_ENTRY *rgb_entry_ptr;
	struct SRGB_ENTRY *found_rgb_entry_ptr;
	UNLONG hash_compare_nr;
	unsigned long min_color_dist,color_dist;

	/* RGB-Code=Arrayindex in Lookuptable erzeugen */
	rgb_code=  ( (r_val>>(8-R_COL_BITS)) & R_COL_BIT_MASK ) << (G_COL_BITS+B_COL_BITS) |
             ( (g_val>>(8-G_COL_BITS)) & G_COL_BIT_MASK ) << B_COL_BITS |
	           ( (b_val>>(8-B_COL_BITS)) & B_COL_BIT_MASK );


	/* Zeiger auf Tabelle holen */
	rgb_entry_ptr=rgb_hash_pointer_map[rgb_code];

	/* falls nur einen Wert (Normalfall) Wert bereits eindeutig gefunden */
	/* sonst alle Farben, die im gleichen Feld sind auf FarbÑhnlichkeit prÅfen */
	if( rgb_entry_ptr->nxt_rgb_entry!=NULL )
	{
		min_color_dist=HUGEST_UNSIGNED_VAL;
		do{
			/* Farbdistanz berechnen */
      #if COLOR_CORRECTION==ON
				color_dist= (r_val-rgb_entry_ptr->r_val) * (r_val-rgb_entry_ptr->r_val) * (R_COL_CORRECTION*R_COL_CORRECTION) +
	                  (g_val-rgb_entry_ptr->g_val) * (g_val-rgb_entry_ptr->g_val) * (G_COL_CORRECTION*G_COL_CORRECTION) +
	                  (b_val-rgb_entry_ptr->b_val) * (b_val-rgb_entry_ptr->b_val) * (B_COL_CORRECTION*B_COL_CORRECTION) ;
      #else
				color_dist= (r_val-rgb_entry_ptr->r_val) * (r_val-rgb_entry_ptr->r_val)+
                    (g_val-rgb_entry_ptr->g_val) * (g_val-rgb_entry_ptr->g_val)+
                    (b_val-rgb_entry_ptr->b_val) * (b_val-rgb_entry_ptr->b_val);
      #endif

			if( color_dist<min_color_dist )					/* Farbe Ñhnlicher? */
			{
				/* merke neuen kleinsten Abstand */
				min_color_dist=color_dist;
				/* merke den Index dieser Farbe */
				found_rgb_entry_ptr=rgb_entry_ptr;
			}
			rgb_entry_ptr=rgb_entry_ptr->nxt_rgb_entry; /* Zeiger auf nÑchste Farbe */
		}while( rgb_entry_ptr!=NULL );			  /* alle Zeiger durchsuchen bis == NULL */

		/* Zeiger auf gefunden Farbwert */
		rgb_entry_ptr=found_rgb_entry_ptr;
	}
	return( (UNLONG)rgb_entry_ptr->color_nr );	/* Farbnummer zurÅckliefern */
}

