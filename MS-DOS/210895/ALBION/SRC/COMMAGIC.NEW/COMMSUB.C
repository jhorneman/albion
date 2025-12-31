/************
 * NAME     : COMMSUB.C
 * AUTOR    : Rainer Reber, BlueByte
 * START    : 24-4-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMMSUB.H
 ************/

/* includes */

#include <stdlib.h>
#include <math.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>
#include <BBERROR.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <GAMETEXT.H>
#include <MAGIC.H>
#include <MAP.H>
#include <EVELOGIC.H>
#include <COMBAT.H>
#include <COMACTS.H>
#include <COMOBS.H>
#include <COMMAGIC.H>
#include <COMMSUB.H>
#include <STATAREA.H>
#include <ITEMLIST.H>
#include <ITMLOGIC.H>
#include <TACTICAL.H>
#include <XFTYPES.H>

#include <FINDCOL.H>

/* global variables */
struct Combat_participant *Current_target_parts[18 + 6];
UNSHORT Current_nr_target_parts;

/******************************************************************************
 ******************************************************************************
 Unterroutinen
 ******************************************************************************
 ******************************************************************************/

/*******************************************************************************
	Hilfsroutinen
*******************************************************************************/


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : Gen_COMOB
 * FUNCTION : erzeugt ein COMOB mit StandartWerten (Hotspot ist in der Mitte)
 * INPUTS   : SILONG x : X,Y,Z Position
 *            SILONG y :
 *            SILONG z :
 *            SILONG life : LifeSpan
 *            SILONG size : Grî·e (fÅr x und y )
 *            MEM_HANDLE gfx : GraphicHandle
 *            SILONG flag : flag fÅr Zeichenmodus
 *													GC_NORM = normales COMOB
 *													GC_TRANS = Transparentes COMOB
 *													GC_FIRE = Luminantes COMOB fÅr FeuerEffekte
 *													GC_MAGIC = TransLuminant fÅr magisch leuchtend
 * RESULT   : struct COMOB *: Zeiger auf COMOB oder NULL wenn Fehler
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct COMOB *Gen_COMOB(SILONG x,SILONG y,SILONG z,SILONG life,SILONG size,MEM_HANDLE gfx,SILONG flag)
{
	struct COMOB *comob;

	/* Add comob */
	comob = Add_COMOB(100);

	/* Success ? */
	if (!comob){
		ERROR_ClearStack();
		return(NULL);
	}

	/* Copy coordinates from firering and add vector */
	comob->X_3D = x;
	comob->Y_3D = y;
	comob->Z_3D = z;

	/* Set random lifespan */
	comob->Lifespan = life;

	/* Set display parameters */
	if(flag&GC_NORM){
		comob->Draw_mode = NORMAL_COMOB_DRAWMODE;
	}
	else if(flag&GC_TRANS){
		comob->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		comob->Special_handle = Transparency_table_handle;
	}
	else if(flag&GC_FIRE){
		comob->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		comob->Special_handle = Luminance_table_handle;
	}
	else if(flag&GC_MAGIC){
		comob->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		comob->Special_handle = Transluminance_table_handle;
	}

	/* Hotspot in der Mitte */
	comob->Hotspot_X_offset = 50;
	comob->Hotspot_Y_offset = 50;

	/* Set random size */
	comob->Display_width = size;
	comob->Display_height = size;

	/* Select gfx type */
	comob->Graphics_handle = gfx;

	/* Set number of animation frames */
	comob->Nr_frames = Get_nr_frames(comob->Graphics_handle);

	/* Select animation frame */
	comob->Frame = 0;

	return(comob);
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME     : ip3d
 * FUNCTION : Wert interpolieren
 * FILE     : D:\ALBION\GAME\COMMAGIC.C
 * AUTHOR   : Rainer Reber
 * FIRST    : 20.06.95 17:08:16
 * LAST     : 20.06.95 17:08:16
 * INPUTS   : SILONG val0 : AnfangsWert
 *            SILONG val1 : EndWert
 *            SILONG anz : Anzahl Interpolationsstufen
 *            SILONG intval : aktueller Interpolierungswert
 * RESULT   : Interpolierter Wert
 * BUGS     :
 * NOTES    :
 * VERSION  : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG ip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval)
{
	return((((val1-val0)*intval)/anz)+val0);
}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME     : aip3d
 * FUNCTION : Wert mit zunehmender Beschleunigung interpolieren
 * FILE     : D:\ALBION\GAME\COMMAGIC.C
 * AUTHOR   : Rainer Reber
 * FIRST    : 20.06.95 17:08:16
 * LAST     : 20.06.95 17:08:16
 * INPUTS   : SILONG val0 : AnfangsWert
 *            SILONG val1 : EndWert
 *            SILONG anz : Anzahl Interpolationsstufen
 *            SILONG intval : aktueller Interpolierungswert
 * RESULT   : Interpolierter Wert
 * BUGS     :
 * NOTES    : Routine lÑuft momentan intval mal durch eine Schleife mit
 *						(Deswegen sollte intval nicht all zu gro· werden
 * VERSION  : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG aip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval)
{
	SILONG t,ges,bf,a;

	/* BeschleunigunsFaktor ausrechnen */
	bf=0;
	ges=(val1-val0);
	a=((2*ges)*128)/(anz*anz);

	/* Beschleunigung bis zum entsprechenden Wert ausrechnen */
	for(t=0;t<intval;t++){
		val0=val0+(bf/128);
		bf=bf+a;
	}

	return(val0);
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME     : dip3d
 * FUNCTION : Wert mit abnehmender Beschleunigung interpolieren
 * FILE     : D:\ALBION\GAME\COMMAGIC.C
 * AUTHOR   : Rainer Reber
 * FIRST    : 20.06.95 17:08:16
 * LAST     : 20.06.95 17:08:16
 * INPUTS   : SILONG val0 : AnfangsWert
 *            SILONG val1 : EndWert
 *            SILONG anz : Anzahl Interpolationsstufen
 *            SILONG intval : aktueller Interpolierungswert
 * RESULT   : Interpolierter Wert
 * BUGS     :
 * NOTES    : Routine lÑuft momentan intval mal durch eine Schleife mit
 *						(Deswegen sollte intval nicht all zu gro· werden
 * VERSION  : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG dip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval)
{
	SILONG t,ges,bf,a;

	/* BeschleunigunsFaktor ausrechnen */
	ges=(val1-val0);
	a=((2*ges)*128)/(anz*anz);
	bf=a*anz;

	/* Beschleunigung bis zum entsprechenden Wert ausrechnen */
	for(t=0;t<intval;t++){
		val0=val0+(bf/128);
		bf=bf-a;
	}

	return(val0);
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : calc_strength
 * FUNCTION : Berechnet AbhÑngig von der Åbergebenen StÑrke (0-100%) den
 *						entsprechend interpolierten Wert zwischen
 * INPUTS   : Strength : StÑrke von 0 - 100 %
 *            min : minimaler Wert
 *            max : Maximaler Wert
 *						min kann auch grî·er sein als max. Der Algortihmus wird dann
 *						entsprechend umgestellt.
 * RESULT   : SILONG : berechneter Interpolatiosnwert
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG calc_strength(SILONG strength,SILONG min,SILONG max)
{
	SILONG ret;

	if(max>min){
		ret=((strength*(max-min))/100)+min;
	}
	else{
		ret=max-((strength*(min-max))/100);
	}

	return(ret);
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : calc_delta_xyz
 * FUNCTION : Anzahl Schritte,sowie delta-move x,y,z Werte ausrechen die nîtig
 *						sind um mit Abstand ab zx,zy,zz zu Erreichen
 *					  erreichen.
 *						fÅr *dx,*dy,*dz kann auch NULL angegeben werden dann wird dieser
 *						DeltaWert nicht berechnet
 * INPUTS   : SILONG sx,sy,sz : Start Koordinaate
 *            SILONG zx,zy,zz :  End Koordinaate
 *						SILONG *dx,*dy,*dz: daraus berechnete DeltaWert
 *            SILONG ab :  Abstand
 * RESULT   : SILONG : Anzahl Schritte
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG calc_delta_xyz(SILONG sx,SILONG sy,SILONG sz,
											SILONG zx,SILONG zy,SILONG zz,
											SILONG *dx,SILONG *dy,SILONG *dz,
											SILONG ab)
{
	SILONG veklen,steps,xlen,ylen,zlen;

	xlen=zx-sx;
	ylen=zy-sy;
	zlen=zz-sz;

	if(xlen==0&&ylen==0&&zlen==0){
		*dx=0;
		*dy=0;
		*dz=0;
		return(0);
	}

	veklen=(SILONG)sqrt(((double)xlen*(double)xlen)+((double)ylen*(double)ylen)+((double)zlen*(double)zlen));
	steps=veklen/ab;

	/* Anzahl Schritte ist >0 */
	if(steps!=0){
		if(dx!=NULL)
			*dx=xlen/steps;
		if(dy!=NULL)
			*dy=ylen/steps;
		if(dz!=NULL)
			*dz=zlen/steps;
	}
	else{
		if(dx!=NULL)
			*dx=0;
		if(dy!=NULL)
			*dy=0;
		if(dz!=NULL)
			*dz=0;
	}

	return(steps);
}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : rndmm
 * FUNCTION : Liefert eine ZufallsZahl zwischen min und max
 * INPUTS   : min : kleinste Zufallszahl einschlie·lich
 *            max : grî·te Zufallszahl einschlie·lich
 * RESULT   : SILONG : Zufallszahl
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG rndmm(SILONG min,SILONG max)
{
	SILONG rnd;

	rnd=rand() % ((max+1)-min);
//	rnd=(((SILONG)rand()+SYSTEM_GetTicks())) % ((max+1)-min);

	return(rnd+min);
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : rndmm2
 * FUNCTION : Liefert eine ZufallsZahl zwischen min und max oder minii und maxii
 * INPUTS   : min : kleinste Zufallszahl einschlie·lich
 *            max : grî·te Zufallszahl einschlie·lich
 *									oder
 * 						minii : kleinste Zufallszahl einschlie·lich
 *            maxii : grî·te Zufallszahl einschlie·lich
 * RESULT   : SILONG : Zufallszahl
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG rndmm2(SILONG min,SILONG max,SILONG minii,SILONG maxii)
{
	SILONG rnd;

	/* Change liegt bei 50 */
	if(rand()<16384){
		rnd=rand() % ((max+1)-min);
		return(rnd+min);
	}
	else{
		rnd=rand() % ((maxii+1)-minii);
		return(rnd+minii);
	}

}

/* #FUNCTION END# */



/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : update_n_times
 * FUNCTION : Screen fÅr eine Gewisse Zeit updaten
 * INPUTS   : SILONG t : Zeitdauer fÅr Screenupdate bzw wie lange der Effekt
 *											 anhalten soll
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void update_n_times(SILONG t)
{
	/* 1 mal auf jeden Fall updaten */
	Update_screen();
	t-=Nr_combat_updates;

	/* Wenn noch Zeit Åbrig ist weiter updaten */
	while(t>0){
		Update_screen();
		t-=Nr_combat_updates;
	}
}

/* #FUNCTION END# */


/*******************************************************************************
	Spezialroutinen
*******************************************************************************/


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_halo_to_COMOB_2
 * FUNCTION  : Add a halo to a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.06.95 13:50
 * LAST      : 21.06.95 13:50
 * INPUTS    : struct COMOB *Source_COMOB - Pointer to source COMOB.
 *             UNSHORT Nr_halo_COMOBs - Number of COMOBs in halo.
 *             UNSHORT Halo_width - Width of halo (in %).
 *             UNSHORT Halo_lifespan - Lifespan of halo.
 *						 UNSHORT flag = HALO_NULL = keine besonderen Verhalten
 *														HALO_CONNECT = Position mit SourceCOMOB verknÅpfen wenn gesetzt
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define MAX_ADD_HALO_COMOBS 16

static struct COMOB *add_halo_comobs[MAX_ADD_HALO_COMOBS];

struct COMOB **Add_halo_to_COMOB_2(struct COMOB *Source_COMOB, UNSHORT Nr_halo_COMOBs,
 UNSHORT Halo_width, UNSHORT Halo_lifespan,UNSHORT flag)
{
	struct COMOB_behaviour *behave;
	struct COMOB *base_COMOB;
	struct COMOB *Halo_COMOB;
	SILONG Left_3D, Top_3D, Right_3D, Bottom_3D;
	UNSHORT Delta_width, Delta_height;
	UNSHORT i;

	/* Source COMOB is NULL / number of COMOBs in the halo is zero /
	 halo width is zero ? */
	if (!Source_COMOB || !Nr_halo_COMOBs || !Halo_width || Nr_halo_COMOBs>MAX_ADD_HALO_COMOBS)
	{
		/* Yes -> Exit */
		return(NULL);
	}

	/* Calculate width and height deltas */
	Delta_width = ((Source_COMOB->Display_width * Halo_width) / 100) /
	 Nr_halo_COMOBs;
	Delta_height = ((Source_COMOB->Display_height * Halo_width) / 100) /
	 Nr_halo_COMOBs;

	/* Quell-COMOB kopieren */
	base_COMOB=Source_COMOB;

	for (i=0;i<Nr_halo_COMOBs;i++)
	{
		/* Duplicate source COMOB */
		Halo_COMOB = Duplicate_COMOB(Source_COMOB);
		if (!Halo_COMOB)
			return(NULL);

		/* Copy display dimensions, but bigger */
		Halo_COMOB->Display_width = Source_COMOB->Display_width +
		 Delta_width;
		Halo_COMOB->Display_height = Source_COMOB->Display_height +
		 Delta_height;

		/* Copy Z-coordinate, but "behinder" */
		Halo_COMOB->Z_3D = Source_COMOB->Z_3D + 1;

		/* First halo COMOB ? */
		if (!i)
		{
			/* Yes -> Set halo COMOB draw mode */
			Halo_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			Halo_COMOB->Special_handle = Transluminance_table_handle;
			Halo_COMOB->Special_offset = 0;

			/* Set halo lifespan */
			Halo_COMOB->Lifespan = Halo_lifespan;

			/* Get rectangle covering source COMOB */
			Get_COMOB_rectangle(Source_COMOB, &Left_3D, &Top_3D, &Right_3D,
			 &Bottom_3D);

			/* Set halo COMOB coordinates in the middle of this rectangle */
			Halo_COMOB->X_3D = Left_3D + (Right_3D - Left_3D) / 2;
			Halo_COMOB->Y_3D = Bottom_3D + (Top_3D - Bottom_3D) / 2;

			/* Set hotspot */
			Halo_COMOB->Hotspot_X_offset = 50;
			Halo_COMOB->Hotspot_Y_offset = 50;

		}

		/* Mit UrsprungsCOMOB direct verbinden */
		if(flag&HALO_CONNECT){
			/* mit dem BlitzverknÅpfen*/
			if(!(behave = Add_COMOB_behaviour(Halo_COMOB, base_COMOB, connect_handler))){
				ERROR_ClearStack();
				return(NULL);
			}
			/* Set random bounce behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+2+4;
			behave->Data.Just_data_w.Data[1] = Halo_COMOB->X_3D-base_COMOB->X_3D;
			behave->Data.Just_data_w.Data[2] = Halo_COMOB->Y_3D-base_COMOB->Y_3D;
			behave->Data.Just_data_w.Data[3] = Halo_COMOB->Z_3D-base_COMOB->Z_3D;
		}

		add_halo_comobs[i]=Halo_COMOB;

		/* Next halo COMOB */
		Source_COMOB = Halo_COMOB;
	}

	/* Zeiger auf HaloListe */
	return(&add_halo_comobs[0]);
}

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : Gen_Funken
 * FUNCTION : Generiert Funken an 3d Position mit grî·tenteils random Parametern
 *						Comobs verschwinden automatisch nach einiger Zeit
 * INPUTS   : SILONG x : X Position
 *            SILONG y : Y   ""
 *            SILONG z : Z   ""
 *            MEM_HANDLE objs : Zeiger auf entsprechendes Object fÅr Funken
 *						SILONG anz: Anzahl Funken
 * RESULT   : True= Ok / FALSE = nicht genug Speicher fÅr alle Objecte
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN Gen_Funken(SILONG x,SILONG y,SILONG z,MEM_HANDLE obj,SILONG anz)
{
	struct COMOB *COMOB;
	struct COMOB_behaviour *Behaviour_data;
	struct COMOB_behaviour *behave_anim;
	SILONG i;

	for (i=0;i<anz;i++)
	{
		/* Add COMOB */
		COMOB = Add_COMOB(25 + (rand() % 50));

		/* Success ? */
		if (!COMOB){
			ERROR_ClearStack();
			return(FALSE);
		}

		/* Calculate random movement vector */
		COMOB->dX_3D = (rand() % (400)) -
			COMOB_DEC_FACTOR;
		COMOB->dY_3D = (rand() % (400)) -
			COMOB_DEC_FACTOR;
		COMOB->dZ_3D = (rand() % (400)) -
			COMOB_DEC_FACTOR;

		/* Copy coordinates from firering and add vector */
		COMOB->X_3D = x;
		COMOB->Y_3D = y;
		COMOB->Z_3D = z;

		/* Set random lifespan */
		COMOB->Lifespan = 0;//20 + (rand() % 10);

		/* Set display parameters */
		COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		COMOB->Special_handle = Luminance_table_handle;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		/* Set random size */
		COMOB->Display_width = 40 + (rand() % 20);
		COMOB->Display_height = COMOB->Display_width;

		/* Select random spark type */
		COMOB->Graphics_handle = obj;

		/* Select random animation frame */
		COMOB->Frame = 0;

		/* Add bounce behaviour */
		Behaviour_data = Add_COMOB_behaviour(COMOB, NULL, Bounce_handler);

		/* Set random bounce behaviour data */
		Behaviour_data->Data.Bounce_data.Gravity = 10 + rand() % 5;
		Behaviour_data->Data.Bounce_data.Bounce = 240;
		Behaviour_data->Data.Bounce_data.Air_friction = 120;

		behave_anim = Add_COMOB_behaviour(COMOB, NULL, Animcontrol_handler);

		if(!behave_anim){
			/* Comob lîschen */
			Delete_COMOB(COMOB);
			ERROR_ClearStack();
			return(FALSE);
		}

		behave_anim->Data.Animcontrol_data.Nr_frames=Get_nr_frames(COMOB->Graphics_handle);
		behave_anim->Data.Animcontrol_data.Nr_repeats=1;
		behave_anim->Data.Animcontrol_data.Duration=20;

	}

	return(TRUE);
}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : Gen_Sparks
 * FUNCTION : amount Funken entstehen an entsprechender Stelle fallen
 *						auf den Boden und prallen ab
 * INPUTS   : SILONG x : Ursprungskoordinaaten
 *            SILONG y :
 *            SILONG z :
 *            SILONG amount : Anzahl
 *						SILONG speed : GEschwindigkeit (varriert von 10 - speed)
 *						SILONG life : LifeSpan (varriert von lifespan - Lifespan*1.5)
 *						SILONG size : Grî·e in % (varriert von Grî·e - Grî·e*1.5)
 *						SILONG sparktyp: momentan mîglich
 *							GEN_SPARK_TYP_BLUE = Blaue Sparks
 *							GEN_SPARK_TYP_ORANGE = Orange Sparks
 *							GEN_SPARK_TYP_ALL = Alle Sparks
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void Gen_Sparks(SILONG x,SILONG y,SILONG z,SILONG amount,SILONG speed,SILONG life,SILONG size,SILONG sparktyp)
{
	struct COMOB *COMOB;
	struct COMOB_behaviour *Behaviour_data;
	SILONG i;

	/* Generate sparks */
	for (i=0;i<amount;i++)
	{
		/* Add spark COMOB */
		COMOB = Add_COMOB(25 + (rand() % 50));

		/* Success ? */
		if (!COMOB)
		{
			/* No -> Clear error stack */
			ERROR_ClearStack();

			/* Stop generating more sparks */
			break;
		}

		/* Calculate random movement vector */
		COMOB->dX_3D = rndmm2(-speed,-10,10,speed);
		COMOB->dY_3D = rndmm2(-speed/2,5,20,speed*2);
		COMOB->dZ_3D = rndmm2(-speed,-10,10,speed);

		/* Copy coordinates from firering and add vector */
		COMOB->X_3D = x;
		COMOB->Y_3D = y;
		COMOB->Z_3D = z;

		/* Set random lifespan */
		COMOB->Lifespan = rndmm(life,life+(life/2));

		/* Set display parameters */
		COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		COMOB->Special_handle = Transluminance_table_handle;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		/* Set random size */
		COMOB->Display_width = rndmm(size,size+(size/2));
		COMOB->Display_height = COMOB->Display_width;

		/* Select random spark type */
		if(sparktyp==GEN_SPARK_TYP_BLUE){
			COMOB->Graphics_handle = Spark_gfx_handles[rndmm(0,2)];
		}
		else if(sparktyp==GEN_SPARK_TYP_ORANGE){
			COMOB->Graphics_handle = Spark_gfx_handles[rndmm(6,7)];
		}
		else{
			COMOB->Graphics_handle = Spark_gfx_handles[rndmm(0,7)];
		}

		/* Set number of animation frames */
		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

		/* Select random animation frame */
		COMOB->Frame = rand() % COMOB->Nr_frames;

		/* Add bounce behaviour */
		Behaviour_data = Add_COMOB_behaviour(COMOB, NULL, Bounce_handler);

		/* Set random bounce behaviour data */
		Behaviour_data->Data.Bounce_data.Gravity = 10 + rand() % 5;
		Behaviour_data->Data.Bounce_data.Bounce = 60;
		Behaviour_data->Data.Bounce_data.Air_friction = 0;

	}

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : groundglow
 * FUNCTION : Generiert BodenGlÅhen an entsprechender Stelle das BodenglÅhen
 *						erscheint kurze Zeit und verschwindet dann wieder
 * INPUTS   : SILONG x : X Koordinaate
 *            SILONG y :
 *            SILONG z :
 * RESULT   : BOOLEAN	TRUE=OK,FALSE = Object konnte nicht generiert werden
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN	groundglow(SILONG x,SILONG z,SILONG size,SILONG duration)
{
	struct COMOB *COMOB;
	struct COMOB_behaviour *behave_anim;
	MEM_HANDLE groundglow_handle = NULL;

	/* Load graphics */
	groundglow_handle = Load_subfile(COMBAT_GFX, 37);
	if (!groundglow_handle){
		ERROR_ClearStack();
		return(FALSE);
	}

	/* Add COMOB */
	COMOB = Add_COMOB(100);

	/* Success ? */
	if (!COMOB){
		/* No -> Clear error stack */
		ERROR_ClearStack();
		return(FALSE);
	}

	/* set coordinates */
	COMOB->X_3D = x;
	COMOB->Y_3D = 0;
	COMOB->Z_3D = z;

	/* Set lifespan */
	COMOB->Lifespan = 0 ;

	/* Set display parameters */
	COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	COMOB->Special_handle = Luminance_table_handle;

	COMOB->Hotspot_X_offset = 50;
	COMOB->Hotspot_Y_offset = 100;

	/* Set random size */
	COMOB->Display_width = size;
	COMOB->Display_height = size;

	/* Select random spark type */
	COMOB->Graphics_handle = groundglow_handle;

	/* Set number of animation frames */
//	COMOB->Nr_frames = 0;//Get_nr_frames(COMOB->Graphics_handle);

	/* Select animation frame */
	COMOB->Frame = 0 ;

	behave_anim = Add_COMOB_behaviour(COMOB, NULL, Animcontrol_handler);

	if(!behave_anim){
		/* Comob lîschen */
		Delete_COMOB(COMOB);
		ERROR_ClearStack();
		return(FALSE);
	}

	behave_anim->Data.Animcontrol_data.Nr_frames=Get_nr_frames(COMOB->Graphics_handle);
	behave_anim->Data.Animcontrol_data.Nr_repeats=1;
	behave_anim->Data.Animcontrol_data.Duration=duration;

	return(TRUE);
}

/* #FUNCTION END# */



/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : makeflame
 * FUNCTION : Flame an entsprechender Koordinaate erstellen die Flamme
 *						erscheint eine gewisse Zeit, wird dabei kleiner und verschwindet
 *					  schlie·lich
 * INPUTS   : SILONG x : X Koordinate
 *            SILONG y : Y    ""
 *            SILONG z : Z    ""
 *						SILONG size: Flammengrî·e in %
 * RESULT   : Ptr to generated COMOB
 *						NULL = ERROR
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct COMOB *makeflame(SILONG x,SILONG y,SILONG z,SILONG flamesize)
{
	struct COMOB *COMOB;
	struct COMOB_behaviour *behave_size;
	MEM_HANDLE flame_handle = NULL;
	SILONG *ldata;

	/* Load Flame graphics */
	flame_handle = Load_subfile(COMBAT_GFX, 54);
	if (!flame_handle){
		ERROR_ClearStack();
		return(FALSE);
	}

	/* Add COMOB */
	COMOB = Add_COMOB(100);

	/* Success ? */
	if (!COMOB){
		ERROR_ClearStack();
		return(NULL);
	}

	/* set coordinates */
	COMOB->X_3D = x;
	COMOB->Y_3D = y;
	COMOB->Z_3D = z;

	/* Set lifespan */
	COMOB->Lifespan = 0 ;

	/* mem handle danach wieder freigeben */
	COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

	/* Set display parameters */
	COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	COMOB->Special_handle = Luminance_table_handle;

	COMOB->Hotspot_X_offset = 50;
	COMOB->Hotspot_Y_offset = 85;

	/* Set random size */
	COMOB->Display_width = flamesize;
	COMOB->Display_height = flamesize;

	/* Select random spark type */
	COMOB->Graphics_handle = flame_handle;

	/* Set number of animation frames */
	COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

	/* Select animation frame */
	COMOB->Frame = 0 ;

	/* Behaviour fÅr Grî0e */
	behave_size = Add_COMOB_behaviour(COMOB, NULL, size_handler);

	/* Grî·e Ñndern */
	if(!behave_size){
		/* Comob lîschen */
		Delete_COMOB(COMOB);
		ERROR_ClearStack();
		return(NULL);
	}

	behave_size->Data.Just_data_w.Data[0]=-100*flamesize/100; /* jeden 4 Tick (100/4) */
	behave_size->Data.Just_data_w.Data[1]=-100*flamesize/100;
	behave_size->Data.Just_data_w.Data[2]=25*flamesize/100; /* bis 25 % */
	behave_size->Data.Just_data_w.Data[3]=25*flamesize/100;

	return(COMOB);
}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : gen_Dummy_COMOB
 * FUNCTION : generiert ein DUMMY COMOB
 * INPUTS   :  :
 * RESULT   : Zeiger auf COMOB oder NULL als Fehler.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct COMOB *gen_Dummy_COMOB()
{
	struct COMOB *COMOB;

	/* Add COMOB */
	COMOB = Add_COMOB(100);

	/* Success ? */
	if (!COMOB){
		ERROR_ClearStack();
		return(NULL);
	}

	/* set coordinates */
	COMOB->X_3D = 0;
	COMOB->Y_3D = 0;
	COMOB->Z_3D = 0;

	/* Set lifespan */
	COMOB->Lifespan = 0 ;

	/* Set display parameters */
	COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

	COMOB->Hotspot_X_offset = 50;
	COMOB->Hotspot_Y_offset = 50;

	COMOB->Display_width = 100;
	COMOB->Display_height = 100;

	/* irgendein dummes Spark als Grafik verwenden */
	COMOB->Graphics_handle = Spark_gfx_handles[0];

	/* Set number of animation frames */
	COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

	COMOB->Frame = 0;

	/* COMOB verstecken */
	Hide_COMOB(COMOB);

	return(COMOB);
}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : Dissolve
 * FUNCTION : Monster lîst sich in kleine Kugeln auf,
 *						welche die Grî·e xsize,ysize besitzen die erstellen Kugeln werden in
 *						die Liste welche **COMOB geschrieben, welche maximal maxcomobs EintrÑge hat
 *
 * INPUTS   : struct Combat_participant *victim : zu verpixelndes Opfer
 *            SILONG xsize : Grî·e in X jeder Kugel
 *            SILONG ysize : Grî·e in Y jeder Kugel
 * 							Minimale Grî·e, wird stufenweise erhîht falls nicht genug
 *							Kugeln vorhanden
 *            struct COMOB **COMOBLIST : Zeiger auf COMOB Liste
 *            SILONG maxcomobs : maximale Anzahl COMOBS
 * RESULT   : Anzahl Kugeln generiert / 0 = Fehler
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG Dissolve(struct Combat_participant *victim,SILONG xsize,SILONG ysize,struct COMOB **comoblist,SILONG maxcomobs)
{
//	struct BBPALETTE Palette;
	struct Gfx_header *Gfx;
	struct COMOB *dis_COMOB;
	UNSHORT Source_width, Source_height;
	UNSHORT i;
	UNBYTE *Ptr;
	UNBYTE *p;
	UNBYTE col;
	SILONG red,green,blue;
	SILONG anz,t;
	SILONG x,y,z;
	SILONG px,py,w,h;
	SILONG x_3d,y_3d,add_x_3d,add_y_3d;
	SILONG pixelxsize,pixelysize;
	SILONG anz_x_kugeln,anz_y_kugeln;
	SILONG left,right,top,bottom;
	SILONG gfxsizew,gfxsizeh;
	SILONG aktcomob;

	/* Comob des Monsters */
	dis_COMOB=victim->Main_COMOB;

	/* Find current frame */
	Gfx = (struct Gfx_header *) (MEM_Claim_pointer(dis_COMOB->Graphics_handle)
	 + dis_COMOB->Graphics_offset);

	/* richtiges Animationsframe */
	for (i=0;i<dis_COMOB->Frame;i++)
	{
		Ptr = (UNBYTE *) (Gfx + 1);
		Gfx = (struct Gfx_header *) (Ptr + (Gfx->Width * Gfx->Height));
	}
	Ptr = (UNBYTE *) (Gfx + 1);

	/* Get source dimensions */
	Source_width = Gfx->Width;
	Source_height = Gfx->Height;

	/* 3D Grî·e des Objects */
	Get_part_rectangle(victim,&left,&top,&right,&bottom);

	/* 3D Position des Objects */
	Get_3D_part_coordinates(victim,&x,&y,&z);

	/* Grî·e einer Kugel */
	pixelxsize=xsize;
	pixelysize=ysize;

	/* Schleife vergrî·ert die Kugeln so lange bis die Anzahl COMOBS ausreicht */
	for(;;){

		anz_x_kugeln=(Source_width/pixelxsize);
		anz_y_kugeln=(Source_height/pixelysize);

		if((anz_x_kugeln*anz_y_kugeln)<maxcomobs){
			break;
		}
		else{
			pixelxsize++;
			pixelysize++;
		}

	}

	/* Grî·e des Sparks auch erhîhen */
	Gfx = (struct Gfx_header *)MEM_Claim_pointer(Spark_gfx_handles[2]);
	w = Gfx->Width;
	h = Gfx->Height;
	gfxsizew=(75*((pixelxsize-xsize)+w))/w;
	gfxsizeh=(75*((pixelysize-ysize)+h))/h;
//			MEM_Free_pointer(Spark_gfx_handles[2]);

	/* DeltaWerte fÅr x,y 3D */
	add_x_3d=(right-left)/anz_x_kugeln;
	add_y_3d=(bottom-top)/anz_y_kugeln;

	/* Anzahl COMOBS auf 0 */
	aktcomob=0;

	/* Bild in Kugeln aufteilen */
	p=Ptr;
	for(y_3d=top,y=0;y<(Source_height-pixelysize);y+=pixelysize,y_3d+=add_y_3d){
		for(x_3d=left,x=0;x<(Source_width-pixelxsize);x+=pixelxsize,x_3d+=add_x_3d){

			/* Farbe fÅr Kugel berechnen */
			red=0;
			green=0;
			blue=0;
			for(py=y;py<(y+pixelysize);py++){
				for(px=x;px<(x+pixelxsize);px++){
					col=*(p+px+(py*Source_width));
					red+=Palette.color[col].red;
					green+=Palette.color[col].green;
					blue+=Palette.color[col].blue;
				}
			}

			/* Anzahl Farben */
			anz=(pixelxsize*pixelysize);

			red/=anz;
			green/=anz;
			blue/=anz;

			/* Keine MaskenFarbe */
			if(red!=0&&green!=0&&blue!=0){

				/* / GesamtAnzahl = Durschnittsfarbe */
				col = Find_closest_colour(red,green,blue);

				/* Add COMOB */
				comoblist[aktcomob] = Add_COMOB(100);

				/* comoblist[aktcomob] in Liste eintragen */

				/* Success ? */
				if (!comoblist[aktcomob]){
					ERROR_ClearStack();
					return(0);
				}

				/* set coordinates */
				comoblist[aktcomob]->X_3D = x_3d;
				comoblist[aktcomob]->Y_3D = y_3d;
				comoblist[aktcomob]->Z_3D = z;

				/* Set lifespan */
				comoblist[aktcomob]->Lifespan = 0 ;

				/* Set display parameters */
				comoblist[aktcomob]->Draw_mode = SILHOUETTE_COMOB_DRAWMODE;
				comoblist[aktcomob]->Colour = col;

				comoblist[aktcomob]->Hotspot_X_offset = 50;
				comoblist[aktcomob]->Hotspot_Y_offset = 50;

				comoblist[aktcomob]->Display_width = gfxsizew;
				comoblist[aktcomob]->Display_height = gfxsizew;

				/* irgendein Spark als Grafik verwenden */
				comoblist[aktcomob]->Graphics_handle = Spark_gfx_handles[2];

				/* Set number of animation frames */
				comoblist[aktcomob]->Nr_frames = 0;

				comoblist[aktcomob]->Frame = 0;

				/* maximale Anzahl comoblist[aktcomob]S erreicht dann beenden */
				if(++aktcomob>=maxcomobs){
					return(0);
				}

			}
		}
	}

	return(aktcomob);
}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : Test_COMOB_mask
 * FUNCTION : Stellt fest ob sich an entsprechender 3D-Position auf dem Object
 *						ein durchsichtiges Pixel(FALSE) oder nichtdurchsichtiges Pixel(TRUE) befindet
 *						befindet sich die Posiiton komplett au·erhalb des Objects ist das
 *					  Ergebnis FALSE
 * INPUTS   : struct Combat_participant *victim :
 *            SILONG x3d : x3D Position
 *            SILONG y3d : y3D Position
 * RESULT   : TRUE = nicht transparent / FALSE = transparent
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN Test_COMOB_mask(struct COMOB *source_COMOB,SILONG x3d,SILONG y3d)
{
	UNBYTE *Ptr;
	struct Gfx_header *Gfx;
	struct COMOB *dis_COMOB;
	UNSHORT Source_width, Source_height;
	SILONG left,right,top,bottom;
	SILONG i,x,y;

	/* Find current frame */
	Gfx = (struct Gfx_header *) (MEM_Claim_pointer(source_COMOB->Graphics_handle)
	 + source_COMOB->Graphics_offset);

	/* richtiges Animationsframe */
	for (i=0;i<source_COMOB->Frame;i++)
	{
		Ptr = (UNBYTE *) (Gfx + 1);
		Gfx = (struct Gfx_header *) (Ptr + (Gfx->Width * Gfx->Height));
	}
	Ptr = (UNBYTE *) (Gfx + 1);

	/* Get source dimensions */
	Source_width = Gfx->Width;
	Source_height = Gfx->Height;

	/* 3D Grî·e des Objects */
	Get_COMOB_rectangle(source_COMOB,&left,&top,&right,&bottom);

	/* au·erhalb shape */
	if(x3d<left||x3d>right||y3d<bottom||y3d>top)
		return(FALSE);

	/* 2d Position im shape */
	x=(x3d-left)*(Source_width)/(right-left);
	y=(y3d-top)*(Source_height)/(bottom-top);

	/* Pixel Transparent */
	if(*(Ptr+(y*Source_width)+x)==0)
		return(FALSE);

	/* Pixel nicht transparent */
	return(TRUE);

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_COMOB_max_size
 * FUNCTION  : Get maximum size of COMOB source Animation frames graphics.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.06.95 13:22
 * LAST      : 17.06.95 13:22
 * INPUTS    : struct COMOB *COMOB - Pointer to COMOB data.
 *             UNSHORT *Width_ptr - Pointer to width.
 *             UNSHORT *Height_ptr - Pointer to height.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_COMOB_max_size(struct COMOB *COMOB, UNSHORT *Width_ptr, UNSHORT *Height_ptr)
{
	struct Gfx_header *Gfx;
	UNSHORT t,i;
	UNBYTE *Ptr;
	SILONG frames,max_width,max_height;

	/* Anzahl Animationsframes */
	frames=Get_nr_frames(COMOB->Graphics_handle);

	/* Maximale Breite und Grî·e fÅr grî·tes Animationsframe */
	max_width=-1;
	max_height=-1;
	for(t=0;t<frames;t++){
		/* Find current frame */
		Gfx = (struct Gfx_header *) (MEM_Claim_pointer(COMOB->Graphics_handle) +
		 COMOB->Graphics_offset);

		for (i=0;i<t;i++)
		{
			Ptr = (UNBYTE *) (Gfx + 1);
			Gfx = (struct Gfx_header *) (Ptr + (Gfx->Width * Gfx->Height));
		}
		Ptr = (UNBYTE *) (Gfx + 1);

		if(Gfx->Width>max_width){
			max_width=Gfx->Width;
		}
		if(Gfx->Height>max_height){
			max_height=Gfx->Height;
		}

	}

	/* Store source dimensions */
	*Width_ptr = max_width;
	*Height_ptr = max_height;

	MEM_Free_pointer(COMOB->Graphics_handle);
}

/*******************************************************************************
	Handler fÅr Behaviour
*******************************************************************************/

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : size_handler
 * FUNCTION :
 * INPUTS   : struct COMOB_behaviour *Behaviour : Bevahiour Daten
 *						SISHORT data[0]=jeweils x%/100 von der Grî·e X abziehen
 *													 100 bedeuted bei jedem Tick 1 % abziehen
 *													 25 bedeuted bei jedem 4 Tick 1 % abziehen
 *						SISHORT data[1]=jeweils x%/100 von der Grî·e Y abziehen
 *													 100 bedeuted bei jedem Tick 1 % abziehen
 *													 25 bedeuted bei jedem 4 Tick 1 % abziehen
 *						SISHORT data[2]=minimaler/maximaler(je nachdem ob data[0] positiv oder negativ ist) Wert Breite
 *						SISHORT data[3]=minimaler/maximaler(je nachdem ob data[1] positiv oder negativ ist) Wert Hîhe
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void size_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SISHORT *wdata;

	/* Get pointer to First COMOB */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (6 maximal mîglich)*/
	wdata=&Behaviour->Data.Just_data_w.Data[0];

	/* Bei erstem Aufruf Grî·e merken */
	if(!wdata[4]){
		wdata[4]=TRUE;
		wdata[5]=(COMOB->Display_width*100);
		wdata[6]=(COMOB->Display_height*100);
	}
	else{
		/* Grî·e verringern */
		wdata[5]+=wdata[0];
		COMOB->Display_width=wdata[5]/100; /* neue Hîhe */
		/* Grî·e verringern */
		wdata[6]+=wdata[1];
		COMOB->Display_height=wdata[6]/100; /* neue Hîhe */

		if(wdata[0]<0){
			/* Minimal-Werte schon erreicht */
			if(wdata[5]<(wdata[2]*100)){
				Delete_COMOB(COMOB);
			}
		}
		else{
			/* Minimal-Werte schon erreicht */
			if(wdata[5]>(wdata[2]*100)){
				Delete_COMOB(COMOB);
			}
		}

		if(wdata[1]<0){
			/* Minimal-Werte schon erreicht */
			if(wdata[6]<(wdata[3]*100)){
				Delete_COMOB(COMOB);
			}
		}
		else{
			/* Minimal-Werte schon erreicht */
			if(wdata[6]>(wdata[3]*100)){
				Delete_COMOB(COMOB);
			}
		}

	}

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : size_II_handler
 * FUNCTION : aktuelle Grî·e bis zu einer gewissen Grenze erhîhen dann wieder
 *						verringern  COMOB danach schlie·lich lîschen
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SISHORT data[0]=jeweils x%/100 zu Grî·e X addieren
 *						SISHORT data[1]=jeweils y%/100 zu Grî·e Y addieren
 *						SISHORT data[2]=maximale Grî·e X
 *						SISHORT data[3]=maximale Grî·e Y
 *						SISHORT data[4]=jeweils x%/100 danach von Grî·e X abziehen
 *						SISHORT data[5]=jeweils y%/100 danach von Grî·e Y abziehen
 *						SISHORT data[6]=minimale Grî·e X
 *						SISHORT data[7]=minimale Grî·e Y
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void size_II_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SISHORT *data;

	/* Get pointer to First COMOB */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (16 maximal mîglich)*/
	data=&Behaviour->Data.Just_data_w.Data[0];

	/* erster Aufruf */
	if(!data[15]){
		data[15]=TRUE;
		data[8]=COMOB->Display_width*100;
		data[9]=COMOB->Display_height*100;
		data[2]*=100;
		data[3]*=100;
		data[6]*=100;
		data[7]*=100;
	}
	else{
		/* maximale Breite schon Åberschritten */
		if(!data[10]){
			/* Breite erhîhen */
			data[8]+=data[0];
			/* maximale Breite erreicht */
			if(data[8]>=data[2]){
				data[10]=TRUE;
			}
		}
		else{
			/* Breite dezimieren */
			data[8]-=data[4];
			/* minimale Breite erreicht dann COMOB lîschen */
			if(data[8]<=data[6]){
				Delete_COMOB(COMOB);
				return;
			}
		}
		/* Breite setzten */
		COMOB->Display_width=data[8]/100;

		/* maximale Hîhe schon Åberschritten */
		if(!data[11]){
			/* Hîhe erhîhen */
			data[9]+=data[1];
			/* maximale Hîhe erreicht */
			if(data[9]>=data[3]){
				data[11]=TRUE;
			}
		}
		else{
			/* Hîhe dezimieren */
			data[9]-=data[5];
			/* minimale Hîhe erreicht dann COMOB lîschen */
			if(data[9]<=data[7]){
				Delete_COMOB(COMOB);
				return;
			}
		}
		/* Hîhe setzten */
		COMOB->Display_height=data[9]/100;

	}

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : size_III_handler
 * FUNCTION : aktuelle Grî·e bis zu einer gewissen Grenze erhîhen dann wieder
 *						verringern wieder erhîhen usw ...
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SISHORT data[0]=jeweils x%/100 zu Grî·e X addieren
 *						SISHORT data[1]=jeweils y%/100 zu Grî·e Y addieren
 *						SISHORT data[2]=maximale Grî·e X
 *						SISHORT data[3]=maximale Grî·e Y
 *						SISHORT data[4]=jeweils x%/100 danach von Grî·e X abziehen
 *						SISHORT data[5]=jeweils y%/100 danach von Grî·e Y abziehen
 *						SISHORT data[6]=minimale Grî·e X
 *						SISHORT data[7]=minimale Grî·e Y
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void size_III_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SISHORT *data;

	/* Get pointer to First COMOB */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (16 maximal mîglich)*/
	data=&Behaviour->Data.Just_data_w.Data[0];

	/* erster Aufruf */
	if(!data[15]){
		data[15]=TRUE;
		data[8]=COMOB->Display_width*100;
		data[9]=COMOB->Display_height*100;
		data[2]*=100;
		data[3]*=100;
		data[6]*=100;
		data[7]*=100;
	}
	else{
		/* maximale Breite schon Åberschritten */
		if(!data[10]){
			/* Breite erhîhen */
			data[8]+=data[0];
			/* maximale Breite erreicht */
			if(data[8]>=data[2]){
				data[10]=TRUE;
			}
		}
		else{
			/* Breite dezimieren */
			data[8]-=data[4];
			/* minimale Breite erreicht dann COMOB lîschen */
			if(data[8]<=data[6]){
				data[10]=FALSE;
			}
		}
		/* Breite setzten */
		COMOB->Display_width=data[8]/100;

		/* maximale Hîhe schon Åberschritten */
		if(!data[11]){
			/* Hîhe erhîhen */
			data[9]+=data[1];
			/* maximale Hîhe erreicht */
			if(data[9]>=data[3]){
				data[11]=TRUE;
			}
		}
		else{
			/* Hîhe dezimieren */
			data[9]-=data[5];
			/* minimale Hîhe erreicht dann COMOB lîschen */
			if(data[9]<=data[7]){
				data[11]=FALSE;
			}
		}
		/* Hîhe setzten */
		COMOB->Display_height=data[9]/100;

	}

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : flash_handler
 * FUNCTION :
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 * 						data[0] = Anzahl Ticks fÅr Auf und Abblitzen
 * 						data[1],[2],[3] = red,green,blue
 * 						data[4] = Bis Wieviel % maximal umblenden
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void flash_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SILONG u;
	SILONG *ldata;

	/* Get pointer to First COMOB */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (6 maximal mîglich)*/
	ldata=&Behaviour->Data.Just_data.Data[0];

	if(ldata[5]<ldata[0]){
		/* Palette flashen */
		u=(ldata[0]/2)-ldata[5];
		if(u<0)
			u=-u;
		u=(ldata[0]/2)-u;
		ldata[5]++;
		Recolour_palette(1,191,ldata[1],ldata[2],ldata[3],u*ldata[4]/(ldata[0]/2));
		if(ldata[5]>=ldata[0]){
			Recolour_palette(1,191,ldata[1],ldata[2],ldata[3],0);
			Delete_COMOB_behaviour(Behaviour);
		}
	}

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : glow_handler
 * FUNCTION :
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 * 						data[0] = StartGlow in %
 * 						data[1],[2],[3] = red,green,blue
 * 						data[4] = min %
 * 						data[5] = max %
 * 						data[6] = Speed in % muss kleiner als max-min sein
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void glow_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SILONG u;
	SILONG *ldata;

	/* Get pointer to First COMOB */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (6 maximal mîglich)*/
	ldata=&Behaviour->Data.Just_data.Data[0];

	/* Palette umfÑrben */
	Recolour_palette(1,191,ldata[1],ldata[2],ldata[3],ldata[0]);

	/* Glow addieren */
	ldata[0]+=ldata[6];
	if(ldata[0]<ldata[4]){
		ldata[0]=ldata[4];
		ldata[6]=-ldata[6];
	}
	if(ldata[0]>ldata[5]){
		ldata[0]=ldata[5];
		ldata[6]=-ldata[6];
	}

}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : snow_handler
 * FUNCTION : Comob verhÑlt sich Ñhnlich wie eine nach rechts fliegende Schneeflocke
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void snow_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SILONG *ldata;

	/* Get pointer to First COMOB */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (8 maximal mîglich)*/
	ldata=&Behaviour->Data.Just_data.Data[0];

	COMOB->X_3D+=ldata[1];
	ldata[1]+=rndmm(-15,40);
	if(ldata[1]<-40){
		ldata[1]+=rndmm(10,20);
	}
	else if(ldata[1]>200){
		ldata[1]-=rndmm(5,20);
	}

	COMOB->Y_3D+=ldata[0];
	if(!ldata[4]){
		/* Beschleunigung x */
		ldata[0]+=rndmm(-100,1);
		/* maximale und minimale Werte */
		if(ldata[0]<-100){
			ldata[0]+=rndmm(40,50);
		}
		else if(ldata[0]>0){
			ldata[0]=0;
		}
	}

	COMOB->Z_3D+=ldata[2];

	if(COMOB->Y_3D<0&&!ldata[4]){
		if(!ldata[4]){
			ldata[0]=-(ldata[0]/4); 	/* y vom Boden zurÅckspringen */
			ldata[1]=-(ldata[1]/2); /* X Bewegung auf 0 */
			ldata[2]=rndmm(-50,50); /* Z Bewegung */
			ldata[4]=TRUE;
		}
	}

	if(ldata[4]){
		if(COMOB->Display_width>5){
			COMOB->Display_width-=2;
		}
		if(COMOB->Display_height>5){
			COMOB->Display_height-=2;
		}
		if(ldata[5]++>20){
			Delete_COMOB(COMOB);
		}
	}


}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : snow_II_handler
 * FUNCTION : erzeugt eine Wand aus Schnee an entsprechender Z-Koordinate
 *					  die Åber die ganze Breite des Bildschirms geht
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SILONG data[0] = X Koordinaate fÅr Schneewand
 *						SILONG data[1] = Y Koordinaate fÅr Schneewand
 *						SILONG data[2] = Z Koordinaate fÅr Schneewand
 *						SILONG data[3] = Anzahl Schneeflocken jedesmal neu erzeugen
 *														 400 = bei jedem Durchgang 4
 *														 100 = bei jedem Durchgang 1
 *														 20 = bei jedem 5 Durchgang 1
 *						SILONG data[4] = Flag fÅr Schneeverhalten
 *												0 = Schnee entsteht nur an dieser Position
 *												1 = Schnee entsteht an dieser Z-Reihe
 *												2 = Schnee entsteht zufÑllig Åber das komplette Feld
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void snow_II_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SILONG *ldata;
	SILONG x,y,z;
	SILONG zx,zy,zz;
	SILONG mx,my,mz;
	SILONG t,steps;

	/* data als SILONG interpretieren (8 maximal mîglich)*/
	ldata=&Behaviour->Data.Just_data.Data[0];

	/* neue Schneeflocke erzeugen */
	if(First_combat_update){

		ldata[7]+=ldata[3];
		if(ldata[7]>=100){

			/* neue Schneeflocken erzeugen */
			for(;ldata[7]>0;ldata[7]-=100){

				/* Direkt an Position */
				if(ldata[4]==0){

					/* ZufÑllige Position */
					x=ldata[0]+rndmm(-100*COMOB_DEC_FACTOR,100*COMOB_DEC_FACTOR);
					y=ldata[1]+rndmm(-10*COMOB_DEC_FACTOR,10*COMOB_DEC_FACTOR);
					z=ldata[2]+rndmm(-100,100);

					/* ZufÑllige EndPosition */
					zx=x+rndmm(-100*COMOB_DEC_FACTOR,100*COMOB_DEC_FACTOR);
					zy=0;
					zz=z+rndmm(-100,100);

				}
				/* In einer Reihe */
				else if(ldata[4]==1){

					/* ZufÑllige Position */
					x=rndmm(-(COMBAT_3D_WIDTH/2),(COMBAT_3D_WIDTH/2));
					y=rndmm((COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR)-50,(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR)+50);
					z=ldata[2]+rndmm(-100,100);

					/* ZufÑllige EndPosition */
					zx=x+rndmm(-200*COMOB_DEC_FACTOR,200*COMOB_DEC_FACTOR);
					if(zx>(COMBAT_3D_WIDTH/2)){
						zx=COMBAT_3D_WIDTH/2;
					}
					if(zx<-(COMBAT_3D_WIDTH/2)){
						zx=-COMBAT_3D_WIDTH/2;
					}
					zy=0;
					zz=ldata[2]+rndmm(-100,100);

				}
				/* Åber das ganze Feld */
				else if(ldata[4]==2){

					/* ZufÑllige Position */
					x=rndmm(-(COMBAT_3D_WIDTH/2),(COMBAT_3D_WIDTH/2));
					y=rndmm((COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR)-50,(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR)+50);
					z=rndmm(-(COMBAT_3D_DEPTH/2),(COMBAT_3D_DEPTH/2));

					/* ZufÑllige EndPosition */
					zx=x+rndmm(-200*COMOB_DEC_FACTOR,200*COMOB_DEC_FACTOR);
					if(zx>(COMBAT_3D_WIDTH/2)){
						zx=COMBAT_3D_WIDTH/2;
					}
					if(zx<-(COMBAT_3D_WIDTH/2)){
						zx=-COMBAT_3D_WIDTH/2;
					}
					zy=0;
					zz=z+rndmm(-100,100);
					if(zz>(COMBAT_3D_DEPTH/2)){
						zz=COMBAT_3D_DEPTH/2;
					}
					if(zz<-(COMBAT_3D_DEPTH/2)){
						zz=-COMBAT_3D_DEPTH/2;
					}

				}

//				zx=x;
//				zz=z;

				/* Bewegung zum Ziel */
				steps=calc_delta_xyz(x,y,z,zx,zy,zz,&mx,&my,&mz,rndmm(200,250));

				/* Schneeflocke erzeugen */
				if((COMOB=Gen_COMOB(x,y,z, steps, 75, Spark_gfx_handles[0], GC_MAGIC))==NULL){
					ERROR_ClearStack();
					return;
				}

				/* ZufÑlliges Frame auswÑhlen */
				COMOB->Nr_frames=0;
			 	COMOB->Frame = rndmm(0,Get_nr_frames(COMOB->Graphics_handle)-1);

				/* DeltaBewegung */
				COMOB->dX_3D=mx;
				COMOB->dY_3D=my;
				COMOB->dZ_3D=mz;

//				COMOB->dY_3D=-2*COMOB_DEC_FACTOR;


			}

		}

	}


}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : FireBounce_handler
 * FUNCTION : Funktioniert Ñhnlich wie BounceHandler nur bei Aufprall
 *					  entsteht BodenglÅhen, sowie Funken spritzen auf
 *						Achtung !!! Da Åber Just_data->Ddata auf die letzen 5 Langwîrter
 *						zugegriffen wird darf sich Bounce-Handler nicht verÑndern
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void FireBounce_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB_behaviour *behave_bounce;
	struct COMOB *COMOB;
	SILONG *ldata;
	SISHORT *wdata;
	SILONG s;

	/* Get COMOB data */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (8 maximal mîglich)*/
	wdata=&Behaviour->Data.Just_data_w.Data[0];

	/* Kein Zusammenstauchen aktiv */
	if(wdata[3]==0){

		/* Implement air friction */
		COMOB->dX_3D = (COMOB->dX_3D * (1000 -
		 (SILONG) Behaviour->Data.Bounce_data.Air_friction)) / 1000;
		COMOB->dY_3D = (COMOB->dY_3D * (1000 -
		 (SILONG) Behaviour->Data.Bounce_data.Air_friction)) / 1000;
		COMOB->dZ_3D = (COMOB->dZ_3D * (1000 -
		 (SILONG) Behaviour->Data.Bounce_data.Air_friction)) / 1000;

		/* Implement gravity */
		COMOB->dY_3D -= (SILONG) Behaviour->Data.Bounce_data.Gravity;

		/* Evaluate movement vector */
		COMOB->X_3D += COMOB->dX_3D;
		COMOB->Y_3D += COMOB->dY_3D;
		COMOB->Z_3D += COMOB->dZ_3D;

		/* Reached the floor ? */
		if (Behaviour->Data.Bounce_data.Gravity && (COMOB->Y_3D <= 0)){

			/* Funken erstellen */
			Gen_Funken(COMOB->X_3D,0,COMOB->Z_3D,Spark_gfx_handles[6],2);

			/* BodenglÅhen */
			groundglow(COMOB->X_3D,COMOB->Z_3D,150,20);

			/* Auf Boden Setzten */
			COMOB->Y_3D = 0;

			/* Maximales Zusammenstauchen */
			s=-COMOB->dY_3D/15;
			if(s<0)
				s=0;
			if(s>50)
				s=50;

			/* Zusammenstauchen vorbereiten */
			wdata[3]=1;
			wdata[4]=COMOB->Display_height;
			wdata[5]=COMOB->Display_height*10/100; /* um 10 % zusammenstauchen  */
			wdata[6]=COMOB->Display_height*(100-s)/100; /* auf 50 % minimal zusammenstauchen  */
			wdata[7]=COMOB->Display_height; /* auf 100 % maximal ausdehenen  */

			wdata[8]=COMOB->dX_3D;
			wdata[9]=COMOB->dY_3D;
			wdata[10]=COMOB->dZ_3D;

			/* BEwegung solange stoppen */
			COMOB->dX_3D=0;
			COMOB->dY_3D=0;
			COMOB->dZ_3D=0;

		}
	}

	/* Kugel schlÑgt auf dem Boden auf und wird in Y zusammengestaucht */
	else if(wdata[3]==1){
		COMOB->Display_height=wdata[4];
		wdata[4]-=wdata[5];
		/* Maximal zusammengestaucht dann das Ganze umkehren */
		if(wdata[4]<=wdata[6]){
			wdata[3]=2;
		}
	}
	else if(wdata[3]==2){
		COMOB->Display_height=wdata[4];
		wdata[4]+=wdata[5];
		/* Wieder auseinandergedehnt dann abspringen lassen */
		if(wdata[4]>=wdata[7]){

			wdata[3]=0;

			COMOB->dX_3D=wdata[8];
			COMOB->dY_3D=wdata[9];
			COMOB->dZ_3D=wdata[10];

			/* endgÅltige Hîhe setzten */
			COMOB->Display_height=wdata[7];

			/* Yes -> Bounce */
			COMOB->Y_3D = 0;
			COMOB->dY_3D = (( -COMOB->dY_3D) * (UNLONG) Behaviour->Data.Bounce_data.Bounce) / 100;

			/* add x % to X and Z Movement */
			COMOB->dX_3D+=(rndmm(-25,25)*COMOB->dY_3D)/50;
			COMOB->dZ_3D+=(rndmm(-25,25)*COMOB->dY_3D)/50;

			/* Breite und Hîhe -10% */
			COMOB->Display_width-=4;
			COMOB->Display_height-=4;

			/* Wenn Grî·e bestimmtes Mass unterschreitetkaum Comob lîschen */
			if(COMOB->Display_width<15||COMOB->Display_height<15){
				Delete_COMOB(COMOB);
			}
		}
	}


}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Oscillate_II_handler
 * FUNCTION  : Oscillate COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.03.95 17:10
 * LAST      : 24.05.95 12:17
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 *		behave->Data.Oscillate_data.Type = OSCILLATE_X;
 *		Typ: mîglich sind X,Y,Z,WIDTH,HEIGHT
 *
 *		behave->Data.Oscillate_data.Period = 90;
 *		 0 passiert gar nichts, entspricht 360 Grad /
 *		 1-x = Periode einer SinusWelle wird x-mal unterteilt bei 90 also jeden 4 Wert
 *
 *		behave->Data.Oscillate_data.Amplitude = 50*COMOB_DEC_FACTOR;
 *		Amplitude = Maximale Grî·e des Auschlags
 *
 *		behave->Data.Oscillate_data.Value = t*90/blitzanz;
 *	  Beginn innerhalb der SinusWelle siehe Period
 *
 *		behave->Data.Just_data_w.Data[7] = Amplitude Add ;
 *	  Wert wird bei jedem Aufruf auf die Amplitude addiert

 *		behave->Data.Just_data_w.Data[8] = Max Amplitude ;
 *	  Bei erreichen dieses Wertes wird das COMOB gelîscht
 *
 *
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Oscillate_II_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SILONG New_delta, Total_delta;
	SISHORT *wdata;
	UNSHORT Value;
	UNSHORT Period;

	/* Get pointer to COMOB */
	COMOB = Behaviour->First_COMOB;

	/* data als SISHORT interpretieren */
	wdata=&Behaviour->Data.Just_data_w.Data[0];

	/* Get oscillate data */
	Value = Behaviour->Data.Oscillate_data.Value;
	Period = Behaviour->Data.Oscillate_data.Period;

	/* Calculate new delta value */
	New_delta = Behaviour->Data.Oscillate_data.Amplitude *
	 sin((Value * 2 * PI) / Period);

	/* increase Amplitude */
	Behaviour->Data.Oscillate_data.Amplitude+=wdata[7];

	/* Max Amplitude erreicht dann COMOB lîschen */
	if(Behaviour->Data.Oscillate_data.Amplitude>wdata[8]){
		Delete_COMOB(COMOB);
		return;
	}

	/* Increase value */
	Value++;
	if (Value == Period)
		Value = 0;

	/* Write current value */
	Behaviour->Data.Oscillate_data.Value = Value;

	/* Calculate total delta */
	Total_delta = New_delta - Behaviour->Data.Oscillate_data.Old_delta;

	/* Store current delta for next time */
	Behaviour->Data.Oscillate_data.Old_delta = New_delta;

	/* What kind of oscillation ? */
	switch(Behaviour->Data.Oscillate_data.Type)
	{
		case OSCILLATE_X:
		{
			COMOB->X_3D += Total_delta;
			break;
		}
		case OSCILLATE_Y:
		{
			COMOB->Y_3D += Total_delta;
			break;
		}
		case OSCILLATE_Z:
		{
			COMOB->Z_3D += Total_delta;
			break;
		}
		case OSCILLATE_WIDTH:
		{
			COMOB->Display_width += (SISHORT) Total_delta;
			break;
		}
		case OSCILLATE_HEIGHT:
		{
			COMOB->Display_height += (SISHORT) Total_delta;
			break;
		}
		case OSCILLATE_SIZE:
		{
			COMOB->Display_width += (SISHORT) Total_delta;
			COMOB->Display_height += (SISHORT) Total_delta;
			break;
		}
	}

}


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : CrazyMove_handler
 * FUNCTION : Object bewegt sich zufÑllig Åber den Bildschirm
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SILONG ldata[0]= Geschwindigkeit in X (0=inaktiv)
 *						SILONG ldata[1]= Geschwindigkeit in Y (0=inaktiv)
 *						SILONG ldata[2]= Geschwindigkeit in Z (0=inaktiv)
 *
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */


void CrazyMove_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SILONG CrazyMove_MAX_SPEED;
	SILONG *ldata;
	SILONG t;

	/* Get COMOB data */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (8 maximal mîglich) */
	ldata=&Behaviour->Data.Just_data.Data[0];

	CrazyMove_MAX_SPEED=ldata[0]*2; /* Maximale Beschleunigung */

	/* X,Y,Z Beschleunigung */

	if(ldata[0]>0){
		ldata[3]+=rndmm(-ldata[0],ldata[0]);
		if(ldata[3]<-CrazyMove_MAX_SPEED){
			ldata[3]+=ldata[0]*2;
		}
		else if(ldata[3]>CrazyMove_MAX_SPEED){
			ldata[3]-=ldata[0]*2;
		}
	}

	if(ldata[1]>0){
		ldata[4]+=rndmm(-ldata[1],ldata[1]);
		if(ldata[4]<-CrazyMove_MAX_SPEED){
			ldata[4]+=ldata[1]*2;
		}
		else if(ldata[4]>CrazyMove_MAX_SPEED){
			ldata[4]-=ldata[1]*2;
		}
	}

	if(ldata[2]>0){
		ldata[5]+=rndmm(-ldata[2],ldata[2]);
		if(ldata[5]<-CrazyMove_MAX_SPEED){
			ldata[5]+=ldata[2]*2;
		}
		else if(ldata[5]>CrazyMove_MAX_SPEED){
			ldata[5]-=ldata[2]*2;
		}
	}


	/* X,Y,Z Bewegung */
	if(ldata[0]>0){
		COMOB->X_3D+=ldata[3];
		if(COMOB->X_3D>(COMBAT_3D_WIDTH/2)){
			COMOB->X_3D=COMBAT_3D_WIDTH/2;
		}
		if(COMOB->X_3D<-(COMBAT_3D_WIDTH/2)){
			COMOB->X_3D=-COMBAT_3D_WIDTH/2;
		}
	}

	if(ldata[1]>0){
		COMOB->Y_3D+=ldata[4];
		if(COMOB->Y_3D>(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR/2)){
			COMOB->Y_3D=COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR/2;
		}
		if(COMOB->Y_3D<-(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR/2)){
			COMOB->Y_3D=-COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR/2;
		}
	}

	if(ldata[2]>0){
		COMOB->Z_3D+=ldata[5];
		if(COMOB->Z_3D>(COMBAT_3D_DEPTH/2)){
			COMOB->Z_3D=COMBAT_3D_DEPTH/2;
		}
		if(COMOB->Z_3D<-(COMBAT_3D_DEPTH/2)){
			COMOB->Z_3D=-COMBAT_3D_DEPTH/2;
		}
	}

}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : connect_handler
 * FUNCTION : verknÅpft die Position des 2 COMOB mit dem 1 COMOB
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SILONG wdata[0]
 *							Bit 0 = X Bewegung dazuaddieren
 *							Bit 1 = Y Bewegung dazuaddieren
 *							Bit 2 = X Bewegung dazuaddieren
 *						SILONG wdata[1]= X Position
 *						SILONG wdata[2]= Y Position
 *						SILONG wdata[3]= Z Position
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void connect_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB0;
	struct COMOB *COMOB1;
	SISHORT *wdata;
	SILONG x,y,z,t;

	/* Get COMOB data */
	COMOB1 = Behaviour->First_COMOB;
	COMOB0 = Behaviour->Second_COMOB;

	/* Existiert as vorige COMOB noch ?,wenn nicht das verbundene lîschen */
//	if(!(COMOB0->System_flags&COMOB_ACTIVE)){
//		Delete_COMOB(COMOB1);
//		return;
//	}

	/* data als SILONG interpretieren (8 maximal mîglich) */
	wdata=&Behaviour->Data.Just_data_w.Data[0];

	/* Position wenn nicht von diesem COMOB */
	x=wdata[1];
	y=wdata[2];
	z=wdata[3];

	/* X verknÅpfen */
	if(wdata[0]&1){
		x+=COMOB0->X_3D;
	}
	/* Y verknÅpfen */
	if(wdata[0]&2){
		y+=COMOB0->Y_3D;
	}
	/* Z verknÅpfen */
	if(wdata[0]&4){
		z+=COMOB0->Z_3D;
	}

	/* Positiondes2 COMBS */
	COMOB1->X_3D=x;
	COMOB1->Y_3D=y;
	COMOB1->Z_3D=z;

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : funken_handler
 * FUNCTION :
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SISHORT wdata[0]
 *							Bit 0 = X Bewegung dazuaddieren
 *							Bit 1 = Y Bewegung dazuaddieren
 *							Bit 2 = X Bewegung dazuaddieren
 *						SISHORT wdata[1]= X Position
 *						SISHORT wdata[2]= Y Position
 *						SISHORT wdata[3]= Z Position
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void funken_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SISHORT *wdata;
	SILONG x,y,z;

	/* Get COMOB data */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (8 maximal mîglich) */
	wdata=&Behaviour->Data.Just_data_w.Data[0];

	/* Position wenn nicht von diesem COMOB */
	x=wdata[1];
	y=wdata[2];
	z=wdata[3];

	/* X verknÅpfen */
	if(wdata[0]&1){
		x+=COMOB->X_3D;
	}
	/* Y verknÅpfen */
	if(wdata[0]&2){
		y+=COMOB->Y_3D;
	}
	/* Z verknÅpfen */
	if(wdata[0]&4){
		z+=COMOB->Z_3D;
	}

	/* Funken */
	Gen_Funken(x,y,z,Spark_gfx_handles[6],wdata[4]);

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : groundglow_handler
 * FUNCTION :
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SISHORT wdata[0]
 *							Bit 0 = X Bewegung dazuaddieren
 *							Bit 2 = X Bewegung dazuaddieren
 *						SISHORT wdata[1]= X Position
 *						SISHORT wdata[3]= Z Position
 *						SISHORT wdata[4]= GlÅhGrî·e
 *						SISHORT wdata[5]= GlÅhdauer
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void groundglow_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SISHORT *wdata;
	SILONG x,y,z;

	/* Get COMOB data */
	COMOB = Behaviour->First_COMOB;

	/* data als SILONG interpretieren (8 maximal mîglich) */
	wdata=&Behaviour->Data.Just_data_w.Data[0];

	/* Position wenn nicht von diesem COMOB */
	x=wdata[1];
	z=wdata[3];

	/* X verknÅpfen */
	if(wdata[0]&1){
		x+=COMOB->X_3D;
	}
	/* Z verknÅpfen */
	if(wdata[0]&4){
		z+=COMOB->Z_3D;
	}

	/* GlÅhen am Boden */
	groundglow(x,z,wdata[4],wdata[5]);

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : lifedead_handler
 * FUNCTION :
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SILONG data[0] = HAndle des Objects das am LifeSpan Ende dargestellt werden soll
 *						SILONG data[1] = Wenn lifespan < diesem Wert ist dann Explosion auslîsen
 *						SILONG data[2] = AnfangsGrî·e x
 *						SILONG data[3] = AnfangsGrî·e y
 *						SILONG data[4]=jeweils x%/100 von der Grî·e X abziehen
 *													 100 bedeuted bei jedem Tick 1 % abziehen
 *													 25 bedeuted bei jedem 4 Tick 1 % abziehen
 *						SILONG data[5]=jeweils x%/100 von der Grî·e X abziehen
 *													 100 bedeuted bei jedem Tick 1 % abziehen
 *													 25 bedeuted bei jedem 4 Tick 1 % abziehen
 *						SILONG data[6] = Handle fÅr TransparentTabelle (Transluminance_table_handle)
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void lifedead_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB_behaviour *behave_size;
	struct COMOB *first_COMOB;
	struct COMOB *COMOB;

	/* First time ? */
	if (First_combat_update)
	{

		/* Get COMOB data */
		first_COMOB = Behaviour->First_COMOB;

		/* Lifespan abgelaufen */
		if(first_COMOB->Lifespan<Behaviour->Data.Just_data.Data[1]){

			if(!Behaviour->Data.Just_data.Data[7]){

				/* Das UrsprungsCOMOB lîschen */
				first_COMOB->Lifespan=1;

				/* Add COMOB */
				COMOB = Add_COMOB(100);

				/* Success ? */
				if (!COMOB){
					ERROR_ClearStack();
					return;
				}

				/* set coordinates */
				COMOB->X_3D = first_COMOB->X_3D;
				COMOB->Y_3D = first_COMOB->Y_3D;
				COMOB->Z_3D = first_COMOB->Z_3D;

				/* Set lifespan */
				COMOB->Lifespan = 0; // ;

				/* Set display parameters */
				COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
				COMOB->Special_handle = (MEM_HANDLE)Behaviour->Data.Just_data.Data[6];

				COMOB->Hotspot_X_offset = 50;
				COMOB->Hotspot_Y_offset = 50;

				COMOB->Display_width = Behaviour->Data.Just_data.Data[2];
				COMOB->Display_height = Behaviour->Data.Just_data.Data[3];

				COMOB->Graphics_handle = (MEM_HANDLE)Behaviour->Data.Just_data.Data[0];

				/* Set number of animation frames */
				COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

				COMOB->Frame = 0;

				/* Behaviour fÅr Grî·e */
				behave_size = Add_COMOB_behaviour(COMOB, NULL, size_handler);

				/* Grî·e Ñndern */
				if(!behave_size){
					/* Comob lîschen */
					Delete_COMOB(COMOB);
					ERROR_ClearStack();
					return;
				}

				behave_size->Data.Just_data_w.Data[0]=Behaviour->Data.Just_data.Data[4]; /* jeden Tick um 1% */
				behave_size->Data.Just_data_w.Data[1]=Behaviour->Data.Just_data.Data[5];
				behave_size->Data.Just_data_w.Data[2]=2; /* bis 25 % */
				behave_size->Data.Just_data_w.Data[3]=2;

				Behaviour->Data.Just_data.Data[7]=-1;

			}
		}
	}

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fairy_handler
 * FUNCTION  : Fairy COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.06.95 16:28
 * LAST      : 11.07.95 14:31
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

static UNBYTE fairy_greycol[17];
static BOOLEAN fairy_greycol_generated=FALSE;

void
Fairy_II_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB_behaviour *Behaviour_data;
	struct COMOB *Fairy_COMOB;
	struct COMOB *Dust_COMOB;
	UNSHORT i;
	SILONG col;

	if(!fairy_greycol_generated){
		SILONG t,col;
		fairy_greycol_generated=TRUE;
		/* Helligkeitsverlauf in 16 Stufen berechnen */
		for(t=0;t<17;t++){
			col=t*256/17;
		 	fairy_greycol[t] = Find_closest_colour(col,col,col);
		}
	}

	/* erster Aufruf dann Tabelle generieren */
//	if(!Behaviour->Data.Just_data.Data[0]){
//		Behaviour->Data.Just_data.Data[0]=TRUE;
//	}

	/* First time ? */
	if (First_combat_update)
	{
		/* Yes -> Get pointer to fairy COMOB */
		Fairy_COMOB = Behaviour->First_COMOB;

		/* Generate dust particles */
//		for (i=0;i<2;i++)
//		{
			/* Create COMOB */
			Dust_COMOB = Add_COMOB(100);
			if (!Dust_COMOB)
				return;

			/* Add dust behaviour */
			Behaviour_data = Add_COMOB_behaviour(Dust_COMOB, NULL, Dust_II_handler);
			if (!Behaviour_data)
			{
				Delete_COMOB(Dust_COMOB);
				return;
			}

			/* Set dust parameters */
			Behaviour_data->Data.Just_data.Data[0] = DUST_LEAD;
			Behaviour_data->Data.Just_data.Data[1] = rndmm(5,10);

			col=rndmm(1,2);
			col+=col;
			if(rand()>(RAND_MAX)/2){
				Behaviour_data->Data.Just_data.Data[2] = -col;
			}
			else{
				Behaviour_data->Data.Just_data.Data[2] = col;
			}

			/* Copy coordinates from fairy COMOB */
			Dust_COMOB->X_3D = Fairy_COMOB->X_3D + rndmm(-1000,1000);
			Dust_COMOB->Y_3D = Fairy_COMOB->Y_3D + rndmm(-100,100);
			Dust_COMOB->Z_3D = Fairy_COMOB->Z_3D + rndmm(-1000,1000);

			/* Set drop speed */
			Dust_COMOB->dY_3D = 0 - ((rand() % (3 * COMOB_DEC_FACTOR)) +
			 COMOB_DEC_FACTOR);

			/* Set lifespan */
			Dust_COMOB->Lifespan = 40 + (rand() % 20);

			/* Set draw mode */
			Dust_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			Dust_COMOB->Special_handle = Transluminance_table_handle;
			Dust_COMOB->Special_offset = 0;

			/* Set hotspot offsets */
			Dust_COMOB->Hotspot_X_offset = 50;
			Dust_COMOB->Hotspot_Y_offset = 50;

			/* Set display dimensions */
			Dust_COMOB->Display_width = 40;
			Dust_COMOB->Display_height = 40;

			/* Set graphics handle and offset */
			Dust_COMOB->Graphics_handle = Spark_gfx_handles[0];
			Dust_COMOB->Graphics_offset = 0;

			/* Set number of animation frames */
			Dust_COMOB->Nr_frames = Get_nr_frames(Dust_COMOB->Graphics_handle);

			/* Set initial animation frame */
			Dust_COMOB->Frame = 0;
//		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dust_II_handler
 * FUNCTION  : Fairy dust COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.06.95 16:30
 * LAST      : 11.07.95 14:32
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dust_II_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB_behaviour *Behaviour_data;
	struct COMOB *Dust_COMOB;
	struct COMOB *New_COMOB;
	SILONG *ldata;
	SILONG t;

	/* Zeiger auf BehaviourDaten */
	ldata=&Behaviour->Data.Just_data.Data[0];

	/* Get pointer to dust COMOB */
	Dust_COMOB = Behaviour->First_COMOB;

	/* Farbe des DustPartikels */
	Dust_COMOB->Colour = fairy_greycol[ldata[1]];

	/* Farbe aufhellen */
	ldata[1]+=ldata[2];

	/* Farbe 'bouncen' */
	if(ldata[1]==16||ldata[1]==0){
		ldata[2]=-ldata[2];
	}

	/* Reached the floor ? */
	if (Dust_COMOB->Y_3D <= 0)
	{
		/* Yes -> Kill */
		Delete_COMOB(Dust_COMOB);
		return;
	}

	/* Is this a big dust particle ? */
	if (ldata[0] == DUST_LEAD)
	{
		/* Yes -> First time ? */
		if (First_combat_update)
		{
			/* Yes -> Generate new dust particle ? */
			if (rand()>(RAND_MAX/2))
			{

				/* Yes -> Create COMOB */
				New_COMOB = Add_COMOB(100);
				if (New_COMOB)
				{
					/* Tiny -> Copy coordinates from lead dust COMOB */
					New_COMOB->X_3D = Dust_COMOB->X_3D + rndmm(-2000,2000);
					New_COMOB->Y_3D = Dust_COMOB->Y_3D + rndmm(-1000,1000);
					New_COMOB->Z_3D = Dust_COMOB->Z_3D + rndmm(-100,100);

					/* Copy drop speed, but slower */
					New_COMOB->dY_3D = Dust_COMOB->dY_3D-(Dust_COMOB->dY_3D/rndmm(7,10));

					/* Set lifespan */
					New_COMOB->Lifespan = 20 + (rand() % 10);

					/* Set draw mode */
					New_COMOB->Draw_mode = SILHOUETTE_COMOB_DRAWMODE;

					/* Set hotspot offsets */
					New_COMOB->Hotspot_X_offset = 50;
					New_COMOB->Hotspot_Y_offset = 50;

					/* Set display dimensions */
					New_COMOB->Display_width  = rndmm(20,40);
					New_COMOB->Display_height = New_COMOB->Display_width;

					/* Set graphics handle and offset */
					New_COMOB->Graphics_handle =Spark_gfx_handles[rndmm(0,2)];
					New_COMOB->Graphics_offset = 0;

					/* Set number of animation frames */
					New_COMOB->Nr_frames = 0;//Get_nr_frames(New_COMOB->Graphics_handle);

					/* Set initial animation frame */
					New_COMOB->Frame = 0;//rndmm(0,New_COMOB->Nr_frames);

					/* Add dust behaviour */
					Behaviour_data = Add_COMOB_behaviour(New_COMOB, NULL,Dust_II_handler);
					if (!Behaviour_data)
					{
						Delete_COMOB(New_COMOB);
						return;
					}

					/* Kleines Staubteilchen */
					Behaviour_data->Data.Just_data.Data[0] = DUST_TINY;
					Behaviour_data->Data.Just_data.Data[1] = rndmm(5,10);
					if(rand()>(RAND_MAX)/2){
						Behaviour_data->Data.Just_data.Data[2] = -rndmm(1,2);
					}
					else{
						Behaviour_data->Data.Just_data.Data[2] = rndmm(1,2);
					}

					/* Farbe des Dusts */
					New_COMOB->Colour = fairy_greycol[Behaviour_data->Data.Just_data.Data[1]];

				}

			}

		}

	}

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Animcontrol_II_handler
 * FUNCTION  : Animcontrol COMOB behaviour handler.
 *						 Im Gegensatz zum normalen animcontrol_handler wird das COMOB
 *						 am Ende nicht gelîscht sondern verbleibt in der letzten Phase
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.06.95 10:28
 * LAST      : 29.06.95 10:28
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Animcontrol_II_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	UNSHORT Frames_per_cycle;
	UNSHORT Total_nr_frames;
	UNSHORT Frame;

	/* Get pointer to COMOB */
	COMOB = Behaviour->First_COMOB;

	/* Circle or wave animation ? */
	if (COMOB->User_flags & COMOB_WAVEANIM)
	{
		/* Wave -> Calculate number of frames per cycle */
		Frames_per_cycle = ((Behaviour->Data.Animcontrol_data.Nr_frames
		 * 2) - 2);
	}
	else
	{
		/* Circle -> Calculate number of frames per cycle */
		Frames_per_cycle = Behaviour->Data.Animcontrol_data.Nr_frames;
	}

	/* Calculate total number of frames */
	Total_nr_frames = Behaviour->Data.Animcontrol_data.Nr_repeats *
	 Frames_per_cycle;

	/* Calculate current animation frame */
	Frame = ((Behaviour->Data.Animcontrol_data.Counter *
	 Total_nr_frames) / Behaviour->Data.Animcontrol_data.Duration) %
	 Frames_per_cycle;

	/* Wave animation ? */
	if (COMOB->User_flags & COMOB_WAVEANIM)
	{
		/* Yes -> On the way back ? */
		if (Frame >= Behaviour->Data.Animcontrol_data.Nr_frames)
		{
			/* Yes -> Determine animation frame */
			Frame = (2 * (Behaviour->Data.Animcontrol_data.Nr_frames - 1)) -
			 Frame;
		}
	}

	/* Set animation frame */
	COMOB->Frame = Frame;

	/* Increase index */
	Behaviour->Data.Animcontrol_data.Counter++;

	/* End ? */
	if (Behaviour->Data.Animcontrol_data.Counter ==
	 Behaviour->Data.Animcontrol_data.Duration)
	{
		/* Yes -> Destroy COMOB */
//		Delete_COMOB(COMOB);
		Delete_COMOB_behaviour(Behaviour);

	}
}






























#if FALSE
/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : add_schweif
 * FUNCTION : Addiert einen Schweif zum COMOB, der Schweif ist nur 2D
 * INPUTS   : struct COMOB *COMOB : UrsprungsCOMOB
 *            SILONG trails : Anzahl Trails
 *            SILONG mindistance : minimale Distance
 *            SILONG maxdistance : maximale Distance
 *						SILONG sizelose : Wert der bei jedem Trail zur Grî·e addiert wird
 * RESULT   : struct COMOB ** : Zeiger auf Liste mit Trail COMOBS
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define MAX_ADD_SCHWEIF_COMOBS 16

static struct COMOB *add_schweif_comobs[MAX_ADD_SCHWEIF_COMOBS];

struct COMOB **Add_schweif(struct COMOB *COMOB,SILONG trails,SILONG mindistance,SILONG maxdistance,SILONG sizelose)
{
	struct COMOB_behaviour *behave_trail;
	struct COMOB *source_COMOB;
	SILONG s,u;

	/* Anzahl Trails begrenzen */
	if(trails>MAX_ADD_SCHWEIF_COMOBS){
		trails=MAX_ADD_SCHWEIF_COMOBS-1;
	}

	/* Add Schweif Energie COMOB */
	source_COMOB=COMOB;
	for(s=0;s<trails;s++){
		add_schweif_comobs[s] = Add_COMOB(100);

		/* Success ? */
		if (!add_schweif_comobs[s]){
			/* No -> Clear error stack */
			ERROR_ClearStack();
			/* Kein Trail */
			return(NULL);
		}

		/* Position kopieren */
		add_schweif_comobs[s]->X_3D = source_COMOB->X_3D;
		add_schweif_comobs[s]->Y_3D = source_COMOB->Y_3D;
		add_schweif_comobs[s]->Z_3D = source_COMOB->Z_3D+(s+1);

		/* delta kopieren */
		add_schweif_comobs[s]->dX_3D = source_COMOB->dX_3D;
		add_schweif_comobs[s]->dY_3D = source_COMOB->dY_3D;
		add_schweif_comobs[s]->dZ_3D = source_COMOB->dZ_3D;

		/* Set Schweifstart display parameters */
		add_schweif_comobs[s]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		add_schweif_comobs[s]->Special_handle = Transluminance_table_handle;

		add_schweif_comobs[s]->Hotspot_X_offset = source_COMOB->Hotspot_X_offset; /* in % */
		add_schweif_comobs[s]->Hotspot_Y_offset = source_COMOB->Hotspot_Y_offset;

		add_schweif_comobs[s]->Display_width = source_COMOB->Display_width+sizelose;
		add_schweif_comobs[s]->Display_height = source_COMOB->Display_width+sizelose;

		add_schweif_comobs[s]->Graphics_handle = source_COMOB->Graphics_handle;
		add_schweif_comobs[s]->Nr_frames = source_COMOB->Nr_frames;
		add_schweif_comobs[s]->Frame = source_COMOB->Frame;

		/* Add Trail behaviour for EnergieObject */
		behave_trail = Add_COMOB_behaviour(add_schweif_comobs[s], source_COMOB, Trail2D_handler);

		if(!behave_trail){
			/* Comob lîschen */
			Delete_COMOB(add_schweif_comobs[s]);
			/* No -> Clear error stack */
			ERROR_ClearStack();
			/* Kein Trail */
			return(NULL);
		}


		/* Set Trail behaviour data */
		behave_trail->Data.Trail_data.Minimum_distance = mindistance;
		behave_trail->Data.Trail_data.Maximum_distance = maxdistance;

		source_COMOB=add_schweif_comobs[s];
	}

	return(&add_schweif_comobs[0]);
}

/* #FUNCTION END# */
/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : Offset_handler
 * FUNCTION :
 * INPUTS   : struct COMOB_behaviour *Behaviour :
 *						SILONG data[0] = X Position
 *						SILONG data[1] = Y Position
 *						SILONG data[2] = Z Position
 *						SILONG data[3] = Zeiger auf OffsetTabelle
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void Offset_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	UNSHORT *wdata;
	SILONG *ldata;
	SILONG frame,t;

	/* Zeiger auf BehaviourDaten */
	ldata=&Behaviour->Data.Just_data.Data[0];

	/* Get pointer to trailing COMOB */
	COMOB = Behaviour->First_COMOB;

	/* Tabelle mit Offsets */
	wdata=(UNSHORT *)ldata[5];

	/* Aktuelles AnimFrame */
	frame = COMOB->Frame;

	/* 79x74 */
	/* Set vertical offset */
	COMOB->X_3D = ldata[0] + (((SILONG)wdata[(frame*2)] *	COMOB->Display_width) * COMOB_DEC_FACTOR) / ldata[3];
	COMOB->Y_3D = ldata[1] + (((SILONG)wdata[(frame*2)+1] *	COMOB->Display_height) * COMOB_DEC_FACTOR) / ldata[4];
	COMOB->Z_3D = ldata[2];

}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Trail2D_handler
 * FUNCTION  : Trail COMOB behaviour handler. only X and Y
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.05.95 15:41
 * LAST      : 29.05.95 15:35
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Trail2D_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *Trailing_COMOB;
	struct COMOB *Trailed_COMOB;
	SILONG dX_3D, dY_3D;
	SILONG Length;
	SILONG Distance;

	/* Get pointer to trailing COMOB */
	Trailing_COMOB = Behaviour->First_COMOB;

 	/* Get pointer to trailed COMOB */
	Trailed_COMOB = Behaviour->Second_COMOB;

	/* Calculate vector */
	dX_3D = Trailed_COMOB->X_3D - Trailing_COMOB->X_3D;
	dY_3D = Trailed_COMOB->Y_3D - Trailing_COMOB->Y_3D;

	/* Calculate vector length */
	Length = (dX_3D * dX_3D) + (dY_3D * dY_3D) ;
	if (Length)
	{
		Length = (SILONG) sqrt((double) Length);
	}

	/* Get current distance */
	Distance = (SILONG) Behaviour->Data.Trail_data.Current_distance;

	/* Is vector length zero ? */
	if (Length)
	{
		/* No -> Determine new distance */
		Distance += sgn(Length - Distance) * COMOB_DEC_FACTOR;

		/* Clip distance */
		Distance = min(max(Distance,
		 (SILONG) Behaviour->Data.Trail_data.Minimum_distance),
		 (SILONG) Behaviour->Data.Trail_data.Maximum_distance);

		/* Calculate vector components */
		dX_3D *= (Length - Distance);
		dY_3D *= (Length - Distance);

		dX_3D /= Length;
		dY_3D /= Length;
	}
	else
	{
		/* Yes -> Clear vector */
		dX_3D = 0;
		dY_3D = 0;
	}

	/* Store vector components */
	Trailing_COMOB->X_3D += dX_3D;
	Trailing_COMOB->Y_3D += dY_3D;

	/* Store current distance */
	Behaviour->Data.Trail_data.Current_distance = (UNSHORT) Distance;
}

#endif
