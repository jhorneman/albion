
void Evaluate_game_arguments(UNSHORT argc, UNCHAR **argv);

	/* Evaluate arguments */
	Evaluate_game_arguments(argc, (UNCHAR **) argv);

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Evaluate_game_arguments
 * FUNCTION  : Evaluate game arguments.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 10:35
 * LAST      : 15.08.95 22:47
 * INPUTS    : UNSHORT argc - Number of arguments.
 *             UNCHAR **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Evaluate_game_arguments(UNSHORT argc, UNCHAR **argv)
{
	static UNCHAR *Commands[NR_GAME_COMMANDS] = {
		"map",
		"svga",
		"x",
		"y",
		"diag",
		"language",
		"noload",
		"vd",
		"combat",
		"demo",
		"cheat",
		"load",
	};

	UNSHORT Argument_index;
	UNSHORT Command_index;
	UNSHORT Value;

	/* Check all arguments */
	for (Argument_index=1;Argument_index<argc;Argument_index++)
	{
		/* Parse current argument */
		Command_index = Parse_argument(argv[Argument_index], NR_GAME_COMMANDS,
		 Commands);

		/* Act depending on command index */
		switch (Command_index)
		{
			case 0:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
				{
					PARTY_DATA.Map_nr = Value;
					PARTY_DATA.X = 10;
					PARTY_DATA.Y = 10;
				}
				break;
			}
			case 1:
			{
				Super_VGA = TRUE;
				break;
			}
			case 2:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
					PARTY_DATA.X = Value;
				break;
			}
			case 3:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
					PARTY_DATA.Y = Value;
				break;
			}
			case 4:
			{
				Diagnostic_mode = TRUE;
				break;
			}
			case 5:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);

				if (Value < MAX_LANGUAGES)
					Language = Value;

				break;
			}
			case 6:
			{
				No_load_flag = TRUE;
				break;
			}
			case 7:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				PARTY_DATA.View_direction = Value;
				break;
			}
			case 9:
			{
				PARTY_DATA.Map_nr = 199;
				PARTY_DATA.X = 10;
				PARTY_DATA.Y = 10;
				PARTY_DATA.View_direction = 1;

				PARTY_DATA.Member_nrs[1] = 2;
				PARTY_DATA.Member_nrs[2] = 5;
				PARTY_DATA.Member_nrs[3] = 4;
				PARTY_DATA.Member_nrs[4] = 6;
				PARTY_DATA.Member_nrs[5] = 9;

				PARTY_DATA.Battle_order[0] = 1;
				PARTY_DATA.Battle_order[1] = 2;
				PARTY_DATA.Battle_order[2] = 3;
				PARTY_DATA.Battle_order[3] = 4;
				PARTY_DATA.Battle_order[4] = 5;

				break;
			}
			case 10:
			{
				Cheat_mode = TRUE;
				break;
			}
			case 11:
			{
				Argument_index++;
				Load_game_nr = Parse_value(argv[Argument_index]);
				break;
			}
		}
	}
}

