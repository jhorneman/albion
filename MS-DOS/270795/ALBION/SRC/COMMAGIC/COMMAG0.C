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
	struct COMOB *korn_COMOB;
	struct COMOB *vp_COMOB;
	struct COMOB *hp_COMOB;
	SILONG sx,sy,sz;
	SILONG zx,zy,zz;
	SILONG mx,my,mz;
	SILONG m_left,m_right,m_top,m_bottom;
	SILONG p_left,p_right,p_top,p_bottom;
	SILONG p_wsize,p_hsize;
	SILONG time,steps,sparks;
	MEM_HANDLE korn_handle = NULL;
	MEM_HANDLE vp_handle = NULL;
	MEM_HANDLE hp_handle = NULL;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	sparks=calc_strength(Strength,C0_Spell_1_MIN_SPARKS,C0_Spell_1_MAX_SPARKS);

	/* Samenkorn laden */
	korn_handle = Load_subfile(COMBAT_GFX, 18);
	if (!korn_handle){
		ERROR_ClearStack();
		return;
	}

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

	/* Position des Zaubernden */
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&sx,&sy,&sz);

	/* Position des Monsters */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* Samenkorn fliegt zum Monster */
	steps=calc_delta_xyz(sx,sy,sz,zx,0,zz,&mx,&my,&mz,3*COMOB_DEC_FACTOR);

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
	Add_halo_to_COMOB_2(korn_COMOB,3,40,steps,HALO_NULL);

	/* Samenkorn bewegen */
	update_n_times(steps);

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
		/* Pflanze erscheint und w„chst am Monster hoch */

		/* Pflanze vorne erstellen */
		if((vp_COMOB=Gen_COMOB(sx,sy,sz-5, 0, 100, vp_handle, GC_NORM))==NULL){
			ERROR_ClearStack();
			return;
		}
		vp_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
		vp_COMOB->Hotspot_Y_offset=100;

		/* Pflanze hinten erstellen */
		if((hp_COMOB=Gen_COMOB(sx,sy,sz+5, 0, 100, hp_handle, GC_NORM))==NULL){
			ERROR_ClearStack();
			return;
		}
		hp_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
		hp_COMOB->Hotspot_Y_offset=100;

		/* 3D-Gr”áe des Monsters */
		Get_part_rectangle(Victim_part,&m_left,&m_top,&m_right,&m_bottom);

		/* untere Seite des Monsters */
		if(m_bottom>0){
			m_bottom=0;
		}

		/* 3D-Gr”áe der Pflanze */
		Get_COMOB_rectangle(vp_COMOB,&p_left,&p_top,&p_right,&p_bottom);

		/* Endgr”áe der Pflanze */
		p_wsize=(m_right-m_left)*100/(p_right-p_left);
		p_hsize=(m_top-m_bottom)*100/(p_top-p_bottom);

		/* Position der Pflanze */
		vp_COMOB->X_3D=m_left+(m_right-m_left)/2;
		vp_COMOB->Y_3D=m_bottom;
		vp_COMOB->Z_3D=zz-5;
		hp_COMOB->X_3D=vp_COMOB->X_3D;
		hp_COMOB->Y_3D=vp_COMOB->Y_3D;
		hp_COMOB->Z_3D=zz+5;

		/* Korn explodiert */
		Gen_Sparks(vp_COMOB->X_3D,5*COMOB_DEC_FACTOR,vp_COMOB->Z_3D,200/*amount*/,rndmm(150,200)/*speed*/,rndmm(40,60)/*life*/,rndmm(50,75)/*size*/,GEN_SPARK_TYP_ALL);

		/* Pflanze auf Monster skalieren */
		vp_COMOB->Display_width=2;
		vp_COMOB->Display_height=2;

		/* Daten von vorderem COMOB */
		hp_COMOB->Display_width=vp_COMOB->Display_width;
		hp_COMOB->Display_height=vp_COMOB->Display_height;

		/* nach Explosion kurz warten */
		update_n_times(4);

		/* Pflanze w„chst am Monster empor */
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
			/* H”he */
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

	/* Samenkorn bewegen */
	update_n_times(20);

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
	struct COMOB *COMOB;
	MEM_HANDLE gift_handle = NULL;
	SILONG left,right,top,bottom;
	SILONG t,x,y,z;
	SILONG time,stars;




	Do_C0_Spell_4(Victim_part, Tactical_X,Tactical_Y,Strength);
	return;




	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
		/* Yes -> Start of effect */
