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
#include <MUSIC.H>
#include <COMBVAR.H>
#include <COMOBS.H>
#include <COMMAGIC.H>
#include <COMMSUB.H>
#include <ITMLOGIC.H>
#include <TACTICAL.H>
#include <XFTYPES.H>

#define C2_SPELL_5_DAMAGE		(16)

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
	struct COMOB_behaviour *behave;

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

			/* HAlos l”schen */
			for(t=0;t<C2_Spell_1_ANZHALOCOMOBS;t++){
				Delete_COMOB(halo_COMOB[t]);
			}

			/* noch kurze Zeit darstellen */
			update_n_times(4);

			/* Speicher fr TransparenzTabellen freigeben */
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
	SILONG max_width,time,yb;
	SILONG x,y,z,s,max_y,t;
	SILONG left,right,top,bottom;
	SILONG nextstar,nostar_y;
	SILONG nextflip,flip;
	UNSHORT Char_bits;

	/* Load Beam graphics */
	beam_handle = Load_subfile(COMBAT_GFX, 40);
	if (!beam_handle){
		ERROR_PopError();
		return;
	}

	/* MonsterPosition */
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

	/* Beam COMOB erstellen */
	if((beam_COMOB=Gen_COMOB(x,(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR),z+5, 0, 100, beam_handle, GC_MAGIC))==NULL){
		ERROR_PopError();
		return;
	}
	beam_COMOB->Hotspot_Y_offset=100;
	beam_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

	/* Breite des Beams an Monster anpassen */
	max_width=(right-left)*100/COMBAT_SQUARE_WIDTH;

	/* Breite des Beams */
	beam_COMOB->Display_width=max_width;
	beam_COMOB->Display_height=50000;

	/* Maximale H”he */
	max_y=(COMBAT_3D_HEIGHT+100)*COMOB_DEC_FACTOR;
	nostar_y=(COMBAT_3D_HEIGHT-50)*COMOB_DEC_FACTOR;

	/* Beam f„llt auf Monster */
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

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Get victim character bits */
	Char_bits = Get_character_type(Victim_part->Char_handle);

	/* Spell not deflected and is a ghost ? */
	if (Strength && (Char_bits & GHOST))
	{
		/* Yes -> Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */
			/* Monster transparent */
			Victim_part->Main_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;

			/* Screen updaten */
			Update_screen();

			/* Funken aus dem Boden schlagen */
			for(t=left;t<right;t+=rndmm(80,160)){
				if((COMOB=Gen_COMOB(t,bottom,z-5, 0, rndmm(40,60), Spark_gfx_handles[1], GC_FIRE))==NULL){
					ERROR_PopError();
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
					/* Comob l”schen */
					Delete_COMOB(COMOB);
					ERROR_PopError();
					return;
				}
				behave->Data.Animcontrol_data.Nr_frames=Get_nr_frames(COMOB->Graphics_handle);
				behave->Data.Animcontrol_data.Nr_repeats=1;
				behave->Data.Animcontrol_data.Duration=rndmm(20,40);

				/* Set Gravity behaviour data */
				behave = Add_COMOB_behaviour(COMOB, NULL, Gravity_handler);
				if(!behave){
					/* Comob l”schen */
					Delete_COMOB(COMOB);
					ERROR_PopError();
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
							ERROR_PopError();
							return;
						}
						COMOB->dY_3D=rndmm(300,400);
						COMOB->Hotspot_Y_offset=100;

						/* Behaviour fr Gr”áe */
						behave = Add_COMOB_behaviour(COMOB, NULL, size_handler);

						/* Gr”áe „ndern */
						if(!behave){
							/* Comob l”schen */
							Delete_COMOB(COMOB);
							ERROR_PopError();
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
		}

		/* Destroy victim */
		Destroy_participant(Victim_part);
	}

	/* Beam l”schen */
	Delete_COMOB(Victim_part->Main_COMOB);
	Delete_COMOB(beam_COMOB);

	/* Bildschirm updaten */
//	update_n_times(32);
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
	MEM_HANDLE Firering_handle = NULL;
	MEM_HANDLE Fireball_handle = NULL;
	struct COMOB *Fireball_COMOB;
	struct COMOB *Firering_COMOB;
	struct COMOB *COMOB;
	struct COMOB_behaviour *Behaviour_data;
	SILONG X_3D, Y_3D, Z_3D;
	SILONG dX_3D, dY_3D, dZ_3D;
	SILONG sx,sy,sz;
	SILONG zx,zy,zz;
	UNSHORT Nr_steps;
	UNSHORT i, j, time;

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
	if (!Fireball_COMOB){
		MEM_Free_memory(Firering_handle);
		MEM_Free_memory(Fireball_handle);
		return;
	}
	Firering_COMOB = Add_COMOB(100);
	if (!Firering_COMOB){
		Delete_COMOB(Fireball_COMOB);
		MEM_Free_memory(Firering_handle);
		MEM_Free_memory(Fireball_handle);
		return;
	}

	/* Position des Zaubernden */
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&sx,&sy,&sz);

	/* Position des Gegners */
	Get_3D_part_coordinates(Victim_part,&zx,&zy,&zz);

	/* Success ? */
	if (Fireball_COMOB && Firering_COMOB)
	{
		/* Yes -> Prepare fireball movement */
		Nr_steps=calc_delta_xyz(sx,sy,sz,zx,zy,zz,&dX_3D,&dY_3D,&dZ_3D,5*COMOB_DEC_FACTOR);

		/* Position Feuerball */
		Fireball_COMOB->X_3D=sx;
		Fireball_COMOB->Y_3D=sy;
		Fireball_COMOB->Z_3D=sz;

		/* Set fireball display parameters */
		Fireball_COMOB->Draw_mode = TRANSLUMINANCE_COMOB_DRAWMODE;

		Fireball_COMOB->Hotspot_X_offset = 50;
		Fireball_COMOB->Hotspot_Y_offset = 50;

		Fireball_COMOB->Display_width = 100 - (15 * 4);
		Fireball_COMOB->Display_height = 100 - (15 * 4);

		Fireball_COMOB->Graphics_handle = Fireball_handle;

		Fireball_COMOB->Nr_frames = Get_nr_frames(Fireball_COMOB->Graphics_handle);

		/* Copy fireball coordinates to firering COMOB */
		Firering_COMOB->X_3D = Fireball_COMOB->X_3D;
		Firering_COMOB->Y_3D = Fireball_COMOB->Y_3D;
		Firering_COMOB->Z_3D = Fireball_COMOB->Z_3D;

		/* Set firering display parameters */
		Firering_COMOB->Draw_mode = TRANSLUMINANCE_COMOB_DRAWMODE;

		Firering_COMOB->Hotspot_X_offset = 50;
		Firering_COMOB->Hotspot_Y_offset = 50;

		Firering_COMOB->Display_width = 100 + (15 * 20);
		Firering_COMOB->Display_height = 100 + (15 * 20);

		Firering_COMOB->Graphics_handle = Firering_handle;

		Firering_COMOB->Nr_frames = Get_nr_frames(Firering_COMOB->Graphics_handle);

		Play_sound(233);

		/* Build up the fireball and the firering */
		for(;;){

			/* Make fireball bigger */
			Fireball_COMOB->Display_width += 2*Nr_combat_updates;
			if(Fireball_COMOB->Display_width>100){
				Fireball_COMOB->Display_width=100;
			}
			Fireball_COMOB->Display_height += 2*Nr_combat_updates;
			if(Fireball_COMOB->Display_height>100){
				Fireball_COMOB->Display_height=100;
			}

			/* Make firering smaller */
			Firering_COMOB->Display_width -= 10*Nr_combat_updates;
			if(Firering_COMOB->Display_width<100||Firering_COMOB->Display_width>32760){
				Firering_COMOB->Display_width=100;
			}
			Firering_COMOB->Display_height -= 10*Nr_combat_updates;
			if(Firering_COMOB->Display_height<100||Firering_COMOB->Display_height>32760){
				Firering_COMOB->Display_height=100;
			}

			/* Draw combat screen */
			Update_screen();

			/* beenden wenn gleiche Gr”áe */
			if(Fireball_COMOB->Display_width==100
			 &&Firering_COMOB->Display_width==100){
				break;
			}

		}

		/* Hide firering */
		Hide_COMOB(Firering_COMOB);

		/* Set fireball's movement vector */
		Fireball_COMOB->dX_3D = dX_3D;
		Fireball_COMOB->dY_3D = dY_3D;
		Fireball_COMOB->dZ_3D = dZ_3D;

		Play_sound(234);

		/* Let fireball move towards target */
		update_n_times(Nr_steps);

	}
	else
	{
		/* No -> Clear error stack (the player need not know) */
		ERROR_PopError();

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
	if (Strength){

		/* No -> Show firering */
		Show_COMOB(Firering_COMOB);

		Play_sound(235);

		/* Get fireball coordinates */
		X_3D = Fireball_COMOB->X_3D;
		Y_3D = Fireball_COMOB->Y_3D;
		Z_3D = Fireball_COMOB->Z_3D;

		/* Delete fireball and firering */
		Delete_COMOB(Fireball_COMOB);
		Delete_COMOB(Firering_COMOB);

		Gen_Sparks(X_3D,Y_3D,Z_3D,75/*amount*/,100/*speed*/,100/*life*/,40/*size*/,GEN_SPARK_TYP_ORANGE);

 		/* Do damage to victim */
		Do_combat_damage(Victim_part, max((Strength * C2_SPELL_5_DAMAGE) /
		 100, 1));

		/* kurz darstellen */
		update_n_times(32);

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
	UNSHORT Duration;

	/* Victim is party member ? */
	if (Victim_part->Type == PARTY_PART_TYPE)
	{
		/* Yes -> Calculate duration */
		Duration = max((Strength * 10) / 100, 1);

		/* Set temporary spells */
		Set_member_temporary_spell
		(
			Victim_part->Number,
			ATTACK_TEMP_SPELL,
			Duration,
			Strength
		);
		Set_member_temporary_spell
		(
			Victim_part->Number,
			DEFENCE_TEMP_SPELL,
			Duration,
			Strength
		);
		Set_member_temporary_spell
		(
			Victim_part->Number,
			ANTI_MAGIC_TEMP_SPELL,
			Duration,
			Strength
		);
	}
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
	MEM_HANDLE lp_handle = NULL;
	UNBYTE *Recolour_table_ptr[C2_Spell_8_ANZHALOCOMOBS];
	SILONG halo_size[C2_Spell_1_ANZHALOCOMOBS];
	MEM_HANDLE scare_handle = NULL;
	struct Character_data *Char;
	struct COMOB *scare_COMOB;
	struct COMOB **halo_COMOB;
	struct COMOB_behaviour *behave;
	SILONG x,y,z;
	SILONG left,right,top,bottom;
	SILONG mleft,mright,mtop,mbottom;
	SILONG tleft,tright,ttop,tbottom;
	SILONG zoommode,time,ypos,max_width,max_height;
	SILONG xscale,yscale,xadd,yadd,s,t;
	BOOLEAN bool;
	UNSHORT Char_bits;

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Get victim character bits */
	Char_bits = Get_character_type(Victim_part->Char_handle);

	/* Spell not deflected and is not an end monster ? */
	if (Strength && !(Char_bits & END_MONSTER))
	{
		/* Yes -> Load Beam graphics */
		scare_handle = Load_subfile(COMBAT_GFX, 28);
		if (!scare_handle){
			ERROR_PopError();
			return;
		}

		/* MonsterPosition */
		Get_3D_part_coordinates(Victim_part,&x,&y,&z);
		Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

		Play_sound(230);

		/* Fratze erstellen */
		if((scare_COMOB=Gen_COMOB(x,top,z-2, 0, 100, scare_handle, GC_MAGIC))==NULL){
			ERROR_PopError();
			return;
		}
		scare_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;
		scare_COMOB->Hotspot_Y_offset=50;

		/* Gr”áe der Fratze */
		Get_COMOB_rectangle(scare_COMOB,&mleft,&mtop,&mright,&mbottom);

		/* Fratzte den Ausmessung her an Monster anpassen */
		max_width=(right-left)*80/(mright-mleft);
		max_height=(top-bottom)*80/(mtop-mbottom);

		if (Victim_part->Type == MONSTER_PART_TYPE){
			/* Monsterhalo erzeugen */
			halo_COMOB=Add_halo_to_COMOB_2(Victim_part->Main_COMOB, C2_Spell_8_ANZHALOCOMOBS, 20, 0,0);
			for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
				halo_COMOB[t]->Draw_mode = SILHOUETTE_COMOB_DRAWMODE;
				halo_COMOB[t]->Colour = Find_closest_colour(t*50, t*50, t*50);
				halo_COMOB[t]->Display_width=100;
				halo_COMOB[t]->Display_height=100;
				halo_size[t]=100+((t+1)*5);
				/* mit dem Main COMOB verknpfen*/
				if(!(behave = Add_COMOB_behaviour(halo_COMOB[t],Victim_part->Main_COMOB, connect_handler))){
					ERROR_PopError();
					return;
				}
				/* Set Connect behaviour data */
				behave->Data.Just_data.Data[0] = CONNECT_POSITION;
				behave->Data.Just_data.Data[1] = halo_COMOB[t]->X_3D-Victim_part->Main_COMOB->X_3D;
				behave->Data.Just_data.Data[2] = halo_COMOB[t]->Y_3D-Victim_part->Main_COMOB->Y_3D; /* verschieben, so das Halo von der Mitte des Objects aus berechnet wird */
				behave->Data.Just_data.Data[3] = halo_COMOB[t]->Z_3D-Victim_part->Main_COMOB->Z_3D;
			}
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
			if (Victim_part->Type == MONSTER_PART_TYPE){
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
			}
			time+=Nr_combat_updates;
		}while(time<60);

		/* Gr”áe der Fratze */
		Get_COMOB_rectangle(scare_COMOB,&tleft,&ttop,&tright,&tbottom);

		Play_sound(231);

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
				if((SISHORT)scare_COMOB->Display_height<=(SISHORT)scare_COMOB->Display_width){
					zoommode=2;
					scare_COMOB->Display_height=scare_COMOB->Display_width;
				}
			}
			else if(zoommode==2){
				Get_COMOB_rectangle(scare_COMOB,&tleft,&ttop,&tright,&tbottom);
				scare_COMOB->Y_3D=((ttop-tbottom)/2)+tbottom;
				scare_COMOB->Hotspot_Y_offset=50;
				if (Victim_part->Type == MONSTER_PART_TYPE){
					scare_COMOB->Display_width-=Nr_combat_updates*8;
					scare_COMOB->Display_height-=Nr_combat_updates*8;
					if((SISHORT)scare_COMOB->Display_height<15){
						break;
					}
				}
				else{
					scare_COMOB->Display_width+=Nr_combat_updates*8;
					scare_COMOB->Display_height+=Nr_combat_updates*8;
					if((SISHORT)scare_COMOB->Display_height>300){
						break;
					}
				}
			}
		}

		/* Fratze l”schen */
		Delete_COMOB(scare_COMOB);

		/* Halo kleinzoomen */
		if (Victim_part->Type == MONSTER_PART_TYPE){
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

			/* Halos l”schen */
			for(t=0;t<C2_Spell_8_ANZHALOCOMOBS;t++){
				Delete_COMOB(halo_COMOB[t]);
			}

		}

		/* Is the victim a party member or a monster ? */
		if (Victim_part->Type == PARTY_PART_TYPE)
		{
			/* Party member -> Cause panic */
			Set_condition(Victim_part->Char_handle, PANICKED);
		}
		else
		{
			/* Monster -> Decrease courage */
			Char = (struct Character_data *) MEM_Claim_pointer(Victim_part->Char_handle);

			Char->Courage -= 10 + (25 * Strength) / 100;

			MEM_Free_pointer(Victim_part->Char_handle);
		}
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
