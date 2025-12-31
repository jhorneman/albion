/************
 * NAME     : COMMAG3.C
 * AUTOR    : Rainer Reber, BlueByte
 * START    : 24-4-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMMAGIC.H
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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_1_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:34
 * LAST      : 14.06.95 10:26
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_1_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_1);
}

void
Do_C3_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	MEM_HANDLE Firering_handle = NULL;
	MEM_HANDLE Fireball_handle = NULL;
	struct COMOB *Fireball_COMOB;
	struct COMOB *Firering_COMOB;
	struct COMOB *COMOB;
	struct COMOB_behaviour *Behaviour_data;
	SILONG X_3D, Y_3D, Z_3D;
	SILONG dX_3D, dY_3D, dZ_3D;
	UNSHORT Nr_steps;
	UNSHORT i, j;

	/* Load graphics */
	Firering_handle = Load_subfile(COMBAT_GFX, 20);
	if (!Firering_handle)
		return;

	Fireball_handle = Load_subfile(COMBAT_GFX, 21);
	if (!Fireball_handle)
	{
		MEM_Free_memory(Firering_handle);
		return;
	}

	/* Make COMOBs */
	Fireball_COMOB = Add_COMOB(100);
	Firering_COMOB = Add_COMOB(100);

	/* Success ? */
	if (Fireball_COMOB && Firering_COMOB)
	{
		/* Yes -> Prepare fireball movement */
		Nr_steps = Prepare_COMOB_movement(Current_use_magic_data.Casting_participant,
		 Fireball_COMOB, Victim_part->Tactical_X, Victim_part->Tactical_Y, 15);

		/* Save movement vector */
		dX_3D = Fireball_COMOB->dX_3D;
		dY_3D = Fireball_COMOB->dY_3D;
		dZ_3D = Fireball_COMOB->dZ_3D;

		/* Clear movement vector */
		Fireball_COMOB->dX_3D = 0;
		Fireball_COMOB->dY_3D = 0;
		Fireball_COMOB->dZ_3D = 0;

		/* Set fireball display parameters */
		Fireball_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		Fireball_COMOB->Special_handle = Transluminance_table_handle;

		Fireball_COMOB->Hotspot_X_offset = 50;
		Fireball_COMOB->Hotspot_Y_offset = 50;

		Fireball_COMOB->Display_width = 100 - (15 * 4);
		Fireball_COMOB->Display_height = 100 - (15 * 4);

		Fireball_COMOB->Graphics_handle = Fireball_handle;

		Fireball_COMOB->Nr_frames =
		 Get_nr_frames(Fireball_COMOB->Graphics_handle);

		/* Copy fireball coordinates to firering COMOB */
		Firering_COMOB->X_3D = Fireball_COMOB->X_3D;
		Firering_COMOB->Y_3D = Fireball_COMOB->Y_3D;
		Firering_COMOB->Z_3D = Fireball_COMOB->Z_3D;

		/* Set firering display parameters */
		Firering_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		Firering_COMOB->Special_handle = Transluminance_table_handle;

		Firering_COMOB->Hotspot_X_offset = 50;
		Firering_COMOB->Hotspot_Y_offset = 50;

		Firering_COMOB->Display_width = 100 + (15 * 20);
		Firering_COMOB->Display_height = 100 + (15 * 20);

		Firering_COMOB->Graphics_handle = Firering_handle;

		Firering_COMOB->Nr_frames =
		 Get_nr_frames(Firering_COMOB->Graphics_handle);

		/* Build up the fireball and the firering */
		for (i=0;i<15;i++)
		{
			/* Draw combat screen */
			Update_screen();

			/* Make fireball bigger */
			Fireball_COMOB->Display_width += 4;
			Fireball_COMOB->Display_height += 4;

			/* Make firering smaller */
			Firering_COMOB->Display_width -= 20;
			Firering_COMOB->Display_height -= 20;
		}

		/* Hide firering */
		Hide_COMOB(Firering_COMOB);

		/* Set fireball's movement vector */
		Fireball_COMOB->dX_3D = dX_3D;
		Fireball_COMOB->dY_3D = dY_3D;
		Fireball_COMOB->dZ_3D = dZ_3D;

		/* Let fireball move towards target */
		for (i=0;i<Nr_steps;i++)
		{
			Update_screen();
		}
	}
	else
	{
		/* No -> Clear error stack (the player need not know) */
		ERROR_ClearStack();

		/* Delete fireball if necessary */
		if (Fireball_COMOB)
			Delete_COMOB(Fireball_COMOB);

		/* Delete firering if necessary */
		if (Firering_COMOB)
			Delete_COMOB(Firering_COMOB);
	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Continue with effect ? */
		if (Fireball_COMOB && Firering_COMOB)
		{
			/* Yes -> Rest of effect */
			/* Show firering */
			Show_COMOB(Firering_COMOB);

			/* Get fireball coordinates */
			X_3D = Fireball_COMOB->X_3D;
			Y_3D = Fireball_COMOB->Y_3D;
			Z_3D = Fireball_COMOB->Z_3D;

			/* Delete fireball and firering */
			Delete_COMOB(Fireball_COMOB);
			Delete_COMOB(Firering_COMOB);

			Gen_Sparks(X_3D + COMOB->dX_3D,Y_3D + COMOB->dY_3D,Z_3D + COMOB->dZ_3D,40/*amount*/,200/*speed*/,100/*life*/,40/*size*/,GEN_SPARK_TYP_ORANGE);

		}

 		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));
	}

	/* Free memory */
	if (Firering_handle)
		MEM_Free_memory(Firering_handle);
	if (Fireball_handle)
		MEM_Free_memory(Fireball_handle);
}


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_2_handler
 *             Long-range combat spell on single Monster (energy)
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    : R.Reber
 * FIRST     : 06.06.95 13:34
 * LAST      : 06.06.95 13:34
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_2_MAX_ENERGIE 8
#define C3_Spell_2_ENERGIE_SIZE 75
#define C3_Spell_2_SCHWEIFLEN 3
#define C3_Spell_2_HITACCELSTEPS 32
#define C3_Spell_2_EFFEKT_VOR_MONSTER_Z 0*COMOB_DEC_FACTOR

#define C3_Spell_2_MAX_LEUCHTKUGELN 8
#define C3_Spell_2_LEUCHTKUGEL_SIZE 150 /* muss > 100 sein */
#define C3_Spell_2_LEUCHTKUGELFALLSTEPS 20
#define C3_Spell_2_LEUCHTKUGELSTERNTEPS 32

void
C3_Spell_2_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_2);
}

void
Do_C3_Spell_2(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	MEM_HANDLE energie_handle = NULL;
	struct COMOB *COMOB[C3_Spell_2_MAX_ENERGIE];
	struct COMOB *SCH_COMOB[C3_Spell_2_MAX_ENERGIE][C3_Spell_2_SCHWEIFLEN];
	struct COMOB_behaviour *behave_x[C3_Spell_2_MAX_ENERGIE];
	struct COMOB_behaviour *behave_y[C3_Spell_2_MAX_ENERGIE];
	struct COMOB_behaviour *behave_shadow[C3_Spell_2_MAX_ENERGIE];
	struct COMOB_behaviour *behave_trail;
	SILONG ox[C3_Spell_2_MAX_ENERGIE],oy[C3_Spell_2_MAX_ENERGIE],oz[C3_Spell_2_MAX_ENERGIE];
	SILONG X_3D, Y_3D, Z_3D,zx,zy,zz;
	SILONG olx[C3_Spell_2_MAX_LEUCHTKUGELN],oly[C3_Spell_2_MAX_LEUCHTKUGELN],olz[C3_Spell_2_MAX_LEUCHTKUGELN];
	SILONG px,pz,mx[C3_Spell_2_MAX_LEUCHTKUGELN],mz[C3_Spell_2_MAX_LEUCHTKUGELN];
	SILONG s,t,u,steps,max_steps;
	SILONG left,right,top,bottom,height;
	SILONG C3_Spell_2_ENERGIE;
	SILONG C3_Spell_2_LEUCHTKUGELN;
	double angle;
	BOOLEAN bool;

	/* Load graphics */
	energie_handle = Load_subfile(COMBAT_GFX, 36);
	if (!energie_handle)
		return;

	/* SpruchstÑrke = Anzahl der EnergieLadungen (3-MAX_Energie) */
	C3_Spell_2_ENERGIE=calc_strength(Strength,3,C3_Spell_2_MAX_ENERGIE-3);

	/* SpruchstÑrke = Anzahl der Leuchtkugeln (3-MAX_Leuchtkugeln) */
	C3_Spell_2_LEUCHTKUGELN=calc_strength(Strength,3,C3_Spell_2_MAX_LEUCHTKUGELN);

	/* Grî·e des Gegners */
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

	/* Hîhe des Viechs */
	height=(top-bottom)/3;

	/* 10 EnergieLadungen generieren */
	for (t=0;t<C3_Spell_2_ENERGIE;t++){

		/* Add Energie COMOB */
		COMOB[t] = Add_COMOB(100);

		/* kein COMOB */
		if (!COMOB[t]){
			/* No -> Clear error stack */
			ERROR_ClearStack();

			goto C3_2_Error;
		}

		/* Koordinaten des Gegners holen */
		Get_3D_part_coordinates(Victim_part,&COMOB[t]->X_3D,&COMOB[t]->Y_3D,&COMOB[t]->Z_3D);

		/* auf oberes Drittel setzen */
		COMOB[t]->Y_3D+=height;
		COMOB[t]->Z_3D-=C3_Spell_2_EFFEKT_VOR_MONSTER_Z;

//		COMOB[t]->X_3D+=(t*10*COMOB_DEC_FACTOR);
//		COMOB[t]->Y_3D+=(t*15*COMOB_DEC_FACTOR);
//		COMOB[t]->Z_3D+=(t*20*COMOB_DEC_FACTOR);

		/* Set Schweifstart display parameters */
		COMOB[t]->Draw_mode = NORMAL_COMOB_DRAWMODE;

		COMOB[t]->Hotspot_X_offset = 50; /* in % */
		COMOB[t]->Hotspot_Y_offset = 50;

		COMOB[t]->Display_width = C3_Spell_2_ENERGIE_SIZE; /* in % */
		COMOB[t]->Display_height = C3_Spell_2_ENERGIE_SIZE;

		COMOB[t]->Graphics_handle = energie_handle;

		COMOB[t]->Nr_frames =
			Get_nr_frames(COMOB[t]->Graphics_handle);

		/* Schatten dazuaddieren */
//		Add_shadow_to_COMOB(COMOB[t]);

		/* Add oscillate behaviour for x */
		behave_x[t] = Add_COMOB_behaviour(COMOB[t], NULL, Oscillate_handler);

		if(!behave_x[t]){
			/* Comob lîschen */
			Delete_COMOB(COMOB[t]);

			/* No -> Clear error stack */
			ERROR_ClearStack();

			goto C3_2_Error;
		}

		/* Set random bounce behaviour data */
		behave_x[t]->Data.Oscillate_data.Type = OSCILLATE_X;
		behave_x[t]->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
		behave_x[t]->Data.Oscillate_data.Amplitude = 50*COMOB_DEC_FACTOR;
		behave_x[t]->Data.Oscillate_data.Value = t*90/C3_Spell_2_ENERGIE;


		/* Add oscillate behaviour for y */
		behave_y[t] = Add_COMOB_behaviour(COMOB[t], NULL, Oscillate_handler);

		if(!behave_y[t]){
			/* Comob lîschen */
			Delete_COMOB(COMOB[t]);

			/* No -> Clear error stack */
			ERROR_ClearStack();

			goto C3_2_Error;
		}

		/* Set random bounce behaviour data */
		behave_y[t]->Data.Oscillate_data.Type = OSCILLATE_Y;
		behave_y[t]->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
		behave_y[t]->Data.Oscillate_data.Amplitude = 50*COMOB_DEC_FACTOR;
		behave_y[t]->Data.Oscillate_data.Value = (t*90/C3_Spell_2_ENERGIE)+23;


		/* Add Schweif Energie COMOB */
		u=100;
		for(s=0;s<C3_Spell_2_SCHWEIFLEN;s++){
			SCH_COMOB[t][s] = Add_COMOB(u);

			/* Success ? */
			if (!SCH_COMOB[t][s]){
				/* No -> Clear error stack */
				ERROR_ClearStack();
				goto C3_2_Error;
			}
			u-=5;

			/* Set Schweifstart display parameters */
			SCH_COMOB[t][s]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			SCH_COMOB[t][s]->Special_handle = Transluminance_table_handle;

			SCH_COMOB[t][s]->Hotspot_X_offset = 50; /* in % */
			SCH_COMOB[t][s]->Hotspot_Y_offset = 50;

			SCH_COMOB[t][s]->Display_width = C3_Spell_2_ENERGIE_SIZE-(s*(C3_Spell_2_ENERGIE_SIZE/(C3_Spell_2_SCHWEIFLEN+1))); /* in % */
			SCH_COMOB[t][s]->Display_height = C3_Spell_2_ENERGIE_SIZE-(s*(C3_Spell_2_ENERGIE_SIZE/(C3_Spell_2_SCHWEIFLEN+1)));

			SCH_COMOB[t][s]->Graphics_handle = energie_handle;

			SCH_COMOB[t][s]->Nr_frames =
				Get_nr_frames(SCH_COMOB[t][s]->Graphics_handle);

			/* Add Trail behaviour for EnergieObject */
			if(s==0){
				behave_trail = Add_COMOB_behaviour(SCH_COMOB[t][s], COMOB[t], Trail_handler);
			}
			else{
				behave_trail = Add_COMOB_behaviour(SCH_COMOB[t][s], SCH_COMOB[t][s-1], Trail_handler);
			}

			if(!behave_trail){
				/* Comob lîschen */
				Delete_COMOB(SCH_COMOB[t][s]);

				/* No -> Clear error stack */
				ERROR_ClearStack();

				goto C3_2_Error;
			}


			/* Set Trail behaviour data */
			behave_trail->Data.Trail_data.Minimum_distance = 4*COMOB_DEC_FACTOR;
			behave_trail->Data.Trail_data.Maximum_distance = 8*COMOB_DEC_FACTOR;

		}


	}

	/* Tuerkise Energieladung rotieren lassen */
	update_n_times(24);

	/* Drehbewegungsbevahiour lîschen */
	for (t=0;t<C3_Spell_2_ENERGIE;t++){
		Delete_COMOB_behaviour(behave_x[t]);
		Delete_COMOB_behaviour(behave_y[t]);
	}

	/* Tuerkise Energieladung kurz stehen lassen */
	update_n_times(3);

	/* StartKoordinaaten zwischenspeichern */
	for (t=0;t<C3_Spell_2_ENERGIE;t++){
		ox[t]=COMOB[t]->X_3D;
		oy[t]=COMOB[t]->Y_3D;
		oz[t]=COMOB[t]->Z_3D;
	}

	/* ZielKoordinate fÅr EnergiebÑlle ausrechnen */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* EnergieLadungen in 16 Stufen auf ObjectMittelpunkt zufliegen lassen */
	/* Nr_combat_updates */
	s=0;
	while(s<C3_Spell_2_HITACCELSTEPS){
		Update_screen();
		for (t=0;t<C3_Spell_2_ENERGIE;t++){
			COMOB[t]->Display_width=C3_Spell_2_ENERGIE_SIZE-s;
			COMOB[t]->Display_height=C3_Spell_2_ENERGIE_SIZE-s;
			COMOB[t]->X_3D=aip3d(ox[t],zx,C3_Spell_2_HITACCELSTEPS,s);
			COMOB[t]->Y_3D=aip3d(oy[t],zy,C3_Spell_2_HITACCELSTEPS,s);
			COMOB[t]->Z_3D=aip3d(oz[t],zz,C3_Spell_2_HITACCELSTEPS,s);
		}
 		s+=Nr_combat_updates;
	}

	/* Wenn das Object getroffen wurde Funken sowie Leuchthalo generieren */

	/* Funken generieren */
	Gen_Funken(zx+rndmm(-5,5)*COMOB_DEC_FACTOR,zy+rndmm(-5,5)*COMOB_DEC_FACTOR,zz,Spark_gfx_handles[1],20);

	Gen_Funken(left+(rndmm(0,10) * COMOB_DEC_FACTOR),zy+(rndmm(-5,5) * COMOB_DEC_FACTOR),zz,Spark_gfx_handles[1],20);
	Gen_Funken(right-(rndmm(0,10) * COMOB_DEC_FACTOR),zy+(rndmm(-5,5) * COMOB_DEC_FACTOR),zz,Spark_gfx_handles[1],20);
	Gen_Funken(zx+(rndmm(-5,5)* COMOB_DEC_FACTOR),top-(rndmm(0,10)* COMOB_DEC_FACTOR),zz,Spark_gfx_handles[1],20);
	Gen_Funken(zx+(rndmm(-5,5)* COMOB_DEC_FACTOR),bottom+(rndmm(0,10)* COMOB_DEC_FACTOR),zz,Spark_gfx_handles[1],20);

	/* Leuchtshilouhette */
	Add_halo_to_COMOB_2(Victim_part->Main_COMOB, 3, 20, 5,HALO_CONNECT);

	/* Leuchthalo kurze Zeit halten */
	update_n_times(5);

	/* Alle Komobs die zum EnergieKreis gehîren lîschen */
	for (t=0;t<C3_Spell_2_ENERGIE;t++){
		Delete_COMOB(COMOB[t]);
		for(s=0;s<C3_Spell_2_SCHWEIFLEN;s++){
			Delete_COMOB(SCH_COMOB[t][s]);
		}
	}


	/* Monster hat nur Schaden wenn Strength >0 ist */
	if(Strength>0){

		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));

		/******************************************************************/
		/* Kugeln fallen am Monster herunter und hinterlassen BodenglÅhen */
		/******************************************************************/

		/* Leuchtkugeln generieren */
		for(t=0;t<C3_Spell_2_LEUCHTKUGELN;t++){

			COMOB[t] = Add_COMOB(100);

			/* kein COMOB */
			if (!COMOB[t]){
				/* No -> Clear error stack */
				ERROR_ClearStack();
				goto C3_2_Error;
			}

			/* Koordinaten des Gegners holen */
			Get_3D_part_coordinates(Victim_part,&COMOB[t]->X_3D,&COMOB[t]->Y_3D,&COMOB[t]->Z_3D);

			/* auf oberes Drittel setzen */
			COMOB[t]->X_3D+=rndmm(-15,15)*COMOB_DEC_FACTOR;
			COMOB[t]->Y_3D+=(rndmm(-10,10)*COMOB_DEC_FACTOR)+height;
			COMOB[t]->Z_3D-=C3_Spell_2_EFFEKT_VOR_MONSTER_Z;

	//		COMOB[t]->X_3D+=(t*10*COMOB_DEC_FACTOR);
	//		COMOB[t]->Y_3D+=(t*15*COMOB_DEC_FACTOR);
	//		COMOB[t]->Z_3D+=(t*20*COMOB_DEC_FACTOR);

			/* Set Schweifstart display parameters */
			COMOB[t]->Draw_mode = NORMAL_COMOB_DRAWMODE;

			COMOB[t]->Hotspot_X_offset = 50; /* in % */
			COMOB[t]->Hotspot_Y_offset = 75;

			COMOB[t]->Display_width = C3_Spell_2_LEUCHTKUGEL_SIZE; /* in % */
			COMOB[t]->Display_height = C3_Spell_2_LEUCHTKUGEL_SIZE;

			COMOB[t]->Graphics_handle = Spark_gfx_handles[2];

			COMOB[t]->Nr_frames =
				Get_nr_frames(COMOB[t]->Graphics_handle);

		}

		/* StartKoordinaaten zwischenspeichern */
		for (t=0;t<C3_Spell_2_LEUCHTKUGELN;t++){
			oly[t]=COMOB[t]->Y_3D;
		}

		/* Leuchtkugeln herunterfallen lassen */
		s=0;
		while(s<C3_Spell_2_LEUCHTKUGELFALLSTEPS){
			Update_screen();
			for(t=0;t<C3_Spell_2_LEUCHTKUGELN;t++){
				COMOB[t]->Y_3D=ip3d(oly[t],0,C3_Spell_2_LEUCHTKUGELFALLSTEPS,s);
			}
			s+=Nr_combat_updates;
		}

		/* StartKoordinaaten zwischenspeichern */
		for (t=0;t<C3_Spell_2_LEUCHTKUGELN;t++){
			COMOB[t]->Y_3D=0;
			olx[t]=COMOB[t]->X_3D;
			olz[t]=COMOB[t]->Z_3D;
		}

		/* Zentrum des Objects */
		Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

		/* Delta-Bewegungswerte ausrechnen und */
		for(t=0;t<C3_Spell_2_LEUCHTKUGELN;t++){
			/* ZufÑllige Position um das Object berechnen
			 		wo die Kugeln sich hinbewegen sollen */
			angle=(double)rndmm(0,359)*3.141592654/180.0;
			px=(SILONG)(100.0*sin(angle));
			pz=(SILONG)(100.0*cos(angle));
			calc_delta_xyz(olx[t],0,olz[t],olx[t]+(px*COMOB_DEC_FACTOR),0,olz[t]+(pz*COMOB_DEC_FACTOR),&mx[t],NULL,&mz[t],3*COMOB_DEC_FACTOR);
		}

		/* Kugel spritzen am Boden sternfîrmig auseinander */
		s=0;
		while(s<C3_Spell_2_LEUCHTKUGELSTERNTEPS){
			Update_screen();
			for(u=0;u<Nr_combat_updates;u++){
				for(t=0;t<C3_Spell_2_LEUCHTKUGELN;t++){
					olx[t]+=mx[t];
					olz[t]+=mz[t];
					COMOB[t]->X_3D=olx[t];
					COMOB[t]->Z_3D=olz[t];
					if(COMOB[t]->Display_width>25){
						COMOB[t]->Display_width-=1;
						COMOB[t]->Display_height=COMOB[t]->Display_width;
					}
					groundglow(COMOB[t]->X_3D,COMOB[t]->Z_3D,100,40);
				}
			}
			s+=Nr_combat_updates;
		}

		/* kurze Zeit warten */
		update_n_times(12);

		/* Alle Komobs die zu den Leuchtkugeln gehîren lîschen */
		for (t=0;t<C3_Spell_2_LEUCHTKUGELN;t++){
			Delete_COMOB(COMOB[t]);
		}

	}