//	}

	/* Anzahl Sterne */
	stars=calc_strength(Strength,C0_Spell_3_MIN_STARS,C0_Spell_3_MAX_STARS);

	/* Handle fr Object gift */
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

		/* Sterne blitzen auf */

		/* Aufblitzem */
		time=120;
		do{
			Update_screen();

			/* n„chster Stern blitzt auf */
			for(t=0;t<stars;t++){

				/* OrginalKoordinaten des Monsters */
				Get_3D_part_coordinates(Victim_part,&x,&y,&z);
				Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

				/* ZufallsPosition auf Object */
				x=rndmm(left,right);
				y=rndmm(bottom,top);

				/* Position geh”rt zur Maske */
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

			/* Shapes bewegen sich spiralf”rmig nach auáen */
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
					/* Comob l”schen */
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
					/* Comob l”schen */
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

			/* Shapes wieder l”schen */
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
				/* Comob l”schen */
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
				/* Comob l”schen */
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
				/* Comob l”schen */
				Delete_COMOB(fairy0_COMOB);
				Delete_COMOB(fairy1_COMOB);
				Delete_COMOB(fairy2_COMOB);
				ERROR_ClearStack();
				return;
			}

			/* noch kurze Zeit darstellen */
			update_n_times(96);

			/* Fairy_COMOB l”schen */
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

void
C0_Spell_5_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_5);
}

void
Do_C0_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

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

		/* Show LP of victim */
		Victim_part->Flags |= PART_SHOW_LP;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_6_handler
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
	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

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

		/* Add temporary effect */
		Add_temporary_effect(Victim_part, FROZEN_TEMP_EFFECT, Strength, 10);
	}
}

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
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_6);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_8_handler
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
C0_Spell_8_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_6);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_10_handler
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
C0_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_10);
}

void
Do_C0_Spell_10(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

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

		/* Add temporary effect */
		Add_temporary_effect(Victim_part, BLINDED_TEMP_EFFECT, Strength, 10);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_11_handler
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

void
C0_Spell_11_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_10);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_12_handler
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

void
C0_Spell_12_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_10);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_13_handler
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
	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

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

		/* Let victim sleep */
		Set_condition(Victim_part->Char_handle, ASLEEP);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_14_handler
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
	/* Is there already a trap on this square ? */
	if (!Is_trap(Tactical_X, Tactical_Y))
	{
		/* No -> Do effect */

		/* Set trap */
		Install_trap(Tactical_X, Tactical_Y, Handle_C0_Spell_14_trap,
		 Strength, NULL);
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

void
Handle_C0_Spell_14_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength, struct COMOB *Trap_COMOB)
{
	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_15_handler
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
	/* Start of effect */

	/* Is there a trap on this square ? */
	if (Is_trap(Tactical_X, Tactical_Y))
	{
		/* Yes -> Rest of effect */

		/* Remove trap */
		Remove_trap(Tactical_X, Tactical_Y);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_20_handler
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
	SISHORT LP;
	UNSHORT Damage;

	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

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
			/* Yes -> Get LP of victim */
			LP = Get_LP(Victim_part->Char_handle);

			/* Can the victim take this damage ? */
			if (LP > Damage)
			{
				/* Yes -> Do damage */
				Do_combat_damage(Victim_part, Damage);

				/* Do survival effect */

			}
			else
			{
				/* No -> Destroy monster */
				Destroy_participant(Victim_part);

				/* Do death effect */
			}
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
#endif

