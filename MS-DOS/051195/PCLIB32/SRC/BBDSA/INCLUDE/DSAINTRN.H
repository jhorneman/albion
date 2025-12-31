#ifndef DSAINTRN_H
#define DSAINTRN_H

/************
 * NAME     :  DSAINTRN.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */

#include <BBDEF.H>
#include <BBEXTRDF.H>

/* Master defines */
#define BBDSA_ERRORHANDLING

/* defines */

#define VESAMD 0 // normaler ByteproPixel Modus (special)
#define ONESEG 1 // 1 Segment Mode ohne Doublebuffer
#define CHAIN4 2 // 320*200/360x240 Chain 4 Modus

#define NODOUBLEBF 0 // kein Doublebuffering
#define DOUBLEBF 1 // Doublebuffering

#define SCREEN_ONESEG 0xA0000 // Bildschirm bei OneSegment Mode
#define SCREEN_CHAIN4 0xA0000 // Bildschirm bei Chain4 Mode

#define MAXSCREENS 4 // Anzahl Bildschirme die maximal m”glich sind

/* Strukturen */

struct VESAMODEDEF
{
	UNSHORT width;
	UNSHORT height;
	UNSHORT depth;
	UNSHORT mode;
	UNSHORT special;
	UNSHORT doublebuffer;
};

struct VESAINFO
{
	UNBYTE Signature[4];
	UNSHORT Version;
	UNSHORT Name[2];				// seg.off small endian -> off.seg
	UNBYTE Capabilities[4];
	UNSHORT Modes[2];				// seg.off small endian -> off.seg
	UNBYTE Reserved[238];
};

/********************************************************************/

struct VESAMODEINFO
{
	UNSHORT Attributes;			// 1.0
	UNBYTE WinAAtt;
	UNBYTE WinBAtt;
	UNSHORT WinGran;
	UNSHORT WinSize;
	UNSHORT WinAStartSeg;
	UNSHORT WinBStartSeg;
	UNLONG WinPosFunc;
	UNSHORT BytePerScanLine;

	UNSHORT PixelWidth;			// OEM
	UNSHORT PixelHeight;
	UNBYTE PixelWidthChar;
	UNBYTE PixelHeightChar;
	UNBYTE NumOfPlanes;
	UNBYTE BitsPerPixel;
	UNBYTE NumOfBanks;
	UNBYTE MemoryModel;
	UNBYTE SizeOfBank;

	UNBYTE RedMaskSize;			// 1.2
	UNBYTE RedFieldPos;
	UNBYTE GreenMaskSize;
	UNBYTE GreenFieldPos;
	UNBYTE BlueMaskSize;
	UNBYTE BlueFieldPos;
	UNBYTE ReservedMaskSize;
	UNBYTE ReservedMaskPos;
	UNBYTE DirectColorMode;
	UNBYTE Reserved;

	UNBYTE FutureUse[256-39];	// X.X
};

/* Error struct */

#ifdef BBDSA_ERRORHANDLING

	struct BBDSA_ErrorStruct
	{
		UNBYTE *errorname;
		UNLONG	errordata;
		UNLONG	errordata2;
	};

#endif

/********************************************************************/

/* externe Variablen */

extern UNSHORT	special; // spezielle Eigenschaften des eingestellen Modus

extern UNBYTE	*videoram[]; // Videoram Position im Videoram Doublebuffer
extern SILONG videoscreenypos[]; // Y Position im Videoram DoubleBuffer

extern UNSHORT doublepage; // aktuell eingeschaltete DoublebufferPage
extern UNSHORT anzdoublepages; // Anzahl Bildschirme
extern UNLONG	screenwidth; // Breite einer Zeile des Bildschirms in Bytes
extern UNLONG	screenheight; // H”he des Bildschirms

extern UNBYTE chain4writeselectok; // wiederherstellen notwendig ?
extern UNBYTE chain4readselectok; // wiederherstellen notwendig ?

/* ErrorPrintFunktion */
#ifdef BBDSA_ERRORHANDLING
	extern void DSA_PrintError( UNCHAR * buffer, UNBYTE * data );
#endif

/* externe Assemblerfunktionen */
extern void _DSA_ASS_ActivatePal(UNBYTE *palptr,UNLONG anz);
#pragma aux _DSA_ASS_ActivatePal parm caller [ESI] [ECX] modify [EAX EBX EDX EDI]; // Put values directly in registers

extern void _DSA_ASS_CopyMainOPMToScreen(UNBYTE *source,UNBYTE *dest);
#pragma aux _DSA_ASS_CopyMainOPMToScreen parm caller [] modify [EAX EBX ECX EDX ESI EDI]; // the caller has to remove parameter from stack

extern void _DSA_ASS_CopyMainOPMToScreenONE(UNBYTE *source,UNBYTE *dest);
#pragma aux _DSA_ASS_CopyMainOPMToScreenONE parm caller [ESI] [EDI] modify [EAX EBX ECX EDX]; // Put values directly in registers

