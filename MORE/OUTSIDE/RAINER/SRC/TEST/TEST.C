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
#include <PCLIB32.H>
#include "..\..\lib\ZOOMFUNC.H"
#include "FINDCOL.H"

#define ZOOMADD 2

/***********************************************************/
/********************** Main PRG ***************************/
/***********************************************************/

static UNBYTE recolortable[256];
static void Calculate_transparency_table(UNBYTE *memptr,UNSHORT Intensity,struct BBPALETTE *palette);

void bbmain ( void )
{
	struct BBPALETTE pal;
	struct SCREENPORT scrp;
	struct OPM myprivateopm;
	struct OPM backopm;
	struct OPM zoomopm;
	struct BBPOINT pnt;
	UNLONG dwidth,dheight;
	SILONG alignx,aligny;
	SILONG s,t;
	UNBYTE *transtable;

	/* Main OPM mit Grîsse 360x240 1 Byte pro Pixel generieren */
	if ( !OPM_New( 360, 240, 1, &myprivateopm, NULL ) ){
		printf( "\n\nKonnte OPM nicht generieren !!!\n\n" );
		return;
	}

	/* Bildschirm im CHAIN4 Modus mit Doublebuffer aufmachen */
	if(!DSA_OpenScreen( &scrp, &myprivateopm, NULL, NULL, 0,0,SCREENTYPE_DOUBLEBUFFER)){
		printf("DSA Screen Konnte nicht geîffnet werden! \n");
		return;
	}

	/* Mauszeiger darstellen Achtung !!! setzt Farbe 0 auf Schwartz und 255 auf weiss */
	SYSTEM_ShowMousePtr(SYSTEM_MousePointerNormal);

	if(!LBM_DisplayLBM("GFX\\back.lbm", &backopm , &pal, LBMSTAT_MAKENEWOPM)){
		printf("CHAR konnte nicht gefunden werden \n");
		return;
	}

	/* Palette fÅr Bildschirm setzen */
	DSA_SetPal( &scrp, &pal, 0, 256, 0);

	/* aktivieren */
	DSA_ActivatePal( &scrp );

	if(!LBM_DisplayLBM("GFX\\test.BBM", &zoomopm , NULL, LBMSTAT_MAKENEWOPM)){
		printf("CHAR konnte nicht gefunden werden \n");
		return;
	}

#if FALSE
	OPM_Del(&zoomopm);
	if ( !OPM_New( 54, 21, 1, &zoomopm, NULL ) ){
		printf( "\n\nKonnte OPM nicht generieren !!!\n\n" );
		return;
	}
	OPM_Box( &zoomopm, 0, 0, zoomopm.width, zoomopm.height, 31);
#endif

/* RecolorTable erstellen */
	for(s=0,t=0;t<256;t++){
		recolortable[t]=s;
		if(s++>7){
			s=0;
		}
	}

/* Speicher fÅr TransparentTabelle belegen */
	if((transtable=BASEMEM_Alloc(65536,BASEMEM_Status_Normal))==NULL){
		printf( "\n\nKonnte SpeicherOPM nicht generieren !!!\n\n" );
		return;
	}

/* TransparentTable erstellen */
	Calculate_transparency_table(transtable,50,&pal);

	dwidth=zoomopm.width;
	dheight=zoomopm.height;

	{
		int leavelop=TRUE;
		int draw=FALSE;

		do
		{
			struct BLEV_Event_struct event;

			/* Bildschirme umschalten */
			DSA_DoubleBuffer();

			/* Events generieren ( Auf Pc im Prinzip unnîtig da Events im Interrupt generiert werden ) */
			SYSTEM_SystemTask();

			/* Mausposition auslesen */
			BLEV_GetMousePos(&pnt);

			if( BLEV_GetEvent( &event ) )
			{
				switch( event.sl_eventtype )
				{
					/* linke Maustaste losgelassen */
					case BLEV_MOUSELSDOWN:
					{
						draw=TRUE;
						break;
					}
					/* linke Maustaste gedrÅckt */
					case BLEV_MOUSELSUP:
					{
						draw=FALSE;
						break;
					}
					case BLEV_KEYDOWN:
					{
						if(event.sl_key_code==BLEV_ESC){
	 						leavelop=FALSE;
						}
						else if(event.sl_key_code=='+'){
							dwidth+=ZOOMADD;
							dheight+=ZOOMADD;
						}
						else if(event.sl_key_code=='-'){
							dwidth-=ZOOMADD;
							dheight-=ZOOMADD;
						}
						break;
					}

				}
			}

			/* Hintergrund lîschen */
			OPM_CopyOPMOPM( &backopm, &myprivateopm,0,0,backopm.width,backopm.height,0,0);

			/* Virtuelles OPM auf Bildschirm zeichnen */
			if(draw){
				alignx=(dwidth/2);
				aligny=(dheight/2);
//				alignx=0;
//				aligny=0;
				Put_zoomed_block(&myprivateopm,pnt.x-alignx-(dwidth*2),pnt.y-aligny,zoomopm.width,zoomopm.height,dwidth,dheight,zoomopm.data);
				Put_zoomed_silhouette(&myprivateopm,pnt.x-alignx-(dwidth*1),pnt.y-aligny,zoomopm.width,zoomopm.height,dwidth,dheight,zoomopm.data,4);
				Put_zoomed_recoloured_silhouette(&myprivateopm,pnt.x-alignx,pnt.y-aligny,zoomopm.width,zoomopm.height,dwidth,dheight,zoomopm.data,&recolortable[0]);
				Put_zoomed_recoloured_block(&myprivateopm,pnt.x-alignx+(dwidth*1),pnt.y-aligny,zoomopm.width,zoomopm.height,dwidth,dheight,zoomopm.data,&recolortable[0]);
				Put_zoomed_transparent_block(&myprivateopm,pnt.x-alignx+(dwidth*2),pnt.y-aligny,zoomopm.width,zoomopm.height,dwidth,dheight,zoomopm.data,transtable);
			}

			DSA_CopyMainOPMToScreen( &scrp,DSA_CMOPMTS_ALWAYS );

		}
		while ( leavelop );
	}

	/* Speicher freigeben fÅr TransparentTable */
	BASEMEM_Free(transtable);

	/* OPMs freigeben */
	OPM_Del(&zoomopm);
	OPM_Del(&backopm);

	/* Bildschirm wieder schliessen */
	DSA_CloseScreen(&scrp);

	/* OPM freigeben */
	OPM_Del(&myprivateopm);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_transparency_table
 * FUNCTION  : Calculate a transparency table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 15:30
 * LAST      : 07.03.95 15:30
 * INPUTS    : UNSHORT Intensity - Intensity in %.
 * RESULT    : MEM_HANDLE : Handle of transparency table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void Calculate_transparency_table(UNBYTE *memptr,UNSHORT Intensity,struct BBPALETTE *palette)
{
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* find_color Initialisieren */
	Prepare_colour_find(palette, 0,255);

	/* Yes -> Calculate the transparency  table */
	Ptr = memptr;

	for (i=0;i<256;i++)
	{
		/* Get the current target colour */
		Target_R = palette->color[i].red;
		Target_G = palette->color[i].green;
		Target_B = palette->color[i].blue;

		for (j=0;j<256;j++)
		{
			/* Get the current source colour */
			Source_R = palette->color[j].red;
			Source_G = palette->color[j].green;
			Source_B = palette->color[j].blue;

			/* Interpolate towards the target colour */
			Source_R = ((Target_R - Source_R) * (100 - Intensity)) / 100 + Source_R;
			Source_G = ((Target_G - Source_G) * (100 - Intensity)) / 100 + Source_G;
			Source_B = ((Target_B - Source_B) * (100 - Intensity)) / 100 + Source_B;

			/* Find the closest matching colour in the palette */
			*(Ptr++) = Find_closest_colour(Source_R, Source_G, Source_B);
		}
	}
}

