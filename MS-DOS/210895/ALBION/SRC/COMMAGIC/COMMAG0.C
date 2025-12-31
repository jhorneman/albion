/************
 * NAME     : COMMAG0.C
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
#include <BBMEM.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <CONTROL.H>
#include <MAGIC.H>
#include <COMBVAR.H>
#include <COMACTS.H>
#include <COMOBS.H>
#include <COMMAGIC.H>
#include <COMMSUB.H>
#include <ITMLOGIC.H>
#include <TACTICAL.H>
#include <XFTYPES.H>



#if FALSE
#include <BBDOS.H>

//	inittexttofile();

void inittexttofile()
{
	char	 dbginit[]="DEBUG-FILE ALBION\n";

	/* DEBUG File initialisieren */
	if(DOS_WriteFile( "FGTDBG.TXT", &dbginit[0], strlen(&dbginit[0]))==NULL){
		printf("Fehler beim Erstellen des Debugfiles\n");
		return;
	}
}

void texttofile(char *string)
{
/* bei Demonstartion inaktiv */
	SISHORT	fh;

	if((fh=DOS_Open( "FGTDBG.TXT", BBDOSFILESTAT_APPEND))==-1)
		return;

	if(DOS_Write(fh, string, strlen(string))!=strlen(string))
		return;

	DOS_Close(fh);

}

		{
			char pr[64];
			UNSHORT Source_width, Source_height;
			struct BBPOINT pnt;

			if(frame>=frames)
				break;

			OPM_FillBox(global_screenport.screenopmptr,0,0,320,200,0);

			/* No -> Find current frame */
			Gfx = (struct Gfx_header *)
				(MEM_Claim_pointer(ast_COMOB->Graphics_handle)
				+ ast_COMOB->Graphics_offset);

			for (i=0;i<frame;i++)
			{
				Ptr = (UNBYTE *) (Gfx + 1);
				Gfx = (struct Gfx_header *) (Ptr + (Gfx->Width * Gfx->Height));
			}
			Ptr = (UNBYTE *) (Gfx + 1);

			/* Get source dimensions */
			Source_width = Gfx->Width;
			Source_height = Gfx->Height;

			Put_zoomed_block(global_screenport.screenopmptr, (max_ast_width/2)-(Source_width/2), max_ast_height-Source_height, Source_width,
				Source_height, Source_width, Source_height, Ptr);

			DSA_CopyMainOPMToScr(DSA_ALWAYS);
			DSA_DoubleBuffer();
			DSA_CopyMainOPMToScr(DSA_ALWAYS);
			DSA_DoubleBuffer();


			while(SYSTEM_GetBLEVStatusLong()!=BLEV_NOKEY){}

			while(SYSTEM_GetBLEVStatusLong()==BLEV_NOKEY){

				DSA_CopyMainOPMToScr(DSA_ALWAYS);
				DSA_DoubleBuffer();

				BLEV_GetMousePos(&pnt);

				x=(max_ast_width/2)-pnt.x;
				y=max_ast_height-pnt.y;

				OPM_FillBox(global_screenport.screenopmptr,0,100,200,24,0);

				OPM_printf(global_screenport.screenopmptr,0,100,255," Frame %ld",frame);
				OPM_printf(global_screenport.screenopmptr,0,110,255," Mouse %ld,%ld",x,y);

			}

			sprintf(pr,"{%ld,%ld}, /* Frame %ld */\n",x,y,frame);
			texttofile(pr);

			Nr_combat_updates=1;
		}
#endif





#if FALSE
/******* Spell for all Monsters or row of Monsters **********/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_7_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:53
 * LAST      : 03.06.95 13:53
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_7_handler(UNSHORT Strength)
{
	struct Combat_participant *Victim_part;
	UNSHORT Temp_strength;
	SILONG i;

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_7);

	/* Get victim participant data */
	Victim_part = Current_target_parts[0];

	/* No -> Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Rest of effect */


	}

	/* Blitz bewegen */
	for (i=0;i<Current_nr_target_parts;i++)
	{
		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

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

			/* Add temporary effect */
			Add_temporary_effect(Victim_part, FROZEN_TEMP_EFFECT, Strength, 10);

		}

	}

}

void
Do_C0_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{

	/* Store pointer */
	Current_target_parts[Current_nr_target_parts] = Victim_part;

	/* Count up */
	Current_nr_target_parts++;

}
#endif




/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : show_samenkorn
 * FUNCTION : SamenKorn fliegt vom Betrachter zur angegebenen Position
 * INPUTS   : UNSHORT Strength : StÑrke (Halo des SamenKorns hÑngt davon ab)
 *						SILONG zx,zy,zz : ZielPosition
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define SHOW_SAMENKORN_MIN_SIZE 20
#define SHOW_SAMENKORN_MAX_SIZE 80

void show_samenkorn(UNSHORT Strength,SILONG zx,SILONG zy,SILONG zz)
{
	MEM_HANDLE korn_handle = NULL;
	struct COMOB *korn_COMOB;
	SILONG sx,sy,sz;
	SILONG mx,my,mz;
	SILONG steps,size;

	/* Grî·e des Halos */
	size=calc_strength(Strength,SHOW_SAMENKORN_MIN_SIZE,SHOW_SAMENKORN_MAX_SIZE);

	/* Samenkorn laden */
	korn_handle = Load_subfile(COMBAT_GFX, 18);
	if (!korn_handle){
		ERROR_ClearStack();
		return;
	}

	/* Position des Zaubernden */
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&sx,&sy,&sz);

	/* Samenkorn fliegt zum Monster */
	steps=calc_delta_xyz(sx,sy,sz,zx,zy,zz,&mx,&my,&mz,5*COMOB_DEC_FACTOR);

	/* Samenkorn erstellen */
	if((korn_COMOB=Gen_COMOB(sx,sy,sz, steps, 100, korn_handle, GC_NORM))==NULL){
		ERROR_ClearStack();
		return;
	}
	korn_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
	korn_COMOB->dX_3D=mx;
	korn_COMOB->dY_3D=my;
	korn_COMOB->dZ_3D=mz;

	/* Halo zum Korn addieren */
	Add_halo_to_COMOB_2(korn_COMOB,3,size,steps,HALO_NULL);

	/* Samenkorn bewegen */
	update_n_times(steps);

}

/* #FUNCTION END# */




/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_1_handler
 *						 Lame Monster
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 29.05.95 15:14
 * LAST      : 02.06.95 16:07
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_Spell_1_MIN_SPARKS 100
#define C0_Spell_1_MAX_SPARKS 200

void
C0_Spell_1_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_1);
}

void
Do_C0_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *vp_COMOB;
	struct COMOB *hp_COMOB;
	SILONG zx,zy,zz;
	SILONG m_left,m_right,m_top,m_bottom;
	SILONG p_left,p_right,p_top,p_bottom;
	SILONG p_wsize,p_hsize;
	SILONG time,sparks;
	MEM_HANDLE vp_handle = NULL;
	MEM_HANDLE hp_handle = NULL;



//	Do_C0_Spell_20(Victim_part, Tactical_X, Tactical_Y, Strength);
//	return;


	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	sparks=calc_strength(Strength,C0_Spell_1_MIN_SPARKS,C0_Spell_1_MAX_SPARKS);

	/* Pflanzenteil vorne laden */
	vp_handle = Load_subfile(COMBAT_GFX, 34);
	if (!vp_handle){
		ERROR_ClearStack();
		return;
	}

	/* Pflanzenteil vorne laden */
	hp_handle = Load_subfile(COMBAT_GFX, 34);
	if (!hp_handle){
		ERROR_ClearStack();
		return;
	}

	/* 3D Part Koordinates */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* Samenkorn fliegt zum Monster */
	show_samenkorn(Strength,zx,0,zz);

	/* Korn explodiert */
	Gen_Sparks(Victim_part->Main_COMOB->X_3D,5*COMOB_DEC_FACTOR,Victim_part->Main_COMOB->Z_3D,50/*amount*/,rndmm(150,200)/*speed*/,rndmm(30,40)/*life*/,rndmm(50,75)/*size*/,GEN_SPARK_TYP_ALL);

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
		/* Pflanze erscheint und wÑchst am Monster hoch */

		/* Pflanze vorne erstellen */
		if((vp_COMOB=Gen_COMOB(zx,zy,zz-5, 0, 100, vp_handle, GC_NORM))==NULL){
			ERROR_ClearStack();
			return;
		}
		vp_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
		vp_COMOB->Hotspot_Y_offset=100;

		/* Pflanze hinten erstellen */
		if((hp_COMOB=Gen_COMOB(zx,zy,zz+5, 0, 100, hp_handle, GC_NORM))==NULL){
			ERROR_ClearStack();
			return;
		}
		hp_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
		hp_COMOB->Hotspot_Y_offset=100;

		/* 3D-Grî·e des Monsters */
		Get_part_rectangle(Victim_part,&m_left,&m_top,&m_right,&m_bottom);

		/* untere Seite des Monsters */
		if(m_bottom>0){
			m_bottom=0;
		}

		/* 3D-Grî·e der Pflanze */
		Get_COMOB_rectangle(vp_COMOB,&p_left,&p_top,&p_right,&p_bottom);

		/* Endgrî·e der Pflanze */
		p_wsize=(m_right-m_left)*100/(p_right-p_left);
		p_hsize=(m_top-m_bottom)*100/(p_top-p_bottom);

		/* Position der Pflanze */
		vp_COMOB->X_3D=m_left+(m_right-m_left)/2;
		vp_COMOB->Y_3D=m_bottom;
		vp_COMOB->Z_3D=zz-5;
		hp_COMOB->X_3D=vp_COMOB->X_3D;
		hp_COMOB->Y_3D=vp_COMOB->Y_3D;
		hp_COMOB->Z_3D=zz+5;

		/* Pflanze auf Monster skalieren */
		vp_COMOB->Display_width=2;
		vp_COMOB->Display_height=2;

		/* Daten von vorderem COMOB */
		hp_COMOB->Display_width=vp_COMOB->Display_width;
		hp_COMOB->Display_height=vp_COMOB->Display_height;

		/* nach Explosion kurz warten */
		update_n_times(4);

		/* Pflanze wÑchst am Monster empor */
		do{
			Update_screen();

			/* Breite */
			if(vp_COMOB->Display_width!=p_wsize){
				vp_COMOB->Display_width+=Nr_combat_updates*2;
				if(vp_COMOB->Display_width>p_wsize){
					vp_COMOB->Display_width=p_wsize;
				}
				hp_COMOB->Display_width=vp_COMOB->Display_width;
			}
			/* Hîhe */
			if(vp_COMOB->Display_height!=p_hsize){
				vp_COMOB->Display_height+=Nr_combat_updates*4;
				if(vp_COMOB->Display_height>p_hsize){
					vp_COMOB->Display_height=p_hsize;
				}
				hp_COMOB->Display_height=vp_COMOB->Display_height;
			}

		}while(vp_COMOB->Display_width!=p_wsize||vp_COMOB->Display_height!=p_hsize);

		/* Pflanze auf Monster skalieren */
		vp_COMOB->Display_width=p_wsize;
		vp_COMOB->Display_height=p_hsize;

		/* Daten von vorderem COMOB */
		hp_COMOB->Display_width=vp_COMOB->Display_width;
		hp_COMOB->Display_height=vp_COMOB->Display_height;

		/* Lame victim */
		Set_condition(Victim_part->Char_handle, LAMED);

	}

	/* warten bis Ende Effekt */
	update_n_times(12);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_3_handler
 *						 Poison
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 29.05.95 15:22
 * LAST      : 02.06.95 16:08
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */
#define C0_Spell_3_MIN_STARS 4
#define C0_Spell_3_MAX_STARS 12

void
C0_Spell_3_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_3);
}