C3_2_Error:
	/* Free memory */
	if (energie_handle)
		MEM_Free_memory(energie_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_3_handler
 *						 Long-range combat spell on a row of Monsters (fire)
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    : R.Reber
 * FIRST     : 06.06.95 13:35
 * LAST      : 06.06.95 13:35
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_3_MIN_SCHNEE_VER 10
#define C3_Spell_3_MAX_SCHNEE_VER 4
#define C3_Spell_3_MIN_SCHNEE_HOR 6
#define C3_Spell_3_MAX_SCHNEE_HOR 14
#define C3_Spell_3_MIN_SCHNEE_POS (-COMBAT_3D_WIDTH+20)
#define C3_Spell_3_MAX_SCHNEE_POS (COMBAT_3D_WIDTH-20)

#define C3_Spell_3_MIN_FLAMESIZE 40
#define C3_Spell_3_MAX_FLAMESIZE 100

void
C3_Spell_3_handler(UNSHORT Strength)
{
	struct COMOB_behaviour *behave_size;
	struct COMOB_behaviour *behave_snow;
	struct COMOB *snow_COMOB;
	struct Combat_participant *Victim_part;
	UNSHORT Temp_strength;
	UNSHORT i;
	SILONG sx=0,sy=0,sz=0;
	SILONG lx,ly,lz;
	SILONG x,y,z;
	SILONG mx,my,mz;
	SILONG s,t,u,steps;
	SILONG flamesize,flameballsize,snowamountver,snowamounthor,createsnow;

	/* SpruchstÑrke = Grî·e der Flamen,FlamenbÑlle */
	flamesize=calc_strength(Strength,C3_Spell_3_MIN_FLAMESIZE,C3_Spell_3_MAX_FLAMESIZE);
	flameballsize=30*flamesize/100;

	/* SpruchstÑrke = Anzahl 'Schnee' der erscheint */
	snowamountver=calc_strength(Strength,C3_Spell_3_MIN_SCHNEE_VER,C3_Spell_3_MAX_SCHNEE_VER);
	snowamounthor=calc_strength(Strength,C3_Spell_3_MIN_SCHNEE_HOR,C3_Spell_3_MAX_SCHNEE_HOR);

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_3);

	/* Koordinaaten des Monsters holen welches den Spruch wirft */
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&sx,&sy,&sz);

	/* Schneeerstellverzîgerungsvariable */
	createsnow=0;

	for (i=0;i<Current_nr_target_parts;i++)
	{
		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
			/* Yes -> Start of effect */
//		}

		/* Letzte 3D Koordinaate von vorigem Monster */
		lx=sx;
		ly=sy;
		lz=sz;
		Get_3D_part_coordinates(Victim_part,&sx,&sy,&sz);

		/* Wenn Flamme am ersten Monster dan Effekt starten */

		/* Delta-Bewegungswerte ausrechnen und */
		steps=calc_delta_xyz(lx,ly,lz,sx,sy,sz,&mx,&my,&mz,12*COMOB_DEC_FACTOR);

		/* alte Koordinaaten merken */
		x=lx;
		y=ly;
		z=lz;

		/* Flame bewegt sich von vorigem Opfer zum nÑchsten */
		s=steps;
		while(s>0){

			/* kurze Zeit warten */
			update_n_times(2);

			createsnow-=Nr_combat_updates;
			if(createsnow<0){

				do{

					/* Reihe horizontaler Schneeflocken an der oberen Seite erstellen */
					for(t=0;t<snowamounthor;t++){
						snow_COMOB=Add_COMOB(100);
						/* kein COMOB */
						if (!snow_COMOB){
							/* No -> Clear error stack */
							ERROR_ClearStack();
							goto no_COMOB;
						}

						/* Set lifespan */
						snow_COMOB->Lifespan = 0;

//						snow_COMOB->X_3D=ip3d(C3_Spell_3_MIN_SCHNEE_POS,C3_Spell_3_MAX_SCHNEE_POS,snowamounthor,t)+rndmm(-3*COMOB_DEC_FACTOR,3*COMOB_DEC_FACTOR);
						snow_COMOB->X_3D=ip3d(C3_Spell_3_MIN_SCHNEE_POS,C3_Spell_3_MAX_SCHNEE_POS,snowamounthor,t)+rndmm(-30*COMOB_DEC_FACTOR,30*COMOB_DEC_FACTOR);
						snow_COMOB->Y_3D=(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR)+rndmm(-15*COMOB_DEC_FACTOR,30*COMOB_DEC_FACTOR)+(30*COMOB_DEC_FACTOR);
						snow_COMOB->Z_3D=sz;

//						snow_COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;
						snow_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	 					snow_COMOB->Special_handle = Luminance_table_handle;

						snow_COMOB->Hotspot_X_offset = 50; /* in % */
						snow_COMOB->Hotspot_Y_offset = 50;

						snow_COMOB->Display_width = 100;
						snow_COMOB->Display_height = 100;

						snow_COMOB->Graphics_handle = Spark_gfx_handles[6];

						snow_COMOB->Nr_frames =	0;//Get_nr_frames(snow_COMOB->Graphics_handle);

						snow_COMOB->Frame = rndmm(0,Get_nr_frames(snow_COMOB->Graphics_handle)-1);

						/* Behaviour fÅr Schneeflocken */
						behave_snow = Add_COMOB_behaviour(snow_COMOB, NULL, snow_handler);

						/* Grî·e Ñndern */
						if(!behave_snow){
							/* Comob lîschen */
							Delete_COMOB(snow_COMOB);
							/* No -> Clear error stack */
							ERROR_ClearStack();
							goto no_COMOB;
						}

//						behave_snow->Data.Just_data.Data[0]=0; /* jeden 4 Tick um .075 % */

					}

					createsnow+=snowamountver;

				}while(createsnow<0);

			}

			for(u=0;u<Nr_combat_updates;u++){
				x+=mx;
				y+=my;
				z+=mz;

				/* Flamme erstellen */
				if(makeflame(x,y,z-(5*COMOB_DEC_FACTOR),flamesize)==NULL){
					ERROR_ClearStack();
					goto no_COMOB;
				}

			}

			s-=Nr_combat_updates;

		}

no_COMOB:

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if(Temp_strength)
		{
			/* No -> Victim is monster ? */
			if (Victim_part->Type == MONSTER_PART_TYPE)
			{
				/* Yes -> Rest of effect */
			}

			/* Aufpralleffekt */
// 		Gen_Sparks(x,y,z,20);

	 		/* Do damage to victim */
			Do_combat_damage(Victim_part, max(((Temp_strength * 50) / 100), 1));

		}
	}

	/* kurze Zeit warten bis Effekt beendet */
	update_n_times(64);

}

void
Do_C3_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Store pointer */
	Current_target_parts[Current_nr_target_parts] = Victim_part;

	/* Count up */
	Current_nr_target_parts++;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_4_handler
 *						 Long-range combat spell on a row of monsters (energy)
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:35
 * LAST      : 06.06.95 13:35
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      :
 *
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_4_MIN_BLITZSIZE 80
#define C3_Spell_4_MAX_BLITZSIZE 260

#define C3_Spell_4_MIN_FLASHTIME 30
#define C3_Spell_4_MAX_FLASHTIME 50
#define C3_Spell_4_MAX_FLASHTIMELIGHT 6

#define C3_Spell_4_ANZHALOCOMOBS 3

