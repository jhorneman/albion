/************
 * NAME     : COMMAG2.C
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
#include <MAGIC.H>
#include <COMBVAR.H>
#include <COMOBS.H>
#include <COMMAGIC.H>
#include <COMMSUB.H>
#include <ITMLOGIC.H>
#include <TACTICAL.H>
#include <XFTYPES.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_1_handler
 *						 Berserker
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:52
 * LAST      : 07.06.95 13:52
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C2_Spell_1_ANZHALOCOMOBS 3

void
C2_Spell_1_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_1);
}

void
Do_C2_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	MEM_HANDLE Recolour_table_handle[C2_Spell_1_ANZHALOCOMOBS];
	MEM_HANDLE lp_handle = NULL;
	UNBYTE *Recolour_table_ptr[C2_Spell_1_ANZHALOCOMOBS];
	SILONG halo_size[C2_Spell_1_ANZHALOCOMOBS];
	struct COMOB **halo_COMOB;
	SILONG s,t,time,size;

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
		/* No -> Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */


			/* zu jedem Halo COMOB eine TransparentFarbe generieren */
			s=20; /* AnfangsTransparenz */
			for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
				Recolour_table_handle[t] = MEM_Allocate_memory(256);
				Recolour_table_ptr[t] = MEM_Claim_pointer(Recolour_table_handle[t]);
				Calculate_recolouring_table(Recolour_table_ptr[t], 1, 191, 160,64,64, s);
				MEM_Free_pointer(Recolour_table_handle[t]);
				s+=30;
			}

			/* Monsterhalo erzeugen */
			halo_COMOB=Add_halo_to_COMOB_2(Victim_part->Main_COMOB, C2_Spell_1_ANZHALOCOMOBS, 20, 0,0);
			for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
				halo_COMOB[t]->Draw_mode = COLOURING_COMOB_DRAWMODE;
				halo_COMOB[t]->Special_handle = Recolour_table_handle[t];
				halo_COMOB[t]->X_3D=Victim_part->Main_COMOB->X_3D;
				halo_COMOB[t]->Y_3D=Victim_part->Main_COMOB->Y_3D;
				halo_COMOB[t]->Hotspot_Y_offset=Victim_part->Main_COMOB->Hotspot_Y_offset;
				halo_size[t]=100+((t+1)*10);
			}

			/* Halo mit Monster hochzoomen */
			size=100;
			time=0;
			do{
				Victim_part->Main_COMOB->Display_width=size;
				Victim_part->Main_COMOB->Display_height=size;
				for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
					halo_COMOB[t]->Display_width=halo_size[t];
 					halo_COMOB[t]->Display_height=halo_size[t];
				}
				Update_screen();
				for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
					halo_size[t]+=Nr_combat_updates;
				}
				size+=Nr_combat_updates;
				time+=Nr_combat_updates;
			}while(time<40);

			/* Monster kleinzoomen */
			s=size;
			time=0;
			do{
				size-=Nr_combat_updates*2;
				if(size<100){
					size=100;
				}
				Update_screen();
				time+=Nr_combat_updates;
				Victim_part->Main_COMOB->Display_width=size;
				Victim_part->Main_COMOB->Display_height=size;
			}while(time<20);

			/* Halo kleinzoomen */
			size=s;
			time=0;
			do{
				for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
					halo_size[t]-=Nr_combat_updates*2;
					if(halo_size[t]<100){
						halo_size[t]=100;
					}
				}
				Update_screen();
				time+=Nr_combat_updates;
				for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
					halo_COMOB[t]->Display_width=halo_size[t];
 					halo_COMOB[t]->Display_height=halo_size[t];
				}
			}while(time<40);

			/* noch kurze Zeit darstellen */
			update_n_times(4);

			/* HAlos lîschen */
			for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
				Delete_COMOB(halo_COMOB[t]);
			}

			/* noch kurze Zeit darstellen */
			update_n_times(4);

			/* Speicher fÅr TransparenzTabellen freigeben */
			for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
				MEM_Free_memory(Recolour_table_handle[t]);
			}

		}

		/* Add temporary effect */
		Add_temporary_effect(Victim_part, BERSERKER_TEMP_EFFECT, Strength,
		 10, NULL);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_2_handler
 *					   Expel Ghost Single
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:54
 * LAST      : 07.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_2_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_2);
}