void
Do_C0_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB_behaviour *behave;
	struct COMOB *shape_COMOB;
	struct COMOB *COMOB;
	MEM_HANDLE gift_handle = NULL;
	SILONG left,right,top,bottom;
	SILONG t,x,y,z;
	SILONG basex,basey;
	SILONG time,stars;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	/* Anzahl Sterne */
	stars=calc_strength(Strength,C0_Spell_3_MIN_STARS,C0_Spell_3_MAX_STARS);

	/* Handle fÅr Object gift */
	gift_handle = Load_subfile(COMBAT_GFX, 25);
	if (!gift_handle){
		ERROR_ClearStack();
		return;
	}

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

		/* Monster Konturen verschwimmen */
		if((shape_COMOB=Duplicate_COMOB(Victim_part->Main_COMOB))==NULL){
			ERROR_ClearStack();
			return;
		}
		shape_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		shape_COMOB->Special_handle = Transluminance_table_handle;
		shape_COMOB->Z_3D-=1;

		/* Add oscillate behaviour for x */
		behave = Add_COMOB_behaviour(shape_COMOB, NULL, Oscillate_handler);

		if(!behave){
			/* Comob lîschen */
			Delete_COMOB(shape_COMOB);
			/* No -> Clear error stack */
			ERROR_ClearStack();
			return;
		}

		/* Set oscillate behaviour data */
		behave->Data.Oscillate_data.Type = OSCILLATE_X;
		behave->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
		behave->Data.Oscillate_data.Amplitude = 150;
		behave->Data.Oscillate_data.Value = 1*90/3;

		/* Add oscillate behaviour for y */
		behave = Add_COMOB_behaviour(shape_COMOB, NULL, Oscillate_handler);

		if(!behave){
			/* Comob lîschen */
			Delete_COMOB(shape_COMOB);

			/* No -> Clear error stack */
			ERROR_ClearStack();

			return;
		}

		/* Set oscillate behaviour data */
		behave->Data.Oscillate_data.Type = OSCILLATE_Y;
		behave->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
		behave->Data.Oscillate_data.Amplitude = 150;
		behave->Data.Oscillate_data.Value = (1*90/3)+23;


		/* Sterne blitzen auf */

		/* Aufblitzem */
		time=120;
		do{
			Update_screen();

			/* nÑchster Stern blitzt auf */
			for(t=0;t<stars;t++){

				/* OrginalKoordinaten des Monsters */
				Get_3D_part_coordinates(Victim_part,&x,&y,&z);
				Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

				/* ZufallsPosition auf Object */
				x=rndmm(left,right);
				y=rndmm(bottom,top);

				/* Position gehîrt zur Maske */
				if(Test_COMOB_mask(Victim_part->Main_COMOB,x,y)){

					/* Stern erstellen */
					if((COMOB=Gen_COMOB(x,y,z, 0, 5, gift_handle, GC_NORM))==NULL){
						ERROR_ClearStack();
						return;
					}

					/* mem handle danach wieder freigeben */
					COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

					/* SizeII verhalten */
					if(!(behave = Add_COMOB_behaviour(COMOB,NULL, size_II_handler))){
						ERROR_ClearStack();
						return;
					}

					/* Set Connect behaviour data */
					behave->Data.Just_data_w.Data[0] = 500;
					behave->Data.Just_data_w.Data[1] = 500;
					behave->Data.Just_data_w.Data[2] = 35;
					behave->Data.Just_data_w.Data[3] = 35;
					behave->Data.Just_data_w.Data[4] = 1500;
					behave->Data.Just_data_w.Data[5] = 1500;
					behave->Data.Just_data_w.Data[6] = 2;
					behave->Data.Just_data_w.Data[7] = 2;

				}

			}

			time-=Nr_combat_updates;
		}while(time>0);

		/* noch kurze Zeit darstellen */
		update_n_times(16);

		/* VErschwimmeffekt beenden */
		Delete_COMOB(shape_COMOB);

		/* Add temporary effect */
		Add_temporary_effect(Victim_part, POISON_TEMP_EFFECT, Strength, 10);

		/* noch kurze Zeit darstellen */
		update_n_times(24);

	}

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_4_handler
 *						 Hurry
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:48
 * LAST      : 03.06.95 13:48
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_Spell_4_MIN_SHAPES 3
#define C0_Spell_4_MAX_SHAPES 6

void
C0_Spell_4_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_4);
}

void
Do_C0_Spell_4(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *COMOB;
	struct COMOB *fairy0_COMOB;
	struct COMOB *fairy1_COMOB;
	struct COMOB *fairy2_COMOB;
	struct COMOB *shape_COMOB[C0_Spell_4_MAX_SHAPES];
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_x[C0_Spell_4_MAX_SHAPES];
	struct COMOB_behaviour *behave_z[C0_Spell_4_MAX_SHAPES];
	SILONG left,right,top,bottom;
	SILONG s,t,x,y,z,nextstar,shapes;
	SILONG size,sizeadd,time;
	BOOLEAN bool;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	/* Anzahl Shapes */
//	shapes=calc_strength(Strength,C0_Spell_4_MIN_SHAPES,C0_Spell_4_MAX_SHAPES);

	shapes=3;

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */

			/* Shapes bewegen sich spiralfîrmig nach au·en */
			size=200;
			sizeadd=20;

			/* Bei Monstern 3 einfarbiges Shape */
			for(t=0;t<shapes;t++){
				/* Stern erstellen */
				if((shape_COMOB[t]=Duplicate_COMOB(Victim_part->Main_COMOB))==NULL){
					ERROR_ClearStack();
					return;
				}
				shape_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
				shape_COMOB[t]->Special_handle = Transluminance_table_handle;
				shape_COMOB[t]->Special_offset = 0;

				/* Add oscillate behaviour for x */
				behave_x[t] = Add_COMOB_behaviour(shape_COMOB[t], NULL, Oscillate_handler);

				if(!behave_x[t]){
					/* Comob lîschen */
					Delete_COMOB(shape_COMOB[t]);
					/* No -> Clear error stack */
					ERROR_ClearStack();
					return;
				}

				/* Set oscillate behaviour data */
				behave_x[t]->Data.Oscillate_data.Type = OSCILLATE_X;
				behave_x[t]->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
				behave_x[t]->Data.Oscillate_data.Amplitude = size;
				behave_x[t]->Data.Oscillate_data.Value = t*90/shapes;

				/* Add oscillate behaviour for y */
				behave_z[t] = Add_COMOB_behaviour(shape_COMOB[t], NULL, Oscillate_handler);

				if(!behave_z[t]){
					/* Comob lîschen */
					Delete_COMOB(shape_COMOB[t]);

					/* No -> Clear error stack */
					ERROR_ClearStack();

					return;
				}

				/* Set oscillate behaviour data */
				behave_z[t]->Data.Oscillate_data.Type = OSCILLATE_Z;
				behave_z[t]->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
				behave_z[t]->Data.Oscillate_data.Amplitude = size;
				behave_z[t]->Data.Oscillate_data.Value = (t*90/shapes)+23;

			}

			bool=TRUE;
			while(bool){
				Update_screen();

				for(t=0;t<shapes;t++){
					behave_x[t]->Data.Oscillate_data.Amplitude = size;
					behave_z[t]->Data.Oscillate_data.Amplitude = size;

					for(s=0;s<Nr_combat_updates;s++){
						size+=sizeadd;
						if(size>5000){
							size=5000;
							sizeadd=-sizeadd;
						}
						if(size<100){
							bool=FALSE;
						}

					}

				}

			}

			/* Shapes wieder lîschen */
			for(t=0;t<shapes;t++){
				Delete_COMOB(shape_COMOB[t]);
			}

			/* noch kurze Zeit darstellen */
			update_n_times(4);

		}
		else{
			/* Yes -> Rest of effect */

			/* Bei PartyMitgliedern Bezauberungseffekt */

			/* Aufblitzem */
			/* OrginalKoordinaten des Monsters */
			Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
			Get_3D_part_coordinates(Victim_part,&x,&y,&z);

			/* Stern erstellen */
			if((fairy0_COMOB=Gen_COMOB(x,COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR,z, 0, 25, Star_gfx_handles[0], GC_NORM))==NULL){
				ERROR_ClearStack();
				return;
			}

			/* Add fairy behaviour */
			behave = Add_COMOB_behaviour(fairy0_COMOB, NULL, Fairy_handler);
			if(!behave){
				/* Comob lîschen */
				Delete_COMOB(fairy0_COMOB);
				ERROR_ClearStack();
				return;
			}

			if((fairy1_COMOB=Duplicate_COMOB(fairy0_COMOB))==NULL){
				Delete_COMOB(fairy0_COMOB);
				ERROR_ClearStack();
				return;
			}
			fairy1_COMOB->X_3D+=10*COMOB_DEC_FACTOR;

			/* Add fairy behaviour */
			behave = Add_COMOB_behaviour(fairy1_COMOB, NULL, Fairy_handler);
			if(!behave){
				/* Comob lîschen */
				Delete_COMOB(fairy0_COMOB);
				Delete_COMOB(fairy1_COMOB);
				ERROR_ClearStack();
				return;
			}

			if((fairy2_COMOB=Duplicate_COMOB(fairy0_COMOB))==NULL){
				Delete_COMOB(fairy0_COMOB);
				Delete_COMOB(fairy1_COMOB);
				ERROR_ClearStack();
				return;
			}
			fairy2_COMOB->X_3D-=10*COMOB_DEC_FACTOR;

			/* Add fairy behaviour */
			behave = Add_COMOB_behaviour(fairy2_COMOB, NULL, Fairy_handler);
			if(!behave){
				/* Comob lîschen */
				Delete_COMOB(fairy0_COMOB);
				Delete_COMOB(fairy1_COMOB);
				Delete_COMOB(fairy2_COMOB);
				ERROR_ClearStack();
				return;
			}

			/* noch kurze Zeit darstellen */
			update_n_times(96);

			/* Fairy_COMOB lîschen */
			Delete_COMOB(fairy0_COMOB);
			Delete_COMOB(fairy1_COMOB);
			Delete_COMOB(fairy2_COMOB);

			/* noch kurze Zeit darstellen */
			update_n_times(32);

		}

		/* Add temporary effect */
		Add_temporary_effect(Victim_part, HURRY_TEMP_EFFECT, Strength, 10);

	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_5_handler
 *						 Show LP
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:50
 * LAST      : 03.06.95 13:50
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_5_COMBAT_FIELDS (5*6)

void
C0_Spell_5_handler(UNSHORT Strength)
{
	struct COMOB **clist;
	struct COMOB *COMOB;
	struct COMOB *chalos[C0_5_COMBAT_FIELDS][3];
	struct Combat_participant *Victim_part;
	BOOLEAN monsterhalo[C0_5_COMBAT_FIELDS];
	UNSHORT Temp_strength;
	struct COMOB *wand_COMOB;
	MEM_HANDLE wand_handle = NULL;
	SILONG left,right,top,bottom;
	SILONG time,i,t;

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_5);

	/* Handle fÅr Object */
	wand_handle = Load_subfile(COMBAT_GFX, 56);
	if (!wand_handle){
		ERROR_ClearStack();
		return;
	}

	/* Get victim participant data */
	Victim_part = Current_target_parts[0];

	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE){

		/* Yes -> Start of effect */

		/* Wand erstellen */
		if((wand_COMOB=Gen_COMOB(0,0,-50*COMOB_DEC_FACTOR, 0, 100, wand_handle, GC_MAGIC))==NULL){
			ERROR_ClearStack();
			return;
		}
		wand_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
		wand_COMOB->Hotspot_Y_offset=100;

		wand_COMOB->dZ_3D=2*COMOB_DEC_FACTOR;

		Get_COMOB_rectangle(wand_COMOB,&left,&top,&right,&bottom);

		wand_COMOB->Display_width=COMBAT_3D_WIDTH*100/(right-left);
		wand_COMOB->Display_height=((COMBAT_3D_HEIGHT+20)*COMOB_DEC_FACTOR)*100/(top-bottom);

		/* Halo noch nicht aktiv */
		for(t=0;t<C0_5_COMBAT_FIELDS;t++){
			monsterhalo[t]=FALSE;
		}

		/* Wand bewegt sich nach hinten */
		time=(COMBAT_3D_DEPTH-(wand_COMOB->Z_3D))/wand_COMOB->dZ_3D;
		do{

			Update_screen();

			/* Feststellen welche Monster im Strahl sind */
			for (i=0;i<Current_nr_target_parts;i++){

				/* Get victim participant data */
				Victim_part = Current_target_parts[i];

				/* Handle magical defense */
				Temp_strength = Handle_magical_defense(Victim_part, Strength);

				Temp_strength=-1;

				/* Spell deflected ? */
				if(Temp_strength){

					/* Monster befindet sich im Strahl dann aufleuchten */
					if(Victim_part->Main_COMOB->Z_3D>(wand_COMOB->Z_3D-500)
					 &&Victim_part->Main_COMOB->Z_3D<(wand_COMOB->Z_3D+500)){

						/* Halo zu Monster */
						if(!monsterhalo[i]){
							clist=Add_halo_to_COMOB_2(Victim_part->Main_COMOB, 3, 10, 0,HALO_CONNECT);
							chalos[i][0]=clist[0];
							chalos[i][1]=clist[1];
							chalos[i][2]=clist[2];
							monsterhalo[i]=TRUE;
							/* Show LP of victim */
							Victim_part->Flags |= PART_SHOW_LP;
						}

					}

				}

			}

			time-=Nr_combat_updates;

		}while(time>0);

		/* Object dann lîschen */
		Delete_COMOB(wand_COMOB);

		/* Alle Halos lîschen */
		for(t=0;t<Current_nr_target_parts;t++){
			chalos[t][0]->Lifespan=1;
			chalos[t][1]->Lifespan=1;
			chalos[t][2]->Lifespan=1;
		}

		/* Halos werden jetzt gelîscht */
		update_n_times(4);

	}

}

void
Do_C0_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C0_Spell_6_handler
 *						 Freeze Bomb Single
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:52
 * LAST      : 03.06.95 13:52
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_6_MAX_EIS 400
#define C0_Spell_6_MIN_SIZE_EIS 20
#define C0_Spell_6_MAX_SIZE_EIS 30

static struct COMOB *eis_COMOB[C0_6_MAX_EIS];

void
C0_Spell_6_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_6);
}

