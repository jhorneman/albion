
#define Length (ULONG) (10*39+2)

BPTR f;
UBYTE *p;

main()
{
	p = AllocMem(Length,MEMF_CLEAR);

	f = Open("RAM:Saves",MODE_NEWFILE);
	Write(f,p,Length);
	Close(f);

	FreeMem(p,Length);

	exit(0);
}
