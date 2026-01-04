
/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_animation
 * FUNCTION  : Play an animation.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 14:08
 * LAST      : 11.08.95 16:18
 * INPUTS    : UNSHORT Flic_nr - Flic number (1...).
 *             SISHORT Playback_X - Screen X-coordinate.
 *             SISHORT Playback_Y - Screen Y-coordinate.
 *             UNSHORT Nr_repeats - Number of repeats.
 *             UNSHORT Play_mode - Animation play mode.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The last frame will be left on the screen to ensure smooth
 *              transitions.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Play_animation(UNSHORT Flic_nr, SISHORT Playback_X, SISHORT Playback_Y,
 UNSHORT Nr_repeats, UNSHORT Play_mode)
{
	struct Flic_playback FLC;
	struct BBRECT Clip, Old;
	struct OPM Background_OPM;
	MEM_HANDLE Flic_handle;
	MEM_HANDLE Background_handle;
	UNLONG T;
	UNLONG dT;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Exit if number of repeats is zero */
	if (!Nr_repeats)
		return;

	/* Load flic */
	Flic_handle = Load_subfile(FLIC, Flic_nr);
	if (!Flic_handle)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return;
	}

	Ptr = MEM_Claim_pointer(Flic_handle);

	/* Prepare playback */
	FLC.Output_frame			= &Main_OPM;
	FLC.Playback_X				= Playback_X;
	FLC.Playback_Y				= Playback_Y;
	FLC.Input_buffer			= Ptr;
	FLC.Palette					= &Palette;
	FLC.Black_colour_index	= BLACK;
	FLC.Trans_colour_index	= 0;

	FLC_Start_flic_playback(&FLC);

	/* Calculate delta time */
	dT = max(1, (FLC.Flic.speed * 6) / 100);

	/* Play outside of the map ? */
	if (Play_mode == OUTSIDE_MAP_PLAY_MODE)
	{
		/* Yes -> Build playback clipping rectangle */
		Clip.left	= max(Playback_X, 0);
		Clip.top		= max(Playback_Y, 0);
		Clip.width	= min(FLC.Width, Screen_width - Playback_X);
		Clip.height	= min(FLC.Height, Screen_height - Playback_Y);
	}
	else
	{
		/* No -> 2D or 3D map ? */
		if (_3D_map)
		{
			/* 3D map -> Relative to 3D map window */
			Playback_X += Window_3D_X;
			Playback_Y += Window_3D_Y;

			/* Build playback clipping rectangle */
			Clip.left	= max(Playback_X, Window_3D_X);
			Clip.top		= max(Playback_Y, Window_3D_Y);
			Clip.width	= min(FLC.Width, Window_3D_X + I3DM.Window_3D_width -
			 Playback_X);
			Clip.height	= min(FLC.Height, Window_3D_Y + I3DM.Window_3D_height -
			 Playback_Y);
		}
		else
		{
			/* 2D map -> Relative to 3D map window */
			Playback_X += MAP_2D_X;
			Playback_Y += MAP_2D_Y;

			/* Build playback clipping rectangle */
			Clip.left	= max(Playback_X, MAP_2D_X);
			Clip.top		= max(Playback_Y, MAP_2D_Y);
			Clip.width	= min(FLC.Width, MAP_2D_X + Map_2D_width - Playback_X);
			Clip.height	= min(FLC.Height, MAP_2D_Y + Map_2D_height - Playback_Y);
		}
	}

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Draw under or without 2D objects ? */
		if ((Play_mode == UNDER_2D_OBJECTS_PLAY_MODE) ||
		 (Play_mode == NO_2D_OBJECTS_PLAY_MODE))
		{
			/* Yes -> Reset 2D object list */
			Clear_2D_object_list();

			/* Draw 2D map */
			Draw_2D_map();
		}

		/* Prepare screen */
		Current_2D_OPM = &Main_OPM;
		Draw_2D_scroll_buffer();
		Current_2D_OPM = NULL;
	}

	/* Hide HDOBs */
	Hide_HDOBs();

	/* Make background buffer and OPM */
	Background_handle = MEM_Allocate_memory(Clip.width * Clip.height);

	Ptr = MEM_Claim_pointer(Background_handle);
	OPM_New(Clip.width, Clip.height, 1, &Background_OPM, Ptr);
	MEM_Free_pointer(Background_handle);

	/* Save background */
	OPM_CopyOPMOPM(&Main_OPM, &Background_OPM, Clip.left, Clip.top,
	 Clip.width, Clip.height, 0, 0);

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Draw under 2D objects ? */
		if (Play_mode == UNDER_2D_OBJECTS_PLAY_MODE)
		{
			/* Yes -> Reset 2D object list */
			Clear_2D_object_list();

			/* Add party objects */
			Display_2D_party();

			/* Add NPC objects */
			Display_2D_NPCs();
		}
	}

	/* Install clip area */
	memcpy(&Old, &(Main_OPM.clip), sizeof(struct BBRECT));
	memcpy(&(Main_OPM.clip), &Clip, sizeof(struct BBRECT));

	/* Repeat animation */
	for (i=0;i<Nr_repeats;i++)
	{
		/* Restart */
		FLC_Restart_flic();

		/* Play each frame */
		for (j=0;j<FLC.Nr_frames;j++)
		{
			/* Get time */
			T = SYSTEM_GetTicks();

			/* Restore background */
			OPM_CopyOPMOPM(&Background_OPM, &Main_OPM, 0, 0, Clip.width,
			 Clip.height, Clip.left, Clip.top);

			/* Current screen is 2D map / play over 2D objects ? */
			if ((Current_screen_type(0) == MAP_2D_SCREEN) &&
			 (Play_mode == OVER_2D_OBJECTS_PLAY_MODE))
			{
				/* Yes -> Redraw objects */
				Redraw_2D_objects();
			}

			/* Decode frame */
			FLC_Playback_flic_frame();

			/* Current screen is 2D map / play under 2D objects ? */
			if ((Current_screen_type(0) == MAP_2D_SCREEN) &&
			 (Play_mode == UNDER_2D_OBJECTS_PLAY_MODE))
			{
				/* Yes -> Redraw objects */
				Redraw_2D_objects();
			}

			/* Has the palette been changed ? */
			if (FLC.Palette_was_changed)
			{
				/* Yes -> Update palette */
				Update_palette();
			}

			/* Show frame */
			Switch_screens();

			/* Wait */
			while (SYSTEM_GetTicks() < T + dT);
		}

		/* Stop playback */
		FLC_Stop_flic_playback();
	}

	/* Restore clip area */
	memcpy(&(Main_OPM.clip), &Old, sizeof(struct BBRECT));

	/* Show HDOBs */
	Show_HDOBs();

	/* Destroy background buffer and OPM */
	OPM_Del(&Background_OPM);
	MEM_Free_memory(Background_handle);

	/* Remove flic */
	MEM_Free_pointer(Flic_handle);
	MEM_Free_memory(Flic_handle);
}

