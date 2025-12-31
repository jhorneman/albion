/************
 * NAME     : BBTGA.c
 * AUTOR    : R.Reber, BlueByte
 * START    : 20.07.94
 * PROJECT  : Poject32
 * NOTES    :
 * SEE ALSO :
 * VERSION  : 1.0
 ************/

/* Includes */
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <PCLIBHI.h>
#include "include\lbmintrn.h"

/* Error handling */
#ifdef BBLBM_ERRORHANDLING

	/* ErrorStruktur */
	struct BBLBM_ErrorStruct bblbm_error;

	/* Name of Library */
	char BBLBM_LibraryName[] = "BBLBM Library";

	/* globales Makro fÅr LBM_Error */
	#define LBM_ERROR(a,b)	bblbm_error.errorname = (a); \
													bblbm_error.errordata = (b); \
													ERROR_PushError( LBM_PrintError, ( UNCHAR * ) &BBLBM_LibraryName[0], sizeof( struct BBLBM_ErrorStruct ), ( UNBYTE * ) &bblbm_error );

#endif

/* defines */
#define PUFFERSIZE 8192 // Grîsse des LEsePuffers fÅr read
#define PUFFERWRITESIZE (3*2048) // Grîsse des SchreibePuffers fÅr Write

/* typedefs */

typedef struct {unsigned short Red, Green, Blue, Filter;} IMAGE_COLOUR;

/* externe Variablen */
extern struct BBDOS_FileHandleStruct BBDOS_FileHandleArray[];

/* globale Variablen */
static UNCHAR idbuf[256];