void
Do_C0_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	MEM_HANDLE Recolour_table_handle;
	UNBYTE *Recolour_table_ptr;
	SILONG zx,zy,zz;
	SILONG x,y,z,s,t,u;
	SILONG eisnr;
	struct COMOB *blitz_COMOB;
	struct COMOB *shape_COMOB;
	struct COMOB *COMOB;
	SILONG left,right,top,bottom;
	SILONG nreis;
	SILONG nreissize;
	struct COMOB_behaviour *behave;
	MEM_HANDLE blitz_handle = NULL;
	MEM_HANDLE eis_handle = NULL;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	/* Blitz laden */
	blitz_handle = Load_subfile(COMBAT_GFX, 33);
	if (!blitz_handle){
		ERROR_ClearStack();
		return;
	}

	/* BlauTransparenzTabelle erstellen */
	Recolour_table_handle = MEM_Allocate_memory(256);
	Recolour_table_ptr = MEM_Claim_pointer(Recolour_table_handle);
	Calculate_recolouring_table(Recolour_table_ptr, 1, 191, 160,160,255, 50);

	/* Anzahl Eiskristalle */
	nreis=3;
	nreissize=calc_strength(Strength,C0_Spell_6_MIN_SIZE_EIS,C0_Spell_6_MAX_SIZE_EIS);

	/* Position des Monsters */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* Samenkorn fliegt zu ZielPosition */
	show_samenkorn(Strength,zx,0,zz);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
//		}

		/* Korn explodiert */
		Gen_Sparks(Victim_part->Main_COMOB->X_3D,5*COMOB_DEC_FACTOR,Victim_part->Main_COMOB->Z_3D,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_BLUE);

		/* Yes -> Rest of effect */

		/* Position des Monsters */
		Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
		Get_3D_part_coordinates(Victim_part,&x,&y,&z);

		/* Monster mit Eiskristallen zukleistern */
		eisnr=0;
		y=bottom;
		do{

			Update_screen();

			for(u=0;u<2;u++){
				for(s=0;s<Nr_combat_updates;s++){
					for(t=0;t<nreis;t++){
						/* ZufallsPosition rechts bis links*/
						x=rndmm(left,right);
						/* Ist Position auf Maske */
						if(Test_COMOB_mask(Victim_part->Main_COMOB,x,y)){
							/* eis laden */
							eis_handle = Load_subfile(COMBAT_GFX, 26);
							if (!eis_handle){
								ERROR_ClearStack();
								return;
							}
							/* Eis erstellen */
							if((eis_COMOB[eisnr]=Gen_COMOB(x,y,z, 0, nreissize, eis_handle, GC_TRANS))==NULL){
								ERROR_ClearStack();
								return;
							}
							eis_COMOB[eisnr]->User_flags|=COMOB_FREE_GFX_ON_DELETE;
							if(++eisnr>=C0_6_MAX_EIS){
								goto leavelop1c06;
							}
						}
					}
					y+=100;
				}
			}

		}while(y<top);

leavelop1c06:

		/* Eiskristalle kurz darstellen */
		update_n_times(8);

		/* Koordinaten Monster */
		Get_3D_part_coordinates(Victim_part,&x,&y,&z);

		/* Blitz blitzt auf */
		if((blitz_COMOB=Gen_COMOB(x,y,z-5, 0, 10, blitz_handle, GC_FIRE))==NULL){
			ERROR_ClearStack();
			return;
		}
		blitz_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

		/* SizeII verhalten */
		if(!(behave = Add_COMOB_behaviour(blitz_COMOB,NULL, size_II_handler))){
			ERROR_ClearStack();
			return;
		}

		/* Set size behaviour data */
		behave->Data.Just_data_w.Data[0] = 2500;
		behave->Data.Just_data_w.Data[1] = 2500;
		behave->Data.Just_data_w.Data[2] = 100;
		behave->Data.Just_data_w.Data[3] = 100;
		behave->Data.Just_data_w.Data[4] = 500;
		behave->Data.Just_data_w.Data[5] = 500;
		behave->Data.Just_data_w.Data[6] = 2;
		behave->Data.Just_data_w.Data[7] = 2;

		/* Blitz blÑht sich auf */
		update_n_times(4);

		/* Transparentes Shape generieren fÅr Eis */
		/* Monster Konturen verschwimmen */
		if((shape_COMOB=Duplicate_COMOB(Victim_part->Main_COMOB))==NULL){
			ERROR_ClearStack();
			return;
		}
		shape_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
		shape_COMOB->Draw_mode = COLOURING_COMOB_DRAWMODE;
		shape_COMOB->Special_handle = Recolour_table_handle;
		shape_COMOB->Z_3D-=1;

		/* Danach explodieren EisKristalle */
		for(t=0;t<eisnr;t++){

			/* Bewegung */
			eis_COMOB[t]->dX_3D=rndmm(-150,150);
			eis_COMOB[t]->dY_3D=rndmm(150,250);
			eis_COMOB[t]->dZ_3D=rndmm(-150,150);

			/* Add bounce behaviour for y */
			behave = Add_COMOB_behaviour(eis_COMOB[t], NULL, Bounce_handler);

			if(!behave){
				/* Comob lîschen */
				ERROR_ClearStack();
				break;
			}

			/* Set bounce behaviour data */
			behave->Data.Bounce_data.Gravity = rndmm(10,20);
			behave->Data.Bounce_data.Bounce = rndmm(10,20);
			behave->Data.Bounce_data.Air_friction = rndmm(3,6);

			/* Behaviour fÅr Grî·e */
			behave = Add_COMOB_behaviour(eis_COMOB[t], NULL, size_handler);

			/* Grî·e Ñndern */
			if(!behave){
				/* Comob lîschen */
				ERROR_ClearStack();
				break;
			}

			behave->Data.Just_data_w.Data[0]=rndmm(-60,-20); /* jeden Tick um 1% */
			behave->Data.Just_data_w.Data[1]=behave->Data.Just_data_w.Data[0];
			behave->Data.Just_data_w.Data[2]=2; /* bis 25 % */
			behave->Data.Just_data_w.Data[3]=2;

		}

		/* Kristalle explodieren und verschwinden */
		update_n_times(128);

		/* COMOB fÅr Shape lîschen */
		Delete_COMOB(shape_COMOB);

		/* Add temporary effect */
		Add_temporary_effect(Victim_part, FROZEN_TEMP_EFFECT, Strength, 10);

	}

	/* Tabelle freigeben */
	MEM_Free_pointer(Recolour_table_handle);
	MEM_Free_memory(Recolour_table_handle);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_7_handler
 *						 Freeze Bomb row
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:53
 * LAST      : 03.06.95 13:53
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_7_MAX_EIS C0_6_MAX_EIS
#define C0_Spell_7_MIN_SIZE_EIS 20
#define C0_Spell_7_MAX_SIZE_EIS 30

#define C0_Spell_7_MIN_SNOW 25
#define C0_Spell_7_MAX_SNOW 75

void
C0_Spell_7_handler(UNSHORT Strength)
{
	MEM_HANDLE blitz_handle = NULL;
	MEM_HANDLE eis_handle = NULL;
	struct COMOB_behaviour *behave;
	struct COMOB *blitz_COMOB;
	struct COMOB *shape_COMOB;
	struct COMOB_behaviour *behave_snow;
	struct COMOB *snow_COMOB;
	struct Combat_participant *Victim_part;
	SILONG zx,zy,zz,snow;
	SILONG left,right,top,bottom;
	SILONG x,y,z;
	UNSHORT Temp_strength;
	SILONG nreis;
	SILONG nreissize;
	SILONG i,s,t,u,eisnr;

	/* Blitz laden */
	blitz_handle = Load_subfile(COMBAT_GFX, 33);
	if (!blitz_handle){
		ERROR_ClearStack();
		return;
	}

	/* Wieviel Schnee erzeugen */
	snow=calc_strength(Strength,C0_Spell_7_MIN_SNOW,C0_Spell_7_MAX_SNOW);

	/* Anzahl Eiskristalle */
	nreis=3;
	nreissize=calc_strength(Strength,C0_Spell_7_MIN_SIZE_EIS,C0_Spell_7_MAX_SIZE_EIS);

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_7);

	/* Get victim participant data */
	Victim_part = Current_target_parts[0];

	/* Position des Monsters */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* SamenKorn fliegt zum MittelPunkt der Reihe */
	show_samenkorn(Strength,0,0,zz);

	/* Korn explodiert */
	Gen_Sparks(0,5*COMOB_DEC_FACTOR,Victim_part->Main_COMOB->Z_3D,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_BLUE);

	/* Schnee darstellen */
	update_n_times(16);

	/* No -> Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Rest of effect */

//	}

	/* Schneeschauer beginnt */

	/* Schnee COMOB erzeugen */
	if((snow_COMOB=gen_Dummy_COMOB())==NULL){
		ERROR_ClearStack();
		return;
	}

	/* permanenten Schneeschauer erzeugen */
	behave_snow = Add_COMOB_behaviour(snow_COMOB, NULL, snow_II_handler);

	if(!behave_snow){
		/* Comob lîschen */
		Delete_COMOB(snow_COMOB);
		ERROR_ClearStack();
		return;
	}

	behave_snow->Data.Just_data.Data[0] = 0; /* X Start-Koordinate */
	behave_snow->Data.Just_data.Data[1] = 0; /* Y Start-Koordinate */
	behave_snow->Data.Just_data.Data[2] = zz; /* Z Start-Koordinate */
	behave_snow->Data.Just_data.Data[3] = snow; /* Anzahl neue Schneeflocken */
	behave_snow->Data.Just_data.Data[4] = 1; /* Schneeflocken nur Åber Reihe */

	/* Schnee darstellen */
	update_n_times(32);

	/* Blitz bewegen */
	for (i=0;i<Current_nr_target_parts;i++)
	{
		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if(Temp_strength)
		{
			/* No -> Victim is monster ? */
//			if (Victim_part->Type == MONSTER_PART_TYPE)
//			{
				/* Yes -> Rest of effect */
//			}

			/* Monster vereisen */

			/* Position des Monsters */
			Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
			Get_3D_part_coordinates(Victim_part,&x,&y,&z);

			/* Monster mit Eiskristallen zukleistern */
			eisnr=0;
			y=bottom;
			do{

				Update_screen();

				for(u=0;u<3;u++){
					for(s=0;s<Nr_combat_updates;s++){
						for(t=0;t<nreis;t++){
							/* ZufallsPosition rechts bis links*/
							x=rndmm(left,right);
							/* Ist Position auf Maske */
							if(Test_COMOB_mask(Victim_part->Main_COMOB,x,y)){
								/* eis laden */
								eis_handle = Load_subfile(COMBAT_GFX, 26);
								if (!eis_handle){
									ERROR_ClearStack();
									return;
								}
								/* Eis erstellen */
								if((eis_COMOB[eisnr]=Gen_COMOB(x,y,z, 0, nreissize, eis_handle, GC_TRANS))==NULL){
									ERROR_ClearStack();
									return;
								}
								eis_COMOB[eisnr]->User_flags|=COMOB_FREE_GFX_ON_DELETE;
								if(++eisnr>=C0_7_MAX_EIS){
									goto leavelop1c07;
								}
							}
						}
						y+=100;
					}
				}

			}while(y<top);

leavelop1c07:

			/* Eiskristalle kurz darstellen */
			update_n_times(8);

			/* Koordinaten Monster */
			Get_3D_part_coordinates(Victim_part,&x,&y,&z);

			/* Blitz blitzt auf */
			if((blitz_COMOB=Gen_COMOB(x,y,z-5, 0, 10, blitz_handle, GC_FIRE))==NULL){
				ERROR_ClearStack();
				return;
			}
			blitz_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

			/* SizeII verhalten */
			if(!(behave = Add_COMOB_behaviour(blitz_COMOB,NULL, size_II_handler))){
				ERROR_ClearStack();
				return;
			}

			/* Set size behaviour data */
			behave->Data.Just_data_w.Data[0] = 2500;
			behave->Data.Just_data_w.Data[1] = 2500;
			behave->Data.Just_data_w.Data[2] = 100;
			behave->Data.Just_data_w.Data[3] = 100;
			behave->Data.Just_data_w.Data[4] = 500;
			behave->Data.Just_data_w.Data[5] = 500;
			behave->Data.Just_data_w.Data[6] = 2;
			behave->Data.Just_data_w.Data[7] = 2;

			/* Blitz blÑht sich auf */
			update_n_times(4);

			/* Transparentes Shape generieren fÅr Eis */




			/* Danach explodieren EisKristalle */
			for(t=0;t<eisnr;t++){

				/* Bewegung */
				eis_COMOB[t]->dX_3D=rndmm(-150,150);
				eis_COMOB[t]->dY_3D=rndmm(150,250);
				eis_COMOB[t]->dZ_3D=rndmm(-150,150);

				/* Add bounce behaviour for y */
				behave = Add_COMOB_behaviour(eis_COMOB[t], NULL, Bounce_handler);

				if(!behave){
					/* Comob lîschen */
					ERROR_ClearStack();
					break;
				}

				/* Set bounce behaviour data */
				behave->Data.Bounce_data.Gravity = rndmm(10,20);
				behave->Data.Bounce_data.Bounce = rndmm(10,20);
				behave->Data.Bounce_data.Air_friction = rndmm(3,6);

				/* Behaviour fÅr Grî·e */
				behave = Add_COMOB_behaviour(eis_COMOB[t], NULL, size_handler);

				/* Grî·e Ñndern */
				if(!behave){
					/* Comob lîschen */
					ERROR_ClearStack();
					break;
				}

				behave->Data.Just_data_w.Data[0]=rndmm(-60,-20); /* jeden Tick um 1% */
				behave->Data.Just_data_w.Data[1]=behave->Data.Just_data_w.Data[0];
				behave->Data.Just_data_w.Data[2]=2; /* bis 25 % */
				behave->Data.Just_data_w.Data[3]=2;

			}

			/* Kristalle explodieren und verschwinden */
			update_n_times(64);

			/* Add temporary effect */
			Add_temporary_effect(Victim_part, FROZEN_TEMP_EFFECT, Strength, 10);

		}

	}

	/* noch kurze Zeit darstellen */
	update_n_times(4);

	/* SchneeShauer beenden */
	Delete_COMOB(snow_COMOB);

	/* noch kurze Zeit darstellen */
	update_n_times(64);

}

