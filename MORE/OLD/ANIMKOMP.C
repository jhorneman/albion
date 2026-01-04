
struct RGB {
	UNBYTE Red, Green, Blue;
};

struct Run_list_entry {
	UNSHORT Start, Length;
};

struct RGB Frame1[640000];
struct RGB Frame2[640000];
UNLONG Delta_frame[640000];
struct BBRECT Delta_bounding_box;

void
Calculate_delta_frame(void)
{
	UNLONG Index;
	SISHORT dRed, dGreen, dBlue;
	UNSHORT i, j;

	/* Calculate delta frame */
	Index = 0;
	for (i=0;i<Height;i++)
	{
		for (j=0;j<Width;j++)
		{
			dRed = Frame2[Index].Red - Frame1[Index].Red;
			dGreen = Frame2[Index].Green - Frame1[Index].Green;
			dBlue = Frame2[Index].Blue - Frame1[Index].Blue;

			Delta_frame[Index] = (dRed * dRed) + (dGreen * dGreen) + (dBlue * dBlue);

			Index++;
		}
	}
}

void
Determine_delta_bounding_box(void)
{
	UNSHORT i, j;

	/* Determine left edge of bounding box of delta frame */
	for (i=0;i<Width;i++)
	{
		for (j=0;j<Height;j++)
		{
			if (Delta_frame[j * Width + i])
				break;
		}
		if (Delta_frame[j * Width + i])
			break;
	}
	Delta_bounding_box.left = i;

	/* Determine right edge of bounding box of delta frame */
	for (i=Width-1;i>=0;i--)
	{
		for (j=0;j<Height;j++)
		{
			if (Delta_frame[j * Width + i])
				break;
		}
		if (Delta_frame[j * Width + i])
			break;
	}
	Delta_bounding_box.width = i - Delta_bounding_box.left + 1;

	/* Determine top edge of bounding box of delta frame */
	for (i=0;i<Height;i++)
	{
		for (j=0;j<Width;j++)
		{
			if (Delta_frame[i * Width + j])
				break;
		}
		if (Delta_frame[i * Width + j])
			break;
	}
	Delta_bounding_box.top = i;

	/* Determine bottom edge of bounding box of delta frame */
	for (i=Height-1;i>=0;i--)
	{
		for (j=0;j<Width;j++)
		{
			if (Delta_frame[i * Width + j])
				break;
		}
		if (Delta_frame[i * Width + j])
			break;
	}
	Delta_bounding_box.height = i - Delta_bounding_box.top + 1;
}

void
Horizontally_subdivide_bounding_box(struct BBRECT Bounding_box)
{
	struct Run_list_entry Run_list[320];
	struct BBRECT Sub_box;
	UNSHORT Full_run_length = 0, Empty_run_length = 0, Index = 0;
	UNSHORT Run_X;
	UNSHORT i, j;
	UNBYTE VFlags[320];

	/* Scan all V-lines to determine if they are full or empty */
	for (i=Bounding_box.left;i<Bounding_box.left + Bounding_box.width - 1;i++)
	{
		VFlags[i] = 0;
		for (j=Bounding_box.top;j<Bounding_box.top + Bounding_box.height - 1;j++)
		{
			if (Delta_frame[j * Width + i])
			{
				VFlags[i] = 0xFF;
				break;
			}
		}
	}

	/* Run-length-encode the V-line full/empty status array */
	Run_X = Bounding_box.left;
	for (i=Bounding_box.left;i<Bounding_box.left + Bounding_box.width - 1;i++)
	{
		/* Is this V-line full or empty ? */
		if (VFlags[i])
		{
			/* Full -> Were we already encoding a full run ? */
			if (Full_run_length)
			{
				/* Yes -> Increase full run length */
				Full_run_length++;
			}
			else
			{
				/* No -> Were we encoding an empty run ? */
				if (Empty_run_length)
				{
					/* Yes -> Reset empty run length */
					Empty_run_length = 0;
				}
				/* Begin a new full run */
				Run_X = i;
				Full_run_length = 1;
			}
		}
		else
		{
			/* Empty -> Were we already encoding an empty run ? */
			if (Empty_run_length)
			{
				/* Yes -> Increase empty run length */
				Empty_run_length++;
			}
			else
			{
				/* No -> Were we encoding a full run ? */
				if (Full_run_length)
				{
					/* Yes -> Add full run data to list */
					Run_list[Index].Start = Run_X;
					Run_list[Index].Length = Full_run_length;
					Index++;

					/* Reset full run length */
					Full_run_length = 0;
				}
				/* Begin a new empty run */
				Empty_run_length = 1;
			}
		}
	}
	/* Were we encoding a full run ? */
	if (Full_run_length)
	{
		/* Yes -> Add full run data to list */
		Run_list[Index].Start = Run_X;
		Run_list[Index].Length = Full_run_length;
		Index++;
	}

	/* Try to vertically sub-divide all runs */
	Sub_box.top = Bounding_box.top;
	Sub_box.height = Bounding_box.height;
	for(i=0;i<Index;i++)
	{
		/* Build bounding box */
		Sub_box.left = Run_list[i].Start;
		Sub_box.width = Run_list[i].Length;

		/* Try to sub-divide it */
		Vertically_subdivide_bounding_box(&Sub_box);
	}
}

