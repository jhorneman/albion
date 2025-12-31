/************
 * NAME     : COMMAG1.C
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
#include <COMACTS.H>
#include <COMOBS.H>
#include <COMMAGIC.H>
#include <COMMSUB.H>
#include <ITMLOGIC.H>
#include <TACTICAL.H>
#include <XFTYPES.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_4_handler
 *						 Blink
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 11:15
 * LAST      : 06.06.95 11:15
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C1_Spell_4_MIN_STARS 2
#define C1_Spell_4_MAX_STARS 4

#define C1_Spell_4_MAX_MAGIC 8

void
C1_Spell_4_handler(UNSHORT Strength)
{
	struct COMOB_behaviour *behave;
	struct COMOB *COMOB;
	struct COMOB *magic_COMOB[C1_Spell_4_MAX_MAGIC];
	struct Combat_participant *Victim_part;
	SILONG time;
	UNSHORT Destination_X, Destination_Y;
	SILONG left,right,top,bottom;
	SILONG t,sx,sy,x,y,z;
	SILONG stars,nextstar,nextlight,addmagic,width,height;
	UNSHORT	old_victim_drawmode;

	/* Get victim participant data */
	Victim_part = Get_magic_combat_part_target();
	if (!Victim_part)
		return;

	/* Anzahl Sterne */
	stars=calc_strength(Strength,C1_Spell_4_MIN_STARS,C1_Spell_4_MAX_STARS);

	/* Gr”áe Monster */
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);

	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
		Play_sound(215);

		/* Bezauberungseffekt auf Monster */
		time=20;
		do{
			Update_screen();

			/* n„chster Stern blitzt auf */
			for(t=0;t<stars;t++){

				/* Zuf„llige Position im Monster */
				sx=rndmm(left,right);
				sy=rndmm(bottom,top);

				/* Ist Stern auf Monster */
				if(Test_COMOB_mask(Victim_part->Main_COMOB,sx,sy)&&rndmm(0,1)){

					/* Stern blitzt auf */
					Show_flashstar(sx,sy,z-5);

				}


			}

			time-=Nr_combat_updates;
		}while(time>0);

		/* Handle magical defense */
		Strength = Handle_magical_defense(Victim_part, Strength);
	}

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Can the victim be blinked ? */
		if (Get_conditions(Victim_part->Char_handle) & BLINK_MASK)
		{
			/* No -> Tell the player */
			Set_permanent_message_nr(721);
		}
		else
		{
			/* Yes -> Get the destination coordinates */
			Destination_X = Current_use_magic_data.Extra_target_data %
			 NR_TACTICAL_COLUMNS;
			Destination_Y = Current_use_magic_data.Extra_target_data /
			 NR_TACTICAL_COLUMNS;

			/* Is the destination occupied ? */
			if (Combat_matrix[Destination_Y][Destination_X].Part)
			{
				/* Yes -> Tell the player */
				Set_permanent_message_nr(722);
			}
			else
			{
				/* No -> Victim is monster ? */
				if (Victim_part->Type == MONSTER_PART_TYPE)
				{

					/* Gr”áe Monster */
					Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
					Victim_part->Main_COMOB->Y_3D=bottom+(top-bottom)/2;
					Victim_part->Main_COMOB->Hotspot_Y_offset=50;
					Get_3D_part_coordinates(Victim_part,&x,&y,&z);

					/* Yes -> Rest of effect */
					Play_sound(226);

					/* Monster wird langsam heller und in der Gr”áe ver„ndert */

					/* Bezauberungseffekt auf Monster */
					nextstar=0;
					nextlight=0;
					addmagic=0;
					for(;;){

						Update_screen();

						for(t=0;t<stars;t++){

							/* Zuf„llige Position im Monster */
							sx=rndmm(left,right);
							sy=rndmm(bottom,top);

							/* Ist Stern auf Monster */
							if(Test_COMOB_mask(Victim_part->Main_COMOB,sx,sy)&&rndmm(0,1)){

								/* Stern blitzt auf */
								Show_flashstar(sx,sy,z-5);

							}


						}

						/* n„chste AufblendVorgang */
						if(nextlight>4){
							nextlight=0;

							if(addmagic==0){
								/* alter Zustand */
								old_victim_drawmode=Victim_part->Main_COMOB->Draw_mode;
								/* transparent darstellen */
								Victim_part->Main_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
							}

							/* Main COMOB duplizieren */
							magic_COMOB[addmagic]=Duplicate_COMOB(Victim_part->Main_COMOB);
							magic_COMOB[addmagic]->Draw_mode = TRANSLUMINANCE_COMOB_DRAWMODE;

							magic_COMOB[addmagic]->Lifespan=0;
							magic_COMOB[addmagic]->Z_3D+=addmagic+1;

							/* mit dem Blitzverknpfen*/
							if(!(behave = Add_COMOB_behaviour(magic_COMOB[addmagic], Victim_part->Main_COMOB, connect_handler))){
								ERROR_PopError();
								return;
							}

							/* Set connect behaviour data */
							behave->Data.Just_data_w.Data[0] = CONNECT_POSITION;

							/* Maximale HelligkeitsStufe ereicht */
							if(++addmagic>=C1_Spell_4_MAX_MAGIC){
								goto leave1C1_4;
							}

						}

						nextlight+=Nr_combat_updates;
						nextstar-=Nr_combat_updates;

					}

leave1C1_4:

					/* Monster f„llt zusammen */
//					Play_sound(227);

					width=100;
					height=100;
					nextstar=0;
					for(;;){

						Update_screen();

						/* n„chster Stern blitzt auf */
						for(t=0;t<stars;t++){

							/* Zuf„llige Position im Monster */
							sx=rndmm(left,right);
							sy=rndmm(bottom,top);

							/* Ist Stern auf Monster */
							if(Test_COMOB_mask(Victim_part->Main_COMOB,sx,sy)&&rndmm(0,1)){

								/* Stern blitzt auf */
								Show_flashstar(sx,sy,z-5);

							}

						}

						Victim_part->Main_COMOB->Display_width=width;
						Victim_part->Main_COMOB->Display_height=height;

						for(t=0;t<addmagic;t++){
							magic_COMOB[t]->Display_width=width;
							magic_COMOB[t]->Display_height=height;
						}

						width-=Nr_combat_updates*4;

						if(width<=2){
							width=2;
						}

						if(width==2){
							height-=Nr_combat_updates*4;
							if(height<=1){
								height=1;
								break;
							}
						}

					}

					/* Stern blitzt auf */
					if((COMOB=Gen_COMOB(x,y,z-5, 0, 20, Star_gfx_handles[1], GC_MAGIC))==NULL){
						ERROR_PopError();
						return;
					}

					/* SizeII verhalten */
					if(!(behave = Add_COMOB_behaviour(COMOB,NULL, size_II_handler))){
						ERROR_PopError();
						return;
					}

					/* Set Connect behaviour data */
					behave->Data.Just_data_w.Data[0] = 1000;
					behave->Data.Just_data_w.Data[1] = 1000;
					behave->Data.Just_data_w.Data[2] = 150;
					behave->Data.Just_data_w.Data[3] = 150;
					behave->Data.Just_data_w.Data[4] = 2000;
					behave->Data.Just_data_w.Data[5] = 2000;
					behave->Data.Just_data_w.Data[6] = 2;
					behave->Data.Just_data_w.Data[7] = 2;

					/* Stern kurz darstellen */
					update_n_times(32);

				}

				/* Copy to destination */
				Combat_matrix[Destination_Y][Destination_X].Part = Victim_part;

				/* Clear source */
				Combat_matrix[Victim_part->Tactical_Y][Victim_part->Tactical_X].Part
				 = NULL;

				/* Set new tactical coordinates */
				Victim_part->Tactical_X = Destination_X;
				Victim_part->Tactical_Y = Destination_Y;

				/* Reset action */
				Victim_part->Current_action = NO_COMACT;

				/* Victim is monster ? */
				if (Victim_part->Type == MONSTER_PART_TYPE)
				{
					/* Yes -> Calculate new 3D coordinates for main COMOB */
					Convert_tactical_to_3D_coordinates(Destination_X,
					 Destination_Y, &(Victim_part->Main_COMOB->X_3D),
					 &(Victim_part->Main_COMOB->Z_3D));

					Victim_part->Main_COMOB->Display_width=100;
					Victim_part->Main_COMOB->Display_height=100;

					/* Gr”áe Monster */
					Get_3D_part_coordinates(Victim_part,&x,&y,&z);
					Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

					Victim_part->Main_COMOB->Display_width=width;
					Victim_part->Main_COMOB->Display_height=height;

					Play_sound(210);

					/* Stern blitzt auf */
					if((COMOB=Gen_COMOB(x,y,z-5, 0, 10, Star_gfx_handles[1], GC_MAGIC))==NULL){
						ERROR_PopError();
						return;
					}

					/* SizeII verhalten */
					if(!(behave = Add_COMOB_behaviour(COMOB,NULL, size_II_handler))){
						ERROR_PopError();
						return;
					}

					/* Set Connect behaviour data */
					behave->Data.Just_data_w.Data[0] = 1000;
					behave->Data.Just_data_w.Data[1] = 1000;
					behave->Data.Just_data_w.Data[2] = 150;
					behave->Data.Just_data_w.Data[3] = 150;
					behave->Data.Just_data_w.Data[4] = 2000;
					behave->Data.Just_data_w.Data[5] = 2000;
					behave->Data.Just_data_w.Data[6] = 2;
					behave->Data.Just_data_w.Data[7] = 2;

					/* Stern kurz darstellen */
					update_n_times(20);

					/* und baut sich am neuen Ort wieder auf */
//					Play_sound(228);

					nextstar=0;
					for(;;){

						Update_screen();

						/* n„chster Stern blitzt auf */
						for(t=0;t<stars;t++){

							/* Zuf„llige Position im Monster */
							sx=rndmm(left,right);
							sy=rndmm(bottom,top);

							/* Ist Stern auf Monster */
							if(Test_COMOB_mask(Victim_part->Main_COMOB,sx,sy)&&rndmm(0,1)){

								/* Stern blitzt auf */
								Show_flashstar(sx,sy,z-5);

							}

						}

						Victim_part->Main_COMOB->Display_width=width;
						Victim_part->Main_COMOB->Display_height=height;
						for(t=0;t<addmagic;t++){
							magic_COMOB[t]->Display_width=width;
							magic_COMOB[t]->Display_height=height;
						}

						height+=Nr_combat_updates*4;

						if(height>=100){
							height=100;
						}

						if(height==100){
							width+=Nr_combat_updates*4;
							if(width>=100){
								width=100;
								break;
							}
						}


					}
					Victim_part->Main_COMOB->Display_width=width;
					Victim_part->Main_COMOB->Display_height=height;

					/* Bezauberungseffekt auf Monster */
					nextstar=0;
					nextlight=-20;
					addmagic=0;
					for(;;){

						Update_screen();

						/* n„chster Stern blitzt auf */
						for(t=0;t<stars;t++){

							/* Zuf„llige Position im Monster */
							sx=rndmm(left,right);
							sy=rndmm(bottom,top);

							/* Ist Stern auf Monster */
							if(Test_COMOB_mask(Victim_part->Main_COMOB,sx,sy)&&rndmm(0,1)){

								/* Stern blitzt auf */
								Show_flashstar(sx,sy,z-5);

							}

						}

						/* n„chste AbblendVorgang */
						if(nextlight>4){
							nextlight=0;

							if(addmagic<C1_Spell_4_MAX_MAGIC){

								/* Aktuelles Magic Comob l”schen */
								Delete_COMOB(magic_COMOB[addmagic]);

								/* alle MagicComobs gel”scht dann Monster normal zeichnen */
								if(++addmagic>=C1_Spell_4_MAX_MAGIC){
									Victim_part->Main_COMOB->Draw_mode = old_victim_drawmode;
								}
							}
							else{
								if(++addmagic>=C1_Spell_4_MAX_MAGIC+1){
									goto leave2C1_4;
								}
							}
						}

						nextlight+=Nr_combat_updates;

					}

leave2C1_4:
					x=x;

				}

				update_n_times(24);

				/* Re-draw combat screen */
				Update_screen();

			}

		}

	}

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_6_handler
 *						 Flee
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     :
 * LAST      :
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_6_handler(UNSHORT Strength)
{
	struct COMOB_behaviour *behave_flash;
	struct COMOB *dummy_COMOB;
	SILONG stars,time,t,sx,sy,sz;
	UNSHORT i;

	/* Anzahl Sterne */
	stars=4;

	/* Monster f„llt zusammen */
	for(time=0;time<80;Update_screen(),time+=Nr_combat_updates){

		/* n„chster Stern blitzt auf */
		for(t=0;t<stars;t++){

			/* Zuf„llige Position im Monster */
			sx=rndmm(-(COMBAT_3D_WIDTH/2),(COMBAT_3D_WIDTH/2));
			sy=rndmm(0,COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR);
			sz=rndmm(-(COMBAT_3D_DEPTH/2),(COMBAT_3D_DEPTH/2));

			/* Stern blitzt auf */
			Show_flashstar(sx,sy,sz);

		}

	}

	/* DUMMY COMOB erzeugen */
	if((dummy_COMOB=gen_Dummy_COMOB())==NULL){
		ERROR_PopError();
		return;
	}

	/* Behaviour fr Gr”áe */
	behave_flash = Add_COMOB_behaviour(dummy_COMOB, NULL, flash_handler);

	/* Gr”áe „ndern */
	if(!behave_flash){
		/* Comob l”schen */
		Delete_COMOB(dummy_COMOB);
		ERROR_PopError();
		return;
	}

	behave_flash->Data.Just_data.Data[0]=48; /* X Ticks lang Palette zur entsprechenden Farbe flashen und 5 Ticks wieder zur Ursprungspalette*/
	behave_flash->Data.Just_data.Data[1]=0; /* nach hellblau umblenden */
	behave_flash->Data.Just_data.Data[2]=0;
	behave_flash->Data.Just_data.Data[3]=0;
	behave_flash->Data.Just_data.Data[4]=100; /* maximal 50 % */
	behave_flash->Data.Just_data.Data[5]=32;

	/* Stern kurz darstellen */
	update_n_times(48+32);

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Party_parts[i].Type == PARTY_PART_TYPE)
		{
			/* Yes -> Set current acting participant data */
			Current_acting_part = &Party_parts[i];

			/* Flee! */
			Flee_combat_action(&Party_parts[i]);

			/* Clear current acting participant data */
			Current_acting_part = NULL;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_9_handler
 *						 Mega Killer Spell
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

#define C1_Spell_9_ANZDISSOLVECOMOBS 600

static struct COMOB *ball_COMOB[C1_Spell_9_ANZDISSOLVECOMOBS];
static UNSHORT dis_ch_nr[C1_Spell_9_ANZDISSOLVECOMOBS];

void
C1_Spell_9_handler(UNSHORT Strength)
{
	struct COMOB_behaviour *behave;
	struct COMOB_behaviour *behave_riss;
	struct COMOB_behaviour *behave_x;
	struct COMOB_behaviour *behave_y;
	struct COMOB **halo_COMOB;
	MEM_HANDLE riss_handle = NULL;
	struct COMOB *riss_COMOB;
	struct COMOB *trans_COMOB;
	struct COMOB **halo_riss_COMOBs;
	struct Combat_participant *Victim_part;
	SILONG dissolve_balls;
	UNSHORT Temp_strength;
	UNSHORT Char_bits;
	SILONG time;
	SILONG hw[3];
	SILONG i,s,t,u,v,x,y,z,rx,ry,rz;
	SILONG steps,maxsteps,nextball;

	Select_mega_killer_target(Strength);

	/* Get victim participant data */
	Victim_part = Current_target_parts[0];

	/* Riá laden */
	riss_handle = Load_subfile(COMBAT_GFX, 57);
	if (!riss_handle){
		ERROR_PopError();
		return;
	}

	/* Position Riss */
	rx=0;
	ry=(COMBAT_3D_HEIGHT*COMOB_DEC_FACTOR)/2;
	rz=COMBAT_3D_DEPTH;

	/* Riss Comob erzeugen */
	if((riss_COMOB=Gen_COMOB(rx,ry,rz, 0, 400, riss_handle, GC_NORM))==NULL){
		ERROR_PopError();
		return;
	}
	riss_COMOB->Nr_frames=0;

	/* Leuchtshilouhette addieren */
	halo_riss_COMOBs=Add_halo_to_COMOB_2(riss_COMOB, 3, 20, 0,CONNECT_ALL);

	/* minimale Breite */
	riss_COMOB->Display_width=1;
	halo_riss_COMOBs[0]->Display_width=1;
	halo_riss_COMOBs[1]->Display_width=1;
	halo_riss_COMOBs[2]->Display_width=1;

	/* Riss Grafik nach l”schen des Comobs freigeben */
	riss_COMOB->User_flags|=COMOB_FREE_GFX_ON_DELETE;

	/* Riá aufbl„hen */
	do{
		Update_screen();
		riss_COMOB->Display_width+=(Nr_combat_updates*6);
	}while(riss_COMOB->Display_width<500);
	riss_COMOB->Display_width=500;

	/* Size III Handler */
	if(!(behave_riss = Add_COMOB_behaviour(riss_COMOB, NULL, size_III_handler))){
		ERROR_PopError();
		return;
	}

	/* Set Connect behaviour data */
	behave_riss->Data.Just_data_w.Data[0] = 200; // X,Y Add
	behave_riss->Data.Just_data_w.Data[1] = 20;
	behave_riss->Data.Just_data_w.Data[2] = 640; // max x,y
	behave_riss->Data.Just_data_w.Data[3] = 405;
	behave_riss->Data.Just_data_w.Data[4] = 200; // X,Y Add
	behave_riss->Data.Just_data_w.Data[5] = 20;
	behave_riss->Data.Just_data_w.Data[6] = 560; // min x,y
	behave_riss->Data.Just_data_w.Data[7] = 385;

	/* Kurz darstellen */
	Update_screen();

	/* Monster dissolven */
	for (i=0;i<Current_nr_target_parts;i++)
	{
		/* Get victim participant data */
		Victim_part = Current_target_parts[i];

		/* Handle magical defense */
		Temp_strength = Handle_magical_defense(Victim_part, Strength);

		/* Get victim character bits */
		Char_bits = Get_character_type(Victim_part->Char_handle);

		/* Spell not deflected and is not an end monster ? */
		if (Temp_strength && !(Char_bits & END_MONSTER))
		{
			/* Yes -> Victim is monster ? */
			if (Victim_part->Type == MONSTER_PART_TYPE){

				/* Yes -> Rest of effect */
//				trans_COMOB=Duplicate_COMOB(Victim_part->Main_COMOB);
//				trans_COMOB->Draw_mode = TRANSLUMINANCE_COMOB_DRAWMODE;
//				trans_COMOB->Z_3D -= 5;

				/* HAlo erzeugen */
				halo_COMOB=Add_halo_to_COMOB_2(Victim_part->Main_COMOB, 3, 10, 0,CONNECT_POSITION);

				/* Halo kurz darstellen */
				update_n_times(12);

				/* Dissolve Monster */
 	 			dissolve_balls=Dissolve(Victim_part,3,3,&ball_COMOB[0],C1_Spell_9_ANZDISSOLVECOMOBS);

				/* Alle Baelle hiden */
				for(t=0;t<dissolve_balls;t++){
					Hide_COMOB(ball_COMOB[t]);
				}

				/* Nr vertauschen */
				for(t=0;t<dissolve_balls;t++){
					dis_ch_nr[t]=t;
				}
				for(t=0;t<dissolve_balls;t++){
					u=rndmm(0,dissolve_balls-1);
					s=dis_ch_nr[t];
					dis_ch_nr[t]=dis_ch_nr[u];
					dis_ch_nr[u]=s;
				}

				/* der Reihe nach darstellen */
				nextball=0;
				for(t=dissolve_balls-1;t>=0;t--){
					if(--nextball<0){
						nextball=6;
						Update_screen();
					}
					Show_COMOB(ball_COMOB[dis_ch_nr[t]]);
				}

				/* Halos verschiwnden */
				for(t=0;t<3;t++){
					halo_COMOB[t]->Lifespan=rndmm(1,3);
				}

				/* Main COMOB ausschalten */
				Delete_COMOB(Victim_part->Main_COMOB);
//				Delete_COMOB(trans_COMOB);

				/* Kugeln zittern */
				time=0;
				do{

					Update_screen();

					for(t=0;t<dissolve_balls;t++){
						ball_COMOB[t]->X_3D+=rndmm(-25,25);
						ball_COMOB[t]->Y_3D+=rndmm(-25,25);
						ball_COMOB[t]->Z_3D+=rndmm(-25,25);
					}

				}while((time+=Nr_combat_updates)<20);

				/* Kugeln werden in den Riss gesaugt */

				/* Nr vertauschen */
				for(t=0;t<dissolve_balls;t++){
					dis_ch_nr[t]=t;
				}
				for(t=0;t<dissolve_balls;t++){
//					u=rndmm(0,dissolve_balls-1);
//					s=dis_ch_nr[t];
//					dis_ch_nr[t]=dis_ch_nr[u];
//					dis_ch_nr[u]=s;
				}

				/* Bewegungsvektoren fr Kugeln ausrechnen */
				maxsteps=0;
				nextball=0;
				for(t=0;t<dissolve_balls;t++){
					s=dis_ch_nr[t];
					if(--nextball<0){
						nextball=4;
						Update_screen();
					}
					x=rx+rndmm(-2500,2500);
					y=ry+rndmm(-10000,10000);
					z=rz;
					steps=calc_delta_xyz(ball_COMOB[s]->X_3D,ball_COMOB[s]->Y_3D,ball_COMOB[s]->Z_3D,x,y,z,&ball_COMOB[s]->dX_3D,&ball_COMOB[s]->dY_3D,&ball_COMOB[s]->dZ_3D,5*COMOB_DEC_FACTOR);
					ball_COMOB[s]->Lifespan=steps;
					if(steps>maxsteps){
						maxsteps=steps;
					}

				}

				/* B„lle fliegen zum Riá */
				update_n_times(maxsteps);

			}

	 		/* Dissolve victim */
			Destroy_participant(Victim_part);

		}

	}

	/* Size_iii_Behaviour l”schen */
	Delete_COMOB_behaviour(behave_riss);

	/* Riá verschwindet */
	do{

		Update_screen();

		riss_COMOB->Display_width-=(Nr_combat_updates*6);

		if(riss_COMOB->Display_width<1||riss_COMOB->Display_width>1000){
			riss_COMOB->Display_width=1;
		}

	}while(riss_COMOB->Display_width>1);

	Update_screen();

	/* Riá l”schen */
	Delete_COMOB(riss_COMOB);

}


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_10_handler
 *						 Irritate
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:50
 * LAST      : 07.06.95 13:50
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#define C1_Spell_10_ANZIRRTCOMOBS 48
#define C1_Spell_10_CIRCLEDIVIDE 32
#define C1_Spell_10_CIRCLERAD 45

void
C1_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C1_Spell_10);
}

