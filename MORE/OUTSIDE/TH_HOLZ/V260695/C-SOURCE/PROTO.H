//------------------------------------------------------------------------------
//Proto.h - gesammelte Vorabdeklarationen fÅr's setup-programm:
//------------------------------------------------------------------------------

//BaseIo.c:
BOOL IsDriveReal (SILONG Drive);
BOOL IfExists (char *Filename);
BOOL IfExistsFile (char *Filename);
BOOL IfExistsDir (char *Filename);
void CreateFile (char *Filename);

//Browser.c:
void BrowseFile (char *Filename);
void LoadBrowseFile (SCRIPT *Script, char *Filename);

//Debug.c:
void	InternalError (SISHORT ErrNum, ...);
void	DoDebugBreak (void);
char *THMalloc (SILONG Size);

//Event.c:
void Update_input(void);
void Get_event(struct BLEV_Event_struct *Event);

//grafix.c:
void Rectangle3d (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c1, SILONG c2, SILONG c3);
void Window3d (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c1, SILONG c2, SILONG c3, SILONG c4);

//ini.c:
BOOL IniSet (char *Filename, char *Name, char *Value);
BOOL IniDelete (char *Filename, char *Name);
char *IniRead (char *Filename, char *Name);
void UpdateIni (void);
void LoadIni (void);
SILONG BenchPixelSpeed (void);

//io.c:
void ChangeDirAndDriveTo (char *String);
void CopyOneFile (char *Source, char *Target);
void ExecuteShell (SILONG AnzParameters, char **Parameter);
void MakeDirs (char *DirString);
void Copy (char *Source, char *Target, BOOL IncludeSubDirs);
void Install (char *Source, char *Target, BOOL IncludeSubDirs);
int __far HarderrHandler (unsigned deverror, unsigned errcode, unsigned far *devhdr);
void Delete (char *Filespec);

//Setup.c:
void main (SILONG argc, SIBYTE *argv[]);

//Script.c:
void 	 SKRIPT_Init (SCRIPT *Script, char *ScriptFilename);
SILONG SCRIPT_GetTokenNumber (SCRIPT *Script, SILONG Linenumber, TOKEN *TokenTable);
SILONG SCRIPT_FindLabelLine (SCRIPT *Script, char *Label);
SILONG SCRIPT_ProcessScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzOpenIf);
char **SCRIPT_ParseScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzParameter);
SILONG SCRIPT_GetIntParameter (char *Parameter, SILONG Line);
SILONG SCRIPT_ScanForIfComplement (SCRIPT *Script, SILONG Line, SILONG AllowedToken);
SILONG SCRIPT_ScanForOpenIfs (SCRIPT *Script, SILONG Line);

//VESA.c:
BOOL VESA_IsAvailable (void);
void *D32DosMemAlloc(long size);
BOOL VESA_UpdateIni (void);
BOOL VESA_IsInSupportList (short Videmode, short *SupportList);

//Window.c:
void ErrorWindow (char *Text);
BOOL AssertWindow (char *Text);
BOOL PrintTextInFrame (char *Text, SILONG FontColor, struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2);
SILONG PixelStringLength (char *Text);
SILONG PixelCharLength (char Zeichen);
SILONG GetButtonAnswer (SILONG AnzButtons, SILONG DefButton, ...);
SILONG GetMenuAnswer (MENU *pMenu, SILONG DefButton);
SILONG SelectionWindow (MENU *pMenu);
void SelectTargetDrive (SILONG NeededKbFree);
void SelectCDDrive (void);
char *UserEntersString (char *Headline, char *DefString);
void AdministrateProgressWindow (SILONG NewState);
void EasyPrintWindow (char *Text);