void
C3_Spell_4_handler(UNSHORT Strength)
{
	struct COMOB_behaviour *behave_size;
	struct COMOB_behaviour *behave_flash;
	MEM_HANDLE Recolour_table_handle[C3_Spell_4_ANZHALOCOMOBS];
	UNBYTE *Recolour_table_ptr[C3_Spell_4_ANZHALOCOMOBS];
	MEM_HANDLE blitz_handle = NULL;
	MEM_HANDLE energie_handle = NULL;
	MEM_HANDLE explode_handle = NULL;
	struct COMOB *energie_COMOB;
	struct COMOB *blitz_COMOB;
	struct COMOB *explode_COMOB;
	struct COMOB *halo_COMOB[C3_Spell_4_ANZHALOCOMOBS];
	struct Combat_participant *Victim_part;
	UNSHORT Temp_strength;
	UNSHORT i;
	SILONG sx=0,sy=0,sz=0;
	SILONG lx,ly,lz;
	SILONG x,y,z;
	SILONG mx,my,mz;
	SILONG left,right,top,bottom,height;
	SILONG s,t,u,v,steps;
	SILONG blitzsize,energieballsize;
	SILONG movez,zadd;
	SILONG fieldmovez;


	/* TestStrength */
//	Strength=1;


	/* SpruchstÑrke = Grî·e der Flamen,FlamenbÑlle */
	blitzsize=calc_strength(Strength,C3_Spell_4_MIN_BLITZSIZE,C3_Spell_4_MAX_BLITZSIZE);
	energieballsize=120*blitzsize/100;

	/* Load Blitz graphics */
	blitz_handle = Load_subfile(COMBAT_GFX, 55);
	if (!blitz_handle){
		ERROR_ClearStack();
		return;
	}

	/* Load EnergieBall graphics */
	energie_handle = Load_subfile(COMBAT_GFX, 36);
	if (!energie_handle){
		ERROR_ClearStack();
		return;
	}

	/* Load Explode graphics */
	explode_handle = Load_subfile(COMBAT_GFX, 33);
	if (!explode_handle){
		ERROR_ClearStack();
		return;
	}

	/* zu jedem Halo COMOB eine TransparentFarbe generieren */
	s=20; /* AnfangsTransparenz */
	for(t=0;t<C3_Spell_4_ANZHALOCOMOBS;t++){
		Recolour_table_handle[t] = MEM_Allocate_memory(256);
		Recolour_table_ptr[t] = MEM_Claim_pointer(Recolour_table_handle[t]);
		Calculate_recolouring_table(Recolour_table_ptr[t], 1, 191, 180,180,255, s);
		s+=30;
	}

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_4);

	/* Frist Victim is PartyMember  all Victims are PartyMembers */
	Victim_part = Current_target_parts[0];
	if (Victim_part->Type != MONSTER_PART_TYPE){
		/* Yes -> Start of effect */
		fieldmovez=-(COMBAT_3D_DEPTH/2);
	}
	else{
		fieldmovez=0;
	}

	/* Koordinaaten des Monsters holen welches den Spruch wirft */
	Get_3D_part_coordinates(Victim_part,&sx,&sy,&sz);

	/* linke HÑlfte des Feldes */
	sx=-(COMBAT_3D_WIDTH/2);

	/*******************/
	/* EnergieBall erstellen */
	/*******************/

	/* Add energie_COMOB */
	energie_COMOB = Add_COMOB(100);

	/* Success ? */
	if (!energie_COMOB){
		ERROR_ClearStack();
		return;
	}

	/* set coordinates */
	energie_COMOB->X_3D = sx;
	energie_COMOB->Y_3D = COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR;
	energie_COMOB->Z_3D = sz+fieldmovez;

	/* Set lifespan */
	energie_COMOB->Lifespan = 0 ;

	/* Set display parameters */
	energie_COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;
//	energie_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
//	energie_COMOB->Special_handle = Luminance_table_handle;

	energie_COMOB->Hotspot_X_offset = 50;
	energie_COMOB->Hotspot_Y_offset = 50;

	/* Set random size */
	energie_COMOB->Display_width = energieballsize;
	energie_COMOB->Display_height = energieballsize;

	/* Select random spark type */
	energie_COMOB->Graphics_handle = energie_handle;

	/* Set number of animation frames */
	energie_COMOB->Nr_frames = Get_nr_frames(energie_COMOB->Graphics_handle);

	/* Select animation frame */
	energie_COMOB->Frame = 0;

	/* Halo Comobs */
	s=120;
	u=C3_Spell_4_ANZHALOCOMOBS;
	for(t=0;t<C3_Spell_4_ANZHALOCOMOBS;t++){
		halo_COMOB[t]=Duplicate_COMOB(energie_COMOB);
		halo_COMOB[t]->Display_width+=s;
		halo_COMOB[t]->Display_height+=s;
		halo_COMOB[t]->Z_3D+=u;
		halo_COMOB[t]->Draw_mode = COLOURING_COMOB_DRAWMODE;
		halo_COMOB[t]->Special_handle = Recolour_table_handle[t];
		s-=40;
		u+=2;
	}

	/*******************/
	/* Blitz erstellen */
	/*******************/

	/* Add blitz_COMOB */
	blitz_COMOB = Add_COMOB(100);

	/* Success ? */
	if (!blitz_COMOB){
		ERROR_ClearStack();
		return;
	}

	/* set coordinates */
	blitz_COMOB->X_3D = sx;
	blitz_COMOB->Y_3D = 0;
	blitz_COMOB->Z_3D = sz+fieldmovez;

	/* Set lifespan */
	blitz_COMOB->Lifespan = 0 ;

	/* Set display parameters */
	blitz_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	blitz_COMOB->Special_handle = Luminance_table_handle;

	blitz_COMOB->Hotspot_X_offset = 50;
	blitz_COMOB->Hotspot_Y_offset = 100;

	/* Set random size */
	blitz_COMOB->Display_width = blitzsize;
	blitz_COMOB->Display_height = 175;

	/* Select random spark type */
	blitz_COMOB->Graphics_handle = blitz_handle;

	/* Set number of animation frames */
	blitz_COMOB->Nr_frames = Get_nr_frames(blitz_COMOB->Graphics_handle);

	/* Select animation frame */
	blitz_COMOB->Frame = 0 ;

	/* Z Bewegung */
	movez=0;
	zadd=0;

	/* AufblitzZeit */
//		flashtime=rndmm(C3_Spell_4_MIN_FLASHTIME,C3_Spell_4_MAX_FLASHTIME);

	/* Blitz bewegen */
	for (i=0;i<Current_nr_target_parts;i++)
	{
		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
			/* Yes -> Start of effect */
//		}

		/* Letzte 3D Koordinaate von vorigem Monster */
		lx=sx;
		ly=sy;
		lz=sz;
		Get_3D_part_coordinates(Victim_part,&sx,&sy,&sz);

		/* Wenn Flamme am ersten Monster dan Effekt starten */

		/* Delta-Bewegungswerte ausrechnen und */
		steps=calc_delta_xyz(lx,ly,lz,sx,sy,sz,&mx,&my,&mz,3*COMOB_DEC_FACTOR);

		/* alte Koordinaaten merken */
		x=lx;
		y=ly;
		z=lz;

		/* blitz bewegt sich von vorigem Opfer zum nÑchsten */
		s=steps;
		while(s>0){

			/* Draw combat screen */
			Update_screen();

			/* Screen aufflashen */
//			flashtime-=Nr_combat_updates;
//			if(flashtime<0){
//				flashtime=rndmm(C3_Spell_4_MIN_FLASHTIME,C3_Spell_4_MAX_FLASHTIME);
//				flashlighttime=C3_Spell_4_MAX_FLASHTIMELIGHT;
//			}

			for(u=0;u<Nr_combat_updates;u++){
				x+=mx;
				y+=my;
				z+=mz;

				/* Blitz bewegen */
				blitz_COMOB->X_3D = x;
				blitz_COMOB->Z_3D = z+zadd;

				/* Energieball bewegen */
				energie_COMOB->X_3D = x;
				energie_COMOB->Z_3D = z+zadd;

				/* Bewegung addieren */
				for(t=0;t<C3_Spell_4_ANZHALOCOMOBS;t++){
					halo_COMOB[t]->X_3D = x;
					halo_COMOB[t]->Z_3D = z+zadd;
				}

				/* Bewegung addieren */
				zadd+=movez;
				if(zadd>1000){
					movez=rndmm(-1000,-500);
				}
				if(zadd<-1000){
					movez=rndmm(500,1000);
				}

				/* Bewegung in z */
				movez+=rndmm(-750,750);

				/* Beschleunigte Bewegung in Z begrenzen */
				if(movez>500){
					movez=500;
				}
				if(movez<-500){
					movez=-500;
				}

				/* GlÅhen am Boden */
				groundglow(x,z+zadd,100,40);

				/* Funken */
				Gen_Funken(x,5*COMOB_DEC_FACTOR,z+zadd,Spark_gfx_handles[6],4);

			}

			s-=Nr_combat_updates;

		}

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if(Temp_strength)
		{
			/* No -> Victim is monster ? */
			if (Victim_part->Type == MONSTER_PART_TYPE)
			{
				/* Yes -> Rest of effect */
			}

			/*******************/
			/* Explosion erstellen */
			/*******************/

			/* Add explode_COMOB */
			explode_COMOB = Add_COMOB(100);

			/* Success ? */
			if (!explode_COMOB){
				ERROR_ClearStack();
				return;
			}

			/* set coordinates */
			explode_COMOB->X_3D = x;
			explode_COMOB->Y_3D = y;
			explode_COMOB->Z_3D = z;

			/* Set lifespan */
			explode_COMOB->Lifespan = 40;

			/* Set display parameters */
			explode_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			explode_COMOB->Special_handle = Transluminance_table_handle;

			explode_COMOB->Hotspot_X_offset = 50;
			explode_COMOB->Hotspot_Y_offset = 50;

			/* Grî·e des Monsters in 3D */
			Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
			height=top-bottom;

			/* Set size */
			explode_COMOB->Display_width = height*150/(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR);
			explode_COMOB->Display_height = height*150/(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR);

			explode_COMOB->Graphics_handle = explode_handle;
			explode_COMOB->Nr_frames = Get_nr_frames(explode_COMOB->Graphics_handle);
			explode_COMOB->Frame = 0;

			/* Behaviour fÅr Grî·e */
			behave_size = Add_COMOB_behaviour(explode_COMOB, NULL, size_handler);

			/* Grî·e Ñndern */
			if(!behave_size){
				/* Comob lîschen */
				Delete_COMOB(explode_COMOB);
				ERROR_ClearStack();
				return;
			}

			behave_size->Data.Just_data_w.Data[0]=-1600; /* jeden Tick um 1% */
			behave_size->Data.Just_data_w.Data[1]=-200;
			behave_size->Data.Just_data_w.Data[2]=2; /* bis 25 % */
			behave_size->Data.Just_data_w.Data[3]=2;

			/* Behaviour fÅr Grî·e */
			behave_flash = Add_COMOB_behaviour(blitz_COMOB, NULL, flash_handler);

			/* Grî·e Ñndern */
			if(!behave_flash){
				/* Comob lîschen */
				Delete_COMOB(blitz_COMOB);
				ERROR_ClearStack();
				return;
			}

			behave_flash->Data.Just_data.Data[0]=8; /* X Ticks lang Palette zur entsprechenden Farbe flashen und 5 Ticks wieder zur Ursprungspalette*/
			behave_flash->Data.Just_data.Data[1]=160; /* nach hellblau umblenden */
			behave_flash->Data.Just_data.Data[2]=160;
			behave_flash->Data.Just_data.Data[3]=255;
			behave_flash->Data.Just_data.Data[4]=50; /* maximal 50 % */

			/* Do damage to victim */
			Do_combat_damage(Victim_part, max(((Temp_strength * 50) / 100), 1));

		}
	}

	/* kurze Zeit warten bis Effekt beendet */
	update_n_times(12);

	/* Halos lîschen */
	for(t=0;t<C3_Spell_4_ANZHALOCOMOBS;t++){
		Delete_COMOB(halo_COMOB[t]);
	}

	/* Blitz lîschen */
	Delete_COMOB(blitz_COMOB);
	/* EnergieBall lîschen */
	Delete_COMOB(energie_COMOB);
	/* Explode lîschen */
	Delete_COMOB(explode_COMOB);

	/* Speicher fÅr TransparenzTabellen freigeben */
	for(t=0;t<C3_Spell_4_ANZHALOCOMOBS;t++){
		MEM_Free_pointer(Recolour_table_handle[t]);
		MEM_Free_memory(Recolour_table_handle[t]);
	}

	/* Explode Handle */
	MEM_Free_memory(explode_handle);
	/* Energie Handle */
	MEM_Free_memory(energie_handle);
	/* Blitz Handle */
	MEM_Free_memory(blitz_handle);

}