void
Do_C2_Spell_2(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *beam_COMOB;
	struct COMOB *COMOB;
	struct COMOB_behaviour *behave;
	MEM_HANDLE beam_handle = NULL;
	MEM_HANDLE fire_handle = NULL;
	struct Character_data *Char;
	BOOLEAN Is_ghost = FALSE;
	SILONG max_width,time,yb;
	SILONG x,y,z,s,max_y,t;
	SILONG left,right,top,bottom;
	SILONG nextstar,nostar_y;
	SILONG nextflip,flip;

	/* Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
 		/* Yes -> Start of effect */
//	}

	/* Is the victim a ghost ? */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Victim_part->Char_handle);

	if (Char->Flags & GHOST)
		Is_ghost = TRUE;

	MEM_Free_pointer(Victim_part->Char_handle);

	if (Is_ghost)
	{
		/* Yes -> Handle magical defense */
		Strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if (Strength)
		{
			/* No -> Victim is monster ? */
			if (Victim_part->Type == MONSTER_PART_TYPE)
			{
				/* Yes -> Rest of effect */

				/* Load Beam graphics */
				beam_handle = Load_subfile(COMBAT_GFX, 40);
				if (!beam_handle){
					ERROR_ClearStack();
					return;
				}

				/* MonsterPosition */
				Get_3D_part_coordinates(Victim_part,&x,&y,&z);
				Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

				/* Beam COMOB erstellen */
				if((beam_COMOB=Gen_COMOB(x,(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR),z+5, 0, 100, beam_handle, GC_MAGIC))==NULL){
					ERROR_ClearStack();
					return;
				}
				beam_COMOB->Hotspot_Y_offset=100;
				beam_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

				/* Breite des Beams an Monster anpassen */
				max_width=(right-left)*100/COMBAT_SQUARE_WIDTH;

				/* Breite des Beams */
				beam_COMOB->Display_width=max_width;
				beam_COMOB->Display_height=50000;

				/* Maximale Hîhe */
				max_y=(COMBAT_3D_HEIGHT+100)*COMOB_DEC_FACTOR;
				nostar_y=(COMBAT_3D_HEIGHT-50)*COMOB_DEC_FACTOR;

				/* Beam fÑllt auf Monster */
				yb=max_y;
				Nr_combat_updates=0;
				for(;;){
					yb-=Nr_combat_updates*1000;
					if(yb<bottom)
						yb=bottom;
					beam_COMOB->Y_3D=yb;
					Update_screen();
					if(yb==bottom)
						break;
				}

				/* Monster transparent */
				Victim_part->Main_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;

				/* Screen updaten */
				Update_screen();

				/* Funken aus dem Boden schlagen */
				for(t=left;t<right;t+=rndmm(80,160)){
					if((COMOB=Gen_COMOB(t,bottom,z-5, 0, rndmm(40,60), Spark_gfx_handles[1], GC_FIRE))==NULL){
						ERROR_ClearStack();
						return;
					}
					COMOB->dX_3D=rndmm(-300,300);
					COMOB->dY_3D=rndmm(2,200);
					COMOB->dZ_3D=rndmm(-300,300);
					COMOB->Nr_frames=0;
					COMOB->Frame=0;

					/* Set anim behave */
					behave = Add_COMOB_behaviour(COMOB, NULL, Animcontrol_handler);
					if(!behave){
						/* Comob lîschen */
						Delete_COMOB(COMOB);
						ERROR_ClearStack();
						return;
					}
					behave->Data.Animcontrol_data.Nr_frames=Get_nr_frames(COMOB->Graphics_handle);
					behave->Data.Animcontrol_data.Nr_repeats=1;
					behave->Data.Animcontrol_data.Duration=rndmm(20,40);

					/* Set Gravity behaviour data */
					behave = Add_COMOB_behaviour(COMOB, NULL, Gravity_handler);
					if(!behave){
						/* Comob lîschen */
						Delete_COMOB(COMOB);
						ERROR_ClearStack();
						return;
					}
					behave->Data.Just_data.Data[0] = bottom;
					behave->Data.Just_data.Data[1] = 5;

				}

				/* Funken kurz darstellen */
				update_n_times(16);

				/* Beam mit Monster hochziehen und Breite verringern */
				yb=bottom;
				nextstar=0;
				for(;;){
					Update_screen();
					if(yb<nostar_y){
						for(t=0;t<3;t++){
							if((COMOB=Gen_COMOB(rndmm(left,right),bottom,z+rndmm(-100,100), 0, rndmm(20,60), Star_gfx_handles[0], GC_MAGIC))==NULL){
								ERROR_ClearStack();
								return;
							}
							COMOB->dY_3D=rndmm(300,400);
							COMOB->Hotspot_Y_offset=100;

							/* Behaviour fÅr Grî·e */
							behave = Add_COMOB_behaviour(COMOB, NULL, size_handler);

							/* Grî·e Ñndern */
							if(!behave){
								/* Comob lîschen */
								Delete_COMOB(COMOB);
								ERROR_ClearStack();
								return;
							}
							behave->Data.Just_data_w.Data[0]=-rndmm(100,150); /* jeden Tick um 1% */
							behave->Data.Just_data_w.Data[1]=-rndmm(100,150);
							behave->Data.Just_data_w.Data[2]=2; /* bis 25 % */
							behave->Data.Just_data_w.Data[3]=2;
						}
					}
					yb+=Nr_combat_updates*250;
					if(yb>max_y)
						yb=max_y;
					beam_COMOB->Y_3D=yb;
					beam_COMOB->Display_width-=Nr_combat_updates*3;
					if(beam_COMOB->Display_width<10){
						beam_COMOB->Display_width=10;
					}
					Victim_part->Main_COMOB->Y_3D=yb;
					Victim_part->Main_COMOB->Display_width-=Nr_combat_updates*3;
					if(Victim_part->Main_COMOB->Display_width<10){
						Victim_part->Main_COMOB->Display_width=10;
					}
					if(yb==max_y)
						break;
				}

				/* Beam lîschen */
				Delete_COMOB(Victim_part->Main_COMOB);
				Delete_COMOB(beam_COMOB);

				/* Bildschirm updaten */
				update_n_times(32);

			}

			/* Destroy victim */
			Destroy_participant(Victim_part);

		}

	}

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_3_handler
 * FUNCTION  : Spell handler.
 *						 Expel Ghost row
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:55
 * LAST      : 07.06.95 13:55
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_3_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_2);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_4_handler
 * FUNCTION  : Spell handler.
 *						 Expel Ghost all
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:55
 * LAST      : 07.06.95 13:55
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_4_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_2);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_5_handler
 *						 Long range Combat Spell on a single Monster
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:37
 * LAST      : 06.06.95 13:37
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_5_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_5);
}