extern void _DSA_ASS_CopyMainOPMToScreenCHN(UNBYTE *source,UNBYTE *dest);
#pragma aux _DSA_ASS_CopyMainOPMToScreenCHN parm caller [] modify [EAX EBX ECX EDX ESI EDI]; // Put values directly in registers

extern void _DSA_ASS_CopyOPMToScreen(UNBYTE *source,UNBYTE *dest, UNLONG nextypos,UNLONG x,UNLONG y,UNLONG width, UNLONG height);
#pragma aux _DSA_ASS_CopyOPMToScreen parm caller [] modify [EAX EBX ECX EDX ESI EDI]; // the caller has to remove parameter from stack

extern void _DSA_ASS_CopyOPMToScreenONE(UNBYTE *source,UNBYTE *dest,UNLONG nextypos,UNLONG x,UNLONG y,UNLONG width, UNLONG height);
#pragma aux _DSA_ASS_CopyOPMToScreenONE parm caller [ESI] [EDI] [] modify [EAX EBX ECX EDX]; // the caller has to remove parameter from stack

extern void _DSA_ASS_CopyOPMToScreenCHN(UNBYTE *source,UNBYTE *dest,UNLONG nextypos,UNLONG x,UNLONG y,UNLONG width, UNLONG height);
#pragma aux _DSA_ASS_CopyOPMToScreenCHN parm caller [] modify [EAX EBX ECX EDX ESI EDI]; // the caller has to remove parameter from stack

extern void _DSA_ASS_CopyScreenToOPM(UNBYTE *dest,UNBYTE *source,UNLONG nextypos,UNLONG x,UNLONG y,UNLONG width, UNLONG height);
#pragma aux _DSA_ASS_CopyScreenToOPM parm caller [] modify [EAX EBX ECX EDX ESI EDI]; // the caller has to remove parameter from stack

extern void _DSA_ASS_CopyScreenToOPMONE(UNBYTE *dest,UNBYTE *source,UNLONG nextypos,UNLONG x,UNLONG y,UNLONG width, UNLONG height);
#pragma aux _DSA_ASS_CopyScreenToOPMONE parm caller [ESI] [EDI] modify [EAX EBX ECX EDX]; // the caller has to remove parameter from stack

extern void _DSA_ASS_CopyScreenToOPMCHN(UNBYTE *dest,UNBYTE *source,UNLONG nextypos,UNLONG x,UNLONG y,UNLONG width, UNLONG height);
#pragma aux _DSA_ASS_CopyScreenToOPMCHN parm caller [] modify [EAX EBX ECX EDX ESI EDI]; // the caller has to remove parameter from stack

extern SISHORT	_DSA_ASS_ActivateCHAINFOUR( UNLONG width,UNLONG height );
#pragma aux _DSA_ASS_ActivateCHAINFOUR parm caller [] value [ax] modify [EBX ECX EDX ESI EDI]; // the caller has to remove parameter from stack

extern void _DSA_ASS_SetVideoramCHN(UNBYTE *vram);
#pragma aux _DSA_ASS_SetVideoramCHN parm caller [EBX] modify [EAX EDX]; // the caller has to remove parameter from stack

extern void _DSA_ASS_WaitVBL( void );
#pragma aux _DSA_ASS_WaitVBL parm caller [] modify [EAX EDX];

extern void _DSA_ASS_WaitEndVBL( void );
#pragma aux _DSA_ASS_WaitEndVBL parm caller [] modify [EAX EDX];

extern void _DSA_ASS_WaitTOF( void );
#pragma aux _DSA_ASS_WaitTOF parm caller [] modify [EAX EDX];

extern void _DSA_ASS_WaitBOF( void );
#pragma aux _DSA_ASS_WaitBOF parm caller [] modify [EAX EDX];

extern void _DSA_ASS_SaveVesaPage( void );
#pragma aux _DSA_ASS_SaveVesaPage parm caller [] modify [EAX];

extern void _DSA_ASS_RestoreVesaPage( void );
#pragma aux _DSA_ASS_RestoreVesaPage parm caller [] modify [EAX EBX ECX EDX ESI EDI];

extern void _DSA_ASS_SaveCHAIN4Page( void );
#pragma aux _DSA_ASS_SaveCHAIN4Page parm caller [] modify [EAX EDX];

extern void _DSA_ASS_RestoreCHAIN4Page( void );
#pragma aux _DSA_ASS_RestoreCHAIN4Page parm caller [] modify [EAX EDX];

extern void _DSA_ASS_Flash( UNBYTE red,UNBYTE green,UNBYTE blue);
#pragma aux _DSA_ASS_Flash parm caller [ECX ESI EDI] modify [EAX EBX EDX ];

#endif