void
Do_C3_Spell_4(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{

	/* Store pointer */
	Current_target_parts[Current_nr_target_parts] = Victim_part;

	/* Count up */
	Current_nr_target_parts++;

}


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_5_handler
 *						 Long-range combat spell on all monsters (fire)
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:35
 * LAST      : 06.06.95 13:35
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_5_MIN_ENERGIEBALLS 8
#define C3_Spell_5_MAX_ENERGIEBALLS 16
#define C3_Spell_5_MIN_FLAMES 6
#define C3_Spell_5_MAX_FLAMES 12

void
C3_Spell_5_handler(UNSHORT Strength)
{
	struct COMOB_behaviour *behave_bounce;
	struct COMOB_behaviour *behave_size;
	struct COMOB_behaviour *behave_move;
	struct COMOB_behaviour *behave_anim;
	struct COMOB_behaviour *behave_glow;
	struct Combat_participant *Victim_part;
	MEM_HANDLE ball_handle = NULL;
	MEM_HANDLE beam_handle = NULL;
	MEM_HANDLE flame_handle = NULL;
	SILONG xpos[C3_Spell_5_MAX_ENERGIEBALLS],ypos[C3_Spell_5_MAX_ENERGIEBALLS],zpos[C3_Spell_5_MAX_ENERGIEBALLS];
	struct COMOB *beam_COMOB;
	struct COMOB *ball_COMOB[C3_Spell_5_MAX_ENERGIEBALLS];
	struct COMOB *flame_COMOB[C3_Spell_5_MAX_FLAMES];
	struct COMOB *glow_COMOB;
	struct COMOB *COMOB;
	UNSHORT Temp_strength;
	SILONG maintime,s,t,u,i;
	SILONG C3_Spell_5_ENERGIEBALLS;
	SILONG C3_Spell_5_FLAMES;
	SILONG fieldmovez;

	/* Anzahl Flamen sowie BÑlle an Spruch anpassen */
	C3_Spell_5_ENERGIEBALLS=calc_strength(Strength,C3_Spell_5_MIN_ENERGIEBALLS,C3_Spell_5_MAX_ENERGIEBALLS);
	C3_Spell_5_FLAMES=calc_strength(Strength,C3_Spell_5_MIN_FLAMES,C3_Spell_5_MAX_FLAMES);

	/* Load Beam graphics */
	beam_handle = Load_subfile(COMBAT_GFX, 43);
	if (!beam_handle){
		ERROR_ClearStack();
		return;
	}

	/* Load ball graphics */
	ball_handle = Load_subfile(COMBAT_GFX, 21);
	if (!ball_handle){
		ERROR_ClearStack();
		return;
	}

	/* Load Flame graphics */
	flame_handle = Load_subfile(COMBAT_GFX, 54);
	if (!flame_handle){
		ERROR_ClearStack();
		return;
	}

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_5);

	/* DUMMY COMOB erzeugen */
	if((glow_COMOB=gen_Dummy_COMOB())==NULL){
		ERROR_ClearStack();
		return;
	}

	/* permanentes GlÅhen erzeugen */
	behave_glow = Add_COMOB_behaviour(glow_COMOB, NULL, glow_handler);

	if(!behave_glow){
		/* Comob lîschen */
		Delete_COMOB(glow_COMOB);
		ERROR_ClearStack();
		return;
	}

	behave_glow->Data.Just_data.Data[0] = 30; /* startglow in % */
	behave_glow->Data.Just_data.Data[1] = 160; /* red,green,blue */
	behave_glow->Data.Just_data.Data[2] = 0;
	behave_glow->Data.Just_data.Data[3] = 0;
	behave_glow->Data.Just_data.Data[4] = 20; /* min glow */
	behave_glow->Data.Just_data.Data[5] = 40; /* max glow */
	behave_glow->Data.Just_data.Data[6] = 1; /* glow add */

	/* Frist Victim is PartyMember  all Victims are PartyMembers */
	Victim_part = Current_target_parts[0];
	if (Victim_part->Type != MONSTER_PART_TYPE){
		/* Yes -> Start of effect */
		fieldmovez=-(COMBAT_3D_DEPTH/2);
	}
	else{
		fieldmovez=0;
	}

	/* 100 Einheiten */
	maintime=500;
	do{
		/* Draw combat screen */
		Update_screen();

		/* abbrechen bei TastenDruck */
//		if(SYSTEM_GetBLEVStatusLong()&BLEV_KEYPRESSED){
//			maintime=0;
//		}

		for(s=0;s<Nr_combat_updates;s++){

			/* Bei erstem Aufruf EnergieBÑlle generieren */
			if(maintime==500){
				/* beams generieren */
				for(t=0;t<C3_Spell_5_ENERGIEBALLS;t++){
					/*******************/
					/* beams erstellen */
					/*******************/

					/* Add beam_COMOB */
					beam_COMOB = Add_COMOB(100);

					/* Success ? */
					if (!beam_COMOB){
						ERROR_ClearStack();
						return;
					}

					/* set coordinates */
					xpos[t] = rndmm(-(COMBAT_3D_WIDTH/2),(COMBAT_3D_WIDTH/2));
					ypos[t] = COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR;
					zpos[t] = rndmm(-(COMBAT_3D_DEPTH/2),(COMBAT_3D_DEPTH/2))+fieldmovez;

					/* set coordinates */
					beam_COMOB->X_3D = xpos[t];
					beam_COMOB->Y_3D = ypos[t];
					beam_COMOB->Z_3D = zpos[t];

					/* Set lifespan */
					beam_COMOB->Lifespan = 0 ;

					/* Set display parameters */
					beam_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
					beam_COMOB->Special_handle = Transluminance_table_handle;

					beam_COMOB->Hotspot_X_offset = 50;
					beam_COMOB->Hotspot_Y_offset = 50;

					beam_COMOB->Display_width = 75;
					beam_COMOB->Display_height = 75;

					beam_COMOB->Graphics_handle = beam_handle;

					beam_COMOB->Frame = 0;

					behave_anim = Add_COMOB_behaviour(beam_COMOB, NULL, Animcontrol_handler);

					if(!behave_anim){
						/* Comob lîschen */
						Delete_COMOB(COMOB);
						ERROR_ClearStack();
						return;
					}

					u=4;
					if(u>Get_nr_frames(beam_COMOB->Graphics_handle)){
						u=Get_nr_frames(beam_COMOB->Graphics_handle);
					}
					behave_anim->Data.Animcontrol_data.Nr_frames=u;
					behave_anim->Data.Animcontrol_data.Nr_repeats=1;
					behave_anim->Data.Animcontrol_data.Duration=20;

				}
			}
			/* Flamen generieren */
			else if(maintime==490){
				for(t=0;t<C3_Spell_5_FLAMES;t++){
					/*********************/
					/* Flammen erstellen */
					/*********************/

					/* Add COMOB */
					flame_COMOB[t] = Add_COMOB(100);

					/* Success ? */
					if (!flame_COMOB[t]){
						ERROR_ClearStack();
						return;
					}

					/* set coordinates */
					flame_COMOB[t]->X_3D = rndmm(-(COMBAT_3D_WIDTH/2),(COMBAT_3D_WIDTH/2));
					flame_COMOB[t]->Y_3D = 0;
					flame_COMOB[t]->Z_3D = rndmm(-(COMBAT_3D_DEPTH/2),(COMBAT_3D_DEPTH/2))+fieldmovez;

					/* Set lifespan */
					flame_COMOB[t]->Lifespan = 0 ;

					/* Set display parameters */
					flame_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
					flame_COMOB[t]->Special_handle = Transluminance_table_handle;

					flame_COMOB[t]->Hotspot_X_offset = 50;
					flame_COMOB[t]->Hotspot_Y_offset = 85;

					/* Set random size */
					flame_COMOB[t]->Display_width = 150;
					flame_COMOB[t]->Display_height = 50;

					/* Select random spark type */
					flame_COMOB[t]->Graphics_handle = flame_handle;

					/* Set number of animation frames */
					flame_COMOB[t]->Nr_frames = Get_nr_frames(flame_COMOB[t]->Graphics_handle);

					/* Select animation frame */
					flame_COMOB[t]->Frame = 0 ;

					/* Add CrazyMove behaviour */
					behave_move = Add_COMOB_behaviour(flame_COMOB[t], NULL, CrazyMove_handler);

					if(!behave_move){
						/* Comob lîschen */
						Delete_COMOB(COMOB);
						ERROR_ClearStack();
						return;
					}

					/* Set Crazy Move data */
					behave_move->Data.Just_data.Data[0] = 250; /* speed in x */
					behave_move->Data.Just_data.Data[1] = 0; /* speed in y */
					behave_move->Data.Just_data.Data[2] = 250; /* speed in z */

				}
			}
			/* nÑchste Phase = EnergieBÑlle entstehen */
			else if(maintime==480){

				/* balls generieren */
				for(t=0;t<C3_Spell_5_ENERGIEBALLS;t++){

					/*******************/
					/* balls erstellen */
					/*******************/

					/* Add ball_COMOB[t] */
					ball_COMOB[t] = Add_COMOB(100);

					/* Success ? */
					if (!ball_COMOB[t]){
						ERROR_ClearStack();
						return;
					}

					/* Move */
					ball_COMOB[t]->dX_3D = rndmm(-150,150);
					ball_COMOB[t]->dY_3D = rndmm(-250,-200);
					ball_COMOB[t]->dZ_3D = rndmm(-150,150);

					/* Koordinaten aus entsprechendem Beam */
					ball_COMOB[t]->X_3D = xpos[t];
					ball_COMOB[t]->Y_3D = ypos[t];
					ball_COMOB[t]->Z_3D = zpos[t];

					/* Set lifespan */
					ball_COMOB[t]->Lifespan = 0 ;

					/* Set display parameters */
					ball_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
					ball_COMOB[t]->Special_handle = Transluminance_table_handle;

					ball_COMOB[t]->Hotspot_X_offset = 50;
					ball_COMOB[t]->Hotspot_Y_offset = 100;

					/* Set size */
					ball_COMOB[t]->Display_width = 75;
					ball_COMOB[t]->Display_height = 75;

					ball_COMOB[t]->Graphics_handle = ball_handle;

					ball_COMOB[t]->Nr_frames = Get_nr_frames(ball_COMOB[t]->Graphics_handle);;
					ball_COMOB[t]->Frame = 0;

					/* Add bounce behaviour */
					behave_bounce = Add_COMOB_behaviour(ball_COMOB[t], NULL, FireBounce_handler);

					if(!behave_bounce){
						/* Comob lîschen */
						Delete_COMOB(ball_COMOB[t]);
						ERROR_ClearStack();
						return;
					}

					/* Set bounce behaviour data */
					behave_bounce->Data.Bounce_data.Gravity = rndmm(15,20);
					behave_bounce->Data.Bounce_data.Bounce = rndmm(50,60);
					behave_bounce->Data.Bounce_data.Air_friction = rndmm(3,4);

				}

			}
			/* nÑchste Phase = beenden */
			else if(maintime==400){
				maintime=0;
			}
			maintime--;

		}

	}while(maintime>0);


	for (i=0;i<Current_nr_target_parts;i++)
	{

		/* abbrechen bei TastenDruck */
//		if(SYSTEM_GetBLEVStatusLong()&BLEV_KEYPRESSED){
//			break;
//		}

		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
			/* Yes -> Start of effect */
//		}

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if(Temp_strength)
		{
			/* No -> Victim is monster ? */
			if (Victim_part->Type == MONSTER_PART_TYPE)
			{
				/* Yes -> Rest of effect */
			}

			/* Aufpralleffekt */
// 		Gen_Sparks(x,y,z,20);

	 		/* Do damage to victim */
			Do_combat_damage(Victim_part, max(((Temp_strength * 50) / 100), 1));

		}
	}

	/* kurze Zeit warten bis Effekt beendet */
	update_n_times(12);

	/* GlÅhen beenden COMOB lîschen */
	Delete_COMOB(glow_COMOB);

	/* restliche Ball COMOBS lîschen */
	for(t=0;t<C3_Spell_5_ENERGIEBALLS;t++){
		Delete_COMOB(ball_COMOB[t]);
	}

	/* restliche Flame COMOBS lîschen */
	for(t=0;t<C3_Spell_5_FLAMES;t++){
		Delete_COMOB(flame_COMOB[t]);
	}

	/* Energie Handle */
	MEM_Free_memory(flame_handle);

	/* Energie Handle */
	MEM_Free_memory(ball_handle);

	/* Blitz Handle */
	MEM_Free_memory(beam_handle);

}

void
Do_C3_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Store pointer */
	Current_target_parts[Current_nr_target_parts] = Victim_part;

	/* Count up */
	Current_nr_target_parts++;
}









/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_6_handler
 *						 Long-range combat spell on all monsters (energie)
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:35
 * LAST      : 06.06.95 13:35
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_6_MIN_BLITZANZ 4
#define C3_Spell_6_MAX_BLITZANZ 6

#define C3_Spell_6_MIN_BLITZSIZE 60
#define C3_Spell_6_MAX_BLITZSIZE 120

#define C3_Spell_6_ANZHALOCOMOBS 3

void
C3_Spell_6_handler(UNSHORT Strength)
{
	MEM_HANDLE Recolour_table_handle[C3_Spell_6_ANZHALOCOMOBS];
	UNBYTE *Recolour_table_ptr[C3_Spell_6_ANZHALOCOMOBS];
	struct Combat_participant *Victim_part;
	UNSHORT Temp_strength;
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_size;
	struct COMOB_behaviour *behave_flash;
	struct COMOB *dummy_COMOB;
	struct COMOB *COMOB;
	struct COMOB *energie_COMOB;
	struct COMOB *halo_COMOB[C3_Spell_6_ANZHALOCOMOBS];
	MEM_HANDLE blitz_handle = NULL;
	MEM_HANDLE explode_handle = NULL;
	MEM_HANDLE energie_handle = NULL;
	SILONG blitzsize,blitzanz,s,t,u,i,aktblitz,energieballsize;
	SILONG x,y,z;
	SILONG left,right,top,bottom,height;

	/* zu jedem Halo COMOB eine TransparentFarbe generieren */
	s=20; /* AnfangsTransparenz */
	for(t=0;t<C3_Spell_6_ANZHALOCOMOBS;t++){
		Recolour_table_handle[t] = MEM_Allocate_memory(256);
		Recolour_table_ptr[t] = MEM_Claim_pointer(Recolour_table_handle[t]);
		Calculate_recolouring_table(Recolour_table_ptr[t], 1, 191, 180,180,255, s);
		MEM_Free_pointer(Recolour_table_handle[t]);
		s+=30;
	}

	/* SpruchstÑrke = Grî·e der Flamen,FlamenbÑlle */
	blitzanz=calc_strength(Strength,C3_Spell_6_MIN_BLITZANZ,C3_Spell_6_MAX_BLITZANZ);
	blitzsize=calc_strength(Strength,C3_Spell_6_MIN_BLITZSIZE,C3_Spell_6_MAX_BLITZSIZE);
	energieballsize=120*blitzsize/100;

	/* DUMMY COMOB erzeugen */
	if((dummy_COMOB=gen_Dummy_COMOB())==NULL){
		ERROR_ClearStack();
		return;
	}

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_6);

	/* Blitze erzeugen */
	for(aktblitz=0;aktblitz<blitzanz;aktblitz++){

		/* Load Blitz graphics */
		blitz_handle = Load_subfile(COMBAT_GFX, 55);
		if (!blitz_handle){
			ERROR_ClearStack();
			return;
		}

		/* Add ball_COMOB[t] */
		COMOB = Add_COMOB(100);

		/* Success ? */
		if (!COMOB){
			ERROR_ClearStack();
			return;
		}

		/* Koordinaten aus entsprechendem Beam */
		COMOB->X_3D = 0;
		COMOB->Y_3D = 0;
		COMOB->Z_3D = 0;

		/* Set lifespan */
		COMOB->Lifespan = rndmm(200,250);

		/* Grafik nach beenden freigeben */
		COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

		/* Set display parameters */
		COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		COMOB->Special_handle = Transluminance_table_handle;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 100;

		/* Set size */
		COMOB->Display_width = blitzsize;
		COMOB->Display_height = 175;

		COMOB->Graphics_handle = blitz_handle;

		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);;
		COMOB->Frame = 0;

		/* Add oscillate behaviour for x */
		behave = Add_COMOB_behaviour(COMOB, NULL, Oscillate_II_handler);

		if(!behave){
			/* Comob lîschen */
			Delete_COMOB(COMOB);
			ERROR_ClearStack();
			return;
		}

		/* Set random bounce behaviour data */
		behave->Data.Oscillate_data.Type = OSCILLATE_X;
		behave->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
		behave->Data.Oscillate_data.Amplitude = rndmm(25,75)*COMOB_DEC_FACTOR;
		behave->Data.Oscillate_data.Value = aktblitz*90/blitzanz;
		behave->Data.Just_data_w.Data[7] = rndmm(25,100);
		behave->Data.Just_data_w.Data[8] = COMBAT_3D_WIDTH/2;

		/* Add oscillate behaviour for y */
		behave = Add_COMOB_behaviour(COMOB, NULL, Oscillate_II_handler);

		if(!behave){
			ERROR_ClearStack();
			return;
		}

		/* Set random bounce behaviour data */
		behave->Data.Oscillate_data.Type = OSCILLATE_Z;
		behave->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
		behave->Data.Oscillate_data.Amplitude = rndmm(25,75)*COMOB_DEC_FACTOR;
		behave->Data.Oscillate_data.Value = (aktblitz*90/blitzanz)+23;
		behave->Data.Just_data_w.Data[7] = rndmm(25,100);
		behave->Data.Just_data_w.Data[8] = COMBAT_3D_WIDTH/2;

		if(Combat_display_detail_level>DEFAULT_COMBAT_DISPLAY_DETAIL_LEVEL){

			/* Funken zum Blitz addieren */
			behave = Add_COMOB_behaviour(COMOB, NULL, funken_handler);

			if(!behave){
				ERROR_ClearStack();
				return;
			}

			/* Set behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+4;
			behave->Data.Just_data_w.Data[1] = 0;
			behave->Data.Just_data_w.Data[2] = 5*COMOB_DEC_FACTOR;
			behave->Data.Just_data_w.Data[3] = 0;
			behave->Data.Just_data_w.Data[4] = 1;

		}
		if(Combat_display_detail_level>=DEFAULT_COMBAT_DISPLAY_DETAIL_LEVEL){

			/* BodenglÅghen zum Blitz addieren */
			behave = Add_COMOB_behaviour(COMOB, NULL, groundglow_handler);

			if(!behave){
				ERROR_ClearStack();
				return;
			}

			/* Set behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+4;
			behave->Data.Just_data_w.Data[1] = 0;
			behave->Data.Just_data_w.Data[2] = 5*COMOB_DEC_FACTOR;
			behave->Data.Just_data_w.Data[3] = 0;
			behave->Data.Just_data_w.Data[4] = 100;
			behave->Data.Just_data_w.Data[5] = 20;

		}

		/* DisplayDetailLevel >MIN dann Energieball darstellen */
		if(Combat_display_detail_level>MIN_COMBAT_DISPLAY_DETAIL_LEVEL){

			/*******************/
			/* EnergieBall erstellen */
			/*******************/

			/* Load EnergieBall graphics */
			energie_handle = Load_subfile(COMBAT_GFX, 36);
			if (!energie_handle){
				ERROR_ClearStack();
				return;
			}

			/* Add energie_COMOB */
			energie_COMOB = Add_COMOB(100);

			/* Success ? */
			if (!energie_COMOB){
				ERROR_ClearStack();
				return;
			}

			/* Set lifespan */
			energie_COMOB->Lifespan = 0 ;

			/* Set display parameters */
			energie_COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

			energie_COMOB->Hotspot_X_offset = 50;
			energie_COMOB->Hotspot_Y_offset = 50;

			/* Grafik nach beenden freigeben */
			energie_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

			/* Set random size */
			energie_COMOB->Display_width = energieballsize;
			energie_COMOB->Display_height = energieballsize;

			/* Select random spark type */
			energie_COMOB->Graphics_handle = energie_handle;

			/* Set number of animation frames */
			energie_COMOB->Nr_frames = Get_nr_frames(energie_COMOB->Graphics_handle);

			/* Select animation frame */
			energie_COMOB->Frame = 0;

			/* mit dem BlitzverknÅpfen*/
			if(!(behave = Add_COMOB_behaviour(energie_COMOB, COMOB, connect_handler))){
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+2+4;
			behave->Data.Just_data_w.Data[1] = 0;
			behave->Data.Just_data_w.Data[2] = COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR;
			behave->Data.Just_data_w.Data[3] = 0;

			/* Halo Comobs */
			if(Combat_display_detail_level>=MAX_COMBAT_DISPLAY_DETAIL_LEVEL){
				s=120;
				u=C3_Spell_6_ANZHALOCOMOBS;
				for(t=0;t<C3_Spell_6_ANZHALOCOMOBS;t++){
					halo_COMOB[t]=Duplicate_COMOB(energie_COMOB);
					halo_COMOB[t]->Display_width+=s;
					halo_COMOB[t]->Display_height+=s;
					halo_COMOB[t]->Draw_mode = COLOURING_COMOB_DRAWMODE;
					halo_COMOB[t]->Special_handle = Recolour_table_handle[t];

					/* mit dem EnergieBall verknÅpfen*/
					if(!(behave = Add_COMOB_behaviour(halo_COMOB[t],energie_COMOB, connect_handler))){
						ERROR_ClearStack();
						return;
					}

					/* Set random bounce behaviour data */
					behave->Data.Just_data_w.Data[0] = 1+2+4;
					behave->Data.Just_data_w.Data[1] = 0;
					behave->Data.Just_data_w.Data[2] = 0;
					behave->Data.Just_data_w.Data[3] = u;

					s-=40;
					u+=2;
				}
			}
		}
	}

	for (i=0;i<Current_nr_target_parts;i++)
	{

		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* Victim is monster ? */
// 	if (Victim_part->Type == MONSTER_PART_TYPE)
// 	{
	 	/* Yes -> Start of effect */
// 	}

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if(Temp_strength)
		{
			/* No -> Victim is monster ? */
			if (Victim_part->Type == MONSTER_PART_TYPE)
			{
				/* Yes -> Rest of effect */
			}

			/*******************/
			/* Explosion erstellen */
			/*******************/

			/* Load Explode graphics */
			explode_handle = Load_subfile(COMBAT_GFX, 33);
			if (!explode_handle){
				ERROR_ClearStack();
				return;
			}

			/* Add COMOB */
			COMOB = Add_COMOB(100);

			/* Success ? */
			if (!COMOB){
				ERROR_ClearStack();

				return;
			}

			/* Koordinaten des Monsters */
			Get_3D_part_coordinates(Victim_part,&x,&y,&z);

			/* set coordinates */
			COMOB->X_3D = x;
			COMOB->Y_3D = y;
			COMOB->Z_3D = z;

			/* Set lifespan */
			COMOB->Lifespan = 40;

			/* Grafik nach beenden freigeben */
			COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

			/* Set display parameters */
			COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			COMOB->Special_handle = Transluminance_table_handle;

			COMOB->Hotspot_X_offset = 50;
			COMOB->Hotspot_Y_offset = 50;

			/* Grî·e des Monsters in 3D */
			Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
			height=top-bottom;

			/* Set size */
			COMOB->Display_width = height*150/(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR);
			COMOB->Display_height = height*150/(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR);

			COMOB->Graphics_handle = explode_handle;
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

			behave_size->Data.Just_data_w.Data[0]=-1600; /* jeden Tick um 1% */
			behave_size->Data.Just_data_w.Data[1]=-200;
			behave_size->Data.Just_data_w.Data[2]=2; /* bis 25 % */
			behave_size->Data.Just_data_w.Data[3]=2;

			/* Behaviour fÅr Grî·e */
			behave_flash = Add_COMOB_behaviour(dummy_COMOB, NULL, flash_handler);

			/* Grî·e Ñndern */
			if(!behave_flash){
				/* Comob lîschen */
				Delete_COMOB(COMOB);
				ERROR_ClearStack();
				return;
			}

			behave_flash->Data.Just_data.Data[0]=8; /* X Ticks lang Palette zur entsprechenden Farbe flashen und 5 Ticks wieder zur Ursprungspalette*/
			behave_flash->Data.Just_data.Data[1]=160; /* nach hellblau umblenden */
			behave_flash->Data.Just_data.Data[2]=160;
			behave_flash->Data.Just_data.Data[3]=255;
			behave_flash->Data.Just_data.Data[4]=50; /* maximal 50 % */

	 		/* Do damage to victim */
			Do_combat_damage(Victim_part, max(((Temp_strength * 50) / 100), 1));

		}
	}

	/* kurze Zeit warten bis Effekt beendet */
	update_n_times(32);

	/* Speicher fÅr TransparenzTabellen freigeben */
	for(t=0;t<C3_Spell_6_ANZHALOCOMOBS;t++){
		MEM_Free_memory(Recolour_table_handle[t]);
	}

}

