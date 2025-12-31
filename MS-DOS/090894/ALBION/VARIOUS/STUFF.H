

/*****************************************************************************
 [ Remove item(s) from a character ]
   IN : Character_data_handle - Character data´s memory handle
        Slot_number - Slot where the items should be removed from (1...33)
        Quantity - Number of items that should be removed (backpack items only)
  Notes :
    - This function does not check whether Slot_number is legal.
    - Nothing is redrawn. This function is meant as a general, safe method of adding items.
    - No event actions or cursed item logic is handled here.
 *****************************************************************************/
void Remove_item(UNSHORT Character_data_handle, UNBYTE Slot_number, UNBYTE Quantity)
{
	struct Character_data *Char;
	struct Item_packet *Packet;
	struct Item_data *Item;

	Char = (struct Character_data *) Claim_pointer(Character_data_handle);
	Packet = &(Char->Inventory[Slot_number-1]);			/* Get pointer to item packet */
	Item = &Item_data[Packet->Object_index-1];			/* Get pointer to item data */

	if (Slot_number<=9)									/* Is it a body item slot ? */
	{
		Char->Hands_occupied -= Item->Hands_used;		/* Decrease number of hands */

		for (i=0;i++;i<2)								/* Skill taxes (if any) */
		{
			j = Item ->Taxed_skill_nr[i];
			if (j)
				Char->Skills[j-1].Magic -= Item->Skill_tax[i];
		};

		if ((Item->Static_item_bits) & Cursed_item))	/* Remove negative or positive boni */
			Positive_boni(Char,Item);
		else
			Negative_boni(Char,Item);

		Quantity = 1;									/* Just to be sure */
	}

	Packet->Quantity -= Quantity;							/* Remove item(s) from packet */
	if (!Packet->Quantity)
		Packet->Object_index = 0;
	Char->Carried_weight -= (Item->Weight)*Quantity;		/* Remove weight */

	Free_pointer(Character_data_handle);
}

/*****************************************************************************
 [ Equip a character with an item ]
   IN : Character_data_handle - Character data´s memory handle
        Slot_number - Slot where the items should be equipped
        Source_packet - Pointer to item packet containing the item
  Notes :
    - This function does not check whether Slot_number is legal.
    - Nothing is redrawn. This function is meant as a general, safe method of adding items.
    - One item will be removed from the source packet, which will be destroyed if empty.
 *****************************************************************************/
void Add_item_to_body(UNSHORT Character_data_handle, UNBYTE Slot_number, struct Item_packet *Source_packet)
{
	struct Character_data *Char;
	struct Item_data *Item;

	Char = (struct Character_data *) Claim_pointer(Character_data_handle);

	Char->Body_items[Slot_number] = *Source_packet;		/* Copy item packet */
	Char->Body_items[Slot_number].Quantity = 1;

	Item = &Item_data[Source_packet->Object_index-1];	/* Get pointer to item data */

	Char->Hands_occupied += Item->Hands_used;			/* Increase number of hands */

	for (i=0;i++;i<2)									/* Skill taxes (if any) */
	{
		j = Item ->Taxed_skill_nr[i];
		if (j)
			Char->Skills[j-1].Magic -= Item->Skill_tax[i];
	};

	if ((Item->Static_item_bits) & Cursed_item))		/* Give negative or positive boni */
		Negative_boni(Char,Item);
	else
		Positive_boni(Char,Item);

	Char->Carried_weight += Item->Weight;				/* Add weight */

	if (!(--(Source_packet->Quantity)))
		Source_packet->Object_index = 0;

	Free_pointer(Character_data_handle);
};

/*****************************************************************************
 [ Give a character boni when an item is equipped ]
   IN : Char - Pointer to character data
        Item - Pointer to item data
 *****************************************************************************/
void Positive_boni(struct Character_data *Char, struct Item_data *Item)
{
	int i;

	/* LP & SP maximum bonus */
	Char->Life_points_magic += Item->LP_max_bonus;
	Char->Spell_points_magic += Item->SP_max_bonus;

	/* Attribute bonus (if any) */
	i = Item->Attribute_nr;
	if (i)
		Char->Attributes[i-1].Magic += Item->Attribute_bonus;

	/* Skill bonus (if any) */
	i = Item ->Skill_nr;
	if (i)
		Char->Skills[i-1].Magic += Item->Skill_bonus;

	/* Damage & protection bonus */
	Char->Damage_magic += Item->Damage_bonus;
	Char->Protection_magic += Item->Protection_bonus;
};

/*****************************************************************************
 [ Give a character negative boni when a cursed item is equipped ]
   IN : Char - Pointer to character data
        Item - Pointer to item data
 *****************************************************************************/
void Negative_boni(struct Character_data *Char, struct Item_data *Item)
{
	int i;

	/* LP & SP maximum bonus */
	Char->Life_points_magic -= Item->LP_max_bonus;
	Char->Spell_points_magic -= Item->SP_max_bonus;

	/* Attribute bonus (if any) */
	i = Item ->Attribute_nr;
	if (i)
		Char->Attributes[i-1].Magic -= Item->Attribute_bonus;

	/* Skill bonus (if any) */
	i = Item ->Skill_nr;
	if (i)
		Char->Skills[i-1].Magic -= Item->Skill_bonus;

	/* Damage & protection bonus */
	Char->Damage_magic -= Item->Damage_bonus;
	Char->Protection_magic -= Item->Protection_bonus;
};