void
Do_C1_Spell_10(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB_behaviour *behave_x;
	struct COMOB_behaviour *behave_z;
	struct COMOB *COMOB;
	struct COMOB **scoms;
	SILONG left,right,top,bottom;
	SILONG x,y,z,mx,my,mz,t,width;
	SILONG max_lifespan;

	/* Start of effect */

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Objecte erstellen */

		/* Gr”áe Monster */
		Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
		Get_3D_part_coordinates(Victim_part,&mx,&my,&mz);
		width=right-left;

		Play_sound(215);

		/* Irritate Objecte erstellen */
		max_lifespan=-1;
		for(t=0;t<C1_Spell_10_ANZIRRTCOMOBS;t++){

			/* Comob erzeugen */
			y=rndmm(bottom,bottom+(5*COMOB_DEC_FACTOR));
			if((COMOB=Gen_COMOB(mx,y,mz, 0, 100, Spark_gfx_handles[rndmm(0,7)], GC_FIRE))==NULL){
				ERROR_PopError();
				return;
			}
//			COMOB->Frame=rndmm(0,COMOB->Nr_frames-1);
			COMOB->Nr_frames=0;
			COMOB->dY_3D=rndmm(150,300);
			COMOB->Lifespan=(top-bottom)/COMOB->dY_3D;

			/* Schweif addieren */
			scoms=Add_schweif(COMOB,2,4000,-20);
			scoms[0]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			scoms[1]->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;

			/* Add oscillate behaviour for x */
			behave_x = Add_COMOB_behaviour(COMOB, NULL, Oscillate_handler);

			if(!behave_x){
				/* Comob l”schen */
				Delete_COMOB(COMOB);

				/* No -> Clear error stack */
				ERROR_PopError();

				return;
			}

			/* Set oscillate behaviour data */
			behave_x->Data.Oscillate_data.Type = OSCILLATE_X;
			behave_x->Data.Oscillate_data.Period = C1_Spell_10_CIRCLERAD; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_x->Data.Oscillate_data.Amplitude = rndmm(width/8,width/3);
			behave_x->Data.Oscillate_data.Value = rndmm(0,C1_Spell_10_CIRCLEDIVIDE)*C1_Spell_10_CIRCLERAD/C1_Spell_10_CIRCLEDIVIDE;

			/* Add oscillate behaviour for y */
			behave_z = Add_COMOB_behaviour(COMOB, NULL, Oscillate_handler);

			if(!behave_z){
				/* Comob l”schen */
				Delete_COMOB(COMOB);

				/* No -> Clear error stack */
				ERROR_PopError();

				return;
			}

			/* Set oscillate behaviour data */
			behave_z->Data.Oscillate_data.Type = OSCILLATE_Z;
			behave_z->Data.Oscillate_data.Period = C1_Spell_10_CIRCLERAD; /* 0 passiert gar nichts, entspricht 360 Grad / x, 1-x = Periode einer SinusWelle wird x-mal unterteilt ) */
			behave_z->Data.Oscillate_data.Amplitude = behave_x->Data.Oscillate_data.Amplitude;
			behave_z->Data.Oscillate_data.Value = behave_x->Data.Oscillate_data.Value+(C1_Spell_10_CIRCLERAD/4);

			if(COMOB->Lifespan>max_lifespan){
				max_lifespan=COMOB->Lifespan;
			}

		}

		/* Kurz darstellen */
		update_n_times(max_lifespan+16);

		/* Irritate victim */
		Set_condition(Victim_part->Char_handle, IRRITATED);
	}

}