void
Do_C2_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{

	Do_C3_Spell_1(Victim_part,Tactical_X,Tactical_Y,Strength);

#if FALSE
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
#endif

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_6_handler
 *						 Magical Defence
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:59
 * LAST      : 07.06.95 13:59
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_6_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_6);
}

void
Do_C2_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct Character_data *Char;
	UNSHORT Added;

	/* Increase magical defense */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Victim_part->Char_handle);

	Added = max(((Strength * 50) / 100), 1);
	Char->xProtection = min((Char->xProtection + Added), 100);

	MEM_Free_pointer(Victim_part->Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_8_handler
 *						 Scare one
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:38
 * LAST      : 06.06.95 13:38
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C2_Spell_8_ANZHALOCOMOBS 3

void
C2_Spell_8_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_8);
}

void
Do_C2_Spell_8(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Shilhouetten erzeugen die am Monster skaliert werden */
	MEM_HANDLE Recolour_table_handle[C2_Spell_8_ANZHALOCOMOBS];
	MEM_HANDLE lp_handle = NULL;
	UNBYTE *Recolour_table_ptr[C2_Spell_8_ANZHALOCOMOBS];
	SILONG halo_size[C2_Spell_1_ANZHALOCOMOBS];
	MEM_HANDLE scare_handle = NULL;
	struct COMOB *scare_COMOB;
	struct COMOB **halo_COMOB;
	SILONG x,y,z;
	SILONG left,right,top,bottom;
	SILONG mleft,mright,mtop,mbottom;
	SILONG tleft,tright,ttop,tbottom;
	SILONG zoommode,time,ypos,max_width,max_height;
	SILONG xscale,yscale,xadd,yadd,s,t;
	BOOLEAN bool;

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
		/* No -> Victim is monster ? */
//	if (Victim_part->Type == MONSTER_PART_TYPE)
//	{
			/* Yes -> Rest of effect */
//	}

		/* Load Beam graphics */
		scare_handle = Load_subfile(COMBAT_GFX, 28);
		if (!scare_handle){
			ERROR_ClearStack();
			return;
		}

		/* zu jedem Halo COMOB eine TransparentFarbe generieren */
		s=20; /* AnfangsTransparenz */
		for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
			Recolour_table_handle[t] = MEM_Allocate_memory(256);
			Recolour_table_ptr[t] = MEM_Claim_pointer(Recolour_table_handle[t]);
			Calculate_recolouring_table(Recolour_table_ptr[t], 1, 191, 96,100,96, s);
			MEM_Free_pointer(Recolour_table_handle[t]);
			s+=30;
		}

		/* MonsterPosition */
		Get_3D_part_coordinates(Victim_part,&x,&y,&z);
		Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

		/* Fratze erstellen */
		if((scare_COMOB=Gen_COMOB(x,top,z-2, 0, 100, scare_handle, GC_MAGIC))==NULL){
			ERROR_ClearStack();
			return;
		}
		scare_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
		scare_COMOB->Hotspot_Y_offset=50;

		/* Grî·e der Fratze */
		Get_COMOB_rectangle(scare_COMOB,&mleft,&mtop,&mright,&mbottom);

		/* Fratzte den Ausmessung her an Monster anpassen */
		max_width=(right-left)*80/(mright-mleft);
		max_height=(top-bottom)*80/(mtop-mbottom);

		/* Monsterhalo erzeugen */
		halo_COMOB=Add_halo_to_COMOB_2(Victim_part->Main_COMOB, C2_Spell_8_ANZHALOCOMOBS, 20, 0,0);
		for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
			halo_COMOB[t]->Draw_mode = SILHOUETTE_COMOB_DRAWMODE;
			halo_COMOB[t]->Colour = Find_closest_colour(t*50, t*50, t*50);