void
Do_C0_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C0_Spell_8_handler
 *						 Freeze Bomb all
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:53
 * LAST      : 03.06.95 13:53
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_8_MAX_EIS C0_6_MAX_EIS
#define C0_Spell_8_MIN_SIZE_EIS 20
#define C0_Spell_8_MAX_SIZE_EIS 30

#define C0_Spell_8_MIN_SNOW 25
#define C0_Spell_8_MAX_SNOW 75

void
C0_Spell_8_handler(UNSHORT Strength)
{
	MEM_HANDLE blitz_handle = NULL;
	MEM_HANDLE eis_handle = NULL;
	struct COMOB_behaviour *behave;
	struct COMOB *blitz_COMOB;
	struct COMOB *shape_COMOB;
	struct COMOB_behaviour *behave_snow;
	struct COMOB *snow_COMOB;
	struct Combat_participant *Victim_part;
	SILONG zx,zy,zz,snow;
	SILONG left,right,top,bottom;
	SILONG x,y,z;
	UNSHORT Temp_strength;
	SILONG nreis;
	SILONG nreissize;
	SILONG i,s,t,u,eisnr;

	/* Blitz laden */
	blitz_handle = Load_subfile(COMBAT_GFX, 33);
	if (!blitz_handle){
		ERROR_ClearStack();
		return;
	}

	/* Wieviel Schnee erzeugen */
	snow=calc_strength(Strength,C0_Spell_8_MIN_SNOW,C0_Spell_8_MAX_SNOW);

	/* Anzahl Eiskristalle */
	nreis=3;
	nreissize=calc_strength(Strength,C0_Spell_8_MIN_SIZE_EIS,C0_Spell_8_MAX_SIZE_EIS);

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_8);

	/* Get victim participant data */
	Victim_part = Current_target_parts[0];

	/* Position des Monsters */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* SamenKorn fliegt zum MittelPunkt der Reihe */
	show_samenkorn(Strength,0,0,COMBAT_3D_DEPTH/2);

	/* Korn explodiert */
	Gen_Sparks(0,5*COMOB_DEC_FACTOR,COMBAT_3D_DEPTH/2,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_BLUE);

	/* Schnee darstellen */
	update_n_times(16);

	/* No -> Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Rest of effect */

//	}

	/* Schneeschauer beginnt */

	/* Schnee COMOB erzeugen */
	if((snow_COMOB=gen_Dummy_COMOB())==NULL){
		ERROR_ClearStack();
		return;
	}

	/* permanenten Schneeschauer erzeugen */
	behave_snow = Add_COMOB_behaviour(snow_COMOB, NULL, snow_II_handler);

	if(!behave_snow){
		/* Comob lîschen */
		Delete_COMOB(snow_COMOB);
		ERROR_ClearStack();
		return;
	}

	behave_snow->Data.Just_data.Data[0] = 0; /* Z Start-Koordinate */
	behave_snow->Data.Just_data.Data[1] = 0; /* Z Start-Koordinate */
	behave_snow->Data.Just_data.Data[2] = zz; /* Z Start-Koordinate */
	behave_snow->Data.Just_data.Data[3] = snow; /* Anzahl neue Schneeflocken */
	behave_snow->Data.Just_data.Data[4] = 2; /* Schneeflocken Åber das ganze Feld */

	/* Schnee darstellen */
	update_n_times(32);

	/* Blitz bewegen */
	for (i=0;i<Current_nr_target_parts;i++)
	{
		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if(Temp_strength)
		{
			/* No -> Victim is monster ? */
//			if (Victim_part->Type == MONSTER_PART_TYPE)
//			{
				/* Yes -> Rest of effect */
//			}

			/* Monster vereisen */

			/* Position des Monsters */
			Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
			Get_3D_part_coordinates(Victim_part,&x,&y,&z);

			/* Monster mit Eiskristallen zukleistern */
			eisnr=0;
			y=bottom;
			do{

				Update_screen();

				for(u=0;u<8;u++){
					for(s=0;s<Nr_combat_updates;s++){
						for(t=0;t<nreis;t++){
							/* ZufallsPosition rechts bis links*/
							x=rndmm(left,right);
							/* Ist Position auf Maske */
							if(Test_COMOB_mask(Victim_part->Main_COMOB,x,y)){
								/* eis laden */
								eis_handle = Load_subfile(COMBAT_GFX, 26);
								if (!eis_handle){
									ERROR_ClearStack();
									return;
								}
								/* Eis erstellen */
								if((eis_COMOB[eisnr]=Gen_COMOB(x,y,z, 0, nreissize, eis_handle, GC_TRANS))==NULL){
									ERROR_ClearStack();
									return;
								}
								eis_COMOB[eisnr]->User_flags|=COMOB_FREE_GFX_ON_DELETE;
								if(++eisnr>=C0_8_MAX_EIS){
									goto leavelop1c08;
								}
							}
						}
						y+=100;
					}
				}

			}while(y<top);

leavelop1c08:

			/* Eiskristalle kurz darstellen */
			update_n_times(4);

			/* Koordinaten Monster */
			Get_3D_part_coordinates(Victim_part,&x,&y,&z);

			/* Blitz blitzt auf */
			if((blitz_COMOB=Gen_COMOB(x,y,z-5, 0, 10, blitz_handle, GC_FIRE))==NULL){
				ERROR_ClearStack();
				return;
			}
			blitz_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

			/* SizeII verhalten */
			if(!(behave = Add_COMOB_behaviour(blitz_COMOB,NULL, size_II_handler))){
				ERROR_ClearStack();
				return;
			}

			/* Set size behaviour data */
			behave->Data.Just_data_w.Data[0] = 2500;
			behave->Data.Just_data_w.Data[1] = 2500;
			behave->Data.Just_data_w.Data[2] = 100;
			behave->Data.Just_data_w.Data[3] = 100;
			behave->Data.Just_data_w.Data[4] = 500;
			behave->Data.Just_data_w.Data[5] = 500;
			behave->Data.Just_data_w.Data[6] = 2;
			behave->Data.Just_data_w.Data[7] = 2;

			/* Blitz blÑht sich auf */
			update_n_times(4);

			/* Transparentes Shape generieren fÅr Eis */




			/* Danach explodieren EisKristalle */
			for(t=0;t<eisnr;t++){

				/* Bewegung */
				eis_COMOB[t]->dX_3D=rndmm(-150,150);
				eis_COMOB[t]->dY_3D=rndmm(150,250);
				eis_COMOB[t]->dZ_3D=rndmm(-150,150);

				/* Add bounce behaviour for y */
				behave = Add_COMOB_behaviour(eis_COMOB[t], NULL, Bounce_handler);

				if(!behave){
					/* Comob lîschen */
					ERROR_ClearStack();
					break;
				}

				/* Set bounce behaviour data */
				behave->Data.Bounce_data.Gravity = rndmm(10,20);
				behave->Data.Bounce_data.Bounce = rndmm(10,20);
				behave->Data.Bounce_data.Air_friction = rndmm(3,6);

				/* Behaviour fÅr Grî·e */
				behave = Add_COMOB_behaviour(eis_COMOB[t], NULL, size_handler);

				/* Grî·e Ñndern */
				if(!behave){
					/* Comob lîschen */
					ERROR_ClearStack();
					break;
				}

				behave->Data.Just_data_w.Data[0]=rndmm(-60,-20); /* jeden Tick um 1% */
				behave->Data.Just_data_w.Data[1]=behave->Data.Just_data_w.Data[0];
				behave->Data.Just_data_w.Data[2]=2; /* bis 25 % */
				behave->Data.Just_data_w.Data[3]=2;

			}

			/* Kristalle explodieren und verschwinden */
			update_n_times(32);

			/* Add temporary effect */
			Add_temporary_effect(Victim_part, FROZEN_TEMP_EFFECT, Strength, 10);

		}

	}

	/* noch kurze Zeit darstellen */
	update_n_times(4);

	/* SchneeShauer beenden */
	Delete_COMOB(snow_COMOB);

	/* noch kurze Zeit darstellen */
	update_n_times(80);

}

void
Do_C0_Spell_8(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C0_Spell_10_handler
 *						 Blind single Monster
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:53
 * LAST      : 03.06.95 13:53
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_10_FLASHRED 220
#define C0_10_FLASHGREEN 180
#define C0_10_FLASHBLUE 180

void
C0_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_10);
}

void
Do_C0_Spell_10(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *COMOB;
	struct COMOB_behaviour *behave_flash;
	MEM_HANDLE beam_handle = NULL;
	SILONG x,y,z,s,t,pal;
	SILONG left,right,top,bottom;
	SILONG max_width;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE){
//		/* Yes -> Start of effect */
//	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* MonsterPosition */
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

	/* SamenKorn fliegt zum Monster */
	show_samenkorn(Strength,x,0,z);

	/* Korn explodiert */
	Gen_Sparks(x,5*COMOB_DEC_FACTOR,COMBAT_3D_DEPTH/2,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_BLUE);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
			/* Yes -> Rest of effect */
//		}

		/* Load Beam graphics */
		beam_handle = Load_subfile(COMBAT_GFX, 40);
		if (!beam_handle){
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


		/* set coordinates */
		COMOB->X_3D = x;
		COMOB->Y_3D = bottom;
		COMOB->Z_3D = z;

		/* Set lifespan */
		COMOB->Lifespan = 0 ;

		/* Set display parameters */
		COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		COMOB->Special_handle = Transluminance_table_handle;

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 100;

		/* Set width so width of Monster */
		max_width=(right-left)*100/COMBAT_SQUARE_WIDTH;
		COMOB->Display_width = 2;
		COMOB->Display_height = 30000;

		/* Select random spark type */
		COMOB->Graphics_handle = beam_handle;

		/* Set number of animation frames */
		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

		/* Select animation frame */
		COMOB->Frame = 0;

		/* den Beam hochzoomen */
		t=2;
		pal=0;
		do{
			Update_screen();
			COMOB->Display_width = t;
			Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,pal);
			t+=Nr_combat_updates*28; /* Beam Geschwindigkeit */
			if(pal<100){
				pal+=Nr_combat_updates*3;
				if(pal>100){
					pal=100;
				}
			}
		}while(t<=max_width);
		COMOB->Display_width = max_width;

		/* laufend Sparks generieren */
		t=0;
		s=0;
		do{
			Update_screen();
			if(s<0){
				if(Combat_display_detail_level>=DEFAULT_COMBAT_DISPLAY_DETAIL_LEVEL){
					Gen_Sparks(left+rndmm(10*COMOB_DEC_FACTOR,20*COMOB_DEC_FACTOR),0,z,30/*amount*/,100/*maxspeed*/,40/*life*/,30/*size*/,GEN_SPARK_TYP_BLUE);
	 				Gen_Sparks(right-rndmm(10*COMOB_DEC_FACTOR,20*COMOB_DEC_FACTOR),0,z,30/*amount*/,100/*maxspeed*/,40/*life*/,30/*size*/,GEN_SPARK_TYP_BLUE);
				}
				s+=10;
			}
			s-=Nr_combat_updates;
			t+=Nr_combat_updates; /* Beam Geschwindigkeit */
		}while(t<75);

		/* danach fÑllt Beam horizontal wieder in sich zusammen */
		t=COMOB->Display_width;
		do{
			Update_screen();
			COMOB->Display_width = t;
			Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,pal);
			if(pal>0){
				pal-=Nr_combat_updates*2;
				if(pal<0){
					pal=0;
				}
			}
			t-=Nr_combat_updates*7; /* Beam zusammenfall Geschwindigkeit */
		}while(t>2);
		COMOB->Display_width = 2;

		/* Palette wieder reseten */
		Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,0);

		/* Add temporary effect */
		Add_temporary_effect(Victim_part, BLINDED_TEMP_EFFECT, Strength, 10);

		/* Noch kurze Zeit darstellen */
		update_n_times(8);

		/* Beam wieder lîschen */
		Delete_COMOB(COMOB);

	}

	/* Handle freigeben */
	MEM_Free_memory(beam_handle);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_11_handler
 *						 Blind row of Monsters
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:54
 * LAST      : 03.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_11_COMBAT_FIELDS (5*6)

