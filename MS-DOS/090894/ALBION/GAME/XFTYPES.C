/************
 * NAME     : XFTYPES.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 27.07.94 16:46
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <XLOAD.H>
#include <XFTYPES.H>

/* global variables */

UNCHAR Filenames[][20+13] = {
	"C:\\ALBION\\XLDLIBS\\MAP_DATA.XLD",
	"C:\\ALBION\\XLDLIBS\\ICONDATA.XLD",
	"C:\\ALBION\\XLDLIBS\\ICON_GFX.XLD",
	"C:\\ALBION\\XLDLIBS\\PALETTES.XLD",
	"C:\\ALBION\\XLDLIBS\\PALETTE.000",
	"C:\\ALBION\\XLDLIBS\\SLAB",
	"C:\\ALBION\\XLDLIBS\\PARTYGFX.XLD"
};

UNCHAR Ftypenames[][20] = {
  "Map data",
  "Icon data",
  "Icon graphics",
  "Palette",
  "",
  "Slab"
  "Party graphics"
};

struct XFile_type Xftypes[] = {
	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[MAP_DATA]
		},
		&Filenames[MAP_DATA],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[ICON_DATA]
		},
		&Filenames[ICON_DATA],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[ICON_GFX]
		},
		&Filenames[ICON_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[PALETTE]
		},
		&Filenames[PALETTE],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			0,
			MEM_KILL_ALWAYS,
			NULL
		},
		&Filenames[BASEPAL],
		NULL
 	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[SLAB]
		},
		&Filenames[SLAB],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[PARTY_GFX]
		},
		&Filenames[PARTY_GFX],
		NULL
	}
};