//			halo_COMOB[t]->Special_handle = Recolour_table_handle[t];
//			halo_COMOB[t]->Draw_mode = COLOURING_COMOB_DRAWMODE;
//			halo_COMOB[t]->Special_handle = Recolour_table_handle[t];
//			halo_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
//			halo_COMOB[t]->X_3D=Victim_part->Main_COMOB->X_3D;
//			halo_COMOB[t]->Y_3D=Victim_part->Main_COMOB->Y_3D;
//			halo_COMOB[t]->Hotspot_Y_offset=Victim_part->Main_COMOB->Hotspot_Y_offset;
			halo_COMOB[t]->Display_width=100;
			halo_COMOB[t]->Display_height=100;
			halo_size[t]=100+((t+1)*5);
		}

		/* Fratze durch x und y Scaling skalieren */
		/* Halo hochzoomen */
		xscale=max_width-(max_width/3);
		yscale=xscale;
		s=(max_width/64);
		xadd=s;
		yadd=-s;
		time=0;
		do{
			for(t=0;t<Nr_combat_updates;t++){
				if(xscale<=max_width-(max_width/2)){
					xadd=s;
				}
				else if(xscale>=max_width-(max_width/4)){
					xadd=-s;
				}
				if(yscale<=max_width-(max_width/2)){
					yadd=s;
				}
				else if(yscale>=max_width-(max_width/4)){
					yadd=-s;
				}
				xscale+=xadd;
				yscale+=yadd;
			}
			scare_COMOB->Display_width=xscale;
			scare_COMOB->Display_height=yscale;
			Update_screen();
			for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
				halo_COMOB[t]->Display_width+=Nr_combat_updates;
				if(halo_COMOB[t]->Display_width>halo_size[t]){
					halo_COMOB[t]->Display_width=halo_size[t];
				}
				halo_COMOB[t]->Display_height+=Nr_combat_updates;
				if(halo_COMOB[t]->Display_height>halo_size[t]){
					halo_COMOB[t]->Display_height=halo_size[t];
				}
			}
			time+=Nr_combat_updates;
		}while(time<60);

		/* Grî·e der Fratze */
		Get_COMOB_rectangle(scare_COMOB,&tleft,&ttop,&tright,&tbottom);

		/* Fratze verbiegen */
		scare_COMOB->Y_3D=ttop;
		scare_COMOB->Hotspot_Y_offset=0;
		zoommode=0;
		for(;;){
			Update_screen();
			if(zoommode==0){
				scare_COMOB->Display_height+=Nr_combat_updates*8;
				Get_COMOB_rectangle(scare_COMOB,&tleft,&ttop,&tright,&tbottom);
				if(tbottom<=(((top-bottom)/2)+bottom)){
					zoommode=1;
				}
//				for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
//					halo_COMOB[t]->Display_width=halo_size[t];
//	 				halo_COMOB[t]->Display_height=halo_size[t];
//				}
//				for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
//					halo_size[t]+=Nr_combat_updates;
// 				}
			}
			else if(zoommode==1){
				Get_COMOB_rectangle(scare_COMOB,&tleft,&ttop,&tright,&tbottom);
				scare_COMOB->Y_3D=tbottom;
				scare_COMOB->Hotspot_Y_offset=100;
				scare_COMOB->Display_height-=Nr_combat_updates*8;
				if(scare_COMOB->Display_height<=scare_COMOB->Display_width){
					zoommode=2;
					scare_COMOB->Display_height=scare_COMOB->Display_width;
				}
			}
			else if(zoommode==2){
				Get_COMOB_rectangle(scare_COMOB,&tleft,&ttop,&tright,&tbottom);
				scare_COMOB->Y_3D=((ttop-tbottom)/2)+tbottom;
				scare_COMOB->Hotspot_Y_offset=50;
				scare_COMOB->Display_width-=Nr_combat_updates*8;
				scare_COMOB->Display_height-=Nr_combat_updates*8;
				if(scare_COMOB->Display_height<15){
					break;
				}
			}
		}

		/* Fratze lîschen */
		Delete_COMOB(scare_COMOB);

		/* Halo kleinzoomen */
		for(;;){
			Update_screen();
			for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
				halo_size[t]-=Nr_combat_updates*2;
				if(halo_size[t]<100){
					halo_size[t]=100;
				}
			}
			bool=TRUE;
			for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
				halo_COMOB[t]->Display_width=halo_size[t];
 				halo_COMOB[t]->Display_height=halo_size[t];
				if(halo_size[t]!=100){
					bool=FALSE;
				}
			}
			if(bool){
				break;
			}
		}

		/* Halo noch kurz darstellen */
		update_n_times(8);

		/* Halos lîschen */
		for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
			Delete_COMOB(halo_COMOB[t]);
		}

		/* Speicher fÅr TransparenzTabellen freigeben */
		for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
			MEM_Free_memory(Recolour_table_handle[t]);
		}

		/* Scare victim */
		Set_condition(Victim_part->Char_handle, PANICKED);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_9_handler
 *						 Scare row
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:38
 * LAST      : 06.06.95 13:38
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_9_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_8);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_10_handler
 *						 Scare all
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:38
 * LAST      : 06.06.95 13:38
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_8);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_11_handler
 *					   Strength Sucker
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 14:03
 * LAST      : 07.06.95 14:03
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C2_Spell_11_ANZHALOCOMOBS 3
#define C2_Spell_11_LP_MOVESPEED 250

