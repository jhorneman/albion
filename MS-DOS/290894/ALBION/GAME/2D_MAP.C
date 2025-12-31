/************
 * NAME     : 2D_MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28.07.94 13:10
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <SCROLL.H>
#include <GFXFUNC.H>
#include <SORT.H>
#include <TEXT.H>

#include <ALBION.H>
#include <CONTROL.H>
#include <MAP.H>
#include <GAME.H>
#include <2D_MAP.H>
#include <ANIMATE.H>
#include <XFTYPES.H>

/* global variables */

struct Module M2_Mod = {
	0, SCREEN_MOD,
	Update_2D_map,
	Init_2D_map,
	Exit_2D_map,
	NULL,
	NULL,
	Display_2D_map
};


struct Method M2_methods[] = {
	{ INIT_METHOD, Init_2D_object },
	{ EXIT_METHOD, Exit_2D_object },
	{ RIGHT_METHOD, Exit_program },
	{ CUSTOMKEY_METHOD, Handle_keys_2D }
};

struct Object_class M2_Class = {
	sizeof(struct Object),
	&M2_methods[0]
};

UNLONG Yoho_X, Yoho_Y;
SISHORT Party_offset[6][3];

UNSHORT M2_Object;

MEM_HANDLE Icondata_handle[2], Icongfx_handle[2];
MEM_HANDLE Party_graphics_handles[6];

UNSHORT Party_gfx_nrs[6] = {
	1,2,2,1,3,3
};

struct Object_2D Party_objects[6];

UNSHORT Party_path[128][3];
UNSHORT Path_indices[6];
UNSHORT Party_frames[6];
UNSHORT Party_animation[] = {0,0,0,1,1,1,2,2,2,1,1,1};

struct Object_2D *Object_2D_list[MAX_2D_OBJECTS];
UNSHORT Nr_2D_objects = 0, Nr_2D_rectangles = 0;

struct Rectangle_2D Rectangle_list[MAX_2D_OBJECTS];

UNSHORT Camera_2D_X, Camera_2D_Y;
UNSHORT Map_buffer[MAPBUF_HEIGHT+3][MAPBUF_WIDTH][MAPBUF_DEPTH][2];
UNBYTE Update_buffer[MAPBUF_HEIGHT+3][MAPBUF_WIDTH];

UNSHORT XYX = 0;

BOOLEAN Big_guy;
SIBYTE Big_guy_list[] = {
	FALSE, FALSE, TRUE, FALSE
};

UNBYTE *Icon_graphics[2];
struct Icon_2D_data *Icon_data[2];

struct Scroll_data Scroll_2D;

struct OPM M2_OPM;

UNCHAR XString1[] = "*** 26-7-1994 ***\
 SCROLL fertiggekriegt.\
 Alle Ordner umorganisiert. Es gibt jetzt STUFF, wo alle Testprogramme drin\
 sind, MODULES, wo alle Module wie XLOAD drin sind, TOOLS, wo Tools wie XPal\
 und Compile drin sind, und ALBION, wo alle Albion-spezifische Sources sind.\
 MODULES und ALBION habe beide ein eigenes LIB-Verzeichnis.\
 Proto-2D-Darstellung angefangen.\
 XPAL tool geschrieben.\
 Fehler in Compile entdeckt und beseitigt : Habe bei 0 angefangen zu z„hlen,\
 obwohl es kein Subfile 0 gibt.\
 Fehler in XLOAD entdeckt und beseitigt : Check auf Library ID in\
 XLD_Open_library() funktioniert nicht. Weiá nicht wieso; hab's enfernt.\
 Fehler in BBDOS entdeckt und beseitigt : In DOS_Seek() sind die Parameter fr\
 lseek() vertauscht. Man k”nnte in alle DOS-Fehlermeldungen noch den\
 Fehlercode von WATCOM einbauen (errno).^\
 *** 18-8-1994 ***\
 Fehler in XLOAD entdeckt und beseitigt : Fehlercodes haben bei 0 angefangen,\
 und stimmten also nicht mit die Fehlermeldungen berein.\
 Fehler in BBDOS entdeckt und beseitigt : In DOS_Read() wird der Parameter\
 bytestoread nicht verglichen mit der Rckgabewert von read().\
 Habe die Funktion BASEMEM_Get_largest_block() geschrieben, nachdem ich in\
 die Help-Dateien von Watcom C ein Beispiel entdeckt habe.\
 Fehler in XLOAD entdeckt und beseitigt : Seek offset wurde falsch berechnet\
 in XLD_Load_batch().^\
 *** 25-7-1994 ***\
 SCROLL angefangen.\
 Habe DSA_CopyOPMToScreen angepasst. Ab jetzt wird bei virtuelle OPM's nicht\
 mehr den Modulo vom Screen, sondern vom Basis OPM verwendet. Zus„tzlich\
 werden virtual_x und virtual_y nicht mehr auf die Darstellungskoordinaten\
 addiert.\
 Habe OPM_DelVirtualOPM geschrieben. Macht nicht viel, ist aber wenigstens da.\
 Habe eine Funktion namens OPM_SetVirtualPosition geschrieben. Hiermit kann\
 man der Position eines virtuelles OPM innerhalb das Basis OPM „ndern. Es\
 gibt ein biáchen Logik und der data-Zeiger wird neu gesetzt.";