void
C0_Spell_11_handler(UNSHORT Strength)
{
	struct COMOB *beam_COMOB[C0_11_COMBAT_FIELDS];
	struct COMOB_behaviour *behave_flash;
	MEM_HANDLE beam_handle = NULL;
	SILONG x,y,z,s,t,pal;
	SILONG beamx[C0_11_COMBAT_FIELDS];
	SILONG beamz[C0_11_COMBAT_FIELDS];
	SILONG beaml[C0_11_COMBAT_FIELDS];
	SILONG beamt[C0_11_COMBAT_FIELDS];
	SILONG beamr[C0_11_COMBAT_FIELDS];
	SILONG beamb[C0_11_COMBAT_FIELDS];
	SILONG bspr[C0_11_COMBAT_FIELDS];
	SILONG beam_max_width[C0_11_COMBAT_FIELDS];
	SILONG nof_beams;
	SILONG left,right,top,bottom;
	SILONG max_width;
	struct Combat_participant *Victim_part;
	UNSHORT Temp_strength;
	SILONG i;

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_11);

	/* Get victim participant data */
	Victim_part = Current_target_parts[0];

	/* No -> Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Rest of effect */
//	}
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);

	/* SamenKorn fliegt zum Monster */
	show_samenkorn(Strength,0,0,z);

	/* Korn explodiert */
	Gen_Sparks(0,5*COMOB_DEC_FACTOR,COMBAT_3D_DEPTH/2,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_BLUE);

	nof_beams=0;
	for (i=0;i<Current_nr_target_parts;i++)
	{
		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* MonsterPosition */
		Get_3D_part_coordinates(Victim_part,&x,&y,&z);
		Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if(Temp_strength)
		{
			/* No -> Victim is monster ? */
//			if (Victim_part->Type == MONSTER_PART_TYPE)
//			{
				/* Yes -> Rest of effect */

//			}

			beamx[nof_beams]=x;
			beamz[nof_beams]=z;
			beaml[nof_beams]=left;
			beamt[nof_beams]=top;
			beamr[nof_beams]=right;
			beamb[nof_beams]=bottom;
			nof_beams++;

			/* Add temporary effect */
			Add_temporary_effect(Victim_part, BLINDED_TEMP_EFFECT, Strength, 10);

		}

	}

	/* Åberhaupt Monster betroffen */
	if(nof_beams>0){
		/* Load Beam graphics */
		beam_handle = Load_subfile(COMBAT_GFX, 40);
		if (!beam_handle){
			ERROR_ClearStack();
			return;
		}

		max_width=-1;

		/* Beams generieren */
		for(t=0;t<nof_beams;t++){

			/* Position Beam */
			x=beamx[t];
			z=beamz[t];
			left=beaml[t];
			top=beamt[t];
			right=beamr[t];
			bottom=beamb[t];

			/* Add beam_COMOB[t] */
			beam_COMOB[t] = Add_COMOB(100);

			/* Success ? */
			if (!beam_COMOB[t]){
				ERROR_ClearStack();
				return;
			}

			/* set coordinates */
			beam_COMOB[t]->X_3D = x;
			beam_COMOB[t]->Y_3D = bottom;
			beam_COMOB[t]->Z_3D = z;

			/* Set lifespan */
			beam_COMOB[t]->Lifespan = 0 ;

			/* Set display parameters */
			beam_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			beam_COMOB[t]->Special_handle = Transluminance_table_handle;

			beam_COMOB[t]->Hotspot_X_offset = 50;
			beam_COMOB[t]->Hotspot_Y_offset = 100;

			/* Set width to width of Monster */
			beam_max_width[t]=(right-left)*100/COMBAT_SQUARE_WIDTH;
			if(beam_max_width[t]>max_width){
				max_width=beam_max_width[t];
			}

			beam_COMOB[t]->Display_width = 2;
			beam_COMOB[t]->Display_height = 30000;

			/* Select random spark type */
			beam_COMOB[t]->Graphics_handle = beam_handle;

			/* Set number of animation frames */
			beam_COMOB[t]->Nr_frames = Get_nr_frames(beam_COMOB[t]->Graphics_handle);

			/* Select animation frame */
			beam_COMOB[t]->Frame = 0;

			/* sparkbeam lîschen */
			bspr[t]=0;
		}

		/* die Beams hochzoomen */
		s=2;
		pal=0;
		do{
			Update_screen();
			for(t=0;t<nof_beams;t++){
				beam_COMOB[t]->Display_width = s;
				if(beam_COMOB[t]->Display_width>beam_max_width[t]){
					beam_COMOB[t]->Display_width=beam_max_width[t];
				}
			}
			Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,pal);
			s+=Nr_combat_updates*28; /* Beam Geschwindigkeit */
			if(pal<100){
				pal+=Nr_combat_updates*3;
				if(pal>100){
					pal=100;
				}
			}
		}while(s<=max_width);
		for(t=0;t<nof_beams;t++){
			beam_COMOB[t]->Display_width = beam_max_width[t];
		}

		/* laufend Sparks generieren */
		s=0;
		do{
			Update_screen();

			/* FÅe jeden Beam Sparks generieren */
			for(t=0;t<nof_beams;t++){
				if(bspr[t]<0){
					if(Combat_display_detail_level==MAX_COMBAT_DISPLAY_DETAIL_LEVEL){
						x=beamx[t];
						z=beamz[t];
						left=beaml[t];
						right=beamr[t];
						Gen_Sparks(left+rndmm(10*COMOB_DEC_FACTOR,20*COMOB_DEC_FACTOR),0,z,10/*amount*/,100/*maxspeed*/,40/*life*/,30/*size*/,GEN_SPARK_TYP_BLUE);
		 				Gen_Sparks(right-rndmm(10*COMOB_DEC_FACTOR,20*COMOB_DEC_FACTOR),0,z,10/*amount*/,100/*maxspeed*/,40/*life*/,30/*size*/,GEN_SPARK_TYP_BLUE);
					}
					bspr[t]=5;
				}
				bspr[t]-=Nr_combat_updates;
			}

			s+=Nr_combat_updates; /* Beam Geschwindigkeit */
		}while(s<75);

		/* danach fÑllt Beam horizontal wieder in sich zusammen */
		s=max_width;
		do{
			Update_screen();
			for(t=0;t<nof_beams;t++){
				beam_max_width[t]-=Nr_combat_updates*7;
				if(beam_max_width[t]<2){
					beam_max_width[t]=2;
				}
				beam_COMOB[t]->Display_width = beam_max_width[t];
			}
			Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,pal);
			if(pal>0){
				pal-=Nr_combat_updates*2;
				if(pal<0){
					pal=0;
				}
			}
			s-=Nr_combat_updates*7; /* Beam zusammenfall Geschwindigkeit */
		}while(s>2);
		for(t=0;t<nof_beams;t++){
			beam_COMOB[t]->Display_width = 2;
		}

		/* Palette wieder reseten */
		Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,0);

		/* Noch kurze Zeit darstellen */
		update_n_times(8);

		/* Handle freigeben */
		MEM_Free_memory(beam_handle);
	}

}

void
Do_C0_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C0_Spell_12_handler
 *						 Blind all Monsters
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:54
 * LAST      : 03.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_12_COMBAT_FIELDS (5*6)

void
C0_Spell_12_handler(UNSHORT Strength)
{
	struct COMOB *beam_COMOB[C0_12_COMBAT_FIELDS];
	struct COMOB_behaviour *behave_flash;
	MEM_HANDLE beam_handle = NULL;
	SILONG x,y,z,s,t,pal;
	SILONG beamx[C0_12_COMBAT_FIELDS];
	SILONG beamz[C0_12_COMBAT_FIELDS];
	SILONG beaml[C0_12_COMBAT_FIELDS];
	SILONG beamt[C0_12_COMBAT_FIELDS];
	SILONG beamr[C0_12_COMBAT_FIELDS];
	SILONG beamb[C0_12_COMBAT_FIELDS];
	SILONG bspr[C0_12_COMBAT_FIELDS];
	SILONG beam_max_width[C0_12_COMBAT_FIELDS];
	SILONG nof_beams;
	SILONG left,right,top,bottom;
	SILONG max_width;
	struct Combat_participant *Victim_part;
	UNSHORT Temp_strength;
	SILONG i;

	/* Clear counter */
	Current_nr_target_parts = 0;

	/* "Do" spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_12);

	/* Get victim participant data */
	Victim_part = Current_target_parts[0];

	/* No -> Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Rest of effect */
//	}
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);

	/* SamenKorn fliegt zum Monster */
	show_samenkorn(Strength,0,0,z);

	/* Korn explodiert */
	Gen_Sparks(0,5*COMOB_DEC_FACTOR,COMBAT_3D_DEPTH/2,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_BLUE);

	nof_beams=0;
	for (i=0;i<Current_nr_target_parts;i++)
	{
		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* MonsterPosition */
		Get_3D_part_coordinates(Victim_part,&x,&y,&z);
		Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if(Temp_strength)
		{
			/* No -> Victim is monster ? */
//			if (Victim_part->Type == MONSTER_PART_TYPE)
//			{
				/* Yes -> Rest of effect */

//			}

			beamx[nof_beams]=x;
			beamz[nof_beams]=z;
			beaml[nof_beams]=left;
			beamt[nof_beams]=top;
			beamr[nof_beams]=right;
			beamb[nof_beams]=bottom;
			nof_beams++;

			/* Add temporary effect */
			Add_temporary_effect(Victim_part, BLINDED_TEMP_EFFECT, Strength, 10);

		}

	}

	/* Åberhaupt Monster betroffen */
	if(nof_beams>0){
		/* Load Beam graphics */
		beam_handle = Load_subfile(COMBAT_GFX, 40);
		if (!beam_handle){
			ERROR_ClearStack();
			return;
		}

		max_width=-1;

		/* Beams generieren */
		for(t=0;t<nof_beams;t++){

			/* Position Beam */
			x=beamx[t];
			z=beamz[t];
			left=beaml[t];
			top=beamt[t];
			right=beamr[t];
			bottom=beamb[t];

			/* Add beam_COMOB[t] */
			beam_COMOB[t] = Add_COMOB(100);

			/* Success ? */
			if (!beam_COMOB[t]){
				ERROR_ClearStack();
				return;
			}

			/* set coordinates */
			beam_COMOB[t]->X_3D = x;
			beam_COMOB[t]->Y_3D = bottom;
			beam_COMOB[t]->Z_3D = z;

			/* Set lifespan */
			beam_COMOB[t]->Lifespan = 0 ;

			/* Set display parameters */
			beam_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			beam_COMOB[t]->Special_handle = Transluminance_table_handle;

			beam_COMOB[t]->Hotspot_X_offset = 50;
			beam_COMOB[t]->Hotspot_Y_offset = 100;

			/* Set width to width of Monster */
			beam_max_width[t]=(right-left)*100/COMBAT_SQUARE_WIDTH;
			if(beam_max_width[t]>max_width){
				max_width=beam_max_width[t];
			}

			beam_COMOB[t]->Display_width = 2;
			beam_COMOB[t]->Display_height = 30000;

			/* Select random spark type */
			beam_COMOB[t]->Graphics_handle = beam_handle;

			/* Set number of animation frames */
			beam_COMOB[t]->Nr_frames = Get_nr_frames(beam_COMOB[t]->Graphics_handle);

			/* Select animation frame */
			beam_COMOB[t]->Frame = 0;

			/* sparkbeam lîschen */
			bspr[t]=0;

		}

		/* die Beams hochzoomen */
		s=2;
		pal=0;
		do{
			Update_screen();
			for(t=0;t<nof_beams;t++){
				beam_COMOB[t]->Display_width = s;
				if(beam_COMOB[t]->Display_width>beam_max_width[t]){
					beam_COMOB[t]->Display_width=beam_max_width[t];
				}
			}
			Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,pal);
			s+=Nr_combat_updates*28; /* Beam Geschwindigkeit */
			if(pal<100){
				pal+=Nr_combat_updates*3;
				if(pal>100){
					pal=100;
				}
			}
		}while(s<=max_width);
		for(t=0;t<nof_beams;t++){
			beam_COMOB[t]->Display_width = beam_max_width[t];
		}

		/* laufend Sparks generieren */
		s=0;
		do{
			Update_screen();

			/* FÅe jeden Beam Sparks generieren */
			for(t=0;t<nof_beams;t++){
				if(bspr[t]<0){
					if(Combat_display_detail_level==MAX_COMBAT_DISPLAY_DETAIL_LEVEL){
						x=beamx[t];
						z=beamz[t];
						left=beaml[t];
						right=beamr[t];
						Gen_Sparks(left+rndmm(10*COMOB_DEC_FACTOR,20*COMOB_DEC_FACTOR),0,z,3/*amount*/,100/*maxspeed*/,40/*life*/,30/*size*/,GEN_SPARK_TYP_BLUE);
		 				Gen_Sparks(right-rndmm(10*COMOB_DEC_FACTOR,20*COMOB_DEC_FACTOR),0,z,3/*amount*/,100/*maxspeed*/,40/*life*/,30/*size*/,GEN_SPARK_TYP_BLUE);
					}
					bspr[t]=5;
				}
				bspr[t]-=Nr_combat_updates;
			}

			s+=Nr_combat_updates; /* Beam Geschwindigkeit */
		}while(s<75);

		/* danach fÑllt Beam horizontal wieder in sich zusammen */
		s=max_width;
		do{
			Update_screen();
			for(t=0;t<nof_beams;t++){
				beam_max_width[t]-=Nr_combat_updates*7;
				if(beam_max_width[t]<2){
					beam_max_width[t]=2;
				}
				beam_COMOB[t]->Display_width = beam_max_width[t];
			}
			Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,pal);
			if(pal>0){
				pal-=Nr_combat_updates*2;
				if(pal<0){
					pal=0;
				}
			}
			s-=Nr_combat_updates*7; /* Beam zusammenfall Geschwindigkeit */
		}while(s>2);
		for(t=0;t<nof_beams;t++){
			beam_COMOB[t]->Display_width = 2;
		}

		/* Palette wieder reseten */
		Recolour_palette(1,191,C0_10_FLASHRED,C0_10_FLASHGREEN,C0_10_FLASHBLUE,0);

		/* Noch kurze Zeit darstellen */
		update_n_times(8);

		/* Handle freigeben */
		MEM_Free_memory(beam_handle);
	}

}