void
Vertically_subdivide_bounding_box(struct BBRECT Bounding_box)
{
	struct Run_list_entry Run_list[200];
	struct BBRECT Sub_box;
	UNSHORT Full_run_length = 0, Empty_run_length = 0, Index = 0;
	UNSHORT Run_Y;
	UNSHORT i, j;
	UNBYTE HFlags[200];

	/* Scan all H-lines to determine if they are full or empty */
	for (i=Bounding_box.top;i<Bounding_box.top + Bounding_box.height - 1;i++)
	{
		HFlags[i] = 0;
		for (j=Bounding_box.left;j<Bounding_box.left + Bounding_box.width - 1;j++)
		{
			if (Delta_frame[i * Width + j])
			{
				HFlags[i] = 0xFF;
				break;
			}
		}
	}

	/* Run-length-encode the H-line full/empty status array */
	Run_Y = Bounding_box.top;
	for (i=Bounding_box.top;i<Bounding_box.top + Bounding_box.height - 1;i++)
	{
		/* Is this H-line full or empty ? */
		if (HFlags[i])
		{
			/* Full -> Were we already encoding a full run ? */
			if (Full_run_length)
			{
				/* Yes -> Increase full run length */
				Full_run_length++;
			}
			else
			{
				/* No -> Were we encoding an empty run ? */
				if (Empty_run_length)
				{
					/* Yes -> Reset empty run length */
					Empty_run_length = 0;
				}
				/* Begin a new full run */
				Run_Y = i;
				Full_run_length = 1;
			}
		}
		else
		{
			/* Empty -> Were we already encoding an empty run ? */
			if (Empty_run_length)
			{
				/* Yes -> Increase empty run length */
				Empty_run_length++;
			}
			else
			{
				/* No -> Were we encoding a full run ? */
				if (Full_run_length)
				{
					/* Yes -> Add full run data to list */
					Run_list[Index].Start = Run_Y;
					Run_list[Index].Length = Full_run_length;
					Index++;

					/* Reset full run length */
					Full_run_length = 0;
				}
				/* Begin a new empty run */
				Empty_run_length = 1;
			}
		}
	}
	/* Were we encoding a full run ? */
	if (Full_run_length)
	{
		/* Yes -> Add full run data to list */
		Run_list[Index].Start= Run_Y;
		Run_list[Index].Length = Full_run_length;
		Index++;
	}

	/* Try to horizontally sub-divide all runs */
	Sub_box.left = Bounding_box.left;
	Sub_box.width = Bounding_box.width;
	for(i=0;i<Index;i++)
	{
		/* Build bounding box */
		Sub_box.top = Run_list[i].Start;
		Sub_box.height = Run_list[i].Length;

		/* Try to sub-divide it */
		Horizontally_subdivide_bounding_box(&Sub_box);
	}
}