void
Init_2D_object(struct Object *Object, union Method_parms *P)
{
	Object->Rect.left += P->Rect.left;
	Object->Rect.top += P->Rect.top;
	Object->Rect.width = P->Rect.width;
	Object->Rect.height = P->Rect.height;
}

void
Exit_2D_object(struct Object *Object, union Method_parms *P)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:14
 * LAST      : 28.07.94 12:14
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_2D_map(void)
{
	UNSHORT i,j;
	UNBYTE *Ptr;

	OPM_CreateVirtualOPM(&Main_OPM, &M2_OPM,
	 MAP2D_X,MAP2D_Y,MAP2D_WIDTH,MAP2D_HEIGHT);
	Push_root(&M2_OPM);

	{
		struct BBRECT x = {MAP2D_X,MAP2D_Y,MAP2D_WIDTH,MAP2D_HEIGHT};

		M2_Object = Add_object(0, &M2_Class, (UNBYTE *) &x);
	}

	/* Load party graphics */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	Big_guy = Big_guy_list[Map_ptr->ICON_SET_1_NR - 1];

	{
		UNSHORT i,j;

		if (Big_guy)
			j = PART1_GFX;
		else
			j = PART2_GFX;

		for (i=0;i<6;i++)
			Party_graphics_handles[i] = XLD_Load_subfile(&Xftypes[j],
			 Party_gfx_nrs[i]);
	}

	/* Load first icon set */
	Icondata_handle[0] = XLD_Load_subfile(&Xftypes[ICON_DATA],
	 (UNSHORT) Map_ptr->ICON_SET_1_NR);
	Icongfx_handle[0] = XLD_Load_subfile(&Xftypes[ICON_GFX],
	 (UNSHORT) Map_ptr->ICON_SET_1_NR);

	/* Load second icon set (if any) */
	if (Map_ptr->ICON_SET_2_NR)
	{
		Icondata_handle[1] = XLD_Load_subfile(&Xftypes[ICON_DATA],
		 (UNSHORT) Map_ptr->ICON_SET_2_NR);
		Icongfx_handle[1] = XLD_Load_subfile(&Xftypes[ICON_GFX],
		 (UNSHORT) Map_ptr->ICON_SET_2_NR);
	}

	/* Initialize animation */
	Init_animation();

	/* Initialize party objects */
	{
		struct Gfx_header *Gfx;

		for (i=0;i<6;i++)
		{
			Gfx = (struct Gfx_header *) MEM_Claim_pointer(Party_graphics_handles[i]);

			Party_frames[i] = rand() % 12;

			Party_objects[i].Width = Gfx->Width;
			Party_objects[i].Height = Gfx->Height;
			Party_objects[i].Level = 0;
			Party_objects[i].Graphics_handle = Party_graphics_handles[i];

			Object_2D_list[Nr_2D_objects] = &Party_objects[i];
			Nr_2D_objects++;

			MEM_Free_pointer(Party_graphics_handles[i]);
		}
	}

	/* Initialize party path */
	{
		UNSHORT X,Y;

		X = PARTY_DATA.X * 16;
	 	Y = PARTY_DATA.Y * 16 + 15;

		for (i=0;i<128;i++)
		{
			Party_path[i][0] = X;
			Party_path[i][1] = Y;
			Party_path[i][2] = PARTY_DATA.View_direction;
		}

		for (i=0;i<6;i++)
		{
			j = (5-i) * 2;
			if (Big_guy)
				j *= 2;

			Path_indices[i] = j;

			Party_objects[i].X = Party_path[j][0];
			Party_objects[i].Y = Party_path[j][1];

			Party_objects[i].Graphics_offset = ((Party_path[j][2] * 3
			 + Party_animation[Party_frames[i]])
			 * Party_objects[i].Width * Party_objects[i].Height)
			 + sizeof(struct Gfx_header);

			if (i)
			{
				Party_offset[i][0] = (rand() & 7) - 3;
				Party_offset[i][1] = 0 - (rand() & 3);
				Party_offset[i][2] = 0 - (rand() & 3);
			}
			else
			{
				Party_offset[i][0] = 0;
				Party_offset[i][1] = 0;
				Party_offset[i][2] = 0;
			}
		}
	}

	/* Initialize scrolling */
	Scroll_2D.Update_unit = Update_2D_unit;
	Scroll_2D.Unit_width = 16;
	Scroll_2D.Unit_height = 16;
	Scroll_2D.Viewport_width = MAP2D_WIDTH;
	Scroll_2D.Viewport_height = MAP2D_HEIGHT;
	Scroll_2D.Playfield_width = Map_width * 16 ;
	Scroll_2D.Playfield_height = Map_height * 16;

	Icon_data[0] = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[0]);
	Icon_data[1] = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[1]);
	Icon_graphics[0] = MEM_Claim_pointer(Icongfx_handle[0]);
	Icon_graphics[1] = MEM_Claim_pointer(Icongfx_handle[1]);

	Init_scroll(&Scroll_2D, 0, 0);

	MEM_Free_pointer(Icongfx_handle[0]);
	MEM_Free_pointer(Icongfx_handle[1]);
	MEM_Free_pointer(Icondata_handle[0]);
	MEM_Free_pointer(Icondata_handle[1]);

	MEM_Free_pointer(Map_handle);

	Camera_2D_X = (UNSHORT)(Scroll_2D.Playfield_X / 16);
	Camera_2D_Y = (UNSHORT)(Scroll_2D.Playfield_Y / 16);
}