void
Do_C3_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Store pointer */
	Current_target_parts[Current_nr_target_parts] = Victim_part;

	/* Count up */
	Current_nr_target_parts++;
}


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_7_handler
 *						 Energie Falle setzen
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:14
 * LAST      : 02.06.95 16:14
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_7_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_7);
}

void
Do_C3_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_x;
	struct COMOB_behaviour *behave_z;
	struct COMOB *COMOB;
	MEM_HANDLE trap_handle = NULL;
	SILONG x,y,z,t,tx,tz;
	SILONG balls;

	/* Koordiaten des Feldes */
	Convert_tactical_to_3D_coordinates(Tactical_X,Tactical_Y,&tx,&tz);

	/* Is there already a trap on this square ? */
	if (!Is_trap(Tactical_X, Tactical_Y))
	{

		/* StÑrke = Anzahl BÑlle */
		balls=calc_strength(Strength,2,6);

		/* No -> Start effect */

		/* Load Blitz graphics */
		trap_handle = Load_subfile(COMBAT_GFX, 29);
		if (!trap_handle){
			ERROR_ClearStack();
			return;
		}

		/* Feenstaub */
		for(t=0;t<balls;t++){

			/* Add COMOB */
			COMOB = Add_COMOB(100);

			/* Success ? */
			if (!COMOB){
				ERROR_ClearStack();
				return;
			}

			/* set coordinates */
			COMOB->X_3D = tx;
			COMOB->Y_3D = 25*COMOB_DEC_FACTOR;
			COMOB->Z_3D = tz;

			/* beim lîschen des COMOBS Falle lîschen */
			COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

			/* Set lifespan */
			COMOB->Lifespan = 50 ;

			/* Set display parameters */
			COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

			COMOB->Hotspot_X_offset = 50;
			COMOB->Hotspot_Y_offset = 50;

			/* Set random size */
			COMOB->Display_width = 100;
			COMOB->Display_height = 100;

			/* Select random spark type */
			COMOB->Graphics_handle = Star_gfx_handles[0];

			/* Set number of animation frames */
			COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

			/* Select animation frame */
			COMOB->Frame = 0;

			/* Add oscillate behaviour for x */
			behave_x = Add_COMOB_behaviour(COMOB, NULL, Oscillate_handler);

			if(!behave_x){
				/* Comob lîschen */
				Delete_COMOB(COMOB);
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave_x->Data.Oscillate_data.Type = OSCILLATE_X;
			behave_x->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_x->Data.Oscillate_data.Amplitude = 30*COMOB_DEC_FACTOR;
			behave_x->Data.Oscillate_data.Value = t*90/balls;


			/* Add oscillate behaviour for y */
			behave_z = Add_COMOB_behaviour(COMOB, NULL, Oscillate_handler);

			if(!behave_z){
				/* Comob lîschen */
				Delete_COMOB(COMOB);
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave_z->Data.Oscillate_data.Type = OSCILLATE_Z;
			behave_z->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_z->Data.Oscillate_data.Amplitude = 30*COMOB_DEC_FACTOR;
			behave_z->Data.Oscillate_data.Value = (t*90/balls)+23;

#if FALSE
			/* Add fairy behaviour */
			behave = Add_COMOB_behaviour(COMOB, NULL, FairyII_handler);

			if(!behave){
				/* Comob lîschen */
				Delete_COMOB(COMOB);
				ERROR_ClearStack();
				return;
			}
#endif

			/* Funken zum Blitz addieren */
			behave = Add_COMOB_behaviour(COMOB, NULL, funken_handler);

			if(!behave){
				ERROR_ClearStack();
				return;
			}

			/* Set behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+4;
			behave->Data.Just_data_w.Data[1] = 0;
			behave->Data.Just_data_w.Data[2] = 5*COMOB_DEC_FACTOR;
			behave->Data.Just_data_w.Data[3] = 0;
			behave->Data.Just_data_w.Data[4] = 1;

		}

		/* Update times */
		update_n_times(24);

		/* Falle setzten */
		/* Add COMOB */
		COMOB = Add_COMOB(100);

		/* Success ? */
		if (!COMOB){
			ERROR_ClearStack();
			return;
		}

		/* set coordinates */
		COMOB->X_3D = tx;
		COMOB->Y_3D = 0;
		COMOB->Z_3D = tz;

		/* Set lifespan */
		COMOB->Lifespan = 0 ;

		/* Set display parameters */
		COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		/* Set random size */
		COMOB->Display_width = 150;
		COMOB->Display_height = 150;

		/* Select random spark type */
		COMOB->Graphics_handle = trap_handle;

		/* Set number of animation frames */
		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

		/* Select animation frame */
		COMOB->Frame = 0;

		/* Update times */
		update_n_times(50);

		/* Set trap */
		Install_trap(Tactical_X, Tactical_Y, Handle_C3_Spell_7_trap,
		 Strength, COMOB);
	}

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_C3_Spell_7_trap
 * FUNCTION  : Trap handler.
 *						 Energie Falle explodiert
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:14
 * LAST      : 02.06.95 16:14
 * INPUTS    : struct Combat_participant *Victim_part - Pointer to victim
 *              participant data.
 *             UNSHORT Strength - Strength of spell (1...100).
 *             struct COMOB *Trap_COMOB - Pointer to trap COMOB.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_7_MIN_BALLS 16
#define C3_Spell_7_MAX_BALLS 96

void
Handle_C3_Spell_7_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength, struct COMOB *Trap_COMOB)
{
	MEM_HANDLE explodeball_handle = NULL;
	MEM_HANDLE explodeblitz_handle = NULL;
	struct COMOB *explodeball_COMOB;
	struct COMOB *explodeblitz_COMOB;
	struct COMOB *COMOB;
	struct COMOB_behaviour *behave;
	SILONG x,y,z;
	SILONG s,t,u;
	SILONG balls;
	SILONG left,right,top,bottom;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	/* SpruchstÑrke = Grî·e der Flamen,FlamenbÑlle */
	balls=calc_strength(Strength,C3_Spell_7_MIN_BALLS,C3_Spell_7_MAX_BALLS);

	/* Load ExplodeBall graphics */
	explodeball_handle = Load_subfile(COMBAT_GFX, 36);
	if (!explodeball_handle){
		ERROR_ClearStack();
		return;
	}

	/* Load ExplodeBlitz graphics */
	explodeblitz_handle = Load_subfile(COMBAT_GFX, 33);
	if (!explodeblitz_handle){
		ERROR_ClearStack();
		return;
	}

	/* StartPunkt der Explosion */
	/* Koordinaten des Monsters */
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

	y=bottom;
	if(y<(5*COMOB_DEC_FACTOR))
		y=5*COMOB_DEC_FACTOR;

	/* ExplodeBall erstellen */
	explodeball_COMOB = Add_COMOB(100);

	/* Success ? */
	if (!explodeball_COMOB){
		ERROR_ClearStack();
		return;
	}

	/* set coordinates */
	explodeball_COMOB->X_3D = x;
	explodeball_COMOB->Y_3D = y;
	explodeball_COMOB->Z_3D = z;

	/* Set lifespan */
	explodeball_COMOB->Lifespan = 0;

	/* Set display parameters */
	explodeball_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	explodeball_COMOB->Special_handle = Transluminance_table_handle;

	explodeball_COMOB->Hotspot_X_offset = 50;
	explodeball_COMOB->Hotspot_Y_offset = 50;

	/* Set size */
	explodeball_COMOB->Display_width = 50;
	explodeball_COMOB->Display_height = 50;

	explodeball_COMOB->Graphics_handle = explodeball_handle;

	explodeball_COMOB->Nr_frames = Get_nr_frames(explodeball_COMOB->Graphics_handle);;
	explodeball_COMOB->Frame = 0;

	/* Explosionsblitz erstellen */
	explodeblitz_COMOB = Add_COMOB(100);

	/* Success ? */
	if (!explodeblitz_COMOB){
		ERROR_ClearStack();
		return;
	}

	/* set coordinates */
	explodeblitz_COMOB->X_3D = x;
	explodeblitz_COMOB->Y_3D = y;
	explodeblitz_COMOB->Z_3D = z;

	/* Set lifespan */
	explodeblitz_COMOB->Lifespan = 0;

	/* Set display parameters */
	explodeblitz_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	explodeblitz_COMOB->Special_handle = Transluminance_table_handle;

	explodeblitz_COMOB->Hotspot_X_offset = 50;
	explodeblitz_COMOB->Hotspot_Y_offset = 50;

	/* Set size */
	explodeblitz_COMOB->Display_width = 10;
	explodeblitz_COMOB->Display_height = 10;

	explodeblitz_COMOB->Graphics_handle = explodeblitz_handle;

	explodeblitz_COMOB->Nr_frames = Get_nr_frames(explodeblitz_COMOB->Graphics_handle);;
	explodeblitz_COMOB->Frame = 0;

	/* ExplosionsKugel zoomen */
	t=50;
	do{
		Update_screen();
		explodeball_COMOB->Display_width = t;
		explodeball_COMOB->Display_height = t;
		t+=Nr_combat_updates*5;
	}while(t<200);
	explodeball_COMOB->Display_width = 200;
	explodeball_COMOB->Display_height = 200;

	/* Daraufhin sagte der Herr "es entstehe ein Blitz" und so geschah es dann auch */
	t=50;
	do{
		Update_screen();
		explodeblitz_COMOB->Display_width = t;
		explodeblitz_COMOB->Display_height = t;
		t+=Nr_combat_updates*15;
	}while(t<100);
	explodeblitz_COMOB->Display_width = 100;
	explodeblitz_COMOB->Display_height = 100;

	/* TÅrkissene Funken erzeugen */
	for(t=0;t<balls;t++){

		/* ExplodeBall erstellen */
		COMOB = Add_COMOB(100);

		/* Success ? */
		if (!COMOB){
			ERROR_ClearStack();
			return;
		}

		/* set coordinates */
		COMOB->dX_3D = rndmm2(-200,-10,10,200);
		COMOB->dY_3D = rndmm(300,600);
		COMOB->dZ_3D = rndmm2(-200,-10,10,200);

		/* set coordinates */
		COMOB->X_3D = x;
		COMOB->Y_3D = y;
		COMOB->Z_3D = z;

		/* Set lifespan */
		COMOB->Lifespan = rndmm(100,200);

		/* Set display parameters */
		COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		COMOB->Special_handle = Transluminance_table_handle;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		/* Set size */
		COMOB->Display_width = 100;
		COMOB->Display_height = 100;

		COMOB->Graphics_handle = Spark_gfx_handles[rndmm(0,2)];

		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);
		COMOB->Frame = rndmm(0,Get_nr_frames(COMOB->Graphics_handle)-1);

		/* Add bounce behaviour */
		behave = Add_COMOB_behaviour(COMOB, NULL, Bounce_handler);

		if(!behave){
			/* Comob lîschen */
			Delete_COMOB(COMOB);
			ERROR_ClearStack();
			return;
		}

		/* Set bounce behaviour data */
		behave->Data.Bounce_data.Gravity = rndmm(15,20);
		behave->Data.Bounce_data.Bounce = rndmm(50,60);
		behave->Data.Bounce_data.Air_friction = rndmm(3,4);

	}

	/* Falle lîschen */
	Delete_COMOB(Trap_COMOB);

	/* gleichzeitig Monster hochschleudern */

	u=Victim_part->Main_COMOB->Y_3D;
	s=30*COMOB_DEC_FACTOR;
	t=200;
	do{
		Update_screen();
		explodeball_COMOB->Display_width = t;
		explodeball_COMOB->Display_height = t;
		Victim_part->Main_COMOB->Y_3D+=s;
		s-=Nr_combat_updates*300;
		t-=Nr_combat_updates*20;
	}while(t>5);
	explodeball_COMOB->Display_width = 5;
	explodeball_COMOB->Display_height = 5;

	/* Ball lîschen */
	Delete_COMOB(explodeball_COMOB);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
			/* Yes -> Rest of effect */
//		}

		t=100;
		do{
			Update_screen();
			explodeblitz_COMOB->Display_width = t;
			explodeblitz_COMOB->Display_height = t;
			Victim_part->Main_COMOB->Y_3D+=s;
			t-=Nr_combat_updates*10;
			s-=Nr_combat_updates*25;
		}while(t>5);
		explodeblitz_COMOB->Display_width = 5;
		explodeblitz_COMOB->Display_height = 5;

		do{
			Update_screen();
			Victim_part->Main_COMOB->Y_3D+=s;
			s-=Nr_combat_updates*25;
		}while(Victim_part->Main_COMOB->Y_3D>u);
		Victim_part->Main_COMOB->Y_3D=u;

		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));

		/* Blitz lîschen */
		Delete_COMOB(explodeblitz_COMOB);

	}

	/* Effect aufbauen */
	update_n_times(24);

	/* Falle lîschen */
