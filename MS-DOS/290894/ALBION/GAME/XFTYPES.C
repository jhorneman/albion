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
	"C:\\ALBION\\XLDLIBS\\PART1GFX.XLD",
	"C:\\ALBION\\XLDLIBS\\PART2GFX.XLD",
	"C:\\ALBION\\XLDLIBS\\LAB_DATA.XLD",
	"C:\\ALBION\\XLDLIBS\\3DWALLS.XLD",
	"C:\\ALBION\\XLDLIBS\\3DOBJECT.XLD",
	"C:\\ALBION\\XLDLIBS\\3DOVERL.XLD",
	"C:\\ALBION\\XLDLIBS\\3DFLOORs.XLD",
	"C:\\ALBION\\XLDLIBS\\SHADETAB.XLD",
	"C:\\ALBION\\XLDLIBS\\BACKGFX.XLD",
	"C:\\ALBION\\XLDLIBS\\FONTS.XLD",
	"C:\\ALBION\\XLDLIBS\\BLOKLIST.XLD"
};

UNCHAR Ftypenames[][32] = {
  	"Map data",
  	"Icon data",
  	"Icon graphics",
  	"Palette",
  	"",
  	"Slab",
  	"Big party graphics",
  	"Small party graphics",
	"Lab data",
	"3D wall",
	"3D object",
	"3D overlay",
	"3D floor",
	"Shade table",
	"Background graphics",
	"Font",
	"Block list"
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
			&Ftypenames[PART1_GFX]
		},
		&Filenames[PART1_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[PART2_GFX]
		},
		&Filenames[PART2_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[LAB_DATA]
		},
		&Filenames[LAB_DATA],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[WALL_3D]
		},
		&Filenames[WALL_3D],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[OBJECT_3D]
		},
		&Filenames[OBJECT_3D],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[OVERLAY_3D]
		},
		&Filenames[OVERLAY_3D],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[FLOOR_3D]
		},
		&Filenames[FLOOR_3D],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[SHADE_TABLE]
		},
		&Filenames[SHADE_TABLE],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[BACKGROUND_GFX]
		},
		&Filenames[BACKGROUND_GFX],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[FONTS]
		},
		&Filenames[FONTS],
		NULL
	},

	{
		{
			MEM_Relocate,
			0xFF,
			128,
			0,
			&Ftypenames[BLOCK_LIST]
		},
		&Filenames[BLOCK_LIST],
		NULL
	}
};