/*
 ******************************************************************************
 */

void
Do_2D_movement(SISHORT dX, SISHORT dY)
{
	UNSHORT v,v2,i,k;
	SISHORT idx;

	v = 8;
	v2 = 2;
	if (Big_guy)
	{
		v = 4;
		v2 = 4;
	}

	for (k=0;k<v;k++)
	{
		Nr_2D_objects = 0;

		for (i=0;i<6;i++)
		{
			idx = Path_indices[i];
			idx++;
			if (idx>=128)
			{
				idx -= 128;

				if (i)
				{
					Party_offset[i][0] = (rand() & 3) - 2;
					Party_offset[i][1] = 0 - (rand() & 3);
					Party_offset[i][2] = 0 - (rand() & 3);
				}
			}
			Path_indices[i] = idx;

			if (!i)
			{
				Party_path[idx][0] = PARTY_DATA.X * 16 + v2 * (dX * k);
				Party_path[idx][1] = PARTY_DATA.Y * 16 + 15 + v2 * (dY * k);
				Party_path[idx][2] = PARTY_DATA.View_direction;

				Yoho_X = Party_path[idx][0];
				Yoho_Y = Party_path[idx][1];
			}

			idx += Party_offset[i][2];
			if (idx<0)
				idx += 128;
			if (idx>=128)
				idx -= 128;

			Party_objects[i].X = Party_path[idx][0] + Party_offset[i][0];
			Party_objects[i].Y = Party_path[idx][1] + Party_offset[i][1];

			Party_objects[i].Graphics_offset = ((Party_path[idx][2] * 3
			 + Party_animation[Party_frames[i]])
			 * Party_objects[i].Width * Party_objects[i].Height)
			 + sizeof(struct Gfx_header);

			Party_frames[i] = (Party_frames[i] + 1) % 12;

			Object_2D_list[Nr_2D_objects] = &Party_objects[i];
			Nr_2D_objects++;
		}

 		Update_2D_map();

		if (k < (v-1))
		{
			Switch_screens();
		}
	}

	PARTY_DATA.X += dX;
	PARTY_DATA.Y += dY;
}