//	Delete_COMOB(Trap_COMOB);

	/* Remove trap */
//	Remove_trap(Tactical_X, Tactical_Y);

	/* Handle freigeben */
	MEM_Free_memory(explodeblitz_handle);

	/* Handle freigeben */
	MEM_Free_memory(explodeball_handle);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_8_handler
 *						 Energie Falle in einer Reihe
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:15
 * LAST      : 02.06.95 16:15
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_8_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_7);
}


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_9_handler
 * FUNCTION  : Spell handler.
 *						 Magische Falle setzten
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:16
 * LAST      : 02.06.95 16:16
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_9_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_9);
}

void
Do_C3_Spell_9(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_x;
	struct COMOB_behaviour *behave_z;
	struct COMOB *COMOB;
	MEM_HANDLE trap_handle = NULL;
	SILONG x,y,z,t,tx,tz;
	SILONG balls;

	/* Koordiaten des Feldes */
	Convert_tactical_to_3D_coordinates(Tactical_X,Tactical_Y,&tx,&tz);

	/* Is there already a trap on this square ? */
	if (!Is_trap(Tactical_X, Tactical_Y))
	{

		/* StÑrke = Anzahl BÑlle */
		balls=calc_strength(Strength,2,6);

		/* No -> Start effect */

		/* Load Blitz graphics */
		trap_handle = Load_subfile(COMBAT_GFX, 29);
		if (!trap_handle){
			ERROR_ClearStack();
			return;
		}

		/* Feenstaub */
		for(t=0;t<balls;t++){

			/* Add COMOB */
			COMOB = Add_COMOB(100);

			/* Success ? */
			if (!COMOB){
				ERROR_ClearStack();
				return;
			}

			/* set coordinates */
			COMOB->X_3D = tx;
			COMOB->Y_3D = 25*COMOB_DEC_FACTOR;
			COMOB->Z_3D = tz;

			/* Set lifespan */
			COMOB->Lifespan = 50 ;

			/* Set display parameters */
			COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

			COMOB->Hotspot_X_offset = 50;
			COMOB->Hotspot_Y_offset = 50;

			/* Set random size */
			COMOB->Display_width = 100;
			COMOB->Display_height = 100;

			/* Select random spark type */
			COMOB->Graphics_handle = Star_gfx_handles[0];

			/* Set number of animation frames */
			COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

			/* Select animation frame */
			COMOB->Frame = 0;

			/* Add oscillate behaviour for x */
			behave_x = Add_COMOB_behaviour(COMOB, NULL, Oscillate_handler);

			if(!behave_x){
				/* Comob lîschen */
				Delete_COMOB(COMOB);
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave_x->Data.Oscillate_data.Type = OSCILLATE_X;
			behave_x->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_x->Data.Oscillate_data.Amplitude = 30*COMOB_DEC_FACTOR;
			behave_x->Data.Oscillate_data.Value = t*90/balls;


			/* Add oscillate behaviour for y */
			behave_z = Add_COMOB_behaviour(COMOB, NULL, Oscillate_handler);

			if(!behave_z){
				/* Comob lîschen */
				Delete_COMOB(COMOB);
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave_z->Data.Oscillate_data.Type = OSCILLATE_Z;
			behave_z->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_z->Data.Oscillate_data.Amplitude = 30*COMOB_DEC_FACTOR;
			behave_z->Data.Oscillate_data.Value = (t*90/balls)+23;

#if FALSE
			/* Add fairy behaviour */
			behave = Add_COMOB_behaviour(COMOB, NULL, FairyII_handler);

			if(!behave){
				/* Comob lîschen */
				Delete_COMOB(COMOB);
				ERROR_ClearStack();
				return;
			}
#endif

			/* Funken zum Blitz addieren */
			behave = Add_COMOB_behaviour(COMOB, NULL, funken_handler);

			if(!behave){
				ERROR_ClearStack();
				return;
			}

			/* Set behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+4;
			behave->Data.Just_data_w.Data[1] = 0;
			behave->Data.Just_data_w.Data[2] = 5*COMOB_DEC_FACTOR;
			behave->Data.Just_data_w.Data[3] = 0;
			behave->Data.Just_data_w.Data[4] = 1;

		}

		/* Update times */
		update_n_times(24);

		/* Falle setzten */
		/* Add COMOB */
		COMOB = Add_COMOB(100);

		/* Success ? */
		if (!COMOB){
			ERROR_ClearStack();
			return;
		}

		/* COMOB nach lîschen erstellen */
		COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

		/* set coordinates */
		COMOB->X_3D = tx;
		COMOB->Y_3D = 0;
		COMOB->Z_3D = tz;

		/* Set lifespan */
		COMOB->Lifespan = 0 ;

		/* Set display parameters */
		COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		/* Set random size */
		COMOB->Display_width = 150;
		COMOB->Display_height = 150;

		/* Select random spark type */
		COMOB->Graphics_handle = trap_handle;

		/* Set number of animation frames */
		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

		/* Select animation frame */
		COMOB->Frame = 0;

		/* Update times */
		update_n_times(50);

		/* Set trap */
		Install_trap(Tactical_X, Tactical_Y, Handle_C3_Spell_9_trap,
		 Strength, COMOB);
	}
}


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_C3_Spell_9_trap
 *						 Magische Falle explodiert
 * FUNCTION  : Trap handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:16
 * LAST      : 03.06.95 17:27
 * INPUTS    : struct Combat_participant *Victim_part - Pointer to victim
 *              participant data.
 *             UNSHORT Strength - Strength of spell (1...100).
 *             struct COMOB *Trap_COMOB - Pointer to trap COMOB.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_9_MIN_STARS 10
#define C3_Spell_9_MAX_STARS 20

void
Handle_C3_Spell_9_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength, struct COMOB *Trap_COMOB)
{
	struct COMOB_behaviour *behave_size;
	struct COMOB *COMOB;
	struct COMOB **halo_COMOB;
	SILONG time,s,t,tx,tz,x,y,z,mx,my,mz,gx,gy,gz;
	SILONG Tactical_X,Tactical_Y;
	SILONG left,right,top,bottom;
	SILONG vleft,vright,vtop,vbottom;
	SILONG stars,steps;
	SILONG hw[3];

	Tactical_X=Victim_part->Tactical_X;
	Tactical_Y=Victim_part->Tactical_Y;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE){
//		/* Yes -> Start of effect */
//	}

	Get_COMOB_rectangle(Trap_COMOB,&left,&top,&right,&bottom);
	Get_part_rectangle(Victim_part,&vleft,&vtop,&vright,&vbottom);
	Get_3D_part_coordinates(Victim_part,&gx,&gy,&gz);

	/* StÑrke = Anzahl Sterne */
	stars=calc_strength(Strength,C3_Spell_9_MIN_STARS,C3_Spell_9_MAX_STARS);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
			/* Yes -> Rest of effect */
//		}
		Trap_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		Trap_COMOB->Special_handle = Transluminance_table_handle;

		/* Leuchtshilouhette */
		halo_COMOB=Add_halo_to_COMOB_2(Victim_part->Main_COMOB, 3, 20, 0,HALO_CONNECT);

		/* MonsterHalo wabert*/
		for(t=0;t<3;t++){
			hw[t]=halo_COMOB[t]->Display_width;
		}

		/* laufend Sterne generieren */
		time=0;
		s=0;
		do{
			Update_screen();
			if(s<0){
				for(t=0;t<stars;t++){
					x=rndmm(left,right);
					y=rndmm(0,1500);
					z=gz+rndmm(-200,200);

					/* Stern erstellen */
					steps=calc_delta_xyz(x,y,z, x,vtop+y,z, &mx,&my,&mz, 300);
					if((COMOB=Gen_COMOB(x,y,z, steps, rndmm(25,75), Star_gfx_handles[rndmm(0,2)], GC_MAGIC))==NULL){
						ERROR_ClearStack();
						return;
					}
					COMOB->dX_3D=rndmm(-150,150);
					COMOB->dY_3D=my;

					/* Size behaviour */
					behave_size = Add_COMOB_behaviour(COMOB, NULL, size_handler);

					/* Grî·e Ñndern */
					if(!behave_size){
						/* Comob lîschen */
						Delete_COMOB(COMOB);
						ERROR_ClearStack();
						return;
					}

					behave_size->Data.Just_data_w.Data[0]=rndmm(-200,-100); /* jeden Tick um 1% */
					behave_size->Data.Just_data_w.Data[1]=behave_size->Data.Just_data_w.Data[0];
					behave_size->Data.Just_data_w.Data[2]=2; /* bis 25 % */
					behave_size->Data.Just_data_w.Data[3]=2;

				}
				s=5;

				/* MonsterHalo wabert*/
				for(t=0;t<3;t++){
					halo_COMOB[t]->Display_width=hw[t]+rndmm(-2,2);
					halo_COMOB[t]->Display_height=hw[t]+rndmm(-2,2);
				}

			}
			s-=Nr_combat_updates;
			time+=Nr_combat_updates; /* Beam Geschwindigkeit */
		}while(time<128);

		/* Halo wieder lîschen */
		halo_COMOB[0]->Lifespan=1;
		halo_COMOB[1]->Lifespan=1;
		halo_COMOB[2]->Lifespan=1;

		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));

		/* noch kurze Zeit darstellen */
		update_n_times(32);

	}

	/* Falle lîschen */
	Delete_COMOB(Trap_COMOB);

	/* Remove trap */
	Remove_trap(Tactical_X, Tactical_Y);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_10_handler
 *						 Magische Falle in einer Reihe
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:16
 * LAST      : 02.06.95 16:16
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_9);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_11_handler
 *						 LP-Sucker
 * FUNCTION  : Spell handler.
 *						 LP-Sucker
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:47
 * LAST      : 06.06.95 13:47
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */
#define C3_Spell_11_ANZHALOCOMOBS 3
#define C3_Spell_11_LP_MOVESPEED 250

#define C3_Spell_11_MIN_BALLS 16
#define C3_Spell_11_MAX_BALLS 32

void
C3_Spell_11_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_11);
}