#if FALSE
				/* Monster Halo vergr”áern und wieder verkleinern */
				time=0;
				for(;;){

					Update_screen();

					for(s=0;s<Nr_combat_updates;s++){

						if(time==0){
							halo_COMOB=Add_halo_to_COMOB_2(Victim_part->Main_COMOB, 3, 20, 0,CONNECT_POSITION);
							/* MonsterHalo auf 0 setzen*/
							for(t=0;t<3;t++){
								hw[t]=halo_COMOB[t]->Display_width;
								halo_COMOB[t]->Display_width=Victim_part->Main_COMOB->Display_width;
								halo_COMOB[t]->Display_height=Victim_part->Main_COMOB->Display_height;
							}

						}
						else if(time>0&&time<=15){
							/* MonsterHalo vergr”áert sich langsam */
							for(t=0;t<3;t++){
								if(halo_COMOB[t]->Display_width<hw[t]){
									halo_COMOB[t]->Display_width+=5;
									halo_COMOB[t]->Display_height+=5;
								}
							}
						}
						else if(time>15&&time<20){
							halo_COMOB[t]->Display_width=hw[t];
							halo_COMOB[t]->Display_height=hw[t];
						}
						else if(time==20){
							for(t=0;t<3;t++){
								halo_COMOB[t]->Lifespan=1;
							}
							goto leave1_C3_5;
						}

						time++;

					}

				}

leave1_C3_5:

#endif