void
Handle_keys_2D(struct Object *Object, union Method_parms *P)
{
	UNSHORT Key_code;

	Key_code = (UNSHORT) (P->Event->sl_key_code);

	switch (Key_code)
	{
		case 'P':
			Scroll_display_OPM(&Scroll_2D, &Main_OPM, MAP2D_X, MAP2D_Y);
			Display_text(&Main_OPM, XString1);
			break;
		case BLEV_UP:
			if (PARTY_DATA.Y > 2)
			{
				PARTY_DATA.View_direction = NORTH;
				Do_2D_movement(0,-1);
			}
			break;
		case BLEV_DOWN:
			if (PARTY_DATA.Y < Map_height-1)
			{
				PARTY_DATA.View_direction = SOUTH;
				Do_2D_movement(0,1);
			}
			break;
 		case BLEV_LEFT:
			if (PARTY_DATA.X > 0)
			{
				PARTY_DATA.View_direction = WEST;
				Do_2D_movement(-1,0);
			}
			break;
 		case BLEV_RIGHT:
			if (PARTY_DATA.X < Map_width-2)
			{
				PARTY_DATA.View_direction = EAST;
				Do_2D_movement(1,0);
			}
			break;
		case BLEV_ESC:
			Quit_program = TRUE;
			break;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_2D_map
 * FUNCTION  : Update the 2D map display.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:18
 * LAST      : 28.07.94 12:18
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_map(void)
{
	SISHORT mvecx, mvecy;
	struct BBPOINT Mouse;

	/* Do colour cycling */
	if (!XYX)
	{
		switch (Palette_nr)
 		{
			case 1:
			case 2:
			{
				Colour_cycle_forward(153,7);
				Colour_cycle_forward(176,5);
				Colour_cycle_forward(181,11);
				DSA_ActivatePal(&Screen);
			}
			case 3:
			{
				Colour_cycle_forward(176,5);
				Colour_cycle_forward(181,11);
				DSA_ActivatePal(&Screen);
			}
		}

		Update_animation();
	}

	XYX++;

	if (XYX > 3)
		XYX = 0;

	/* Determine scroll vector */
/*	BLEV_GetMousePos(&Mouse);

	mvecx = 0;
	mvecy = 0;

	if (Mouse.x < 16)
		mvecx = 0 - (16 - Mouse.x) / 2;
	if (Mouse.x > Screen_width-16)
		mvecx = (Mouse.x - Screen_width+16) / 2;
	if (Mouse.y < 16)
		mvecy = 0 - (16 - Mouse.y) / 2;
	if (Mouse.y > Screen_height-16)
		mvecy = (Mouse.y - Screen_height+16) / 2; */

	/* Initialize pointers */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	Icon_data[0] = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[0]);
	Icon_data[1] = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle[1]);
	Icon_graphics[0] = MEM_Claim_pointer(Icongfx_handle[0]);
	Icon_graphics[1] = MEM_Claim_pointer(Icongfx_handle[1]);

	/* Update display */
	//Do_scroll(&Scroll_2D, mvecx, mvecy);
	{
		UNLONG X,Y;

		X = Yoho_X - 180 + 16;
		Y = Yoho_Y - 96 - 8;

		Set_scroll_position(&Scroll_2D, X, Y);

		Do_2D_map_display();
	}

	/* Free pointers */
	MEM_Free_pointer(Icongfx_handle[0]);
	MEM_Free_pointer(Icongfx_handle[1]);
	MEM_Free_pointer(Icondata_handle[0]);
	MEM_Free_pointer(Icondata_handle[1]);
	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94
 * LAST      : 28.07.94
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_2D_map(void)
{
	Scroll_display(&Scroll_2D, &Screen, MAP2D_X, MAP2D_Y);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94
 * LAST      : 28.07.94
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_2D_map(void)
{
	UNSHORT i;

	Exit_scroll(&Scroll_2D);

	Pop_root();
	OPM_Del(&M2_OPM);

	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	MEM_Free_memory(Icondata_handle[0]);
	MEM_Free_memory(Icongfx_handle[0]);

	if (Map_ptr->ICON_SET_2_NR)
	{
		MEM_Free_memory(Icondata_handle[1]);
		MEM_Free_memory(Icongfx_handle[1]);
	}

	MEM_Free_pointer(Map_handle);
	MEM_Free_memory(Map_handle);

	for (i=0;i<6;i++)
		MEM_Free_memory(Party_graphics_handles[i]);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_2D_unit
 * FUNCTION  : Updates a scroll unit of the 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 16:00
 * LAST      : 28.07.94 16:00
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             UNSHORT Buffer_X - X-coordinate of unit in scroll buffer.
 *             UNSHORT Buffer_Y - Y-coordinate of unit in scroll buffer.
 *             UNSHORT Playfield_X - X-coordinate of unit in playfield.
 *             UNSHORT Playfield_Y - Y-coordinate of unit in playfield.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Several pointers (for instance to icon data and graphics)
 *              must have been initialized.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X, UNSHORT Buffer_Y,
 UNSHORT Playfield_X, UNSHORT Playfield_Y)
{
	struct Icon_2D_data *Data;
	UNBYTE *Map_layer_ptr, *Icon;
	UNSHORT Offset, Frame;
	UNSHORT B1,B2,B3;
	UNSHORT Underlay_nr,Overlay_nr;

	Camera_2D_X = (UNSHORT)(Scroll_2D.Playfield_X >> 4);
	Camera_2D_Y = (UNSHORT)(Scroll_2D.Playfield_Y >> 4);

	/* Calculate offset / hash number */
	Offset = Playfield_X + Playfield_Y * Map_width;

	/* Calculate pointer to map layer */
	Map_layer_ptr = (UNBYTE *) (Map_ptr + 1)
	 + (Offset * sizeof(struct Square_2D));

	/* Read bytes from map */
	B1 = (UNSHORT) *Map_layer_ptr++;
	B2 = (UNSHORT) *Map_layer_ptr++;
	B3 = (UNSHORT) *Map_layer_ptr++;

	/* Build overlay and underlay number */
	Underlay_nr = ((B2 & 0x0F) << 8) | B3;
	Overlay_nr = (B1 << 4) | (B2 >> 4);

	/* Any underlay ? */
	if (Underlay_nr > 1)
	{
		/* Yes -> Get icon data */
		Data = &Icon_data[0][Underlay_nr-2];

		/* Animated ? */
		if (Data->Nr_frames == 1)
		{
			/* No -> Get frame number */
			Frame = Data->First_frame + Get_animation_frame(Data->Flags,
			 Data->Nr_frames, Offset);

			/* Get graphics address */
			Icon = Icon_graphics[0] + 256 * Frame;

			/* Display underlay */
			Put_unmasked_block(&(Scroll->Base_OPM), Buffer_X, Buffer_Y, 16, 16,
		 	 Icon);
		}
	}

	/* Any overlay ? */
	if (Overlay_nr > 1)
	{
		/* Yes -> Get icon data */
		Data = &Icon_data[0][Overlay_nr-2];

		/* Animated ? */
		if (Data->Nr_frames == 1)
		{
			/* No -> Get frame number */
			Frame = Data->First_frame + Get_animation_frame(Data->Flags,
			 Data->Nr_frames, Offset);

			/* Get graphics address */
			Icon = Icon_graphics[0] + 256 * Frame;

			/* Display overlay */
			Put_masked_block(&(Scroll->Base_OPM), Buffer_X, Buffer_Y, 16, 16,
		 	 Icon);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_2D_map_display
 * FUNCTION  : Draw all objects and animated parts of the 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 28.07.94 18:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Several pointers (for instance to icon data and graphics)
 *              must have been initialized.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_2D_map_display(void)
{
	struct Object_2D *Ptr;
	UNSHORT i,j,k;
	UNSHORT Remaining_2D_objects;
	UNSHORT U, Current_object = 0;

	/* Arrange for the removal of previous objects */
	Set_erase_rectangles();

	/* Read the currently visible part of the map */
	Fill_map_buffer();

	/* Arrange for the updating of animated parts */
	Set_animation_update();

	/* Update current objects */
	for (i=0;i<Nr_2D_objects;i++)
	{
		Ptr = Object_2D_list[i];

		/* Set update rectangle */
		Set_update_rectangle(Ptr->X, Ptr->Y - (Ptr->Level * 16),
		 Ptr->Width, Ptr->Height);

		/* Calculate display index */
		Ptr->Index = ((Ptr->X + Ptr->Width - 1) / 16) - Camera_2D_X
		 + ((Ptr->Y / 16) - Camera_2D_Y) * MAPBUF_WIDTH;
	}

	/* Sort 2D objects */
	Shellsort(Swap_2D_objects, Compare_2D_objects, Nr_2D_objects, NULL);

	/* Display 2D objects */
	Remaining_2D_objects = Nr_2D_objects;

	for (i=0;i<MAPBUF_HEIGHT+3;i++)
	{
		for (j=0;j<MAPBUF_WIDTH;j++)
		{
			/* Update ? */
			U = Update_buffer[i][j];

			if (U)
			{
				/* Yes -> Do layers */
				for (k=0;k<MAPBUF_DEPTH-1;k++)
				{
					/* Update this layer ? */
					if (U & (1 << k))
					{
						if (((i-k) >= 0) && ((i-k) < MAPBUF_HEIGHT))
						{
							/* Yes -> Display underlay and overlay */
							Display_underlay(Map_buffer[i][j][k][0], j, i-k);
							Display_overlay(Map_buffer[i][j][k][1], j, i-k);
						}
					}
				}
			}

			/* Any objects left ? */
			while (Remaining_2D_objects)
			{
				/* Yes -> Get data */
				Ptr = Object_2D_list[Current_object];

				/* Draw this one ? */
				if (Ptr->Index <= i * MAPBUF_WIDTH + j)
				{
					/* Yes -> Draw */
					Display_2D_object(Ptr);

					/* Count down */
					Remaining_2D_objects--;
					Current_object++;
				}
				else
					/* No -> Exit */
					break;
			}
		}
	}


	/* Display */
	k = MAPBUF_DEPTH-1;
	for (i=0;i<MAPBUF_HEIGHT+3;i++)
	{
		for (j=0;j<MAPBUF_WIDTH;j++)
		{
			/* Update ? */
			U = Update_buffer[i][j];

			if (U)
			{
				/* Yes -> Clear */
				Update_buffer[i][j] = 0;

				/* Update this layer ? */
				if (U & (1 << k))
				{
					if (((i-k) >= 0) && ((i-k) < MAPBUF_HEIGHT))
					{
						/* Yes -> Display underlay and overlay */
						Display_underlay(Map_buffer[i][j][k][0], j, i-k);
						Display_overlay(Map_buffer[i][j][k][1], j, i-k);
					}
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_underlay
 * FUNCTION  : Display an underlay in the scroll buffer.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 15.08.94 11:24
 * INPUTS    : UNSHORT Overlay_nr - Number from map / 0 (empty).
 *             UNSHORT X - X-coordinate relative to camera.
 *             UNSHORT Y - Y-coordinate relative to camera.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_underlay(UNSHORT Underlay_nr, UNSHORT X, UNSHORT Y)
{
	SILONG nX, nY;
	struct Icon_2D_data *Data;
	UNBYTE *Icon;
	UNSHORT Frame;

	/* Any underlay ? */
	if (Underlay_nr)
	{
		/* Get scroll buffer coordinates */
		nX = ((SILONG) (Camera_2D_X + X)) * 16;
		nY = ((SILONG) (Camera_2D_Y + Y)) * 16;
		Convert_scroll_coordinates(&Scroll_2D, &nX, &nY);

		if ((nX == -32768) && (nY == -32768))
			return;

		/* Yes -> Get frame number */
		Data = &Icon_data[0][Underlay_nr-2];

		Frame = Data->First_frame + Get_animation_frame(Data->Flags,
	 	 Data->Nr_frames, (X + Y * Map_width));

		/* Get graphics address */
		Icon = Icon_graphics[0] + 256 * Frame;

		/* Display underlay */
	 	Put_unmasked_block(&(Scroll_2D.Base_OPM), nX, nY, 16, 16, Icon);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_overlay
 * FUNCTION  : Display an overlay in the scroll buffer.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 15.08.94 11:24
 * INPUTS    : UNSHORT Overlay_nr - Number from map / 0 (empty).
 *             UNSHORT X - X-coordinate relative to camera.
 *             UNSHORT Y - Y-coordinate relative to camera.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_overlay(UNSHORT Overlay_nr, UNSHORT X, UNSHORT Y)
{
	SILONG nX, nY;
	struct Icon_2D_data *Data;
	UNBYTE *Icon;
	UNSHORT Frame;

	/* Any overlay ? */
	if (Overlay_nr)
	{
		/* Yes -> Get scroll buffer coordinates */
		nX = ((SILONG) (Camera_2D_X + X)) * 16;
		nY = ((SILONG) (Camera_2D_Y + Y)) * 16;
		Convert_scroll_coordinates(&Scroll_2D, &nX, &nY);

		if ((nX == -32768) && (nY == -32768))
			return;

		/* Get frame number */
		Data = &Icon_data[0][Overlay_nr-2];

		Frame = Data->First_frame + Get_animation_frame(Data->Flags,
	 	 Data->Nr_frames, (X + Y * Map_width));

		/* Get graphics address */
		Icon = Icon_graphics[0] + 256 * Frame;

		/* Display overlay */
		Put_masked_block(&(Scroll_2D.Base_OPM), nX, nY, 16, 16, Icon);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_object
 * FUNCTION  : Display a 2D object in the scroll buffer.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 18:54
 * LAST      : 16.08.94 14:12
 * INPUTS    : struct Object_2D *Object - Pointer to object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_2D_object(struct Object_2D *Object)
{
	struct BBRECT *Clip;
	SILONG X,Y;
	SILONG vpW,vpH;
	UNSHORT vpX,vpY;
	UNSHORT objW,objH;
	UNSHORT W,H;
	SISHORT wX, wY;
	UNBYTE *Ptr;

	/* Get object dimensions */
	objW = Object->Width;
	objH = Object->Height;

	/* Get scroll buffer coordinates and dimensions */
	vpX = Scroll_2D.Viewport_X;
	vpY = Scroll_2D.Viewport_Y;
	vpW = (SILONG) Scroll_2D.Base_OPM.width;
	vpH = (SILONG) Scroll_2D.Base_OPM.height;

	/* Calculate object coordinates in playfield */
	X = (SILONG) Object->X;
	Y = (SILONG) (Object->Y - (Object->Level * 16) - objH + 1);

	/* Convert to scroll buffer coordinates */
	X -= Scroll_2D.Playfield_X;
	Y -= Scroll_2D.Playfield_Y;

	/* Inside the scroll buffer ? */
 	if ((X <= 0 - objW) || (X >= vpW - 16)
	 || (Y <= 0 - objH) || (Y >= vpH - 16))
		return;

	/* Yes -> Get graphics address */
	Ptr = MEM_Claim_pointer(Object->Graphics_handle)
	 + Object->Graphics_offset;

	/* Add viewport coordinates */
	wX = X + vpX;
	wY = Y + vpY;

 	/* Get clipping rectangle */
	Clip = &Scroll_2D.Base_OPM.clip;

	/* Draw object */
	if (vpY < 16)
	{
		if (vpX < 16)
		{
			/* Scroll buffer is not divided */
			/* Draw object in first quadrant */
			Clip->left = vpX;
			Clip->top = vpY;
			Clip->width = vpX + vpW - 16;
			Clip->height = vpY + vpH - 16;

			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		}
		else
		{
			/* Scroll buffer is divided horizontally */
			/* Draw object in first quadrant */
			Clip->left = vpX;
			Clip->top = vpY;
			Clip->width = vpW - vpX;
			Clip->height = vpY + vpH - 16;

			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in second quadrant */
			Clip->left = 0;
			Clip->width = vpX - 16;

			wX -= vpW;
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		}
	}
	else
	{
		if (vpX < 16)
		{
			/* Scroll buffer is divided vertically */
			/* Draw object in first quadrant */
			Clip->left = vpX;
			Clip->top = vpY;
			Clip->width = vpX + vpW - 16;
			Clip->height = vpH - vpY;

			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in third quadrant */
			Clip->top = 0;
			Clip->height = vpY - 16;

			wY -= vpH;

			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		}
		else
		{
			/* Scroll buffer is divided horizontally AND vertically */
			/* Draw object in first quadrant */
			Clip->left = vpX;
			Clip->top = vpY;
			Clip->width = vpW - vpX;
			Clip->height = vpH - vpY;

			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in second quadrant */
			Clip->left = 0;
			Clip->width = vpX - 16;

			wX -= vpW;
			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in third quadrant */
			Clip->top = 0;
			Clip->height = vpY - 16;

			wY -= vpH;

			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);

			/* Draw object in fourth quadrant */
			Clip->left = vpX;
			Clip->width = vpW - vpX;

			wX += vpW;

			Put_masked_block(&(Scroll_2D.Base_OPM), wX, wY, objW, objH, Ptr);
		}
	}

	/* Restore clipping rectangle */
	Clip->left = 0;
	Clip->top = 0;
	Clip->width = vpW;
	Clip->height = vpH;

	MEM_Free_pointer(Object->Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Swap_2D_objects
 * FUNCTION  : Swap two 2D objects (for sorting).
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 19:08
 * LAST      : 28.07.94 19:08
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Swap_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Object_2D *T;

	T = Object_2D_list[A];
	Object_2D_list[A] = Object_2D_list[B];
	Object_2D_list[B] = T;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_2D_objects
 * FUNCTION  : Compare two 2D objects (for sorting).
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 19:07
 * LAST      : 28.07.94 19:07
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 *	RESULT    : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Object_2D *O1, *O2;

	O1 = Object_2D_list[A];
	O2 = Object_2D_list[B];

//	if (O1->Index == O2->Index)
//	{
		if (O1->Y == O2->Y)
			return(O1->X > O2->X);
		else
			return(O1->Y > O2->Y);
/*	}
	else
		return(O1->Index > O2->Index); */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.08.94 11:11
 * LAST      : 01.08.94 11:11
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_animation_update(void)
{
	struct Icon_2D_data *Data;
	UNSHORT Underlay, Overlay;
	UNSHORT i,j,k,l;

	/* Check the entire map buffer */
	for (i=0;i<MAPBUF_HEIGHT+3;i++)
	{
		for (j=0;j<MAPBUF_WIDTH;j++)
		{
			/* Do layers */
			for (k=0;k<MAPBUF_DEPTH;k++)
			{
				/* Get underlay */
				Underlay = Map_buffer[i][j][k][0];

				/* Any underlay ? */
				if (Underlay)
				{
					/* Yes -> Get icon data */
					Data = &Icon_data[0][Underlay-2];

					/* Animated ? */
					if (Data->Nr_frames > 1)
					{
						/* Yes -> Update */
						for (l=0;l<MAPBUF_DEPTH;l++)
						{
							if (((i-k+l) >= 0) && ((i-k+l) < MAPBUF_HEIGHT + 3))
								Update_buffer[i-k+l][j] |= (1 << l);
						}
					}
				}

				/* Get overlay */
				Overlay = Map_buffer[i][j][k][1];

				/* Any overlay ? */
				if (Overlay)
				{
					/* Yes -> Get icon data */
					Data = &Icon_data[0][Overlay-2];

					/* Animated ? */
					if (Data->Nr_frames > 1)
					{
						/* Yes -> Update */
						for (l=0;l<MAPBUF_DEPTH;l++)
						{
							if (((i-k+l) >= 0) && ((i-k+l) < MAPBUF_HEIGHT + 3))
								Update_buffer[i-k+l][j] |= (1 << l);
						}
					}
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_update_rectangle
 * FUNCTION  : Set update rectangle.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 16:20
 * LAST      : 28.07.94 16:20
 * INPUTS    : SISHORT X - Left X-coordinate.
 *             SISHORT Y - Bottom (!) Y-coordinate.
 *             UNSHORT Width - Width in pixels.
 *             UNSHORT Height - Height in pixels.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_update_rectangle(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	struct Rectangle_2D *Rect;
	UNSHORT W, H;
	UNSHORT oX, oY;
	UNSHORT i,j,k;
	SISHORT X2, Y2;

	X2 = X + Width;
	Y2 = Y;
	Y = Y2 - Height + 1;

	X = X >> 4;
	Y = Y >> 4;
	X2 = (X2 + 15) >> 4;
	Y2 = (Y2 + 16) >> 4;

	/* Insert in 2D rectangle list */
	Rect = &Rectangle_list[Nr_2D_rectangles];

	Rect->X = X;
	Rect->Y = Y;
	Rect->X2 = X2;
	Rect->Y2 = Y2;

	Nr_2D_rectangles++;

	/* Adapt to camera coordinates */
	X -= Camera_2D_X;
	Y -= Camera_2D_Y;
	X2 -= Camera_2D_X;
	Y2 -= Camera_2D_Y;

	/* Exit if completely out of the viewport */
	if ((X2 < 0) || (X > MAPBUF_WIDTH) || (Y2 < 0) || (Y > MAPBUF_HEIGHT))
		return;

	/* Calculate width and height */
	if (X < 0)
	{
		W = X2;
		X = 0;
	}
	else
	{
		W = X2 - X;
	}

	if (Y < 0)
	{
		H = Y2;
		Y = 0;
	}
	else
	{
		H = Y2 - Y;
	}

	if (X2 >= MAPBUF_WIDTH)
		W = MAPBUF_WIDTH - X;

	if (Y2 >= MAPBUF_HEIGHT)
		H = MAPBUF_HEIGHT - Y;

	/* Anything left ? */
	if (!W || !H)
		return;

	/* Set rectangle in update buffer */
	for (i=0;i<H;i++)
	{
		for (j=0;j<W;j++)
		{
			for (k=0;k<MAPBUF_DEPTH;k++)
			{
					Update_buffer[Y + i + k][X + j] |= (1 << k);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_erase_rectangles
 * FUNCTION  : Set update rectangles from previous frame.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 16:30
 * LAST      : 28.07.94 16:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_erase_rectangles(void)
{
	SISHORT X,Y,X2,Y2;
	UNSHORT W,H;
	UNSHORT h,i,j,k;
	UNSHORT wX,wY;

	for (h=0;h<Nr_2D_rectangles;h++)
	{
		/* Get rectangle data from previous frame */
		X = Rectangle_list[h].X;
		Y = Rectangle_list[h].Y;
		X2 = Rectangle_list[h].X2;
		Y2 = Rectangle_list[h].Y2;

		/* Adapt to camera coordinates */
		X -= Camera_2D_X;
		Y -= Camera_2D_Y;
		X2 -= Camera_2D_X;
		Y2 -= Camera_2D_Y;

		/* Exit if completely out of the viewport */
		if ((X2 < 0) || (X > MAPBUF_WIDTH) || (Y2 < 0) || (Y > MAPBUF_HEIGHT))
			continue;

		/* Calculate width and height */
		W = X2 - X;
		H = Y2 - Y;

		/* Anything left ? */
		if (!W || !H)
			continue;

		/* Make sure initial coordinates are within viewport */
		if (X < 0)
			X += MAPBUF_WIDTH;

		if (X >= MAPBUF_WIDTH)
			X -= MAPBUF_WIDTH;

		if (Y < 0)
			Y += MAPBUF_HEIGHT;

		if (Y >= MAPBUF_HEIGHT)
			Y -= MAPBUF_HEIGHT;

		/* Set rectangle in update buffer */
		wY = Y;
		for (i=0;i<H;i++)
		{
			wX = X;
			for (j=0;j<W;j++)
			{
				for (k=0;k<MAPBUF_DEPTH;k++)
				{
					Update_buffer[wY + k][wX] |= (1 << k);
				}
				wX++;
				if (wX >= MAPBUF_WIDTH)
					wX -= MAPBUF_WIDTH;
			}
			wY++;
			if (wY >= MAPBUF_HEIGHT)
				wY -= MAPBUF_HEIGHT;
		}
	}

	/* Clear counter */
	Nr_2D_rectangles = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  :
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.08.94 12:30
 * LAST      : 01.08.94 12:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fill_map_buffer(void)
{
	struct Icon_2D_data *Data;
	UNBYTE *Ptr, *Ptr2;
	UNSHORT B1,B2,B3;
	UNSHORT Underlay_nr,Overlay_nr;
	UNSHORT hUnder = 0, hOver;
	UNSHORT i,j,k;

	/* Calculate camera coordinates */
	Camera_2D_X = (UNSHORT)(Scroll_2D.Playfield_X >> 4);
	Camera_2D_Y = (UNSHORT)(Scroll_2D.Playfield_Y >> 4);

	/* Calculate pointer to map layer */
	Ptr = (UNBYTE *) (Map_ptr + 1)
	 + (sizeof(struct Square_2D) * (Camera_2D_X + (Camera_2D_Y * Map_width)));

	for (i=0;i<MAPBUF_HEIGHT;i++)
	{
		Ptr2 = Ptr;

		for (j=0;j<MAPBUF_WIDTH;j++)
		{
			for (k=0;k<MAPBUF_DEPTH;k++)
			{
				Map_buffer[i+k][j][k][0] = 0;
				Map_buffer[i+k][j][k][1] = 0;
			}

			/* Read bytes from map */
			B1 = (UNSHORT) *Ptr2++;
			B2 = (UNSHORT) *Ptr2++;
			B3 = (UNSHORT) *Ptr2++;

			/* Build overlay_nr and underlay_nr number */
			Underlay_nr = ((B2 & 0x0F) << 8) | B3;
			Overlay_nr = (B1 << 4) | (B2 >> 4);

			/* Any underlay_nr ? */
			if (Underlay_nr > 1)
			{
				/* Yes -> Get icon data */
				Data = &Icon_data[0][Underlay_nr-2];

				/* Get icon height */
				hUnder = (Data->Flags & ICON_HEIGHT) >> ICON_HEIGHT_B;

				/* Insert underlay_nr in map buffer */
				Map_buffer[i+hUnder][j][hUnder][0] = Underlay_nr;
			}

			/* Any overlay_nr ? */
			if (Overlay_nr > 1)
			{
				/* Yes -> Get icon data */
				Data = &Icon_data[0][Overlay_nr-2];

				/* Get icon height */
				hOver = (Data->Flags & ICON_HEIGHT) >> ICON_HEIGHT_B;

				/* Underlay_nr over overlay_nr ? */
				if (hUnder > hOver)
					/* Yes -> Bump up */
					hOver = hUnder;

				/* Insert overlay_nr in map buffer */
				Map_buffer[i+hOver][j][hOver][1] = Overlay_nr;
			}
		}
		Ptr += 3 * Map_width;
	}
}

