
UNLONG Customkeys_Tactical_window_object(struct Object *Object, union Method_parms *P);

	{ CUSTOMKEY_METHOD,	Customkeys_Tactical_window_object },

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_Tactical_window_object
 * FUNCTION  : Customkeys method of Tactical window object.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 13:07
 * LAST      : 14.03.95 13:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Customkeys_Tactical_window_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Key_code;
	UNSHORT i;

	/* Get key-code */
	Key_code = (UNSHORT) (P->Event->sl_key_code);

	switch (Key_code)
	{
		case BLEV_UP:
			if (Combat_camera_height < 1000)
				Combat_camera_height += 1;
			break;
		case BLEV_DOWN:
			if (Combat_camera_height > 0)
				Combat_camera_height -= 1;
			break;
		case BLEV_LEFT:
			if (Combat_projection_factor < 32768)
				Combat_projection_factor += 8;
			break;
		case BLEV_RIGHT:
			if (Combat_projection_factor > 16)
				Combat_projection_factor -= 8;
			break;
		case 'q':
			if (Combat_Z_offset < 1024)
				Combat_Z_offset += 8;
			break;
		case 'a':
			if (Combat_Z_offset > (0 - 4096))
				Combat_Z_offset -= 8;
			break;

		#if FALSE
		case 'w':
			if (Combat_square_width < 100 * COMOB_DEC_FACTOR)
			{
				Combat_square_width += COMOB_DEC_FACTOR;

				for (i=0;i<Nr_monsters;i++)
				{
					Convert_tactical_to_3D_coordinates(Monster_parts[i].Tactical_X,
					 Monster_parts[i].Tactical_Y, &(Monster_parts[i].Main_COMOB->X_3D),
					  &(Monster_parts[i].Main_COMOB->Z_3D));
					Monster_parts[i].Main_COMOB->Y_3D = 0;

					Monster_parts[i].Shadow_COMOB->X_3D = Monster_parts[i].Main_COMOB->X_3D;
					Monster_parts[i].Shadow_COMOB->Z_3D = Monster_parts[i].Main_COMOB->Z_3D;
					Monster_parts[i].Shadow_COMOB->Y_3D = 0;
				}
			}
			break;
		case 's':
			if (Combat_square_width > 10 * COMOB_DEC_FACTOR)
			{
				Combat_square_width -= COMOB_DEC_FACTOR;

				for (i=0;i<Nr_monsters;i++)
				{
					Convert_tactical_to_3D_coordinates(Monster_parts[i].Tactical_X,
					 Monster_parts[i].Tactical_Y, &(Monster_parts[i].Main_COMOB->X_3D),
					  &(Monster_parts[i].Main_COMOB->Z_3D));
					Monster_parts[i].Main_COMOB->Y_3D = 0;

					Monster_parts[i].Shadow_COMOB->X_3D = Monster_parts[i].Main_COMOB->X_3D;
					Monster_parts[i].Shadow_COMOB->Z_3D = Monster_parts[i].Main_COMOB->Z_3D;
					Monster_parts[i].Shadow_COMOB->Y_3D = 0;
				}
			}
			break;
		case 'e':
			if (Combat_square_depth < 100 * COMOB_DEC_FACTOR)
			{
				Combat_square_depth += COMOB_DEC_FACTOR;

				for (i=0;i<Nr_monsters;i++)
				{
					Convert_tactical_to_3D_coordinates(Monster_parts[i].Tactical_X,
					 Monster_parts[i].Tactical_Y, &(Monster_parts[i].Main_COMOB->X_3D),
					  &(Monster_parts[i].Main_COMOB->Z_3D));
					Monster_parts[i].Main_COMOB->Y_3D = 0;

					Monster_parts[i].Shadow_COMOB->X_3D = Monster_parts[i].Main_COMOB->X_3D;
					Monster_parts[i].Shadow_COMOB->Z_3D = Monster_parts[i].Main_COMOB->Z_3D;
					Monster_parts[i].Shadow_COMOB->Y_3D = 0;
				}
			}
			break;
		case 'd':
			if (Combat_square_depth > 10 * COMOB_DEC_FACTOR)
			{
				Combat_square_depth -= COMOB_DEC_FACTOR;

				for (i=0;i<Nr_monsters;i++)
				{
					Convert_tactical_to_3D_coordinates(Monster_parts[i].Tactical_X,
					 Monster_parts[i].Tactical_Y, &(Monster_parts[i].Main_COMOB->X_3D),
					  &(Monster_parts[i].Main_COMOB->Z_3D));
					Monster_parts[i].Main_COMOB->Y_3D = 0;

					Monster_parts[i].Shadow_COMOB->X_3D = Monster_parts[i].Main_COMOB->X_3D;
					Monster_parts[i].Shadow_COMOB->Z_3D = Monster_parts[i].Main_COMOB->Z_3D;
					Monster_parts[i].Shadow_COMOB->Y_3D = 0;
				}
			}
			break;
		#endif

		case 't':
			if (Combat_grid_flag)
			{
				Draw_tactical_window_flag = TRUE;
				Combat_grid_flag = FALSE;
			}
			else
			{
				Draw_tactical_window_flag = FALSE;
				Combat_grid_flag = TRUE;
			}

			Draw_combat_screen();
			Get_tactical_window_background();
			Update_screen();

			break;
		#if FALSE
		case '1':
			Do_explosion(0, 100 * COMOB_DEC_FACTOR, 180, 180);
			break;
		case '2':
		{
			UNSHORT Select;
			UNSHORT j;

			Select = rand() % Nr_monsters;

			for (i=0;i<3;i++)
			{
				for (j=0;j<NR_TACTICAL_COLUMNS;j++)
				{
					if (Combat_matrix[i][j].Part)
					{
						if (Select)
							Select--;
						else
						{
							Do_fireball(Combat_matrix[i][j].Part);
							return;
						}
					}
				}
			}
			break;
		}
		#endif
		case '3':
			End_combat_flag = TRUE;
			break;
	}
}

