{
	static SISHORT Extra_view_table[8][3][2] = {
		{{-1, -2}, {0, -2}, {1, -2}},		/* N */
		{{1, -2}, {2, -2}, {2, -1}},		/* NE */
		{{2, -1}, {2, 0}, {2, 1}},			/* E */
		{{2, 1}, {2, 2}, {1, 2}},			/* SE */
		{{-1, 2}, {0, 2}, {1, 2}},			/* S */
		{{-2, 1}, {-2, 2}, {-1, 2}},		/* SW */
		{{-2, -1}, {-2, 0}, {-2, 1}},		/* W */
		{{-2, -2}, {-1, -2}, {-2, -1}}	/* NW */
	};

	SISHORT X, Y, X2, Y2, X3, Y3;
	UNSHORT i, j;

	/* Can the party see ? */
	if (Party_can_see())
	{
		/* Yes -> Get party coordinates */
		X = PARTY_DATA.X;
		Y = PARTY_DATA.Y;

		/* Update current position */
		Set_automap_bit(X, Y);

		/* Update around player */
		for (i=0;i<8;i++)
		{
			/* Get coordinates for current direction */
			X2 = X + Offsets8[i][0];
			Y2 = Y + Offsets8[i][1];

			/* Update position */
			Set_automap_bit(X2, Y2);

			/* Vision blocked ? */
			if (!Vision_blocked(X2, Y2))
			{
				/* No -> Extra view */
				for (j=0;j<3;j++)
				{
					/* Get extra view coordinates */
					X3 = X2 + Extra_view_table[i][j][0];
					Y3 = Y2 + Extra_view_table[i][j][1];

					/* Update position */
					Set_automap_bit(X2, Y2);
				}
			}
		}
	}
}