void
Do_C0_Spell_12(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C0_Spell_13_handler
 *						 Make Monster Sleep
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:54
 * LAST      : 03.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_13_MAXTIME 120

#define C0_13_MIN_FAIRY 24
#define C0_13_MAX_FAIRY 16

void
C0_Spell_13_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_13);
}

void
Do_C0_Spell_13(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB **halo_COMOB;
	struct COMOB_behaviour *fairy_behave;
	struct COMOB_behaviour *osc_behave;
	struct COMOB_behaviour *behave;
	struct COMOB *COMOB;
	struct COMOB *fairy_COMOB;
	SILONG x,y,z,fx,fy,fz,w,h,t,upd;
	SILONG time,nextfairy,nexthalo,col,faries;
	SILONG left,right,top,bottom;
	SILONG hw[3];

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	faries=calc_strength(Strength,C0_13_MIN_FAIRY,C0_13_MAX_FAIRY);

	/* MonsterPosition */
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

	/* SamenKorn fliegt zum Monster */
	show_samenkorn(Strength,x,top,z);

	/* Korn explodiert */
	Gen_Sparks(x,top,z,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_BLUE);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
			/* Yes -> Rest of effect */
//	}

		/* noch kurze Zeit warten wegen Sparks */
		update_n_times(16);

		if((fairy_COMOB=Gen_COMOB(x,top,z, 0, 100, Star_gfx_handles[0], GC_NORM))==NULL){
			ERROR_ClearStack();
			return;
		}

		/* Behaviour fÅr Feenstaub */
		fairy_behave = Add_COMOB_behaviour(fairy_COMOB, NULL, Fairy_II_handler);

		/* Grî·e Ñndern */
		if(!fairy_behave){
			Delete_COMOB(fairy_COMOB);
			ERROR_ClearStack();
			return;
		}

		/* Add oscillate behaviour for x */
		osc_behave = Add_COMOB_behaviour(fairy_COMOB, NULL, Oscillate_handler);

		if(!osc_behave){
			/* Comob lîschen */
 			Delete_COMOB(fairy_COMOB);
			/* No -> Clear error stack */
			ERROR_ClearStack();
			return;
		}

		/* Set oscillate behaviour data */
		osc_behave->Data.Oscillate_data.Type = OSCILLATE_X;
		osc_behave->Data.Oscillate_data.Period = 90; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
		osc_behave->Data.Oscillate_data.Amplitude = (right-left)/2;
		osc_behave->Data.Oscillate_data.Value = 0;

		/* darstellen */
		time=C0_13_MAXTIME;
		nextfairy=0;
		nexthalo=0;
		do{
			Update_screen();

			if(nextfairy<0){
				nextfairy+=faries;

				fx=rndmm(left,right);
				fy=rndmm(bottom,top);
				fz=z-5;

				/* Feenstaub auf Monster generieren */
				if(Test_COMOB_mask(Victim_part->Main_COMOB,fx,fy)){

					if((COMOB=Gen_COMOB(fx,fy,fz, 20, 100, Star_gfx_handles[0], GC_NORM))==NULL){
						ERROR_ClearStack();
						return;
					}

					/* Behaviour fÅr Feenstaub */
					behave = Add_COMOB_behaviour(COMOB, NULL, Fairy_II_handler);

					/* Grî·e Ñndern */
					if(!behave){
						Delete_COMOB(COMOB);
						ERROR_ClearStack();
						return;
					}

				}

			}

			if(nexthalo<0){
				nexthalo+=10;

 				if(time<=(C0_13_MAXTIME-25)&&time>=40){
					w=rndmm(-1,1);
					h=rndmm(-1,1);
					/* MonsterHalo wabert*/
					for(t=0;t<3;t++){
						halo_COMOB[t]->Display_width+=w;
						halo_COMOB[t]->Display_height+=h;
						/* maximale Grenze Breite */
						if(halo_COMOB[t]->Display_width>(hw[t]+10)){
							halo_COMOB[t]->Display_width=hw[t]+10;
						}
						if(halo_COMOB[t]->Display_width<(hw[t]-10)){
							halo_COMOB[t]->Display_width=hw[t]-10;
						}
						/* maximale Grenze Hîhe */
						if(halo_COMOB[t]->Display_height>(hw[t]+10)){
							halo_COMOB[t]->Display_height=hw[t]+10;
						}
						if(halo_COMOB[t]->Display_height<(hw[t]-10)){
							halo_COMOB[t]->Display_height=hw[t]-10;
						}
					}
				}

			}

			nextfairy-=Nr_combat_updates;
			nexthalo-=Nr_combat_updates;

			/* Zeit herunterzÑhlen */
			for(upd=0;upd<Nr_combat_updates;upd++){

				if(time==C0_13_MAXTIME){
					/* Leuchtshilouhette */
					halo_COMOB=Add_halo_to_COMOB_2(Victim_part->Main_COMOB, 3, 20, 0,HALO_CONNECT);
					/* MonsterHalo auf 0 setzen*/
					for(t=0;t<3;t++){
						hw[t]=halo_COMOB[t]->Display_width;
						halo_COMOB[t]->Display_width=Victim_part->Main_COMOB->Display_width;
						halo_COMOB[t]->Display_height=Victim_part->Main_COMOB->Display_height;
					}
				}
				else if(time>(C0_13_MAXTIME-25)&&time<C0_13_MAXTIME){
					/* MonsterHalo vergrî·ert sich langsam */
					for(t=0;t<3;t++){
						if(halo_COMOB[t]->Display_width<hw[t]){
							halo_COMOB[t]->Display_width++;
							halo_COMOB[t]->Display_height++;
						}
					}
				}
				else if(time==(C0_13_MAXTIME-25)){
					halo_COMOB[t]->Display_width=hw[t];
					halo_COMOB[t]->Display_height=hw[t];
				}
				else if(time<40){
					/* MonsterHalo verkleinert sich langsam */
					for(t=0;t<3;t++){
						if(halo_COMOB[t]->Display_width>Victim_part->Main_COMOB->Display_width){
							halo_COMOB[t]->Display_width--;
						}
						if(halo_COMOB[t]->Display_height>Victim_part->Main_COMOB->Display_height){
							halo_COMOB[t]->Display_height--;
						}
					}
				}

				time--;

			}


		}while(time>0);

		/* Halo wieder lîschen */
		halo_COMOB[0]->Lifespan=1;
		halo_COMOB[1]->Lifespan=1;
		halo_COMOB[2]->Lifespan=1;

		/* kurz anzeigen */
		update_n_times(4);

		/* Feenstaub COMOB lîschen */
		Delete_COMOB(fairy_COMOB);

		/* Halo lîschen */
		update_n_times(48);

		/* Let victim sleep */
		Set_condition(Victim_part->Char_handle, ASLEEP);
	}

	/* noch kurze Zeit darstellen */
	update_n_times(24);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_14_handler
 *						 Trap
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 15:17
 * LAST      : 02.06.95 15:17
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_14_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_14);
}

void
Do_C0_Spell_14(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *COMOB;
	MEM_HANDLE trap_handle = NULL;
	SILONG tx,tz;

	/* Is there already a trap on this square ? */
	if (!Is_trap(Tactical_X, Tactical_Y))
	{
		/* Load trap graphics */
		trap_handle = Load_subfile(COMBAT_GFX, 29);
		if (!trap_handle){
			ERROR_ClearStack();
			return;
		}

		/* No -> Do effect */
		Convert_tactical_to_3D_coordinates(Tactical_X,Tactical_Y,&tx,&tz);

		/* SamenKorn fliegt zum Monster */
		show_samenkorn(Strength,tx,5*COMOB_DEC_FACTOR,tz);

		/* Korn explodiert */
		Gen_Sparks(tx,5*COMOB_DEC_FACTOR,tz,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_ALL);

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
		Install_trap(Tactical_X, Tactical_Y, Handle_C0_Spell_14_trap,
		 Strength, COMOB);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_C0_Spell_14_trap
 * FUNCTION  : Trap handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 15:17
 * LAST      : 02.06.95 15:17
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

static SILONG ast_offsetxy[][2]=
{
	{0,1}, /* Frame 0 */
	{0,3}, /* Frame 1 */
	{7,3}, /* Frame 2 */
	{16,3}, /* Frame 3 */
	{25,3}, /* Frame 4 */
	{26,3}, /* Frame 5 */
	{27,3}, /* Frame 6 */
	{29,3}, /* Frame 7 */
	{29,3}, /* Frame 8 */
	{29,3}, /* Frame 9 */
	{27,3}, /* Frame 10 */
	{13,3}, /* Frame 11 */
	{-2,3}, /* Frame 12 */
	{-10,4}, /* Frame 13 */
	{-14,4}, /* Frame 14 */
	{-7,4}, /* Frame 15 */
	{0,4}, /* Frame 16 */
	{3,4}, /* Frame 17 */
	{0,2}, /* Frame 18 */
	{0,2}, /* Frame 19 */
	{0,0}, /* Frame 20 */
};

#define C0_14_ANIMFOLLOW 1
#define C0_14_MAXTRAILS 16

void
Handle_C0_Spell_14_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength, struct COMOB *Trap_COMOB)
{
	SILONG i;
	struct Gfx_header *Gfx;
	UNBYTE *Ptr;
	SILONG x,y,z,tx,ty,tz,u,t,fv,frame,tframe,frames;
	BOOLEAN leave;
	SILONG Tactical_X,Tactical_Y;
	SILONG p_wsize,p_hsize;
	SILONG anz_trails;
	UNSHORT max_ast_width,max_ast_height;
	UNSHORT max_victim_width,max_victim_height;
	struct COMOB *ast_COMOB;
	struct COMOB *source_COMOB;
	struct COMOB *scoms[C0_14_MAXTRAILS];
	MEM_HANDLE ast_handle = NULL;

	Tactical_X=Victim_part->Tactical_X;
	Tactical_Y=Victim_part->Tactical_Y;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}
	/* Load trap graphics */
	ast_handle = Load_subfile(COMBAT_GFX, 44);
	if (!ast_handle){
		ERROR_ClearStack();
		return;
	}

	/* No -> Do effect */
	Get_3D_part_coordinates(Victim_part,&tx,&ty,&tz);

	/* Ast erstellen */
	if((ast_COMOB=Gen_COMOB( tx,0,tz, 0, 120, ast_handle, GC_NORM))==NULL){
		ERROR_ClearStack();
		return;
	}
	ast_COMOB->Hotspot_X_offset=50;
	ast_COMOB->Hotspot_Y_offset=100;
	ast_COMOB->Nr_frames=0;
	ast_COMOB->Frame=0;

	/* maximale Breite / Hîhe des Asts herausfinden */
	Get_COMOB_max_size(ast_COMOB,&max_ast_width,&max_ast_height);

	/* maximale Breite / Hîhe des Monsters */
	Get_COMOB_max_size(Victim_part->Main_COMOB,&max_victim_width,&max_victim_height);

	/* Grî·e des Asts an Monster anpassen */
	ast_COMOB->Display_width = ((SILONG)max_victim_width)*80/((SILONG)max_ast_width);
	ast_COMOB->Display_height = ((SILONG)max_victim_height)*80/((SILONG)max_ast_height);

	/* Anzahl trails */
	anz_trails=3;

	/* Add Schweif COMOB */
	source_COMOB=ast_COMOB;
	for(t=0;t<anz_trails;t++){

		scoms[t] = Duplicate_COMOB(source_COMOB);

		/* Success ? */
		if (!scoms[t]){
			/* No -> Clear error stack */
			ERROR_ClearStack();
			/* Kein Trail */
			return;
		}

		/* Set Schweifstart display parameters */
		scoms[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		scoms[t]->Special_handle = Transluminance_table_handle;

 		source_COMOB=scoms[t];

	}
	frames=Get_nr_frames(ast_COMOB->Graphics_handle);

	/* Animation abspielen */
	frame=0;
	fv=0;
	leave=FALSE;
	do{

		Update_screen();

		tframe=frame;
		if(tframe>=frames){
			tframe=frames-1;
		}
		x = ((ast_offsetxy[tframe][0] *	(SILONG)ast_COMOB->Display_width) * COMOB_DEC_FACTOR) / (SILONG)max_ast_width;
		y = ((ast_offsetxy[tframe][1] *	(SILONG)ast_COMOB->Display_height) * COMOB_DEC_FACTOR) / (SILONG)max_ast_height;
		ast_COMOB->X_3D=tx+x;
		ast_COMOB->Y_3D=y-(5*COMOB_DEC_FACTOR);
		ast_COMOB->Z_3D=tz;
		ast_COMOB->Frame=tframe;

		for(t=0;t<anz_trails;t++){

			tframe=frame-((t+1)*C0_14_ANIMFOLLOW);
			if(tframe<0){
				tframe=0;
			}
			if(tframe>=frames){
				tframe=frames-1;
			}
			x = ((ast_offsetxy[tframe][0] * scoms[t]->Display_width) * COMOB_DEC_FACTOR) / (SILONG)max_ast_width;
			y = ((ast_offsetxy[tframe][1] *	scoms[t]->Display_height) * COMOB_DEC_FACTOR) / (SILONG)max_ast_height;
			scoms[t]->X_3D=tx+x;
			scoms[t]->Y_3D=y-(5*COMOB_DEC_FACTOR);
			scoms[t]->Z_3D=tz+(t+1);
			scoms[t]->Frame=tframe;

		}

		/* Frame updaten */
		for(t=0;t<Nr_combat_updates;t++){
			if(++fv>3){
				fv=0;
				frame++;
				if(frame==18){
					leave=TRUE;
				}
			}
		}

	}while(!leave);

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */
		}

		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));

	}

	/* Animation abspielen */
	leave=FALSE;
	do{

		Update_screen();

		tframe=frame;
		if(tframe>=frames){
			tframe=frames-1;
		}
		x = ((ast_offsetxy[tframe][0] *	(SILONG)ast_COMOB->Display_width) * COMOB_DEC_FACTOR) / max_ast_width;
		y = ((ast_offsetxy[tframe][1] *	(SILONG)ast_COMOB->Display_height) * COMOB_DEC_FACTOR) / max_ast_height;
		ast_COMOB->X_3D=tx+x;
		ast_COMOB->Y_3D=y;
		ast_COMOB->Z_3D=tz;
		ast_COMOB->Frame=tframe;

		for(t=0;t<anz_trails;t++){

			tframe=frame-((t+1)*C0_14_ANIMFOLLOW);
			if(tframe<0){
				tframe=0;
			}
			if(tframe>=frames){
				tframe=frames-1;
			}
			x = ((ast_offsetxy[tframe][0] * scoms[t]->Display_width) * COMOB_DEC_FACTOR) / max_ast_width;
			y = ((ast_offsetxy[tframe][1] *	scoms[t]->Display_height) * COMOB_DEC_FACTOR) / max_ast_height;
			scoms[t]->X_3D=tx+x;
			scoms[t]->Y_3D=y;
			scoms[t]->Z_3D=tz+(t+1);
			scoms[t]->Frame=tframe;

		}

		/* Frame updaten */
		for(t=0;t<Nr_combat_updates;t++){
			if(++fv>3){
				fv=0;
				frame++;
				if(frame==frames){
					leave=TRUE;
				}
			}
		}

	}while(!leave);

	/* Falle lîschen */
	Delete_COMOB(Trap_COMOB);

	/* rest der Animation darstellen */
	update_n_times(10);

	/* Ast lîschen */
	Delete_COMOB(ast_COMOB);

	/* trails lîschen */
	for(t=0;t<anz_trails;t++){
		Delete_COMOB(scoms[t]);
	}

	/* kurze Zeit darstellen */
	update_n_times(16);

	/* Remove trap */
	Remove_trap(Tactical_X, Tactical_Y);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_15_handler
 *				     Remove Trap
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:46
 * LAST      : 03.06.95 13:46
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_Spell_15_RANGE 1600