//static UNBYTE *tgafilename;
static SISHORT tgafilehandle;
static SILONG tgafilelength;
static SILONG tgafilesize;
static UNLONG tgaflag;
static UNBYTE *tgamemptr;
static UNBYTE *pufferstartptr;
static UNBYTE *pufferendptr;
static UNBYTE *pufferptr;


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : TGA_SetPixel
 * FUNCTION :
 * INPUTS   :  struct OPM * opmptr :
 *             SISHORT x :
 *             SISHORT y :
 *             UNBYTE red :
 *             UNBYTE green :
 *             UNBYTE blue :
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void TGA_SetPixel( struct OPM * opmptr, SISHORT x, SISHORT y, UNBYTE red, UNBYTE green, UNBYTE blue)
{
	/* Calc absolute offset */
	x += opmptr->xoffset;
	y += opmptr->yoffset;

	/* X out of clipping */
	/* X out of clipping */
	if((UNSHORT)x>=opmptr->clipwidth||(UNSHORT)y>=opmptr->clipheight)
		return;

	/* Set changed bit in status flag set */
	opmptr->status |= (OPMSTAT_CHANGED);

	/* word pro Pixel */
	if(opmptr->depth==2){
		*(opmptr->data+(y*opmptr->nextypos+x))=HCOL(red,green,blue);
	}
	/* Truecolor */
	else if(opmptr->depth==3||opmptr->depth==4){
		/* Zeiger auf Datenbereich OPM */
		UNBYTE *data=(UNBYTE *)opmptr->data;
		data+=(y*opmptr->nextypos+x)*opmptr->depth;
		*(data+2)=red;
		*(data+1)=green;
		*(data)=blue;
	}

}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : TGA_GetPixel
 * FUNCTION :
 * INPUTS   :  struct OPM * opmptr :
 *             SISHORT x :
 *             SISHORT y :
 *             UNBYTE *red :
 *             UNBYTE *green :
 *             UNBYTE *blue :
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void TGA_GetPixel( struct OPM * opmptr, SISHORT x, SISHORT y, UNBYTE *red, UNBYTE *green, UNBYTE *blue)
{

	/* Calc absolute offset */
	x += opmptr->xoffset;
	y += opmptr->yoffset;

	/* X out of clipping */
	/* X out of clipping */
	if((UNSHORT)x>=opmptr->clipwidth||(UNSHORT)y>=opmptr->clipheight)
		return;

	/* Set changed bit in status flag set */
	opmptr->status |= (OPMSTAT_CHANGED);

	/* word pro Pixel */
	if(opmptr->depth==2){
		UNSHORT col=*(opmptr->data+(y*opmptr->nextypos+x));
		*(red)=HCOLB(col);
		*(green)=HCOLG(col);
		*(blue)=HCOLR(col);
	}
	/* Truecolor */
	else if(opmptr->depth==3||opmptr->depth==4){
		/* Zeiger auf Datenbereich OPM */
		UNBYTE *data=(UNBYTE *)opmptr->data;
		data+=(y*opmptr->nextypos+x)*opmptr->depth;
		*(red)=*(data);
		*(green)=*(data+1);
		*(blue)=*(data+2);
	}

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : convert_targa_color
 * FUNCTION :
 * INPUTS   : IMAGE_COLOUR *tcolor :
 *            int pixelsize :
 *            unsigned char *bytes :
 * RESULT   : TRUE = OK / FALSE = ERROR
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN convert_targa_color(IMAGE_COLOUR *tcolor,int pixelsize,unsigned char *bytes)
{
  switch (pixelsize)
  {
  case 1:
    tcolor->Red   = bytes[0];
    tcolor->Green = bytes[0];
    tcolor->Blue  = bytes[0];
    tcolor->Filter = 0;
    break;
  case 2:
    tcolor->Red   = ((bytes[1] & 0x7c) << 1);
    tcolor->Green = (((bytes[1] & 0x03) << 3) |
      ((bytes[0] & 0xe0) >> 5)) << 3;
    tcolor->Blue  = (bytes[0] & 0x1f) << 3;
    tcolor->Filter = (bytes[1] & 0x80 ? 255 : 0);
    break;
  case 3:
    tcolor->Red   = bytes[2];
    tcolor->Green = bytes[1];
    tcolor->Blue  = bytes[0];
    tcolor->Filter = 0;
    break;
  case 4:
    tcolor->Red   = bytes[2];
    tcolor->Green = bytes[1];
    tcolor->Blue  = bytes[0];
    tcolor->Filter = bytes[3];
    break;
  default:
		LBM_ERROR("LBM_DisplayTGA: Bad Pixel Size in Targa Color",pixelsize);
		return(FALSE);
	}
	return(TRUE);
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : tga_read
 * FUNCTION :
 * INPUTS   : UNBYTE *destmem :
 *            SILONG bytestoread :
 * RESULT   : static SILONG :
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

static SILONG tga_read(UNBYTE *destmem,SILONG bytestoread)
{
	/* Aus Speicher lesen */
	if(tgaflag&LBMSTAT_GETFROMMEM){
		BASEMEM_CopyMem(tgamemptr, destmem, bytestoread);
		tgamemptr+=bytestoread;
	}
	/* von Device lesen */
	else{
		SILONG bytes;
		SILONG restbytes;

		/* Wird Åber das Ende des Files hinausgelesen ? */
		if(bytestoread>tgafilesize){
			return(-1);
		}

		/* Gesamtanzahl zu lesende Bytes */
		bytes=bytestoread;

		for(;;){

			restbytes=pufferendptr-pufferptr;

			/* Nicht mehr genÅgend Bytes im Puffer auffÅllen */
			if(restbytes<bytes){
				/* restliche Bytes umkopieren */
				BASEMEM_CopyMem(pufferptr, destmem,restbytes);
				destmem+=restbytes;
				bytes-=restbytes;
				pufferptr=pufferstartptr;
				/* Puffer wieder auffÅllen */
				if(tgafilesize>PUFFERSIZE){
					DOS_Read( tgafilehandle,pufferptr,PUFFERSIZE);
					tgafilesize-=PUFFERSIZE;
				}
				else{
					DOS_Read( tgafilehandle,pufferptr,tgafilesize);
					pufferendptr=pufferstartptr+tgafilesize;
				}
			}
			else{
				BASEMEM_CopyMem(pufferptr, destmem,bytes);
				pufferptr+=bytes;
				break;
			}

		}

	}

	return(bytestoread);

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LBM_ReadTGA
 * FUNCTION  : TGA Bild laden in ein OPM mit 2,3,4 Byte Pro Pixel
 * FILE      : BBLBM.C
 * AUTHOR    : R.Reber
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : UNBYTE * tganame: name of TGA
 *             struct OPM * opmptr: pointer to OPM to display LBM in.
 *						 flag = How to Use Given OPM (look in BBLBM.H)
 * RESULT    : TRUE = OK ,FALSE = Error
 * BUGS      :
 * NOTES     : Ein 8 KB gro·er Puffer um das Lesen zu beschleunigen wird
 *						 temporÑr alloziert
 *			LBMSTAT_USEOPM		0L				in Åbergebenes OPM direkt einschreiben
 * 			LBMSTAT_MAKENEWOPM	( 1L << 0L ) WORD PRO PIXEL OPM von der Grîsse des Bildes anlegen
 * 			LBMSTAT_MAKENEWOPM24 ( 1L << 2L )	3 BYTE pro Pixel OPM von der Grîsse des Bildes anlegen
 * 			LBMSTAT_MAKENEWOPM32 ( 1L << 2L )	Langwort pro Pixel OPM (R,G,B,Alpha) von der Grîsse des Bildes anlegen
 *			LBMSTAT_WRITESIZEOPM	( 1L << 1L )	Die Breite und Hîhe direkt in OPM schreiben
 *																				 vorhandenen OPMSpeicherPtr Åbernehmen
 *																				OPM muss mit OPM_New initialisiert sein
 *			LBMSTAT_GETFROMMEM ( 1L << 16L )  Statt eines Zeigers auf einen Namen
 *																			 wird ein Zeiger auf einen Speicherbereich
 *																				 Åbergeben und daraus gelesen
 *						 Achtung, lÑdt momentan gepackte TGA's noch nicht richtig
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN LBM_ReadTGA( UNBYTE * tganame, struct OPM * opmptr, UNLONG flag)
{
	SILONG ftype, idlen, cmlen, /*cmsiz,*/ psize, orien, compression_flag;
	SILONG width, height;
	SILONG row,column,ypos,yposadd;
	SILONG rle,i;
	UNBYTE readbyte;
	UNBYTE bytes[4],tgaheader[18];
	BOOLEAN fill_mode;
	IMAGE_COLOUR pixel;

/* lokale Variablen in globale kopieren */
//	tgafilename=tganame;
	tgaflag=flag;

/* TGA File îffnen falls nicht aus Speicher lesen */
	if(tgaflag&LBMSTAT_GETFROMMEM){
		tgamemptr=tganame;
	}
	else{
		if((tgafilehandle=DOS_Open( tganame, BBDOSFILESTAT_READ))==-1){
			LBM_ERROR("LBM_DisplayTGA: Cannot open Targa File",0);
			return(FALSE);
		}
		/* Funktion zur Bestimmung der FilelÑnge direkt aufrufen
			 um unnîtiges îffnen und Schlie·en zu vermeiden */
		if((tgafilelength=filelength( BBDOS_FileHandleArray[tgafilehandle].refNum)) == -1 ){
			LBM_ERROR("LBM_DisplayTGA: Cannot get Length of Targa File",0);
			DOS_Close(tgafilehandle);
			return(FALSE);
		}
		/* Speicher fÅr Puffer anlegen */
		if((pufferstartptr=BASEMEM_Alloc(PUFFERSIZE,BASEMEM_Status_Normal))==NULL){
			LBM_ERROR("LBM_DisplayTGA: Not enough Memory for ReadPuffer",PUFFERSIZE);
			DOS_Close(tgafilehandle);
			return(FALSE);
		}
		pufferendptr=pufferstartptr+PUFFERSIZE;
		pufferptr=pufferendptr;
		tgafilesize=tgafilelength;
	}

/* Header einlesen */
	if(tga_read(tgaheader,18)!=18){
		LBM_ERROR("LBM_DisplayTGA: Cannot read Targa File Header (18 Bytes) ",0);
		DOS_Close(tgafilehandle);
		return(FALSE);
	}

  /* Decipher the header information */
  idlen  = tgaheader[ 0];		     // LÑnge des Kommentars
  ftype  = tgaheader[ 2];		     // wenn !=1 Dann RGB
  cmlen  = tgaheader[ 5] + (tgaheader[ 6] << 8);
//  cmsiz  = tgaheader[ 7] / 8;
  width  = tgaheader[12] + (tgaheader[13] << 8);
  height = tgaheader[14] + (tgaheader[15] << 8);
  psize  = tgaheader[16] / 8; 	 // Anzahl der Bits bez. hier Bytes pro Pixel 16,24,32
  orien  = tgaheader[17] & 0x20; /* Right side up ? */

  /* Determine if this is a supported Targa type */
  if (ftype == 9 || ftype == 10)
    compression_flag = TRUE;
  else if (ftype == 1 || ftype == 2 || ftype == 3)
    compression_flag = FALSE;
	else
	{
		LBM_ERROR("LBM_DisplayTGA: unsupported Targa Type",0);
		DOS_Close(tgafilehandle);
		return(FALSE);
	}
  if (cmlen > 0){
		LBM_ERROR("LBM_DisplayTGA: unsupported Targa Type (Index Color Format)",0);
		DOS_Close(tgafilehandle);
		return(FALSE);
  }

	/* Skip over the picture ID information */
  if (idlen > 0 && tga_read(idbuf, idlen) != idlen){
		LBM_ERROR("LBM_DisplayTGA: error reading identification field",0);
		DOS_Close(tgafilehandle);
		return(FALSE);
	}

	/* OPM entsprechend der Grîsse des LBM's erstellen */
	if(tgaflag&LBMSTAT_MAKENEWOPM){
		/* Create OPM */
		if ( !OPM_New( width, height, 2, opmptr, NULL ) ){
			LBM_ERROR("LBM_DisplayTGA: Cannot Alloc Mem for OPM",0);
			DOS_Close(tgafilehandle);
			return(FALSE);
		}
	}
	/* OPM entsprechend der Grîsse des LBM's erstellen */
	else if(tgaflag&LBMSTAT_MAKENEWOPM24){
		/* Create OPM */
		if ( !OPM_New( width, height, 3, opmptr, NULL ) ){
			LBM_ERROR("LBM_DisplayTGA: Cannot Alloc Mem for OPM",0);
			DOS_Close(tgafilehandle);
			return(FALSE);
		}
	}
	/* OPM entsprechend der Grîsse des LBM's erstellen */
	else if(tgaflag&LBMSTAT_MAKENEWOPM32){
		/* Create OPM */
		if ( !OPM_New( width, height, 4, opmptr, NULL ) ){
			LBM_ERROR("LBM_DisplayTGA: Cannot Alloc Mem for OPM",0);
			DOS_Close(tgafilehandle);
			return(FALSE);
		}
	}
	/* Grîsse des Bildes in OPM eintragen (fÅr Speicherptr hochzÑhlen) */
	else if(tgaflag&LBMSTAT_WRITESIZEOPM){
		opmptr->width = width;
		opmptr->height  = height;
		opmptr->nextypos = opmptr->width;
		opmptr->datasize= opmptr->width*opmptr->height*opmptr->depth;
		opmptr->clipwidth=opmptr->width;
		opmptr->clipheight=opmptr->height;
	}

	/* Orientierung */
	if(orien==0)
	{
		ypos=height-1;
		yposadd=-1;
	}
	else
	{
		ypos=0;
		yposadd=1;
	}

	row=0; 		/* row counter */
  column=0; /* column counter */

	while (row < height)
	{
		if( compression_flag == TRUE )
		{
			if(tga_read(&readbyte,1)!=1){
				LBM_ERROR("LBM_DisplayTGA: compression_flag -> Premature EOF in Image file",0);
				DOS_Close(tgafilehandle);
				return(FALSE);
			}
			rle=(SILONG)readbyte;

			if(rle&0x80)
				fill_mode=TRUE;
			else
				fill_mode=FALSE;

			rle&=0x7f;
		}
		else
		{
			/* Daten unkomprimiert */
			rle=width*height;
			fill_mode=FALSE;
		}

		for(i=0;i<rle;i++)
		{
			/* je nach Mode nur beim ersten Mal FÅllwert laden, oder immer */
			if(fill_mode==FALSE || i==0)
			{
				if(tga_read(&bytes[0],psize)!=psize){
					LBM_ERROR("LBM_DisplayTGA: imagedata -> Premature EOF in Image file",0);
					DOS_Close(tgafilehandle);
					return(FALSE);
				}

				/* Pixel je nach Anzahl Bytes pro Pixel konvertieren */
				convert_targa_color(&pixel,psize,&bytes[0]);
			}

			/* Pixel im OPM setzen */
			TGA_SetPixel(opmptr,column,ypos,pixel.Red,pixel.Green,pixel.Blue);

			column++;
			if(column==width)
			{
				row++;
				ypos+=yposadd;
				column=0;
			}
		}
	} /* End while */

	if(!(tgaflag&LBMSTAT_GETFROMMEM)){
		/* File schlie·en */
		DOS_Close(tgafilehandle);
		/* Speicher fÅr Bildschirm mit maximal 800x600 anlegen */
		BASEMEM_Free(pufferstartptr);
	}

	return(TRUE);
}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LBM_WriteTGA
 * FUNCTION  :
 * FILE      : BBLBM.C
 * AUTHOR    : R.Reber
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : UNBYTE * tganame: name of TGA
 *             struct OPM * opmptr: pointer to OPM to display LBM in.
 *						 flag = momentan nicht unterstÅtzt
 * RESULT    : TRUE = OK ,FALSE = Error
 * BUGS      :
 * NOTES     : Ein 8 KB gro·er Puffer um das Lesen zu beschleunigen wird
 *						 temporÑr alloziert
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN LBM_WriteTGA( UNBYTE * tganame, struct OPM * opmptr, UNLONG flag)
{
	UNLONG width,height;
	SILONG x,y,i;
	UNBYTE tgaheader[18];

	width=opmptr->width;
	height=opmptr->height;


	/* Cipher the header information */
  for (i = 0; i < 12; i++)	/* 00, 00, 02, then 9 00's... */
		tgaheader[i]=0;

  /*ftype  =*/ tgaheader[ 2]=2;		     // wenn !=1 Dann RGB
  /*width  =*/ tgaheader[12]=width&0xff;
							 tgaheader[13]=width>>8;
  /*height =*/ tgaheader[14]=height&0xff;
							 tgaheader[15]=height>>8;
  /*psize  =*/ tgaheader[16]=24;	 	 	// Anzahl der Bits bez. hier Bytes pro Pixel 16,24,32
  /*orien  =*/ tgaheader[17]=0x20; 	 	/* Right side up ? */


	/* Speicher fÅr Puffer anlegen */
	if((pufferstartptr=BASEMEM_Alloc(PUFFERWRITESIZE,BASEMEM_Status_Normal))==NULL){
		LBM_ERROR("LBM_DisplayTGA: Not enough Memory for WritePuffer",0);
		return(FALSE);
	}
	pufferptr=pufferstartptr;
	pufferendptr=pufferstartptr+PUFFERWRITESIZE;

	if((tgafilehandle=DOS_Open( tganame, BBDOSFILESTAT_WRITE))==-1){
		LBM_ERROR("LBM_WriteTGA: Cannot open Targa File for writing",0);
		return(FALSE);
	}

	if(DOS_Write( tgafilehandle,tgaheader,18)!=18){
		LBM_ERROR("LBM_WriteTGA: Cannot Write TGA-Header",0);
		DOS_Close(tgafilehandle);
		return(FALSE);
	}

	/* TGA Speichern */
	for(y=0;y<(SILONG)height;y++){
		for(x=0;x<(SILONG)width;x++){
			TGA_GetPixel(opmptr,x,y,pufferptr,pufferptr+1,pufferptr+2);
			pufferptr+=3;
			if(pufferptr>=pufferendptr){
				if(DOS_Write( tgafilehandle,pufferstartptr,PUFFERWRITESIZE)!=PUFFERWRITESIZE){
					LBM_ERROR("LBM_WriteTGA: Cannot Write TGA-DATA",0);
					DOS_Close(tgafilehandle);
					return(FALSE);
				}
				pufferptr=pufferstartptr;
			}
		}
	}
	/* Restliche Bytes im Puffer schreiben */
	if(DOS_Write( tgafilehandle,pufferstartptr,pufferptr-pufferstartptr)!=(pufferptr-pufferstartptr)){
		LBM_ERROR("LBM_WriteTGA: Cannot Write TGA-DATA-Final",0);
		DOS_Close(tgafilehandle);
		return(FALSE);
	}

	/* File schlie·en und Speicher freigeben */
	DOS_Close(tgafilehandle);
	BASEMEM_Free(pufferstartptr);

	return(TRUE);
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME      : LBM_PrintError
 * FUNCTION  : Print error function for DOS library
 * INPUTS    : UNCHAR * buffer	:Pointer to string buffer.
 *             UNBYTE * data	:Pointer to error stack data area.
 * RESULT    : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#ifdef BBLBM_ERRORHANDLING

	void
	LBM_PrintError( UNCHAR * buffer, UNBYTE * data )
	{
		/* Local vars */
		struct BBLBM_ErrorStruct * LBMErrorStructPtr=( struct BBLBM_ErrorStruct * ) data;

		/* sprintf error message into string buffer */
		sprintf( ( char * ) buffer, " %s - DATA: %ld", ( char * ) LBMErrorStructPtr->errorname, LBMErrorStructPtr->errordata );
	}

#endif

/* #FUNCTION END# */

