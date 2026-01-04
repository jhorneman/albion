
static UNSHORT Main_menu_button_objects[8];

/* Main menu buttons */
static union Button_data Main_menu_button_data[8];
static struct Button_OID Main_menu_button_OIDs[8] = {
	{
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Main_menu_button_data[0],
		Main_menu_Start_new_game
	},
	{
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Main_menu_button_data[1],
		Main_menu_Continue_game
	},
	{
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Main_menu_button_data[2],
		Main_menu_Load_game
	},
	{
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Main_menu_button_data[3],
		Main_menu_Save_game
	},
	{
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Main_menu_button_data[4],
		Main_menu_Options
	},
	{
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Main_menu_button_data[5],
		Main_menu_View_intro
	},
	{
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Main_menu_button_data[6],
		Main_menu_View_credits
	},
	{
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Main_menu_button_data[7],
		Main_menu_End_game
	},
};

	/* Add main menu buttons */
	for (i=0;i<8;i++)
	{
		/* Prepare button data */
		Main_menu_button_data[i].Text_button_data.Text =
		 System_text_ptrs[702 + i];

		/* Add button */
		Main_menu_button_objects[i] = Add_object
		(
			Earth_object,
			&Button_Class,
			(UNBYTE *) &Main_menu_button_OIDs[i],
			130,
			20 + (i * 20),
			100,
			17
		);

		/* Draw button */
		Execute_method(Main_menu_button_objects[i], DRAW_METHOD, NULL);
	}

	UNSHORT i;

	/* Delete main menu buttons */
	for (i=0;i<8;i++)
	{
		Delete_object(Main_menu_button_objects[i]);
	}

