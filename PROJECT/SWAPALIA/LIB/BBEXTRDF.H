/************
 * NAME     : BB_EEXTERN_DEFS
 * AUTOR    : COOLHAND, BlueByte
 * START    : 08/01/94   14:18:35
 * PROJECT  : BASELIB
 * NOTES    : Das File ist ein reines H-File ohne C-Sourcefile und stellt diverse,
 *				globale Typedefinietionen, insbesondere für alte BB-Datenformate
 *				zur Verfügung
 * SEE ALSO :
 * VERSION  : 1.0
 ************/

#ifndef BB_EXTERN_DEFS_H
#define BB_EXTERN_DEFS_H

/* RMI Interrupt Struktur */
struct RMINFO {
    long EDI;
    long ESI;
    long EBP;
    long reserved_by_system;
    long EBX;
    long EDX;
    long ECX;
    long EAX;
    short flags;
    short ES,DS,FS,GS,IP,CS,SP,SS;
};

/* GTO: the good old BlueByte GTO. A graphical object format. */
#define GTOMAXNAME 8
typedef struct GTOII
{
	UNCHAR	ac_infochunk[4];		/* INFO chunk */
	UNCHAR	ac_name[8];			/* Name */
	UNBYTE	ub_transcol;			/* Transparent color */
	UNBYTE	ub_plane;				/* Plane ??? Set to 0 */
	SISHORT	ss_xoffset;			/* X offset */
	SISHORT	ss_yoffset;			/* Y offset */
	UNSHORT	us_width;				/* Width */
	UNSHORT	us_height;				/* Height */
	UNCHAR	ac_bodychunk[4];		/* BODY chunk */
								/* The image starts directly behind the BODY chunk */
								/* Behind the image you find a ENDE chunk */
} GTO_structII;

struct GTO
{
	UNCHAR	infochunk[4];		/* INFO chunk */
	UNCHAR	name[8];			/* Name */
	UNBYTE	transcol;			/* Transparent color */
	UNBYTE	plane;				/* Plane ??? Set to 0 */
	SISHORT	xoffset;			/* X offset */
	SISHORT	yoffset;			/* Y offset */
	UNSHORT	width;				/* Width */
	UNSHORT	height;				/* Height */
	UNCHAR	bodychunk[4];		/* BODY chunk */
								/* The image starts directly behind the BODY chunk */
								/* Behind the image you find a ENDE chunk */
};


// Typen für die Standard-TCT-Library. Der Erste Typ definiert den Header, der der Library vorangeht,
// aus Einträgen des letzten Typs setzt sich die Pointertabelle am Ende der Library zusammen
typedef struct tct_library{
	UNLONG	ul_offset;		// Offset zur Tabelle
	UNCHAR	ac_kennid[4];	// Hier steht immer 'TCT '
	UNSHORT	us_objcount;	// Anzahl Objekte in der Library
	UNCHAR	c_type;			// 'I' == Intel, 'M' == Motorola
	UNCHAR	c_pad;
} TCT_Library_struct;


typedef struct tct_table{
	UNCHAR	as_filename[8];	// Ursprünglicher Filename des Eintrages ohne Erweiterung
	UNLONG	ul_offset;		// Offset des Datenbeginns vom Dateianfang
} TCT_Table_struct;




/* Header eines TPACK-gepackten Files */
typedef struct tpack_header{
	UNBYTE		ac_header[4];	/* Muß TPWM sein bei TPACK-File */
	UNLONG		ul_real_size;	/* Entpackte Größe der Daten */
}TPACK_Header_struct;


/* TPACK - Kennung im Feld ac_header */
#define TPACK_ID	"TPWM"




#endif