#define C2_Spell_11_MIN_BALLS 16
#define C2_Spell_11_MAX_BALLS 32

void
C2_Spell_11_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_11);
}

void
Do_C2_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	MEM_HANDLE Recolour_table_handle[C2_Spell_11_ANZHALOCOMOBS];
	MEM_HANDLE lp_handle = NULL;
	UNBYTE *Recolour_table_ptr[C2_Spell_11_ANZHALOCOMOBS];
	struct COMOB *halo_COMOB[C2_Spell_11_ANZHALOCOMOBS];
	struct COMOB *lp_COMOB[C2_Spell_11_MAX_BALLS];
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_x;
	struct COMOB_behaviour *behave_y;
	SILONG delta_x[C2_Spell_11_MAX_BALLS];
	SILONG delta_y[C2_Spell_11_MAX_BALLS];
	SILONG delta_z[C2_Spell_11_MAX_BALLS];
	SILONG delta_steps;
	UNSHORT Attribute;
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
		balls=calc_strength(Strength,C2_Spell_11_MIN_BALLS,C2_Spell_11_MAX_BALLS);

		/* Load LP graphics */
		lp_handle = Load_subfile(COMBAT_GFX, 36);
		if (!lp_handle){
			ERROR_ClearStack();
			return;
		}

		/* No -> Calculate "damage" */
		Damage = max(((Strength * 50) / 100), 1);

		/* Get victim's strength attribute */
		Attribute = Get_attribute(Victim_part->Char_handle, STRENGTH);

		/* Is the victim's strength attribute high enough ? */
		Damage = min(Damage, Attribute);

		/* Victim is monster ? */
