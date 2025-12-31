/************
 * NAME     : dump.c
 * AUTOR    : VIPER, BlueByte
 * START    : 26.05.94 08:00
 * PROJECT  : Poject32/Macintosh
 * NOTES    : InitSystem for PCLIB32 inits all Libraries
 * SEE ALSO :
 * VERSION  : 1.0
 ************/

#include <stdio.h>

/* Includes */

#include <BBDEF.H>
#include <BBBASMEM.h>
#include <BBDOS.h>
#include <BBDSA.H>
#include <BBERROR.h>
#include <BBEVENT.h>
#include <BBOPM.h>
#include <BBSYSTEM.h>

/* Defines */
#define UMBRUCH 32 // ZeilenUmbruch nach x Werten


/* Global variables */


/***********************************************************/
/********************** Main PRG ***************************/
/***********************************************************/

void main ( int argc, char *argv[] )
{
	/*** Help ***/
	if (  ( argc == 2 ) && ( *( argv[1] ) == '?' ) ){
		printf("DUMP V1.1 by R.Reber (c) 1994 Blue Byte.\n");
		printf("Usage : DUMP binfile dumpfile !\n");
		return;
	}

	/*** How to use ***/
	if ( argc < 3 ){
		printf("Usage : DUMP binfile dumpfile !\n");
		return;
	}

	/* init necessary Libraries */

	/* Init BASEMEM system */
	if( !BASEMEM_Init() )
		return;

	/* Init DOS system */
	if( !DOS_Init() )
		return;

	{
		UNBYTE *src,*startdst,*dst;
		SILONG len,t;
		SILONG spalte,zeile;

		printf("\n Reading File: %s \n",argv[1]);

		/* Read Source File */
		if((src = DOS_ReadFile( argv[1], NULL, NULL ) ) == NULL ){
			printf( "Error Reading SourceFile !\n" );
			return;
		}

		/* Length of Source file */
		if((len=DOS_GetFileLength( argv[1] ))==-1){
			printf( "Error Length of SourceFile !\n" );
			return;
		}

		/* 5 fache L„nge des Orginalfiles erzeugen */
		if((dst=BASEMEM_Alloc((len*5),BASEMEM_Status_Normal))==NULL){
			printf( "Error Couldn't allocate Buffer !\n" );
			return;
		}

		printf("\n Generating C-Source Code ! \n\n");

		startdst=dst;

		//sprintf(dst,"/* Source generated with DUMP V1.1 by R.Reber */\n");
		//dst+=49;

		sprintf(dst,"{\r\n  ");
		dst+=4;
		/* Source Code generieren */
		spalte=0;zeile=0;
		for(t=0;t<len;t++){
			if(spalte++>UMBRUCH){
				spalte=0;
				sprintf(dst,"\r\n  ");
				dst+=3;
				printf(" Zeile: %5ld \b\b\b\b\b\b\b\b\b\b\b\b\b\b",++zeile);
			}
			sprintf(dst,"%3d,",*(src++));
			dst+=4;
		}
		sprintf(dst,"\r\n}");
		dst+=2;

		printf("\n\n Writing File: %s \n",argv[2]);

		/* Write Dest File */
		if(! DOS_WriteFile( argv[2], startdst, dst-startdst )){
			printf( "Error Writing DestFile !\n" );
			return;
		}

	}

	/* Exit DOS system */
	DOS_Exit();

	/* Exit BASEMEM system */
	BASEMEM_Exit();

	return;
}


