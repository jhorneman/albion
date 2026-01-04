#include <dos.h>
#include	<fcntl.h>

void
Play_intro_sound(void)
{
	MEM_HANDLE Handle;
	HSAMPLE Sample_handle;
	int File_handle;
	UNLONG File_length;
	UNBYTE *Ptr;

	File_handle = open
	(
		"G:\\ALBION\\OUTSIDE\\INTRO\\CDALBION.WAV",
		O_RDONLY | O_BINARY
	);

	File_length = filelength(File_handle);

	Handle = MEM_Allocate_memory(File_length);

	Ptr = MEM_Claim_pointer(Handle);

	read
	(
		File_handle,
		Ptr,
		File_length
	);

	close(File_handle);

	Sample_handle = AIL_allocate_file_sample
	(
		Digital_driver,
		Ptr,
		0
	);

   AIL_start_sample(Sample_handle);
}

				Play_intro_sound();

				Close_screen();
//				SYSTEM_Exit();

				spawnl
				(
					P_WAIT,
					"G:\\PROJEKTE\\SMACKER.20\\SMACKPLY.EXE",
					"SMACKPLY",
					"G:\\ALBION\\OUTSIDE\\INTRO\\INTRO.SMK",
					NULL
				);

//				SYSTEM_Init ();
				Open_screen();