//		if (Victim_part->Type == MONSTER_PART_TYPE)
//		{
			/* Yes -> Rest of effect */
//		}

		/* zu jedem Halo COMOB eine TransparentFarbe generieren */
		s=20; /* AnfangsTransparenz */
		for(t=0;t<C2_Spell_11_ANZHALOCOMOBS;t++){
			Recolour_table_handle[t] = MEM_Allocate_memory(256);
			Recolour_table_ptr[t] = MEM_Claim_pointer(Recolour_table_handle[t]);
			Calculate_recolouring_table(Recolour_table_ptr[t], 1, 191, 128,128,255, s);
			MEM_Free_pointer(Recolour_table_handle[t]);
			s+=30;
		}

		/* Halo Comobs */
		s=30;
		for(t=0;t<C2_Spell_11_ANZHALOCOMOBS;t++){

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
			behave->Data.Just_data.Data[0] = CONNECT_POSITION;
			behave->Data.Just_data.Data[1] = halo_COMOB[t]->X_3D-Victim_part->Main_COMOB->X_3D;
			behave->Data.Just_data.Data[2] = halo_COMOB[t]->Y_3D-Victim_part->Main_COMOB->Y_3D; /* verschieben, so das Halo von der Mitte des Objects aus berechnet wird */
			behave->Data.Just_data.Data[3] = halo_COMOB[t]->Z_3D-Victim_part->Main_COMOB->Z_3D;

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
			if((s=calc_delta_xyz(sx,sy,sz,dx,dy,dz,&delta_x[t],&delta_y[t],&delta_z[t],C2_Spell_11_LP_MOVESPEED))>delta_steps){
				delta_steps=s;
			}

			/* Set lifespan */
			lp_COMOB[t]->Lifespan = 0 ;

			/* Set display parameters */
//			lp_COMOB[t]->Draw_mode = NORMAL_COMOB_DRAWMODE;
			lp_COMOB[t]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;

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
		for(t=0;t<C2_Spell_11_ANZHALOCOMOBS;t++){
			Delete_COMOB(halo_COMOB[t]);
		}

 		/* Remove strength from victim */
		Set_attribute(Victim_part->Char_handle, STRENGTH, Attribute - Damage);

		/* Give strength to caster */
		Attribute =
		 Get_attribute(Current_use_magic_data.Casting_participant->Char_handle,
		 STRENGTH);
		Set_attribute(Current_use_magic_data.Casting_participant->Char_handle,
		 STRENGTH, Attribute + Damage);

		/* Speicher fÅr TransparenzTabellen freigeben */
		for(t=0;t<C2_Spell_11_ANZHALOCOMOBS;t++){
			MEM_Free_memory(Recolour_table_handle[t]);
		}

		/* LP Handle */
		MEM_Free_memory(lp_handle);

	}
}








#if FALSE
				time=0;
				flip=1;
				nextflip=2;
				do{
					Update_screen();
					for(t=0;t<Nr_combat_updates;t++){
						if(nextflip<0){
							nextflip=2;
							if(flip){
								/* Monster transparent */
								Victim_part->Main_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
							}
							else{
								/* Monster normal */
								Victim_part->Main_COMOB->Draw_mode = NORMAL_COMOB_DRAWMODE;
							}
							flip=1-flip;
						}
						nextflip--;
					}
					time+=Nr_combat_updates;
				}while(time<20);
#endif
