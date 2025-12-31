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
#include <3D_MAP.H>

/* global variables */

UNCHAR Filenames[][20+13] = {
	"XLDLIBS\\MAP_DATA.XLD",
	"XLDLIBS\\ICONDATA.XLD",
	"XLDLIBS\\ICON_GFX.XLD",
	"XLDLIBS\\PALETTES.XLD",
	"XLDLIBS\\PALETTE.000",
	"XLDLIBS\\SLAB",
	"XLDLIBS\\PART1GFX.XLD",
	"XLDLIBS\\PART2GFX.XLD",
	"XLDLIBS\\LAB_DATA.XLD",
	"XLDLIBS\\3DWALLS.XLD",
	"XLDLIBS\\3DOBJECT.XLD",
	"XLDLIBS\\3DOVERL.XLD",
	"XLDLIBS\\3DFLOORs.XLD",
	"XLDLIBS\\SHADETAB.XLD",
	"XLDLIBS\\BACKGFX.XLD",
	"XLDLIBS\\FONTS.XLD",
	"XLDLIBS\\BLOKLIST.XLD"
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
		Convert_lab_data
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

