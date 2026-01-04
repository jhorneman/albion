
void C0_Spell_2_handler(UNSHORT Strength);
void Do_C0_Spell_2(UNSHORT Member_nr, UNSHORT Strength);

void C2_Spell_11_handler(UNSHORT Strength);
void Do_C2_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C5_Spell_2_handler(UNSHORT Strength);
void C5_Spell_4_handler(UNSHORT Strength);
void C5_Spell_6_handler(UNSHORT Strength);
void C5_Spell_8_handler(UNSHORT Strength);
void C5_Spell_9_handler(UNSHORT Strength);

void C1_Spell_5_handler(UNSHORT Strength);

void C5_Spell_1_handler(UNSHORT Strength);
void Do_C5_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C5_Spell_3_handler(UNSHORT Strength);
void Do_C5_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C5_Spell_5_handler(UNSHORT Strength);
void Do_C5_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

void C5_Spell_7_handler(UNSHORT Strength);
void Do_C5_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_2_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 17:40
 * LAST      : 02.05.95 17:40
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_2_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C0_Spell_2);
}

void
Do_C0_Spell_2(UNSHORT Member_nr, UNSHORT Strength)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	MEM_HANDLE Char_handle;
	UNSHORT Counter;
	UNSHORT i;

	/* Get character data */
	Char_handle = Party_char_handles[Member_nr - 1];
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check body items */
	Counter = 0;
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Is this packet empty ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* No -> Get item data */
			Item_data = Get_item_data(&(Char->Body_items[i]));

			/* Is cursed / organic item ? */
			if ((Char->Body_items[i].Flags & CURSED) &&
			 (Item_data->Flags & METALLIC))
			{
				/* Yes -> De-curse */
				Char->Body_items[i].Flags &= ~CURSED;

				/* Show */

				/* Count up */
				Counter++;
			}
			Free_item_data();
		}
	}
	MEM_Free_pointer(Char_handle);

	/* Any objects de-cursed ? */
	if (!Counter)
	{
		/* No -> Tell player */
	}
}

UNSHORT Enter_C0_Spell_1_target(UNSHORT Class_nr, UNSHORT Spell_nr,
 UNSHORT *Target_item_slot_index_ptr);
UNSHORT Charge_organic_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_C0_Spell_1_target
 * FUNCTION  : Spell target entry exception handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 17:46
 * LAST      : 02.05.95 17:46
 * INPUTS    : UNSHORT Class_nr - Spell class number (0...MAX_SPELL_CLASSES - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 *             UNSHORT *Target_item_slot_index_ptr - Pointer to target item
 *              slot index.
 * RESULT    : UNSHORT : Target mask.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Enter_C0_Spell_1_target(UNSHORT Class_nr, UNSHORT Spell_nr,
 UNSHORT *Target_item_slot_index_ptr)
{
	UNSHORT Member_index;
	UNSHORT Target_item_slot_index;
	UNSHORT Party_mask = 0;

	/* Select a party member */
	Member_index = Select_party_member(System_text_ptrs[90], NULL, 0);

	/* Any member selected ? */
	if (Member_index && (Member_index != 0xFFFF))
	{
		/* Yes -> Select item */
		Target_item_slot_index =
		 Select_character_item(Party_char_handles[Member_index - 1],
		 System_text_ptrs[91], Charge_organic_item_evaluator);

		/* Any item selected ? */
		if (Target_item_slot_index != 0xFFFF)
		{
			/* Yes -> Build target mask */
			Party_mask = (1 << Member_index);
		}
	}

	return Party_mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Charge organic_item_evaluator
 * FUNCTION  : Check if item is organic and can be charged
 *              (item select window evaluator).
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 17:50
 * LAST      : 03.05.95 10:44
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Charge_organic_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet)
{
	struct Item_data *Item_data;
	UNSHORT Message_nr;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Magical item ? */
	if (Item_data->Spell_nr)
	{
		/* Yes -> Organic item ? */
		if (!(Item_data->Flags & METALLIC))
		{
			/* Yes -> Maximum charges ? */
			if (Packet->Charges < Item_data->Max_charges)
			{
				/* No -> Item is OK */
				Message_nr = 0;
			}
			else
			{
				/* Yes -> "Maximum charges" */
				Message_nr = 129;
			}
		}
		else
		{
			/* No -> "Item is not organic" */
			Message_nr = 128;
		}
	}
	else
	{
		/* No -> "Item is not magical" */
		Message_nr = 93;
	}

	Free_item_data();

	return Message_nr;
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
		shape_COMOB->Draw_mode = TRANSLUMINANCE_COMOB_DRAWMODE;
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
		Add_temporary_effect(Victim_part, POISON_TEMP_EFFECT, Strength,
		 10, NULL);

		/* noch kurze Zeit darstellen */
		update_n_times(24);

	}

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_8_handler
 *						 Monster knowledge
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:49
 * LAST      : 07.06.95 13:49
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_8_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C1_Spell_8);

}

void
Do_C1_Spell_8(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct COMOB *COMOB;
	SILONG left,right,top,bottom;
	SILONG mleft,mright,mtop,mbottom;
	SILONG x,y,z,px,py,pz,mx,my,mz,steps,time;

	/* Grî·e Monster */
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);
	Get_part_rectangle(Current_use_magic_data.Casting_participant,&mleft,&mtop,&mright,&mbottom);
	Get_3D_part_coordinates(Current_use_magic_data.Casting_participant,&px,&py,&pz);

	/* Effect */
	COMOB=Duplicate_COMOB(Victim_part->Main_COMOB);
	COMOB->Hotspot_X_offset=50;
	COMOB->Hotspot_Y_offset=50;
	COMOB->Display_width=10;
	COMOB->Display_height=10;
	COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	COMOB->X_3D = left+((right-left)/2);
	COMOB->Y_3D = bottom+((top-bottom)/2);

	/* Anzahl Schritte bis zum Spiel */
//	px=mleft+((mright-mleft)/2);
//	py=mbottom+((mtop-mbottom)/2);
	steps=calc_delta_xyz(COMOB->X_3D,COMOB->Y_3D,z,px,py,pz,&COMOB->dX_3D,&COMOB->dY_3D,&COMOB->dZ_3D,5*COMOB_DEC_FACTOR);

 	/* Schweif addieren */
	Add_schweif(COMOB,6,4000,0);

	/* kurz darstellen */
	update_n_times(steps+6);

	/* Kopie wieder lîschen */
	Delete_COMOB(COMOB);

}