#define C0_Spell_15_MIN_STARS 8
#define C0_Spell_15_MAX_STARS 2

#define C0_Spell_15_MIN_SPARKS 6
#define C0_Spell_15_MAX_SPARKS 2


void
C0_Spell_15_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_15);
}

void
Do_C0_Spell_15(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB_behaviour *behave;
	struct COMOB *COMOB;
	SILONG time,nextstar,nextspark,stars,sparks;
	SILONG x,y,z;

	stars=calc_strength(Strength,C0_Spell_15_MIN_STARS,C0_Spell_15_MAX_STARS);
	sparks=calc_strength(Strength,C0_Spell_15_MIN_SPARKS,C0_Spell_15_MAX_SPARKS);

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
				if((COMOB=Gen_COMOB(x+rndmm(-C0_Spell_15_RANGE,C0_Spell_15_RANGE),0,z+rndmm(-C0_Spell_15_RANGE,C0_Spell_15_RANGE), 0, 10, Star_gfx_handles[0], GC_NORM))==NULL){
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
				if((COMOB=Gen_COMOB(x+rndmm(-C0_Spell_15_RANGE,C0_Spell_15_RANGE),5*COMOB_DEC_FACTOR,z+rndmm(-C0_Spell_15_RANGE,C0_Spell_15_RANGE), 0, 100, Spark_gfx_handles[rndmm(0,2)], GC_TRANS))==NULL){
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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_20_handler
 *						 Fungification
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:55
 * LAST      : 03.06.95 13:55
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C0_20_MAX_PILZE 400

static struct COMOB *pilz_COMOB[C0_20_MAX_PILZE];
static struct COMOB *pilzhalo_COMOB[C0_20_MAX_PILZE];
static UNSHORT pilz_nr[C0_20_MAX_PILZE];

void
C0_Spell_20_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_20);
}


void
Do_C0_Spell_20(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB_behaviour *behave;
	struct COMOB *COMOB;
	MEM_HANDLE pilz_handle = NULL;
	MEM_HANDLE halopilz_handle = NULL;
	SILONG left,right,top,bottom;
	SILONG x,y,z,s,t,u,v,time;
	SILONG pilznr,pilzgfxnr;
	SISHORT LP;
	UNSHORT Damage;
	UNSHORT max_victim_width,max_victim_height;
	SILONG nrpilz,nrpilzsize,sizechange;
	BOOLEAN leave;

	/* Anzahl Pilze */
	nrpilz=2;

	/* MonsterPosition */
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

	/* SamenKorn fliegt zum Monster */
	show_samenkorn(Strength,x,0,z);

	/* Korn explodiert */
	Gen_Sparks(x,0,z,100/*amount*/,rndmm(50,100)/*speed*/,rndmm(25,35)/*life*/,rndmm(30,40)/*size*/,GEN_SPARK_TYP_ORANGE);

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */

//	}

	/* maximale Breite / Hîhe des Monsters */
	Get_COMOB_source_size(Victim_part->Main_COMOB,&max_victim_width,&max_victim_height);

	/* Grî·e des Pilzes */
	nrpilzsize= ((SILONG)max_victim_width*(SILONG)max_victim_height)*100/45000;

	if(nrpilzsize<15)
		nrpilzsize=15;

	if(nrpilzsize>150)
		nrpilzsize=150;

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Calculate damage */
		Damage = max(((Strength * 50) / 100), 1);

		/* Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Monster wÑchst mit Pilzen zu */
			/* Position des Monsters */
			Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
			Get_3D_part_coordinates(Victim_part,&x,&y,&z);

			/* Monster mit Eiskristallen zukleistern */
			pilznr=0;
			y=bottom;
			for(;;){

				Update_screen();

				for(s=0;s<Nr_combat_updates;s++){

					for(u=0;u<2;u++){

						for(t=0;t<nrpilz;t++){

							/* ZufallsPosition rechts bis links*/
							x=rndmm(left,right);

							if(Test_COMOB_mask(Victim_part->Main_COMOB,x,y)){

								/* zufÑlliger Pilz */
								pilzgfxnr=rndmm(12,15);

								/* Pilz laden */
								pilz_handle = Load_subfile(COMBAT_GFX,pilzgfxnr);
								if (!pilz_handle){
									ERROR_ClearStack();
									return;
								}
								/* Pilz erstellen */
								if((pilz_COMOB[pilznr]=Gen_COMOB(x,y,z, 0, nrpilzsize, pilz_handle, GC_NORM))==NULL){
									ERROR_ClearStack();
									return;
								}
								/* Pilz Grafik nach beenden freigeben */
								pilz_COMOB[pilznr]->User_flags|=COMOB_FREE_GFX_ON_DELETE;
								/* Pilz vergrî·ert und verkleinert sich permanent */
								/* Halo mit Pilz verbinden */
								if(!(behave = Add_COMOB_behaviour(pilz_COMOB[pilznr], NULL, size_III_handler))){
									ERROR_ClearStack();
									return;
								}

								/* Pilze 'wabbeln' unterschiedlich schnell */
								sizechange=rndmm(200,300);

								/* Set Connect behaviour data */
								behave->Data.Just_data_w.Data[0] = sizechange;
								behave->Data.Just_data_w.Data[1] = sizechange;
								behave->Data.Just_data_w.Data[2] = nrpilzsize+10;
								behave->Data.Just_data_w.Data[3] = behave->Data.Just_data_w.Data[2];
								behave->Data.Just_data_w.Data[4] = sizechange;
								behave->Data.Just_data_w.Data[5] = sizechange;
								behave->Data.Just_data_w.Data[6] = nrpilzsize-10;
								behave->Data.Just_data_w.Data[7] = behave->Data.Just_data_w.Data[6];

								/* HaloPilz laden */
								if(pilzgfxnr==12||pilzgfxnr==14){
									pilzgfxnr=16;
								}
								else{
									pilzgfxnr=17;
								}
								/* Halo fÅr Pilz laden */
								halopilz_handle = Load_subfile(COMBAT_GFX, pilzgfxnr);
								if (!halopilz_handle){
									ERROR_ClearStack();
									return;
								}
								/* Halo um Pilz erstellen */
								if((pilzhalo_COMOB[pilznr]=Gen_COMOB(x,y,z, 0, nrpilzsize, halopilz_handle, GC_TRANS))==NULL){
									ERROR_ClearStack();
									return;
								}
								/* Halo Pilz Grafik nach beenden freigeben */
								pilzhalo_COMOB[pilznr]->User_flags|=COMOB_FREE_GFX_ON_DELETE;
								/* Halo mit Pilz verbinden */
								if(!(behave = Add_COMOB_behaviour(pilzhalo_COMOB[pilznr], NULL, size_III_handler))){
									ERROR_ClearStack();
									return;
								}
								/* Set Connect behaviour data */
								behave->Data.Just_data_w.Data[0] = sizechange;
								behave->Data.Just_data_w.Data[1] = sizechange;
								behave->Data.Just_data_w.Data[2] = nrpilzsize+10;
								behave->Data.Just_data_w.Data[3] = behave->Data.Just_data_w.Data[2];
								behave->Data.Just_data_w.Data[4] = sizechange;
								behave->Data.Just_data_w.Data[5] = sizechange;
								behave->Data.Just_data_w.Data[6] = nrpilzsize-10;
								behave->Data.Just_data_w.Data[7] = behave->Data.Just_data_w.Data[6];

								/* Halo mit Pilz verbinden */
								if(!(behave = Add_COMOB_behaviour(pilzhalo_COMOB[pilznr], pilz_COMOB[pilznr], connect_handler))){
									ERROR_ClearStack();
									return;
								}
								/* Set connect behaviour data */
								behave->Data.Just_data_w.Data[0] = 1+2+4;
								behave->Data.Just_data_w.Data[1] = 0;
								behave->Data.Just_data_w.Data[2] = 0;
								behave->Data.Just_data_w.Data[3] = 0;

								if(++pilznr>=C0_20_MAX_PILZE){
									goto leavelop1c020;
								}

							}

						}

						y+=((nrpilzsize*2)+(nrpilzsize/2));

						if(y>=top){
							goto leavelop1c020;
						}

					}

				}


			}

leavelop1c020:

			/* Yes -> Get LP of victim */
			LP = Get_LP(Victim_part->Char_handle);

			/* Can the victim take this damage ? */
			if (LP > Damage)
			{

				/* Pilze kurz darstellen */
				update_n_times(24);

				/* Yes -> Do damage */
				Do_combat_damage(Victim_part, Damage);

				/* Do survival effect */

				/* Pilze sprengen */

				for(t=0;;){

					Update_screen();

					for(u=0;u<3;u++){

						for(s=0;s<Nr_combat_updates;s++){

							/* Bewegung */
							pilz_COMOB[t]->dX_3D=rndmm(-150,150);
							pilz_COMOB[t]->dY_3D=rndmm(150,250);
							pilz_COMOB[t]->dZ_3D=rndmm(-150,150);
							pilz_COMOB[t]->Lifespan=rndmm(5,60);

							/* Add bounce behaviour */
							behave = Add_COMOB_behaviour(pilz_COMOB[t], NULL, Bounce_handler);

							if(!behave){
								/* Comob lîschen */
								Delete_COMOB(pilz_COMOB[t]);
								ERROR_ClearStack();
								return;
							}

							/* Set bounce behaviour data */
							behave->Data.Bounce_data.Gravity = rndmm(15,20);
							behave->Data.Bounce_data.Bounce = rndmm(50,60);
							behave->Data.Bounce_data.Air_friction = rndmm(3,6);

							if(++t>=pilznr){
								goto leavelop2c020;
							}

						}
					}

				}

leavelop2c020:

				/* kurze Zeit darstellen */
				update_n_times(64);

			}
			else
			{

				/* Do death effect */

				/* Pilze kurz darstellen */
				update_n_times(8);

				/* transparent darstellen */
				Victim_part->Main_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
				Victim_part->Main_COMOB->Special_handle = Transparency_table_handle;

				for(t=0;t<4;t++){
					/* Main COMOB duplizieren */
					COMOB=Duplicate_COMOB(Victim_part->Main_COMOB);
					COMOB->Lifespan=(t+1)*8;

					/* mit dem BlitzverknÅpfen*/
					if(!(behave = Add_COMOB_behaviour(COMOB, Victim_part->Main_COMOB, connect_handler))){
						ERROR_ClearStack();
						return;
					}

					/* Set connect behaviour data */
					behave->Data.Just_data_w.Data[0] = 1+2+4;
					behave->Data.Just_data_w.Data[1] = 0;
					behave->Data.Just_data_w.Data[2] = 0;
					behave->Data.Just_data_w.Data[3] = 0;
				}

				/* Monster wird transparenter */
				update_n_times(40);

				/* Main COMOB lîschen */
				Delete_COMOB(Victim_part->Main_COMOB);

				/* No -> Destroy monster */
				Destroy_participant(Victim_part);

				/* Pilze fallen zusammen */
				for(t=0;;){

					Update_screen();

					for(u=0;u<3;u++){

						for(s=0;s<Nr_combat_updates;s++){

							/* Bewegung */
							pilz_COMOB[t]->dX_3D=rndmm(-30,30);
							pilz_COMOB[t]->dY_3D=0;
							pilz_COMOB[t]->dZ_3D=rndmm(-30,30);
							pilz_COMOB[t]->Lifespan=rndmm(5,96);

							/* Add bounce behaviour */
							behave = Add_COMOB_behaviour(pilz_COMOB[t], NULL, Bounce_handler);

							if(!behave){
								/* Comob lîschen */
								Delete_COMOB(pilz_COMOB[t]);
								ERROR_ClearStack();
								return;
							}

							/* Set bounce behaviour data */
							behave->Data.Bounce_data.Gravity = rndmm(15,20);
							behave->Data.Bounce_data.Bounce = rndmm(50,60);
							behave->Data.Bounce_data.Air_friction = rndmm(3,6);

							if(++t>=pilznr){
								goto leavelop3c020;
							}

						}

					}

				}
leavelop3c020:

				/* kurze Zeit darstellen */
				update_n_times(64);

			}

			/* Noch Kurze Zeit warten bis alle Pilze weg sind */
			update_n_times(48);

		}
		else
		{
			/* No -> Do damage to victim */
			Do_combat_damage(Victim_part, Damage);
		}
	}

}










