void
Do_C3_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	MEM_HANDLE Recolour_table_handle[C3_Spell_11_ANZHALOCOMOBS];
	MEM_HANDLE lp_handle = NULL;
	UNBYTE *Recolour_table_ptr[C3_Spell_11_ANZHALOCOMOBS];
	struct COMOB *halo_COMOB[C3_Spell_11_ANZHALOCOMOBS];
	struct COMOB *lp_COMOB[C3_Spell_11_MAX_BALLS];
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_x;
	struct COMOB_behaviour *behave_y;
	SILONG delta_x[C3_Spell_11_MAX_BALLS];
	SILONG delta_y[C3_Spell_11_MAX_BALLS];
	SILONG delta_z[C3_Spell_11_MAX_BALLS];
	SILONG delta_steps;
	UNSHORT Damage;
	UNSHORT LP;
	UNSHORT c_width,c_height;
	SILONG s,t,u,v;
	SILONG sx,sy,sz;
	SILONG dx,dy,dz;
	SILONG balls;
	SILONG Left_3D, Top_3D, Right_3D, Bottom_3D;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{

		/* SpruchstÑrke = Grî·e der Flamen,FlamenbÑlle */
		balls=calc_strength(Strength,C3_Spell_11_MIN_BALLS,C3_Spell_11_MAX_BALLS);

		/* Load LP graphics */
		lp_handle = Load_subfile(COMBAT_GFX, 32);
		if (!lp_handle){
			ERROR_ClearStack();
			return;
		}


		/* No -> Calculate damage */
		Damage = max(((Strength * 50) / 100), 1);

		/* Give LP to caster */
		LP = Get_LP(Current_use_magic_data.Casting_participant->Char_handle);

		/* Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{



			/* Yes -> Rest of effect */
//		}
		/* zu jedem Halo COMOB eine TransparentFarbe generieren */
		s=20; /* AnfangsTransparenz */
		for(t=0;t<C3_Spell_11_ANZHALOCOMOBS;t++){
			Recolour_table_handle[t] = MEM_Allocate_memory(256);
			Recolour_table_ptr[t] = MEM_Claim_pointer(Recolour_table_handle[t]);
			Calculate_recolouring_table(Recolour_table_ptr[t], 1, 191, 128,255,128, s);
			MEM_Free_pointer(Recolour_table_handle[t]);
			s+=30;
		}

		/* Halo Comobs */
		s=30;
		for(t=0;t<C3_Spell_11_ANZHALOCOMOBS;t++){

			halo_COMOB[t]=Duplicate_COMOB(Victim_part->Main_COMOB);
			halo_COMOB[t]->Display_width+=s;
			halo_COMOB[t]->Display_height+=s;
			halo_COMOB[t]->Hotspot_X_offset = 50;
			halo_COMOB[t]->Hotspot_Y_offset = 50;
			halo_COMOB[t]->Draw_mode = COLOURING_COMOB_DRAWMODE;
			halo_COMOB[t]->Special_handle = Recolour_table_handle[t];

			/* Get rectangle covering source COMOB */
			Get_part_rectangle(Victim_part, &Left_3D, &Top_3D, &Right_3D, &Bottom_3D);

			/* Set halo COMOB coordinates in the middle of this rectangle */
			halo_COMOB[t]->X_3D = Left_3D + (Right_3D - Left_3D) / 2;
			halo_COMOB[t]->Y_3D = Bottom_3D + (Top_3D - Bottom_3D) / 2;
			halo_COMOB[t]->Z_3D = Victim_part->Main_COMOB->Z_3D+(t*2)+2;

			/* mit dem EnergieBall verknÅpfen*/
			if(!(behave = Add_COMOB_behaviour(halo_COMOB[t],Victim_part->Main_COMOB, connect_handler))){
				ERROR_ClearStack();
				return;
			}

			/* Set Connect behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+2+4;
			behave->Data.Just_data_w.Data[1] = halo_COMOB[t]->X_3D-Victim_part->Main_COMOB->X_3D;
			behave->Data.Just_data_w.Data[2] = halo_COMOB[t]->Y_3D-Victim_part->Main_COMOB->Y_3D; /* verschieben, so das Halo von der Mitte des Objects aus berechnet wird */
			behave->Data.Just_data_w.Data[3] = halo_COMOB[t]->Z_3D-Victim_part->Main_COMOB->Z_3D;

			s-=10;
		}

		/* Maximale Anzahl Schritte bis zum Ziel */
		delta_steps=-1;

		/* BÑlle erstellen */
		for(t=0;t<balls;t++){

			/* Add COMOB */
			lp_COMOB[t] = Add_COMOB(100);

			/* Success ? */
			if (!lp_COMOB[t]){
				ERROR_ClearStack();
				return;
			}

			/* Monster */
			Get_3D_part_coordinates(Victim_part,&sx,&sy,&sz);

			/* Zufallsbeginn */
			sx+=rndmm(-1500,1500);
			sy+=rndmm(-1500,1500);
			sz+=rndmm(-250,250);

			/* set coordinates */
			lp_COMOB[t]->X_3D = sx;
			lp_COMOB[t]->Y_3D = sy;
			lp_COMOB[t]->Z_3D = sz;

			/* Caster */
			Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&dx,&dy,&dz);

			/* Zufallsbeginn */
			dx+=rndmm(-1500,1500);
			dy+=rndmm(-1500,1500);
			dz+=rndmm(-250,250);

			/* Bewegungswerte ausrechnen */
			if((s=calc_delta_xyz(sx,sy,sz,dx,dy,dz,&delta_x[t],&delta_y[t],&delta_z[t],C3_Spell_11_LP_MOVESPEED))>delta_steps){
				delta_steps=s;
			}

			/* Set lifespan */
			lp_COMOB[t]->Lifespan = 0 ;

			/* Set display parameters */
//			lp_COMOB[t]->Draw_mode = NORMAL_COMOB_DRAWMODE;
			lp_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
//			lp_COMOB[t]->Special_handle = Transluminance_table_handle;
			lp_COMOB[t]->Special_handle = Transparency_table_handle;

			lp_COMOB[t]->Hotspot_X_offset = 50;
			lp_COMOB[t]->Hotspot_Y_offset = 50;

			/* Set random size */
			lp_COMOB[t]->Display_width = rndmm(50,100);
			lp_COMOB[t]->Display_height = lp_COMOB[t]->Display_width;

			/* Select random spark type */
			lp_COMOB[t]->Graphics_handle = lp_handle;

			/* Set number of animation frames */
			lp_COMOB[t]->Nr_frames = Get_nr_frames(lp_COMOB[t]->Graphics_handle);

			/* Select animation frame */
			lp_COMOB[t]->Frame = 0;

			/* Add oscillate behaviour for x */
			behave_x = Add_COMOB_behaviour(lp_COMOB[t], NULL, Oscillate_handler);

			if(!behave_x){
				/* Comob lîschen */
				Delete_COMOB(lp_COMOB[t]);
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave_x->Data.Oscillate_data.Type = OSCILLATE_X;
			behave_x->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_x->Data.Oscillate_data.Amplitude = rndmm(10,20)*COMOB_DEC_FACTOR;
			behave_x->Data.Oscillate_data.Value = t*90/balls;

			/* Add oscillate behaviour for y */
			behave_y = Add_COMOB_behaviour(lp_COMOB[t], NULL, Oscillate_handler);

			if(!behave_y){
				/* Comob lîschen */
				Delete_COMOB(lp_COMOB[t]);
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave_y->Data.Oscillate_data.Type = OSCILLATE_Y;
			behave_y->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_y->Data.Oscillate_data.Amplitude = rndmm(10,20)*COMOB_DEC_FACTOR;
			behave_y->Data.Oscillate_data.Value = (t*90/balls)+23;

		}

		/* LP's fliegen zum Castenden */
		s=0;
		do{
			Update_screen();
			for(u=0;u<Nr_combat_updates;u++){
				for(t=0;t<balls;t++){
					lp_COMOB[t]->X_3D+=delta_x[t];
					lp_COMOB[t]->Y_3D+=delta_y[t];
					lp_COMOB[t]->Z_3D+=delta_z[t];
				}
				s++;
			}
		}while(s<delta_steps);

		/* LP's lîschen */
		for(t=0;t<balls;t++){
			Delete_COMOB(lp_COMOB[t]);
		}

		/* Halo einige Zeit aufrecht erhalten */
		update_n_times(4);

		/* Halo COMOBS lîschen */
		for(t=0;t<C3_Spell_11_ANZHALOCOMOBS;t++){
			Delete_COMOB(halo_COMOB[t]);
		}

 		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));

		/* LP to Caster */
		Set_LP(Current_use_magic_data.Casting_participant->Char_handle,
		 LP + Damage);

		/* Speicher fÅr TransparenzTabellen freigeben */
		for(t=0;t<C3_Spell_11_ANZHALOCOMOBS;t++){
			MEM_Free_memory(Recolour_table_handle[t]);
		}

		/* LP Handle */
		MEM_Free_memory(lp_handle);

	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_12_handler
 *						 SP-Sucker
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:49
 * LAST      : 06.06.95 13:49
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_12_ANZHALOCOMOBS 3
#define C3_Spell_12_SP_MOVESPEED 250

#define C3_Spell_12_MIN_BALLS 16
#define C3_Spell_12_MAX_BALLS 32

void
C3_Spell_12_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_12);
}

