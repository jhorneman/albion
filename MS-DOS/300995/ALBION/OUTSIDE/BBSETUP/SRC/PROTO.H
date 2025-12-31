//------------------------------------------------------------------------------
//Proto.h - gesammelte Vorabdeklarationen fÅr's setup-programm:
//------------------------------------------------------------------------------

#include <bbdef.h>          //Die Typendeklarationen

//BaseIo.c:
SILONG GetLastdrive (void);
BOOL IsDriveReal (SILONG Drive);
BOOL IfExists (UNCHAR *Filename);
BOOL IfExistsFile (UNCHAR *Filename);
BOOL IfExistsDir (UNCHAR *Filename);
void CreateFile (UNCHAR *Filename);
//BOOL IfWindows (void);

//BaseWin.c:
void DrawArrow (struct OPM *pOpm, SILONG x, SILONG y, SILONG Color, SILONG Orientation);
void Rectangle (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c);
void Rectangle3d (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c1, SILONG c2, SILONG c3);
void Window3d (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG c1, SILONG c2, SILONG c3, SILONG c4);
void DrawButton (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2, SILONG FrameColor);
void DrawWinShadow (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2);
void PushButton (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2);
void UnPushButton (struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2);
void RefreshFixedPalette (void);
BOOL PrintTextInFrame (UNCHAR *Text, SILONG FontColor, struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2);
BOOL PrintCenteredTextInFrame (UNCHAR *Text, SILONG FontColor, struct OPM *pOpm, SILONG x1, SILONG y1, SILONG x2, SILONG y2);
SILONG PixelStringLength (UNCHAR *Text);
SILONG PixelCharLength (UNCHAR Zeichen);
void ScaleCopyOpm (struct OPM *pSourceOpm, struct OPM *pTargetOpm);

//Browser.c:
void BrowseFile (UNCHAR *Filename);
void LoadBrowseFile (SCRIPT *Script, UNCHAR *Filename);

//Debug.c:
void InternalError (SISHORT ErrNum, ...);
void DoDebugBreak (void);
UNCHAR *THMalloc (SILONG Size);

//Event.c:
void Update_input(void);
void Get_event(struct BLEV_Event_struct *Event);

//ini.c:
BOOL IniSet (UNCHAR *Filename, UNCHAR *Section, UNCHAR *Name, UNCHAR *Value);
BOOL BrutalIniSet (UNCHAR *Filename, UNCHAR *Section, UNCHAR *Name, UNCHAR *Value);
BOOL IniDelete (UNCHAR *Filename, UNCHAR *Section, UNCHAR *Name);
UNCHAR *IniRead (UNCHAR *Filename, UNCHAR *Section, UNCHAR *Name);
void UpdateIni (void);
void LoadIni (void);
SILONG BenchPixelSpeed (void);
void CalculateIniFilename (void);

//io.c:
void ChangeDirAndDriveTo (UNCHAR *String);
void CopyOneFile (UNCHAR *Source, UNCHAR *Target);
void ExecuteShell (SILONG AnzParameters, UNCHAR **Parameter);
void MakeDirs (UNCHAR *DirString);
void Copy (UNCHAR *Source, UNCHAR *Target, BOOL IncludeSubDirs);
void Install (UNCHAR *Source, UNCHAR *Target, BOOL IncludeSubDirs);
int __far HarderrHandler (unsigned deverror, unsigned errcode, unsigned far *devhdr);
void Delete (UNCHAR *Filespec);

//Setup.c:
void main (SILONG argc, SIBYTE *argv[]);
void Init_mouse_pointer(UNSHORT Width, UNSHORT Height, UNSHORT Hotspot_X, UNSHORT Hotspot_Y, UNBYTE *Graphics);
UNBYTE *Write_chunk_name(UNBYTE *Address, UNCHAR Chunk_name[]);

BOOLEAN Init_AIL_system(void);
void Exit_AIL_system(void);

//Script.c:
void 	 SKRIPT_Init (SCRIPT *Script, UNCHAR *ScriptFilename);
SILONG SCRIPT_GetTokenNumber (SCRIPT *Script, SILONG Linenumber, TOKEN *TokenTable);
SILONG SCRIPT_FindLabelLine (SCRIPT *Script, UNCHAR *Label);
SILONG SCRIPT_ProcessScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzOpenIf);
UNCHAR **SCRIPT_ParseScriptLine (SCRIPT *Script, SILONG Line, SILONG *AnzParameter);
SILONG SCRIPT_GetIntParameter (UNCHAR *Parameter, SILONG Line);
SILONG SCRIPT_ScanForIfComplement (SCRIPT *Script, SILONG Line, SILONG AllowedToken);
SILONG SCRIPT_ScanForOpenIfs (SCRIPT *Script, SILONG Line);

//VESA.c:
BOOL VESA_IsAvailable (void);
void *D32DosMemAlloc(long size);
BOOL VESA_UpdateIni (void);
BOOL VESA_IsInSupportList (short Videmode, short *SupportList);

//Window.c:
void ErrorWindow (UNCHAR *Text);
void InfoWindow (UNCHAR *Text);
void GeneralWindow (UNCHAR *Text, UNCHAR *Headline, UNCHAR *Button, UNCHAR Color, UNCHAR ColorLight, UNCHAR ColorDark, UNCHAR ColorHeadlineBack, UNCHAR ColorHeadlineText, UNCHAR ColorButtonFrame, UNCHAR ColorButtonHighlight);
BOOL AssertWindow (UNCHAR *Text);
SILONG GetButtonAnswer (SILONG AnzButtons, SILONG DefButton, ...);
SILONG GetMenuAnswer (MENU *pMenu, SILONG DefButton);
SILONG SelectionWindow (MENU *pMenu);
BOOL SelectTargetDrive (SILONG NeededKbFree);
BOOL SelectCDDrive (void);
UNCHAR *UserEntersString (UNCHAR *Headline, UNCHAR *DefString);
void AdministrateProgressWindow (SILONG NewState);
void EasyPrintWindow (UNCHAR *Text);

// Andreas Nitsche_Special
SILONG aSelectionWindow (MENU *pMenu);