#if FALSE

COMOB=Gen_COMOB(0/*x*/,0/*y*/,0/*z*/, 0/*Lifespan*/,100/*size*/,obj/*Graphics*/,GC_TRANS);


//	ast_COMOB->Frame=ast_COMOB->Nr_frames-1;

	/* 3D-Grî·e des Monsters */
	Get_part_rectangle(Victim_part,&m_left,&m_top,&m_right,&m_bottom);

	/* untere Seite des Monsters */
	if(m_bottom>0){
		m_bottom=0;
	}

	/* 3D-Grî·e der Pflanze */
	Get_COMOB_rectangle(ast_COMOB,&p_left,&p_top,&p_right,&p_bottom);

	/* Endgrî·e der Pflanze */
	p_wsize=(m_right-m_left)*100/(p_right-p_left);
	p_hsize=(m_top-m_bottom)*100/(p_top-p_bottom);

	/* Grî·e des Asts */
	ast_COMOB->Display_width=p_wsize;
	ast_COMOB->Display_height=p_hsize;






	/* Animverhalten fÅr Ast */
	behave_anim = Add_COMOB_behaviour(ast_COMOB, NULL, Animcontrol_II_handler);

	if(!behave_anim){
		/* Comob lîschen */
		Delete_COMOB(ast_COMOB);
		ERROR_ClearStack();
		return;
	}

	u=Get_nr_frames(ast_COMOB->Graphics_handle);
	behave_anim->Data.Animcontrol_data.Nr_frames=u;
	behave_anim->Data.Animcontrol_data.Nr_repeats=1;
	behave_anim->Data.Animcontrol_data.Duration=80;
	behave_anim->Data.Animcontrol_data.Counter=aktframe;

	/* Offset addieren fÅr jedes Animationsframe */
	behave = Add_COMOB_behaviour(ast_COMOB, NULL, Offset_handler);

	if(!behave){
		/* Comob lîschen */
		Delete_COMOB(ast_COMOB);
		ERROR_ClearStack();
		return;
	}

	behave->Data.Just_data.Data[0]=tx;
	behave->Data.Just_data.Data[1]=0;
	behave->Data.Just_data.Data[2]=tz;
	behave->Data.Just_data.Data[3]=79; /* MAximale Breite */
	behave->Data.Just_data.Data[4]=76; /* Maximale Hîhe */
	behave->Data.Just_data.Data[5]=(SILONG)&ast_offsetxy[0][0];

	/* Animverhalten fÅr Trails */
	for(t=0;t<anz_trails;t++){

		aktframe-=C0_14_ANIMFOLLOW;

		behave_anim = Add_COMOB_behaviour(scoms[t], NULL, Animcontrol_II_handler);

		if(!behave_anim){
			/* Comob lîschen */
			Delete_COMOB(ast_COMOB);
			ERROR_ClearStack();
			return;
		}

		u=Get_nr_frames(scoms[t]->Graphics_handle);
		behave_anim->Data.Animcontrol_data.Nr_frames=u;
		behave_anim->Data.Animcontrol_data.Nr_repeats=1;
		behave_anim->Data.Animcontrol_data.Duration=80;
		behave_anim->Data.Animcontrol_data.Counter=aktframe;

		/* Offset addieren fÅr jedes Animationsframe */
		behave = Add_COMOB_behaviour(scoms[t], NULL, Offset_handler);

		if(!behave){
			/* Comob lîschen */
			Delete_COMOB(ast_COMOB);
			ERROR_ClearStack();
			return;
		}

		behave->Data.Just_data.Data[0]=tx;
		behave->Data.Just_data.Data[1]=0;
		behave->Data.Just_data.Data[2]=tz;
		behave->Data.Just_data.Data[3]=79; /* MAximale Breite */
		behave->Data.Just_data.Data[4]=76; /* Maximale Hîhe */
		behave->Data.Just_data.Data[5]=(SILONG)&ast_offsetxy[0][0];

	}

	/* Animation darstellen */
	update_n_times(60);




				OPM_printf(global_screenport.screenopmptr,0,0,255," Frame %ld",frame);
				DSA_CopyMainOPMToScr(DSA_ALWAYS);
				DSA_DoubleBuffer();
				DSA_CopyMainOPMToScr(DSA_ALWAYS);
				DSA_DoubleBuffer();

				while(SYSTEM_GetBLEVStatusLong()!=BLEV_NOKEY);
				while(SYSTEM_GetBLEVStatusLong()==BLEV_NOKEY);



		OPM_FillBox(global_screenport.screenopmptr,0,110,100,20,0);

		OPM_printf(global_screenport.screenopmptr,0,110,255," astoffset %ld,%ld",ast_offsetxy[tframe][0],ast_offsetxy[tframe][1]);
		OPM_printf(global_screenport.screenopmptr,0,120,255," x,y %ld,%ld",x,y);

		DSA_CopyMainOPMToScr(DSA_ALWAYS);
		DSA_DoubleBuffer();
		DSA_CopyMainOPMToScr(DSA_ALWAYS);
		DSA_DoubleBuffer();

		while(SYSTEM_GetBLEVStatusLong()!=BLEV_NOKEY){}
		while(SYSTEM_GetBLEVStatusLong()==BLEV_NOKEY){}

		Nr_combat_updates=1;

//		ast_COMOB->Hotspot_X_offset=ast_offsetxy[tframe][0]*100/max_ast_width;
//		ast_COMOB->Hotspot_Y_offset=ast_offsetxy[tframe][1]*100/max_ast_height;

//			scoms[t]->Hotspot_X_offset=ast_offsetxy[tframe][0]*100/max_ast_width;
//			scoms[t]->Hotspot_Y_offset=ast_offsetxy[tframe][1]*100/max_ast_height;


			/* Nr an Pilze verteilen und zufÑllig austauschen */
			for(t=0;t<pilznr;t++){
				pilz_nr[t]=t;
			}
			for(s=0;s<300;s++){
				u=rndmm(0,pilznr-1);
				v=rndmm(0,pilznr-1);
				t=pilz_nr[u];
				pilz_nr[u]=pilz_nr[v];
				pilz_nr[v]=t;
			}

			/* WÑhrend dieser Zeitspanne Pilze schrumpfen */
			u=0;
			leave=FALSE;
			do{

				/* Screen updaten */
				Update_screen();

				for(s=0;s<Nr_combat_updates;s++){

					for(v=0;v<2;v++){

						/* Pilze schrumpfen */
						t=pilz_nr[u];

						/* Behaviour fÅr Grî·e */
						behave = Add_COMOB_behaviour(pilz_COMOB[t], NULL, size_handler);

						/* Grî·e Ñndern */
						if(!behave){
							/* restliche Comobs sofort lîschen */
							for(s=t;s<pilznr;s++){
								Delete_COMOB(pilz_COMOB[t]);
							}
							for(s=0;s<pilznr;s++){
								Delete_COMOB(pilzhalo_COMOB[t]);
							}
							ERROR_ClearStack();
							return;
						}

						s=rndmm(100,600);

						behave->Data.Just_data_w.Data[0]=-s; /* jeden Tick um 1% */
						behave->Data.Just_data_w.Data[1]=behave->Data.Just_data_w.Data[0];
						behave->Data.Just_data_w.Data[2]=2; /* bis 25 % */
						behave->Data.Just_data_w.Data[3]=2;

						/* Behaviour fÅr Grî·e */
						behave = Add_COMOB_behaviour(pilzhalo_COMOB[t], NULL, size_handler);

						/* Grî·e Ñndern */
						if(!behave){
							/* restliche Comobs sofort lîschen */
							for(s=t;s<pilznr;s++){
								Delete_COMOB(pilzhalo_COMOB[t]);
							}
							ERROR_ClearStack();
							return;
						}
						behave->Data.Just_data_w.Data[0]=-s; /* jeden Tick um 1% */
						behave->Data.Just_data_w.Data[1]=behave->Data.Just_data_w.Data[0];
						behave->Data.Just_data_w.Data[2]=2; /* bis 25 % */
						behave->Data.Just_data_w.Data[3]=2;

						u++;
						if(u>=pilznr){
							leave=TRUE;
							break;
						}

					}

				}

			}while(!leave);

				/* Dissolve Monster */
				dissolve_balls=Dissolve(Victim_part,4,4,&dis_COMOB[0],C0_Spell_20_ANZDISSOLVECOMOBS);

				/* Monster in Pilze aufteilen */
				for(t=0;t<dissolve_balls;t++){

					/* zufÑlliger Pilz */
					pilzgfxnr=rndmm(12,15);

					/* Pilz laden */
					pilz_handle = Load_subfile(COMBAT_GFX,pilzgfxnr);
					if (!pilz_handle){
						ERROR_ClearStack();
						return;
					}

					/* in PilzGrafik Ñndern */
					dis_COMOB[t]->Graphics_handle=pilz_handle;
					dis_COMOB[t]->User_flags|=COMOB_FREE_GFX_ON_DELETE;

					/* Normaler ZeichenModus */
					dis_COMOB[t]->Draw_mode=NORMAL_COMOB_DRAWMODE;

					/* Grî·e Ñndern */
					dis_COMOB[t]->Display_width=dis_COMOB[t]->Display_width*2/8;
					dis_COMOB[t]->Display_height=dis_COMOB[t]->Display_height*2/8;

					/* nach hinten versetzten */
					dis_COMOB[t]->Z_3D+=5;

				}

				/* kurze Zeit darstellen */
				update_n_times(16);

				/* Monster fÑllt zusammen */
				for(t=0;;){

					Update_screen();

					for(u=0;u<3;u++){

						for(s=0;s<Nr_combat_updates;s++){

							/* Bewegung */
							dis_COMOB[t]->dX_3D=rndmm(-30,30);
							dis_COMOB[t]->dY_3D=0;
							dis_COMOB[t]->dZ_3D=rndmm(-30,30);
							dis_COMOB[t]->Lifespan=rndmm(5,32);

							/* Add bounce behaviour */
							behave = Add_COMOB_behaviour(dis_COMOB[t], NULL, Bounce_handler);

							if(!behave){
								/* Comob lîschen */
								Delete_COMOB(dis_COMOB[t]);
								ERROR_ClearStack();
								return;
							}

							/* Set bounce behaviour data */
							behave->Data.Bounce_data.Gravity = rndmm(15,20);
							behave->Data.Bounce_data.Bounce = rndmm(50,60);
							behave->Data.Bounce_data.Air_friction = rndmm(3,6);

							if(++t>=dissolve_balls){
								goto leavelop4c020;
							}

						}

					}

				}
leavelop4c020:

	struct COMOB *dis_COMOB[C0_Spell_20_ANZDISSOLVECOMOBS];

#define C0_Spell_20_ANZDISSOLVECOMOBS 128

#endif