void
Do_C3_Spell_12(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	MEM_HANDLE Recolour_table_handle[C3_Spell_12_ANZHALOCOMOBS];
	MEM_HANDLE sp_handle = NULL;
	UNBYTE *Recolour_table_ptr[C3_Spell_12_ANZHALOCOMOBS];
	struct COMOB *halo_COMOB[C3_Spell_12_ANZHALOCOMOBS];
	struct COMOB *sp_COMOB[C3_Spell_12_MAX_BALLS];
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_x;
	struct COMOB_behaviour *behave_y;
	SILONG delta_x[C3_Spell_12_MAX_BALLS];
	SILONG delta_y[C3_Spell_12_MAX_BALLS];
	SILONG delta_z[C3_Spell_12_MAX_BALLS];
	SILONG delta_steps;
	UNSHORT c_width,c_height;
	SILONG Left_3D, Top_3D, Right_3D, Bottom_3D;
	SILONG s,t,u,v;
	SILONG sx,sy,sz;
	SILONG dx,dy,dz;
	SILONG balls;
	UNSHORT Damage;
	UNSHORT SP;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* SpruchstÑrke = Grî·e der Flamen,FlamenbÑlle */
	balls=calc_strength(Strength,C3_Spell_12_MIN_BALLS,C3_Spell_12_MAX_BALLS);

	/* Spell deflected ? */
	if (Strength)
	{

		/* Load SP graphics */
		sp_handle = Load_subfile(COMBAT_GFX, 31);
		if (!sp_handle){
			ERROR_ClearStack();
			return;
		}

		/* No -> Calculate "damage" */
		Damage = max(((Strength * 50) / 100), 1);

		/* Get victim's SP */
		SP = Get_SP(Victim_part->Char_handle);

		/* Does the victim have this many SP ? */
		Damage = min(Damage, SP);

		/* zu jedem Halo COMOB eine TransparentFarbe generieren */
		s=20; /* AnfangsTransparenz */
		for(t=0;t<C3_Spell_12_ANZHALOCOMOBS;t++){
			Recolour_table_handle[t] = MEM_Allocate_memory(256);
			Recolour_table_ptr[t] = MEM_Claim_pointer(Recolour_table_handle[t]);
			Calculate_recolouring_table(Recolour_table_ptr[t], 1, 191, 255,128,0, s);
			MEM_Free_pointer(Recolour_table_handle[t]);
			s+=30;
		}

		/* Halo Comobs */
		s=30;
		for(t=0;t<C3_Spell_12_ANZHALOCOMOBS;t++){

			halo_COMOB[t]=Duplicate_COMOB(Victim_part->Main_COMOB);
			halo_COMOB[t]->Display_width+=s;
			halo_COMOB[t]->Display_height+=s;
			halo_COMOB[t]->Hotspot_X_offset = 50;
			halo_COMOB[t]->Hotspot_Y_offset = 50;
			halo_COMOB[t]->Draw_mode = COLOURING_COMOB_DRAWMODE;
			halo_COMOB[t]->Special_handle = Recolour_table_handle[t];

			/* Get rectangle covering source COMOB */
			Get_part_rectangle(Victim_part, &Left_3D, &Top_3D, &Right_3D, &Bottom_3D);

			/* Set halo COMOB coordinates in the middle of this rectangle */
			halo_COMOB[t]->X_3D = Left_3D + (Right_3D - Left_3D) / 2;
			halo_COMOB[t]->Y_3D = Bottom_3D + (Top_3D - Bottom_3D) / 2;
			halo_COMOB[t]->Z_3D = Victim_part->Main_COMOB->Z_3D+(t*2)+2;

			/* mit dem EnergieBall verknÅpfen*/
			if(!(behave = Add_COMOB_behaviour(halo_COMOB[t],Victim_part->Main_COMOB, connect_handler))){
				ERROR_ClearStack();
				return;
			}

			/* Set Connect behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+2+4;
			behave->Data.Just_data_w.Data[1] = halo_COMOB[t]->X_3D-Victim_part->Main_COMOB->X_3D;
			behave->Data.Just_data_w.Data[2] = halo_COMOB[t]->Y_3D-Victim_part->Main_COMOB->Y_3D; /* verschieben, so das Halo von der Mitte des Objects aus berechnet wird */
			behave->Data.Just_data_w.Data[3] = halo_COMOB[t]->Z_3D-Victim_part->Main_COMOB->Z_3D;

			s-=10;
		}

		/* Maximale Anzahl Schritte bis zum Ziel */
		delta_steps=-1;

		/* BÑlle erstellen */
		for(t=0;t<balls;t++){

			/* Add COMOB */
			sp_COMOB[t] = Add_COMOB(100);

			/* Success ? */
			if (!sp_COMOB[t]){
				ERROR_ClearStack();
				return;
			}

			/* Monster */
			Get_3D_part_coordinates(Victim_part,&sx,&sy,&sz);

			/* Zufallsbeginn */
			sx+=rndmm(-1500,1500);
			sy+=rndmm(-1500,1500);
			sz+=rndmm(-250,250);

			/* set coordinates */
			sp_COMOB[t]->X_3D = sx;
			sp_COMOB[t]->Y_3D = sy;
			sp_COMOB[t]->Z_3D = sz;

			/* Caster */
			Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&dx,&dy,&dz);

			/* Zufallsbeginn */
			dx+=rndmm(-1500,1500);
			dy+=rndmm(-1500,1500);
			dz+=rndmm(-250,250);

			/* Bewegungswerte ausrechnen */
			if((s=calc_delta_xyz(sx,sy,sz,dx,dy,dz,&delta_x[t],&delta_y[t],&delta_z[t],C3_Spell_12_SP_MOVESPEED))>delta_steps){
				delta_steps=s;
			}

			/* Set lifespan */
			sp_COMOB[t]->Lifespan = 0 ;

			/* Set display parameters */
//			sp_COMOB[t]->Draw_mode = NORMAL_COMOB_DRAWMODE;
			sp_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
//			sp_COMOB[t]->Special_handle = Transluminance_table_handle;
			sp_COMOB[t]->Special_handle = Transparency_table_handle;

			sp_COMOB[t]->Hotspot_X_offset = 50;
			sp_COMOB[t]->Hotspot_Y_offset = 50;

			/* Set random size */
			sp_COMOB[t]->Display_width = rndmm(50,100);
			sp_COMOB[t]->Display_height = sp_COMOB[t]->Display_width;

			/* Select random spark type */
			sp_COMOB[t]->Graphics_handle = sp_handle;

			/* Set number of animation frames */
			sp_COMOB[t]->Nr_frames = Get_nr_frames(sp_COMOB[t]->Graphics_handle);

			/* Select animation frame */
			sp_COMOB[t]->Frame = 0;

			/* Add oscillate behaviour for x */
			behave_x = Add_COMOB_behaviour(sp_COMOB[t], NULL, Oscillate_handler);

			if(!behave_x){
				/* Comob lîschen */
				Delete_COMOB(sp_COMOB[t]);
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave_x->Data.Oscillate_data.Type = OSCILLATE_X;
			behave_x->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_x->Data.Oscillate_data.Amplitude = rndmm(10,20)*COMOB_DEC_FACTOR;
			behave_x->Data.Oscillate_data.Value = t*90/balls;

			/* Add oscillate behaviour for y */
			behave_y = Add_COMOB_behaviour(sp_COMOB[t], NULL, Oscillate_handler);

			if(!behave_y){
				/* Comob lîschen */
				Delete_COMOB(sp_COMOB[t]);
				ERROR_ClearStack();
				return;
			}

			/* Set random bounce behaviour data */
			behave_y->Data.Oscillate_data.Type = OSCILLATE_Y;
			behave_y->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_y->Data.Oscillate_data.Amplitude = rndmm(10,20)*COMOB_DEC_FACTOR;
			behave_y->Data.Oscillate_data.Value = (t*90/balls)+23;

		}

		/* SP's fliegen zum Castenden */
		s=0;
		do{
			Update_screen();
			for(u=0;u<Nr_combat_updates;u++){
				for(t=0;t<balls;t++){
					sp_COMOB[t]->X_3D+=delta_x[t];
					sp_COMOB[t]->Y_3D+=delta_y[t];
					sp_COMOB[t]->Z_3D+=delta_z[t];
				}
				s++;
			}
		}while(s<delta_steps);

		/* SP's lîschen */
		for(t=0;t<balls;t++){
			Delete_COMOB(sp_COMOB[t]);
		}

		/* Halo einige Zeit aufrecht erhalten */
		update_n_times(4);

		/* Halo COMOBS lîschen */
		for(t=0;t<C3_Spell_12_ANZHALOCOMOBS;t++){
			Delete_COMOB(halo_COMOB[t]);
		}

		/* Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
			/* Yes -> Rest of effect */
//		}

 		/* Remove SP from victim */
		Set_SP(Victim_part->Char_handle, SP - Damage);

		/* Give SP to caster */
		SP = Get_SP(Current_use_magic_data.Casting_participant->Char_handle);
		Set_SP(Current_use_magic_data.Casting_participant->Char_handle,
		 SP + Damage);

		/* Speicher fÅr TransparenzTabellen freigeben */
		for(t=0;t<C3_Spell_12_ANZHALOCOMOBS;t++){
			MEM_Free_memory(Recolour_table_handle[t]);
		}

		/* SP Handle */
		MEM_Free_memory(sp_handle);

	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_13_handler
 *						 Personal Defence
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:54
 * LAST      : 06.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_13_handler(UNSHORT Strength)
{
	struct COMOB *glow_COMOB;
	struct COMOB_behaviour *behave_glow;
	struct Character_data *Char;
	UNSHORT Added;

	/* Increase magical defense */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Current_use_magic_data.Casting_participant->Char_handle);

	Added = max(((Strength * 50) / 100), 1);
	Char->xProtection_magic = min((Char->xProtection_magic + Added), 100);

	/* free mem */
	MEM_Free_pointer(Current_use_magic_data.Casting_participant->Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_14_handler
 *						 Dissolve Monster
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 14:06
 * LAST      : 07.06.95 14:06
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_14_ANZDISSOLVECOMOBS 600
#define C3_Spell_14_MIN_BALLS 8
#define C3_Spell_14_MAX_BALLS 16

#define C3_Spell_14_MAX_LIGHTCOMOB 6

#define C3_Spell_14_MOVESPEED 175

void
C3_Spell_14_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_14);
}

static struct COMOB *ball_COMOB[C3_Spell_14_ANZDISSOLVECOMOBS];

void
Do_C3_Spell_14(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *load_COMOB[C3_Spell_14_MAX_BALLS];
	struct COMOB *COMOB;
	struct COMOB *light_COMOB[C3_Spell_14_MAX_LIGHTCOMOB];
	struct COMOB *energie_COMOB;
	struct COMOB *flash_COMOB;
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_flash;
	MEM_HANDLE explode_handle = NULL;
	SILONG dissolve_balls,aktlightcomob;
	SILONG delta_x;
	SILONG delta_y;
	SILONG delta_z;
	SILONG delta_steps;
	SILONG sx,sy,sz;
	SILONG dx,dy,dz;
	SILONG orgx,orgy,orgz;
	SILONG balls;
	SILONG s,t,etime,nextball,nextlight;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	/* Load explode graphics */
	explode_handle = Load_subfile(COMBAT_GFX, 24);
	if (!explode_handle){
		ERROR_ClearStack();
		return;
	}

	/* SpruchstÑrke = Grî·e der Flamen,FlamenbÑlle */
	balls=calc_strength(Strength,C3_Spell_14_MIN_BALLS,C3_Spell_14_MAX_BALLS);

	/* OrginalKoordinaten des Monsters */
	Get_3D_part_coordinates(Victim_part,&orgx,&orgy,&orgz);

	/* Monster aufladen */
	nextball=0;
	nextlight=40;
	etime=160;
	aktlightcomob=0;
	do{

		Update_screen();

		if(nextlight<0){
			nextlight+=40;
			/* Monster wird heller */
			light_COMOB[aktlightcomob]=Duplicate_COMOB(Victim_part->Main_COMOB);
			light_COMOB[aktlightcomob]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			light_COMOB[aktlightcomob]->Special_handle = Transluminance_table_handle;
//			light_COMOB[aktlightcomob]->Special_handle = Transparency_table_handle;

			/* mit dem Main COMOB verknÅpfen*/
			if(!(behave = Add_COMOB_behaviour(light_COMOB[aktlightcomob],Victim_part->Main_COMOB, connect_handler))){
				ERROR_ClearStack();
				return;
			}

			/* Set Connect behaviour data */
			behave->Data.Just_data_w.Data[0] = 1+2+4;
			behave->Data.Just_data_w.Data[1] = 0;
			behave->Data.Just_data_w.Data[2] = 0;
			behave->Data.Just_data_w.Data[3] = 0;

			aktlightcomob++;
		}

		if(nextball<0){
			nextball+=4;

			/* BÑlle erstellen */
			for(t=0;t<balls;t++){

				/* Add COMOB */
				energie_COMOB = Add_COMOB(100);

				/* Success ? */
				if (!energie_COMOB){
					ERROR_ClearStack();
					return;
				}

				/* Monster */
				Get_3D_part_coordinates(Victim_part,&sx,&sy,&sz);

				/* Zufallsbeginn */
				sx+=rndmm2(-4500,-3500,3500,4500);
				sy+=rndmm2(-4500,-3500,3500,4500);
				sz+=rndmm2(-4500,-3500,3500,4500);

				/* set coordinates */
				energie_COMOB->X_3D = sx;
				energie_COMOB->Y_3D = sy;
				energie_COMOB->Z_3D = sz;

				/* Monster */
				Get_3D_part_coordinates(Victim_part,&dx,&dy,&dz);

				/* Zufallsbewegung */
				dx+=rndmm(-500,500);
				dy+=rndmm(-500,500);
				dz+=rndmm(-500,500);

				/* Bewegungswerte ausrechnen */
				if((s=calc_delta_xyz(sx,sy,sz,dx,dy,dz,&delta_x,&delta_y,&delta_z,C3_Spell_14_MOVESPEED))>delta_steps){
					delta_steps=s;
				}

				energie_COMOB->dX_3D = delta_x;
				energie_COMOB->dY_3D = delta_y;
				energie_COMOB->dZ_3D = delta_z;

				/* Set lifespan */
				energie_COMOB->Lifespan = 50 ;

				/* Set display parameters */
//				energie_COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;
				energie_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
				energie_COMOB->Special_handle = Transluminance_table_handle;
//				energie_COMOB->Special_handle = Transparency_table_handle;

				energie_COMOB->Hotspot_X_offset = 50;
				energie_COMOB->Hotspot_Y_offset = 50;

				/* Set random size */
				energie_COMOB->Display_width = 50;
				energie_COMOB->Display_height = energie_COMOB->Display_width;

				/* Select random spark type */
				energie_COMOB->Graphics_handle = Spark_gfx_handles[1];

				/* Set number of animation frames */
				energie_COMOB->Nr_frames = Get_nr_frames(energie_COMOB->Graphics_handle);

				/* Select animation frame */
				energie_COMOB->Frame = 0;

				/* Wenn LifeSpan 0 erreicht wurd anderes COMOB kurz darstellen */
				if(!(behave = Add_COMOB_behaviour(energie_COMOB,NULL, lifedead_handler))){
					ERROR_ClearStack();
					return;
				}

				/* Set Connect behaviour data */
				behave->Data.Just_data.Data[0] = (SILONG)explode_handle;
				behave->Data.Just_data.Data[1] = 20;
				behave->Data.Just_data.Data[2] = 25;
				behave->Data.Just_data.Data[3] = 25;
				behave->Data.Just_data.Data[4] = -800;
				behave->Data.Just_data.Data[5] = -800;
				behave->Data.Just_data.Data[6] = (SILONG)Luminance_table_handle;

			}
		}

		etime-=Nr_combat_updates;
		nextball-=Nr_combat_updates;
		nextlight-=Nr_combat_updates;

	}while(etime>0);

	update_n_times(40);

	/* Monster wieder abhellen */
	etime=12;
	nextlight=3;
	aktlightcomob--;
	do{
		Update_screen();

		if(nextlight<0){
			Delete_COMOB(light_COMOB[aktlightcomob]);
			if(--aktlightcomob<0){
				break;
			}
			nextlight+=3;
		}

		nextlight-=Nr_combat_updates;
		etime-=Nr_combat_updates;

	}while(etime>0);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */

			/* Dissolve Monster */
			dissolve_balls=Dissolve(Victim_part,3,3,&ball_COMOB[0],C3_Spell_14_ANZDISSOLVECOMOBS);

			/* Main COMOB ausschalten */
			Delete_COMOB(Victim_part->Main_COMOB);

			/* aufflashen */
			if((flash_COMOB=gen_Dummy_COMOB())==NULL){
				ERROR_ClearStack();
				return;
			}

			behave_flash = Add_COMOB_behaviour(flash_COMOB, NULL, flash_handler);

			/* Grî·e Ñndern */
			if(!behave_flash){
				/* Comob lîschen */
				Delete_COMOB(flash_COMOB);
				ERROR_ClearStack();
				return;
			}

			behave_flash->Data.Just_data.Data[0]=8; /* X Ticks lang Palette zur entsprechenden Farbe flashen und 5 Ticks wieder zur Ursprungspalette*/
			behave_flash->Data.Just_data.Data[1]=160; /* nach hellblau umblenden */
			behave_flash->Data.Just_data.Data[2]=160;
			behave_flash->Data.Just_data.Data[3]=200;
			behave_flash->Data.Just_data.Data[4]=60; /* maximal 50 % */

			/* Updaten */
			Update_screen();

			/* Kugeln zittern */
			etime=80;
			do{

				Update_screen();

				for(t=0;t<dissolve_balls;t++){
					ball_COMOB[t]->X_3D+=rndmm(-5,5);
					ball_COMOB[t]->Y_3D+=rndmm(-5,5);
					ball_COMOB[t]->Z_3D+=rndmm(-5,5);
				}

				etime-=Nr_combat_updates;

			}while(etime>0);

			/* und explodieren dann */
			for(t=0;t<dissolve_balls;t++){

				/* nach oben schleudern */
				ball_COMOB[t]->dX_3D=rndmm(-30,30);
				ball_COMOB[t]->dY_3D=0;
				ball_COMOB[t]->dZ_3D=rndmm(-30,30);

				/* Lifespan setzten */
				ball_COMOB[t]->Lifespan=rndmm(96,180);

				/* Add bounce behaviour for y */
				behave = Add_COMOB_behaviour(ball_COMOB[t], NULL, Bounce_handler);

				if(!behave){
					/* Comob lîschen */
					ERROR_ClearStack();
					break;
				}

				/* Set bounce behaviour data */
				behave->Data.Bounce_data.Gravity = rndmm(15,30);
				behave->Data.Bounce_data.Bounce = rndmm(40,60);
				behave->Data.Bounce_data.Air_friction = rndmm(3,6);

				/* Wenn LifeSpan 0 erreicht wurd anderes COMOB kurz darstellen */
				if(!(behave = Add_COMOB_behaviour(ball_COMOB[t],NULL, lifedead_handler))){
					ERROR_ClearStack();
					return;
				}

				/* Set Connect behaviour data */
				behave->Data.Just_data.Data[0] = (SILONG)explode_handle;
				behave->Data.Just_data.Data[1] = 25;
				behave->Data.Just_data.Data[2] = 60;
				behave->Data.Just_data.Data[3] = 60;
				behave->Data.Just_data.Data[4] = -1200;
				behave->Data.Just_data.Data[5] = -1200;
				behave->Data.Just_data.Data[6] = (SILONG)Transluminance_table_handle;

			}

			/* darstellen */
			update_n_times(180);

		}

 		/* Dissolve victim */
		Destroy_participant(Victim_part);
	}

	/* explode Handle */
	MEM_Free_memory(explode_handle);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_15_handler
 *						 Remove Traps
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 15:46
 * LAST      : 07.06.95 15:46
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C3_Spell_15_RANGE 1600

#define C3_Spell_15_MIN_STARS 8
#define C3_Spell_15_MAX_STARS 2

#define C3_Spell_15_MIN_SPARKS 6
#define C3_Spell_15_MAX_SPARKS 2

void
C3_Spell_15_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_15);
}

void
Do_C3_Spell_15(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB_behaviour *behave;
	struct COMOB *COMOB;
	SILONG time,nextstar,nextspark,stars,sparks;
	SILONG x,y,z;

	stars=calc_strength(Strength,C3_Spell_15_MIN_STARS,C3_Spell_15_MAX_STARS);
	sparks=calc_strength(Strength,C3_Spell_15_MIN_SPARKS,C3_Spell_15_MAX_SPARKS);

	/* Start of effect */

	/* Is there a trap on this square ? */
	if (Is_trap(Tactical_X, Tactical_Y))
	{
		/* Yes -> Rest of effect */

		/* Sterne blitzen auf */

		/* Falle entfernen */
		time=140;
		nextstar=0;
		nextspark=0;
		do{
			Update_screen();

			/* nÑchster Stern blitzt auf */
			if(nextstar<0){
				nextstar=stars;

				/* Koordiaten des Feldes */
				Convert_tactical_to_3D_coordinates(Tactical_X,Tactical_Y,&x,&z);

				/* Stern erstellen */
				if((COMOB=Gen_COMOB(x+rndmm(-C3_Spell_15_RANGE,C3_Spell_15_RANGE),0,z+rndmm(-C3_Spell_15_RANGE,C3_Spell_15_RANGE), 0, 10, Star_gfx_handles[0], GC_NORM))==NULL){
					ERROR_ClearStack();
					return;
				}

				/* SizeII verhalten */
				if(!(behave = Add_COMOB_behaviour(COMOB,NULL, size_II_handler))){
					ERROR_ClearStack();
					return;
				}

				/* Set Connect behaviour data */
				behave->Data.Just_data_w.Data[0] = 500;
				behave->Data.Just_data_w.Data[1] = 500;
				behave->Data.Just_data_w.Data[2] = 100;
				behave->Data.Just_data_w.Data[3] = 100;
				behave->Data.Just_data_w.Data[4] = 1500;
				behave->Data.Just_data_w.Data[5] = 1500;
				behave->Data.Just_data_w.Data[6] = 2;
				behave->Data.Just_data_w.Data[7] = 2;

			}

			/* nÑchstes Spark steigt hoch */
			if(nextspark<0){
				nextspark=sparks;

				/* Koordiaten des Feldes */
				Convert_tactical_to_3D_coordinates(Tactical_X,Tactical_Y,&x,&z);

				/* Stern erstellen */
				if((COMOB=Gen_COMOB(x+rndmm(-C3_Spell_15_RANGE,C3_Spell_15_RANGE),5*COMOB_DEC_FACTOR,z+rndmm(-C3_Spell_15_RANGE,C3_Spell_15_RANGE), 0, 100, Spark_gfx_handles[rndmm(0,2)], GC_TRANS))==NULL){
					ERROR_ClearStack();
					return;
				}
				COMOB->Nr_frames=0;
				COMOB->dY_3D=rndmm(25,75);

				/* Size Verhalten */
				behave = Add_COMOB_behaviour(COMOB, NULL, size_handler);

				/* Grî·e Ñndern */
				if(!behave){
					/* Comob lîschen */
					Delete_COMOB(COMOB);
					ERROR_ClearStack();
					return;
				}

				behave->Data.Just_data_w.Data[0]=rndmm(-200,-100); /* jeden Tick um 1% */
				behave->Data.Just_data_w.Data[1]=rndmm(-200,-100);
				behave->Data.Just_data_w.Data[2]=5; /* bis x % */
				behave->Data.Just_data_w.Data[3]=5;

			}

			nextstar-=Nr_combat_updates;
			nextspark-=Nr_combat_updates;
			time-=Nr_combat_updates;
		}while(time>0);

		/* Remove trap */
		Remove_trap(Tactical_X, Tactical_Y);

		/* Noch kurze Zeit darstellen */
		update_n_times(48);

	}
}




