/*--------------------------------------------------------------------------------*/
/* Playback example: Play Stereo Sinus in selected Voicemeeter input Strips       */
/*--------------------------------------------------------------------------------*/
/* 'C' Sample Code to use Voicemeeter as Soundboard          V.Burel (c)2016-2021 */
/*                                                                                */
/*  THIS PROGRAM PLAYS BACB A 220 Hz SINUS -20 dB IN SINGLE STRIP (SELECTABLE)    */
/*                                                                                */
/*  This program example shows                                                    */
/*  - How to link VoicemeeterRemote.dll                                           */
/*  - How to Login / logout                                                       */
/*  - How to install Audio Callback as input insert.                              */
/*  - How to playback a simple sound in a selected input steip.                   */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  COMPILATION DIRECTIVES:                                                       */
/*                                                                                */
/*  To compile With Microsoft VC2005 or higher, you need to create an             */
/*  empty Win32 project with the following options:                               */
/*  - Configuration Properties / General : Char Set = NOT SET                     */
/*  - Configuration Properties / C/C++ / Preprocessor : _CRT_SECURE_NO_DEPRECATE  */
/*                                                                                */
/*  This source code can be compiled for 32bit or 64 bits targets as well         */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  LICENSING: VoicemeeterRemote.dll usage is driven by a license agreement       */
/*             given in VoicemeeterRemoteAPI.pdf or readme.txt                    */
/*                                                                                */
/*--------------------------------------------------------------------------------*/

#ifndef __cplusplus
	#ifndef STRICT
		#define STRICT
	#endif
#endif

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "vmr_play.h"
#include "VoicemeeterRemote.h"




/*******************************************************************************/
/**                           GET VOICEMEETER DIRECTORY                       **/
/*******************************************************************************/

static char uninstDirKey[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

#define INSTALLER_UNINST_KEY	"VB:Voicemeeter {17359A74-1236-5467}"


void RemoveNameInPath(char * szPath)
{
	long ll;
	ll=(long)strlen(szPath);
	while ((ll>0) && (szPath[ll]!='\\')) ll--;
	if (szPath[ll] == '\\') szPath[ll]=0;
}

#ifndef KEY_WOW64_32KEY
	#define KEY_WOW64_32KEY 0x0200
#endif

BOOL __cdecl RegistryGetVoicemeeterFolder(char * szDir)
{
	char szKey[256];
	char sss[1024];
	DWORD nnsize=1024;
	HKEY hkResult;
	LONG rep;
	DWORD pptype=REG_SZ;
	sss[0]=0;

	// build Voicemeeter uninstallation key
	strcpy(szKey,uninstDirKey);
	strcat(szKey,"\\");
	strcat(szKey,INSTALLER_UNINST_KEY);

	// open key
	rep=RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKey,0, KEY_READ, &hkResult);
	if (rep != ERROR_SUCCESS)
	{
		// if not present we consider running in 64bit mode and force to read 32bit registry
		rep=RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKey,0, KEY_READ | KEY_WOW64_32KEY, &hkResult); 
	}
	if (rep != ERROR_SUCCESS) return FALSE;
	// read uninstall profram path
	rep=RegQueryValueEx(hkResult,"UninstallString",0,&pptype,(unsigned char *)sss,&nnsize);
	RegCloseKey(hkResult);
	
	if (pptype != REG_SZ) return FALSE;
	if (rep != ERROR_SUCCESS) return FALSE;
	// remove name to get the path only
	RemoveNameInPath(sss);
	if (nnsize>512) nnsize=512;
	strncpy(szDir,sss,nnsize);
	
	return TRUE;
}






/*******************************************************************************/
/**                                GET DLL INTERFACE                          **/
/*******************************************************************************/

static HMODULE G_H_Module=NULL;
static T_VBVMR_INTERFACE iVMR;

//if we directly link source code (for development only)
#ifdef VBUSE_LOCALLIB

	long InitializeDLLInterfaces(void)
	{
		iVMR.VBVMR_Login					=VBVMR_Login;
		iVMR.VBVMR_Logout					=VBVMR_Logout;
		iVMR.VBVMR_RunVoicemeeter			=VBVMR_RunVoicemeeter;
		iVMR.VBVMR_GetVoicemeeterType		=VBVMR_GetVoicemeeterType;
		iVMR.VBVMR_GetVoicemeeterVersion	=VBVMR_GetVoicemeeterVersion;
		iVMR.VBVMR_IsParametersDirty		=VBVMR_IsParametersDirty;
		iVMR.VBVMR_GetParameterFloat		=VBVMR_GetParameterFloat;
		iVMR.VBVMR_GetParameterStringA		=VBVMR_GetParameterStringA;
		iVMR.VBVMR_GetParameterStringW		=VBVMR_GetParameterStringW;

		iVMR.VBVMR_GetLevel					=VBVMR_GetLevel;
		iVMR.VBVMR_GetMidiMessage			=VBVMR_GetMidiMessage;
		iVMR.VBVMR_SetParameterFloat		=VBVMR_SetParameterFloat;
		iVMR.VBVMR_SetParameters			=VBVMR_SetParameters;
		iVMR.VBVMR_SetParametersW			=VBVMR_SetParametersW;
		iVMR.VBVMR_SetParameterStringA		=VBVMR_SetParameterStringA;
		iVMR.VBVMR_SetParameterStringW		=VBVMR_SetParameterStringW;

		iVMR.VBVMR_Output_GetDeviceNumber	=VBVMR_Output_GetDeviceNumber;
		iVMR.VBVMR_Output_GetDeviceDescA	=VBVMR_Output_GetDeviceDescA;
		iVMR.VBVMR_Output_GetDeviceDescW	=VBVMR_Output_GetDeviceDescW;
		iVMR.VBVMR_Input_GetDeviceNumber	=VBVMR_Input_GetDeviceNumber;
		iVMR.VBVMR_Input_GetDeviceDescA		=VBVMR_Input_GetDeviceDescA;
		iVMR.VBVMR_Input_GetDeviceDescW		=VBVMR_Input_GetDeviceDescW;

		iVMR.VBVMR_AudioCallbackRegister	=VBVMR_AudioCallbackRegister;
		iVMR.VBVMR_AudioCallbackStart		=VBVMR_AudioCallbackStart;
		iVMR.VBVMR_AudioCallbackStop		=VBVMR_AudioCallbackStop;
		iVMR.VBVMR_AudioCallbackUnregister	=VBVMR_AudioCallbackUnregister;
		return 0;
	}

//Dynamic link to DLL in 'C' (regular use)
#else

	long InitializeDLLInterfaces(void)
	{
		char szDllName[1024];
		memset(&iVMR,0,sizeof(T_VBVMR_INTERFACE));

		//get folder where is installed Voicemeeter
		if (RegistryGetVoicemeeterFolder(szDllName) == FALSE) 
		{
			// voicemeeter not installed
			return -100; 
		}
		//use right dll according O/S type
		if (sizeof(void*) == 8) strcat(szDllName,"\\VoicemeeterRemote64.dll");
		else strcat(szDllName,"\\VoicemeeterRemote.dll");
		
		// Load Dll
		G_H_Module=LoadLibrary(szDllName);
		if (G_H_Module == NULL) return -101;

		// Get function pointers
		iVMR.VBVMR_Login					=(T_VBVMR_Login)GetProcAddress(G_H_Module,"VBVMR_Login");
		iVMR.VBVMR_Logout					=(T_VBVMR_Logout)GetProcAddress(G_H_Module,"VBVMR_Logout");
		iVMR.VBVMR_RunVoicemeeter			=(T_VBVMR_RunVoicemeeter)GetProcAddress(G_H_Module,"VBVMR_RunVoicemeeter");
		iVMR.VBVMR_GetVoicemeeterType		=(T_VBVMR_GetVoicemeeterType)GetProcAddress(G_H_Module,"VBVMR_GetVoicemeeterType");
		iVMR.VBVMR_GetVoicemeeterVersion	=(T_VBVMR_GetVoicemeeterVersion)GetProcAddress(G_H_Module,"VBVMR_GetVoicemeeterVersion");
		
		iVMR.VBVMR_IsParametersDirty		=(T_VBVMR_IsParametersDirty)GetProcAddress(G_H_Module,"VBVMR_IsParametersDirty");
		iVMR.VBVMR_GetParameterFloat		=(T_VBVMR_GetParameterFloat)GetProcAddress(G_H_Module,"VBVMR_GetParameterFloat");
		iVMR.VBVMR_GetParameterStringA		=(T_VBVMR_GetParameterStringA)GetProcAddress(G_H_Module,"VBVMR_GetParameterStringA");
		iVMR.VBVMR_GetParameterStringW		=(T_VBVMR_GetParameterStringW)GetProcAddress(G_H_Module,"VBVMR_GetParameterStringW");
		iVMR.VBVMR_GetLevel					=(T_VBVMR_GetLevel)GetProcAddress(G_H_Module,"VBVMR_GetLevel");
		iVMR.VBVMR_GetMidiMessage			=(T_VBVMR_GetMidiMessage)GetProcAddress(G_H_Module,"VBVMR_GetMidiMessage");

		iVMR.VBVMR_SetParameterFloat		=(T_VBVMR_SetParameterFloat)GetProcAddress(G_H_Module,"VBVMR_SetParameterFloat");
		iVMR.VBVMR_SetParameters			=(T_VBVMR_SetParameters)GetProcAddress(G_H_Module,"VBVMR_SetParameters");
		iVMR.VBVMR_SetParametersW			=(T_VBVMR_SetParametersW)GetProcAddress(G_H_Module,"VBVMR_SetParametersW");
		iVMR.VBVMR_SetParameterStringA		=(T_VBVMR_SetParameterStringA)GetProcAddress(G_H_Module,"VBVMR_SetParameterStringA");
		iVMR.VBVMR_SetParameterStringW		=(T_VBVMR_SetParameterStringW)GetProcAddress(G_H_Module,"VBVMR_SetParameterStringW");

		iVMR.VBVMR_Output_GetDeviceNumber	=(T_VBVMR_Output_GetDeviceNumber)GetProcAddress(G_H_Module,"VBVMR_Output_GetDeviceNumber");
		iVMR.VBVMR_Output_GetDeviceDescA	=(T_VBVMR_Output_GetDeviceDescA)GetProcAddress(G_H_Module,"VBVMR_Output_GetDeviceDescA");
		iVMR.VBVMR_Output_GetDeviceDescW	=(T_VBVMR_Output_GetDeviceDescW)GetProcAddress(G_H_Module,"VBVMR_Output_GetDeviceDescW");
		iVMR.VBVMR_Input_GetDeviceNumber	=(T_VBVMR_Input_GetDeviceNumber)GetProcAddress(G_H_Module,"VBVMR_Input_GetDeviceNumber");
		iVMR.VBVMR_Input_GetDeviceDescA		=(T_VBVMR_Input_GetDeviceDescA)GetProcAddress(G_H_Module,"VBVMR_Input_GetDeviceDescA");
		iVMR.VBVMR_Input_GetDeviceDescW		=(T_VBVMR_Input_GetDeviceDescW)GetProcAddress(G_H_Module,"VBVMR_Input_GetDeviceDescW");

		iVMR.VBVMR_AudioCallbackRegister	=(T_VBVMR_AudioCallbackRegister)GetProcAddress(G_H_Module,"VBVMR_AudioCallbackRegister");
		iVMR.VBVMR_AudioCallbackStart		=(T_VBVMR_AudioCallbackStart)GetProcAddress(G_H_Module,"VBVMR_AudioCallbackStart");
		iVMR.VBVMR_AudioCallbackStop		=(T_VBVMR_AudioCallbackStop)GetProcAddress(G_H_Module,"VBVMR_AudioCallbackStop");
		iVMR.VBVMR_AudioCallbackUnregister	=(T_VBVMR_AudioCallbackUnregister)GetProcAddress(G_H_Module,"VBVMR_AudioCallbackUnregister");
		// check pointers are valid
		if (iVMR.VBVMR_Login == NULL) return -1;
		if (iVMR.VBVMR_Logout == NULL) return -2;
		if (iVMR.VBVMR_RunVoicemeeter == NULL) return -2;
		if (iVMR.VBVMR_GetVoicemeeterType == NULL) return -3;
		if (iVMR.VBVMR_GetVoicemeeterVersion == NULL) return -4;
		if (iVMR.VBVMR_IsParametersDirty == NULL) return -5;
		if (iVMR.VBVMR_GetParameterFloat == NULL) return -6;
		if (iVMR.VBVMR_GetParameterStringA == NULL) return -7;
		if (iVMR.VBVMR_GetParameterStringW == NULL) return -8;
		if (iVMR.VBVMR_GetLevel == NULL) return -9;
		if (iVMR.VBVMR_SetParameterFloat == NULL) return -10;
		if (iVMR.VBVMR_SetParameters == NULL) return -11;
		if (iVMR.VBVMR_SetParametersW == NULL) return -12;
		if (iVMR.VBVMR_SetParameterStringA == NULL) return -13;
		if (iVMR.VBVMR_SetParameterStringW == NULL) return -14;
		if (iVMR.VBVMR_GetMidiMessage == NULL) return -15;

		if (iVMR.VBVMR_Output_GetDeviceNumber == NULL) return -30;
		if (iVMR.VBVMR_Output_GetDeviceDescA == NULL) return -31;
		if (iVMR.VBVMR_Output_GetDeviceDescW == NULL) return -32;
		if (iVMR.VBVMR_Input_GetDeviceNumber == NULL) return -33;
		if (iVMR.VBVMR_Input_GetDeviceDescA == NULL) return -34;
		if (iVMR.VBVMR_Input_GetDeviceDescW == NULL) return -35;
		
		if (iVMR.VBVMR_AudioCallbackRegister == NULL) return -40;
		if (iVMR.VBVMR_AudioCallbackStart == NULL) return -41;
		if (iVMR.VBVMR_AudioCallbackStop == NULL) return -42;
		if (iVMR.VBVMR_AudioCallbackUnregister == NULL) return -43;
		return 0;
	}


#endif




/*******************************************************************************/
/*                                    COLOR / PEN TOOLS                        */
/*******************************************************************************/

typedef struct tagCOLORPENBRUSH
{
	COLORREF	color;
	HPEN		pen;
	HBRUSH		brush;
} T_COLORPENBRUSH, *PT_COLORPENBRUSH, *LPT_COLORPENBRUSH;

long CreateColorPenBrush(LPT_COLORPENBRUSH lpgdi, COLORREF color)
{
	LOGBRUSH	lb;
	if (lpgdi == NULL) return -1;
	lpgdi->color=color;
	lpgdi->pen=CreatePen(PS_SOLID,0,color);

	lb.lbStyle=BS_SOLID;
	lb.lbColor=color;
	lpgdi->brush=CreateBrushIndirect(&lb);
	return 0;
}

long DestroyColorPenBrush(LPT_COLORPENBRUSH lpgdi)
{
	if (lpgdi == NULL) return -1;
	
	lpgdi->color=0;
	if (lpgdi->pen != NULL) DeleteObject(lpgdi->pen);
	lpgdi->pen=NULL;
	if (lpgdi->brush != NULL) DeleteObject(lpgdi->brush);
	lpgdi->brush=NULL;
	return 0;
}




/*******************************************************************************/
/*                                APPLICATION CONTEXT                          */
/*******************************************************************************/

#define NBPARAM_DISPLAYED 9

typedef struct tagAUDIODSDPCTX
{
	float	freq;
	float	gain;
	int		nuStripSelected;
	float	* pSinusBuffer;
	long	SinusBuffer_nbSample;
	long	SinusBuffer_index;
} T_AUDIODSDPCTX, *PT_AUDIODSDPCTX, *LPT_AUDIODSDPCTX;

typedef struct tagAPP_CONTEXT
{
	HWND		hwnd_MainWindow;
	HINSTANCE	hinstance;
	size_t		wTimer;
	int			vbvmr_error;
	int			vbvmr_connect;
	int			vbvmr_nbstrip;
	char **		vbvmr_pStripName;
	long *		vbvmr_pStripChannelIndex;

	HFONT		font_small;
	HFONT		font_med;
	HFONT		font_big;

	T_COLORPENBRUSH	gdiobjects_black;
	T_COLORPENBRUSH	gdiobjects_bkg_bluedark;
	T_COLORPENBRUSH	gdiobjects_bkg_blue0;
	T_COLORPENBRUSH	gdiobjects_bkg_blue1;
	T_COLORPENBRUSH	gdiobjects_bkg_blue2;
	T_COLORPENBRUSH	gdiobjects_bkg_orange;

	RECT		rect_select;
	RECT		rect_control;

	T_AUDIODSDPCTX dsp;
} T_APP_CONTEXT, *PT_APP_CONTEXT, *LPT_APP_CONTEXT;

static T_APP_CONTEXT G_MainAppCtx = {NULL, NULL};



long InitResources(LPT_APP_CONTEXT lpapp)
{
	LOGFONT		lf;

	if (lpapp == NULL) return -1;

	//make Font
	memset(&lf,0, sizeof(LOGFONT));
	lf.lfHeight	= 16;
	lf.lfWeight	= 400;
	strcpy(lf.lfFaceName,"Arial");
	lpapp->font_small =CreateFontIndirect(&lf);

	lf.lfHeight	= 18;
	lf.lfWeight	= 800;
	strcpy(lf.lfFaceName,"Arial");
	lpapp->font_med =CreateFontIndirect(&lf);
	
	lf.lfHeight	= 24;
	lf.lfWeight	= 800;
	strcpy(lf.lfFaceName,"Arial");
	lpapp->font_big =CreateFontIndirect(&lf);
	//make pen brush 
	CreateColorPenBrush(&(lpapp->gdiobjects_black), RGB(100,100,100));
	CreateColorPenBrush(&(lpapp->gdiobjects_bkg_bluedark), RGB(5,5,30));
	CreateColorPenBrush(&(lpapp->gdiobjects_bkg_blue0), RGB(5,70,100));
	CreateColorPenBrush(&(lpapp->gdiobjects_bkg_blue1), RGB(10,120,150));
	CreateColorPenBrush(&(lpapp->gdiobjects_bkg_blue2), RGB(100,240,240));
	CreateColorPenBrush(&(lpapp->gdiobjects_bkg_orange), RGB(170,110,0));

	return 0;
}

long EndResources(LPT_APP_CONTEXT lpapp)
{
	if (lpapp == NULL) return -1;
	//Delete font Object
	if (lpapp->font_small != NULL) DeleteObject(lpapp->font_small);
	lpapp->font_small=NULL;
	if (lpapp->font_med != NULL) DeleteObject(lpapp->font_med);
	lpapp->font_med=NULL;
	if (lpapp->font_big != NULL) DeleteObject(lpapp->font_big);
	lpapp->font_big=NULL;
	//Delete Pen Brush
	DestroyColorPenBrush(&(lpapp->gdiobjects_black));
	DestroyColorPenBrush(&(lpapp->gdiobjects_bkg_bluedark));
	DestroyColorPenBrush(&(lpapp->gdiobjects_bkg_blue0));
	DestroyColorPenBrush(&(lpapp->gdiobjects_bkg_blue1));
	DestroyColorPenBrush(&(lpapp->gdiobjects_bkg_blue2));
	DestroyColorPenBrush(&(lpapp->gdiobjects_bkg_orange));
	return 0;
}




/*******************************************************************************/
/*                                                                             */
/*                              AUDIO CALLBACK EXAMPLE                         */
/*                                                                             */
/*******************************************************************************/
/* With VB-Audio Callback Functions, it's now possible to use voicemeeter as   */
/* audio board and get signal from different point to analyze or process it.   */
/*******************************************************************************/



void PROCESSING_initmyDSPContext(LPT_APP_CONTEXT lpapp)
{
	LPT_AUDIODSDPCTX lpdsp;
	lpdsp = &(lpapp->dsp);
	memset(lpdsp , 0 , sizeof(T_AUDIODSDPCTX));
	lpdsp->freq = 220.0f; // 220 Hz Sinus Generator
	lpdsp->gain = 0.1f; //-20 dB
}

void PROCESSING_AllocateInternalMemory(LPT_APP_CONTEXT lpapp, long SampleRate)
{
	float * wavbuffer, value;
	double ww, ratei;
	long vi;
	LPT_AUDIODSDPCTX lpdsp;
	lpdsp = &(lpapp->dsp);

	lpdsp->SinusBuffer_nbSample = SampleRate;
	lpdsp->SinusBuffer_index =0;

	// we allocate our internal buffer on one second (1000 ms)
	lpdsp->pSinusBuffer	= malloc(lpdsp->SinusBuffer_nbSample * sizeof(float));
	if (lpdsp->pSinusBuffer == NULL) return;

	// containing our specific Sinus signal
	ratei=6.28318530717958647692528676655901*lpdsp->freq/SampleRate;
	wavbuffer = lpdsp->pSinusBuffer;
	for (vi=0;vi<lpdsp->SinusBuffer_nbSample;vi++)
	{
		ww=((double)vi)*ratei;
		value=(float)(sin(ww));
		*wavbuffer++ = value;	// normalized float32 signal (-1.0 to +1.0)
	}

}

void PROCESSING_ReleaseInternalMemory(LPT_APP_CONTEXT lpapp)
{
	LPT_AUDIODSDPCTX lpdsp;
	lpdsp = &(lpapp->dsp);
	if (lpdsp->pSinusBuffer	 != NULL) free(lpdsp->pSinusBuffer);
	lpdsp->pSinusBuffer=NULL;
}

// REM: signal is stored in normalized float32 signal (-1.0 to +1.0)
void PROCESSING_StereoPlayback(LPT_APP_CONTEXT lpapp, float * pOutL, float * pOutR, long nbSample)
{
	float *pSinusSignal, Signal, gain;
	long vi, index, iMax;
	LPT_AUDIODSDPCTX lpdsp;
	lpdsp = &(lpapp->dsp);
	index=lpdsp->SinusBuffer_index;			// get current sinus buffer index
	iMax = lpdsp->SinusBuffer_nbSample;
	gain = lpdsp->gain;
	if (lpdsp->pSinusBuffer == NULL)
	{
		//if sinus buffer is not defined, we just provide silence
		for (vi=0;vi<nbSample;vi++)
		{
			*pOutL++ =0.0f;
			*pOutR++ =0.0f;
		}
	}
	else
	{
		//sinus buffer is define we read our internal audio buffer containing our sinus signal
		pSinusSignal = lpdsp->pSinusBuffer + index;
		for (vi=0;vi<nbSample;vi++)
		{
			Signal = *pSinusSignal++;		// we get sample from our Sinus buffer
			Signal = Signal* gain;			// we apply gain
			*pOutL++ =Signal;				// we copy it into Left channel
			*pOutR++ =Signal;				// we copy it into Right channel
			index++;						// inc index (in our internal Sinus Buffer)
			if (index >=iMax)				// manage our sinus Circular Buffer
			{
				index=0;
				pSinusSignal = lpdsp->pSinusBuffer;
			}
		}
		
	}
	lpdsp->SinusBuffer_index=index;			// store current sinus buffer index
}


/*******************************************************************************/
/*                             AUDIO CALLBACK EXAMPLE                          */
/*******************************************************************************/

long __stdcall PROCESSING_MyCallback(void * lpUser, long nCommand, void * lpData, long nnn)
{
	float fzero=0.0f;
	float * lpBufferIn;
	float * lpBufferOut;
	int nuChannel,nbs, nuOut;
	LPT_APP_CONTEXT lpapp;
	LPT_AUDIODSDPCTX lpdsp;
	VBVMR_LPT_AUDIOINFO pinfo;
	VBVMR_LPT_AUDIOBUFFER lpa;

	lpapp = (LPT_APP_CONTEXT)lpUser;
	lpdsp = &(lpapp->dsp);
	switch(nCommand)
	{
	
	//--------------------------------------
	// Init/End your object and allocate your memory
	//--------------------------------------
	case VBVMR_CBCOMMAND_STARTING:
		pinfo = (VBVMR_LPT_AUDIOINFO)lpData;
		PROCESSING_AllocateInternalMemory(lpapp,pinfo->samplerate);
		break;		
	case VBVMR_CBCOMMAND_ENDING:
		pinfo = (VBVMR_LPT_AUDIOINFO)lpData;
		PROCESSING_ReleaseInternalMemory(lpapp);
		break;		
	case VBVMR_CBCOMMAND_CHANGE:
		pinfo = (VBVMR_LPT_AUDIOINFO)lpData;
		PostMessage(lpapp->hwnd_MainWindow,WM_COMMAND, IDM_RESTART,0);
		break;		
	//--------------------------------------
	// process buffer for Input INSERT      
	//--------------------------------------
	case VBVMR_CBCOMMAND_BUFFER_IN:
		lpa =(VBVMR_LPT_AUDIOBUFFER)lpData;
		if (lpdsp->nuStripSelected >= lpapp->vbvmr_nbstrip) lpdsp->nuStripSelected=0;
		// get channel number according selected strip
		nuChannel = lpapp->vbvmr_pStripChannelIndex[lpdsp->nuStripSelected];
		
		if (nuChannel > (lpa->audiobuffer_nbi)) break; //check that the given channels are existing
		// we play a stereo stream only in 2 first channels of the strip
		PROCESSING_StereoPlayback(lpapp, lpa->audiobuffer_w[nuChannel], lpa->audiobuffer_w[nuChannel+1], lpa->audiobuffer_nbs);

		// remember its an INSERT PROCESS so we must copy other channels 
		// (otherwise there will be no sound in other Strip).
		for (nuOut =0; nuOut < lpa->audiobuffer_nbi; nuOut++)
		{
			if ((nuOut < nuChannel) || (nuOut >= (nuChannel+2)))
			{
				lpBufferIn = lpa->audiobuffer_r[nuOut]; 
				lpBufferOut = lpa->audiobuffer_w[nuOut]; 
				for (nbs=0;nbs < lpa->audiobuffer_nbs; nbs++) lpBufferOut[nbs]=lpBufferIn[nbs];
			}
		}
		break;
	}
	return 0;
}





/*******************************************************************************/
/*                                DISPLAY FUNCTIONS                            */
/*******************************************************************************/

void DrawStripSelector(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc)
{
	char sss[512];
	int nStrip,vi;
	int x0,y0,dx,dy;
	RECT rect,rect2;
	HBRUSH oldbrush;
	HPEN oldpen;
	HFONT oldfont;

	oldpen = (HPEN)SelectObject(dc,lpapp->gdiobjects_bkg_bluedark.pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpapp->gdiobjects_bkg_blue1.brush);
	oldfont = (HFONT)SelectObject(dc,lpapp->font_big);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(0,0,0));

	rect=lpapp->rect_select;
	dx=(rect.right-rect.left) / 8;
	dy = rect.bottom-rect.top;
	x0=rect.left;
	y0=rect.top;	

	nStrip=lpapp->vbvmr_nbstrip;
	if ((nStrip < 1) || (lpapp->vbvmr_pStripName == NULL))
	{
		rect2 = rect;
		strcpy(sss,"Voicemeeter Audio Component is not running... Launch Voicemeeter");
		Rectangle(dc,rect2.left,rect2.top,rect2.right,rect2.bottom);
		DrawText(dc,sss,(int)strlen(sss),&rect2,DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}
	else
	{
		for (vi=0;vi<8;vi++)
		{
			sss[0]=0;
			if (vi < nStrip)
			{
				strcpy(sss,lpapp->vbvmr_pStripName[vi]);
			}
			if (sss[0] == 0) SelectObject(dc,lpapp->gdiobjects_bkg_bluedark.brush);
			else
			{
				if (vi == lpapp->dsp.nuStripSelected) SelectObject(dc,lpapp->gdiobjects_bkg_blue2.brush);
				else SelectObject(dc,lpapp->gdiobjects_bkg_blue1.brush);
			}

			rect2.left=x0;
			rect2.top=y0;
			rect2.right=rect2.left+dx-10;
			rect2.bottom=rect2.top+dy;
			
			Rectangle(dc,rect2.left,rect2.top,rect2.right,rect2.bottom);
			DrawText(dc,sss,(int)strlen(sss),&rect2,DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			
			x0=x0+dx;
		}
	}	
	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);
}

void DrawAllStuff(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc)
{
	char sss[128];
	double dBGain;
	HBRUSH oldbrush;
	HPEN oldpen;
	HFONT oldfont;
	RECT rect;

	GetClientRect(hw,&rect);
	rect.left+=10;
	rect.top+=10;
	rect.right-=10;
	rect.bottom-=10;
		
	lpapp->rect_select = rect;
	lpapp->rect_select.bottom = lpapp->rect_select.top+50;

	rect.top+=80;
	rect.bottom-=10;
	lpapp->rect_control = rect;

	DrawStripSelector(lpapp, hw, dc);

	// Draw Control Area
	oldpen = (HPEN)SelectObject(dc,lpapp->gdiobjects_bkg_bluedark.pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpapp->gdiobjects_bkg_bluedark.brush);
	oldfont = (HFONT)SelectObject(dc,lpapp->font_big);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(200,200,200));

	rect= lpapp->rect_control;
	if (lpapp->dsp.gain < 1.0e-5f) dBGain=-100.0f;
	else dBGain = 20.0*log10(lpapp->dsp.gain);
	sprintf(sss,"Play Stereo Sinus %0.1f Hz / %0.1f dB", lpapp->dsp.freq, dBGain);
	Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);
	DrawText(dc,sss,(int)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);

}


/*******************************************************************************/
/*                             USER ACTIONS MANAGEMENT                         */
/*******************************************************************************/

#define APP_ZONE_BUS	50

#define APP_ZONE_CONTROL	100

int APP_WhereAmI(LPT_APP_CONTEXT lpapp, int x0, int y0, int * pnuOut, int * pnuIn)
{
	RECT rect,rect2;
	int dx,dy;
	int vi;

	//check if the click is in the BUS Selector
	rect=lpapp->rect_select;
	dx=(rect.right-rect.left) / 8;
	dy = rect.bottom-rect.top;

	rect2.left=rect.left;
	rect2.top=rect.top;
	rect2.right=rect2.left+dx-10;
	rect2.bottom=rect2.top+dy;
	for (vi=0;vi<8;vi++)
	{
		if ((x0>=rect2.left) && (x0<=rect2.right) && (y0>=rect2.top) && (y0<=rect2.bottom)) return (APP_ZONE_BUS+vi);
		rect2.left +=dx;
		rect2.right +=dx;
	}

	//check if we click in the control area
	rect=lpapp->rect_control;
		
	return -1;
}

void APP_ManageLRButtonDown(LPT_APP_CONTEXT lpapp, HWND hw, int x0, int y0, int fRightClick)
{
	HDC dc;
	LPT_AUDIODSDPCTX lpdsp;
	int rep, nuIn,nuOut, nnn;
	rep= APP_WhereAmI(lpapp, x0, y0, &nuOut, &nuIn);	
	if (rep <0) return;

	lpdsp= &(lpapp->dsp);
	// if we clikc in the BUS Selector
	if ((rep >= APP_ZONE_BUS) && (rep < (APP_ZONE_BUS+lpapp->vbvmr_nbstrip)))
	{
		nnn=rep-APP_ZONE_BUS;
		lpdsp->nuStripSelected= nnn;

		dc=GetDC(hw);
		DrawStripSelector(lpapp, hw, dc);
		ReleaseDC(hw,dc);
		return;
	}
}

/*******************************************************************************/
/*                               Init / End Software                           */
/*******************************************************************************/

static char * G_szStripNameList_v1[3]={"P.IN1", "P.IN2", "V.IN1"};
static char * G_szStripNameList_v2[5]={"P.IN1", "P.IN2", "P.IN3", "V.IN1", "V.IN2"};
static char * G_szStripNameList_v3[8]={"P.IN1", "P.IN2", "P.IN3", "P.IN4", "P.IN5", "V.IN1", "V.IN2", "V.IN3"};

static long G_StripChannelIndex_v1[3]={0, 2, 4};
static long G_StripChannelIndex_v2[5]={0, 2, 4, 6, 14};
static long G_StripChannelIndex_v3[8]={0, 2, 4, 6, 8, 10, 18, 26};


void DetectVoicemeeterType(LPT_APP_CONTEXT lpapp, HWND hw)
{
	long rep,vmType;

	lpapp->vbvmr_nbstrip		=0;
	lpapp->vbvmr_pStripName		=NULL;

	rep=iVMR.VBVMR_GetVoicemeeterType(&vmType);
	if (rep == 0) 
	{
		if (lpapp->vbvmr_connect != vmType)
		{
			lpapp->vbvmr_connect =vmType;
			switch(vmType)
			{
			case 1://Voicemeeter
				lpapp->vbvmr_nbstrip		=3;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v1;
				lpapp->vbvmr_pStripChannelIndex =G_StripChannelIndex_v1;
				break;
			case 2://Voicemeeter Banana
				lpapp->vbvmr_nbstrip		=5;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v2;
				lpapp->vbvmr_pStripChannelIndex =G_StripChannelIndex_v2;
				break;
			case 3://Voicemeeter 8
				lpapp->vbvmr_nbstrip		=8;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v3;
				lpapp->vbvmr_pStripChannelIndex =G_StripChannelIndex_v3;
				break;
			}
			InvalidateRect(hw,NULL,TRUE);
		}
	}
}


BOOL InitSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	char szMessage[512];
	char szClientName[64]="VB-Audio Pre-Fader Input Player Example";
	char szTitle[]="Init Error";
	long rep;

	//init context
	lpapp->hwnd_MainWindow = hw;
	PROCESSING_initmyDSPContext(lpapp);
	
	//init resources
	InitResources(lpapp);

	//get DLL interface
	rep=InitializeDLLInterfaces();
	if (rep < 0)
	{
		if (rep == -100) MessageBox(hw,"Voicemeeter is not installed",szTitle,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		else MessageBox(hw,"Failed To Link To VoicemeeterRemote.dll",szTitle,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return FALSE;
	}
	//Log in
	rep=iVMR.VBVMR_Login();
	if (rep < 0)
	{
		MessageBox(hw,"Failed To Login",szTitle,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return FALSE;
	}
	//call this to get first parameters state (if server already launched)
	lpapp->vbvmr_error = iVMR.VBVMR_IsParametersDirty();
	if (lpapp->vbvmr_error == 0)
	{
		DetectVoicemeeterType(lpapp, hw);
	}
	else lpapp->vbvmr_connect=0;
	//register callback on output Insert point
	rep=iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_IN, PROCESSING_MyCallback, lpapp, szClientName);
	if (rep == 1)
	{
		sprintf(szMessage,"Voicemeeter Input Insert already in use by:\n%s", szClientName);
		MessageBox(hw,szMessage,szTitle,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

BOOL EndSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	if (iVMR.VBVMR_Logout != NULL) iVMR.VBVMR_Logout();
	if (iVMR.VBVMR_AudioCallbackUnregister != NULL) iVMR.VBVMR_AudioCallbackUnregister();

	EndResources(lpapp);
	return TRUE;
}



void ManageMenu(LPT_APP_CONTEXT lpapp, HWND hw, WPARAM wparam,	LPARAM lparam)
{
	switch(LOWORD(wparam))
	{
		case IDM_RESTART:
			Sleep(50);
			if (lpapp->vbvmr_connect == 0) break;
			if (iVMR.VBVMR_AudioCallbackStart != NULL) iVMR.VBVMR_AudioCallbackStart();
			break;
	}
}

/*******************************************************************************/
/*                                WIN CALLBACK                                 */
/*******************************************************************************/

#define MYTIMERID 16489

LRESULT CALLBACK MainWindowManageEvent(HWND hw,			//handle of the window.
											UINT msg,   //Message Ident.
											WPARAM wparam,	//parameter 1.
											LPARAM lparam)	//parameter 2
{
	LPT_APP_CONTEXT lpapp;
	HDC dc;
	PAINTSTRUCT ps;

	lpapp = &G_MainAppCtx;
	switch (msg)
	{


		case WM_CREATE:
			if (InitSoftware(lpapp,hw) == FALSE) return -1;//return -1 here cancel the window creation
			lpapp->wTimer = SetTimer(hw,MYTIMERID, 500,NULL);//2 Hz timer
			PostMessage(lpapp->hwnd_MainWindow,WM_COMMAND, IDM_RESTART,0);
			break;
		case WM_LBUTTONDOWN:
			APP_ManageLRButtonDown(lpapp, hw, (short int)LOWORD(lparam),(short int)HIWORD(lparam),0);
			break;	
		case WM_COMMAND:
			ManageMenu(lpapp,hw,wparam,lparam);
			break;
		case WM_TIMER:
			if (wparam == MYTIMERID)
			{
				//check if voicemeeter type has changed
				lpapp->vbvmr_error=iVMR.VBVMR_IsParametersDirty();
				if (lpapp->vbvmr_error >= 0)
				{
					if (lpapp->vbvmr_connect == 0)
					{
						DetectVoicemeeterType(lpapp, hw);
						if (lpapp->vbvmr_nbstrip > 0) PostMessage(lpapp->hwnd_MainWindow,WM_COMMAND, IDM_RESTART,0);
					}
				}
				else 
				{
					if (lpapp->vbvmr_connect != 0)
					{
						//Voicemeeter has been shut down
						lpapp->vbvmr_connect	=0;
						lpapp->vbvmr_nbstrip	=0;
						InvalidateRect(hw,NULL,TRUE);
					}
				}
			}
			break;
		case WM_PAINT:
			dc=BeginPaint(hw,&ps);
			DrawAllStuff(lpapp,hw, dc);
			EndPaint(hw,&ps);
	        break;
		case WM_DESTROY:
			if (lpapp->wTimer != 0) KillTimer(hw,lpapp->wTimer);
			lpapp->wTimer=0;
			EndSoftware(lpapp,hw);
			PostQuitMessage(0);
			break;
		default:
			return (DefWindowProc(hw,msg,wparam,lparam));

	}
	return (0L);
}


/*******************************************************************************/
/**                              MAIN PROCDURE                                **/
/*******************************************************************************/

int APIENTRY WinMain(HINSTANCE handle_app,			//Application hinstance.
							HINSTANCE handle_prev,  //NULL.
							LPTSTR param,           //Command Line Parameter.
							int com_show)           //How to display window (optionnal).
{
	HANDLE	hMutex;
	HWND	hh;
	long	wstyle;
	MSG		msg;    
	char	szWindowClassName[]="MainWindowClass";
	char *	title="Sorry";

	WNDCLASS	wc;
	//we first store the APP Hinstance
	G_MainAppCtx.hinstance=handle_app;

	//
	//Mutex.
	//
	hMutex = CreateMutex(NULL,TRUE,APP_UNIQUEMUTEX);
	if (hMutex == NULL) return -1;
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex(hMutex);
		return -1;
	}
	else WaitForSingleObject(hMutex,INFINITE);

#ifdef VBUSE_LOCALLIB
	//if we directly link source code (development only)
	VBVMR_SetHinstance(handle_app);
#endif
	//here you can make some early initialization and analyze command line if any.


	//we define a window class to create a window from this class 
	wc.style		=CS_HREDRAW | CS_VREDRAW;  	  		//property.
	wc.lpfnWndProc=(WNDPROC)MainWindowManageEvent;		//Adresse of our Callback.
	wc.cbClsExtra =0;					  				//Possibility to store some byte inside a class object.
	wc.cbWndExtra =0;                          			//Possibility to store some byte inside a window object.
	wc.hInstance  =handle_app; 	                		//handle of the application hinstance.
	wc.hIcon      =LoadIcon(NULL, IDI_APPLICATION);    	//handle of icon displayed in the caption.
	wc.hCursor    =LoadCursor(NULL,IDC_ARROW);			//handle of cursor mouse .
	wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);//Background color.
	wc.lpszMenuName=NULL;			    				//pointer on menu defined in resource.
	wc.lpszClassName=szWindowClassName;       			//pointer on class name.

	//register class.
	if (RegisterClass(&wc)==0)
	{
		MessageBox(NULL,"Failed to register main class...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}
	
	
	//classical Main Window
	wstyle=WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	wstyle= wstyle & (~WS_THICKFRAME);

	hh=CreateWindow(szWindowClassName,					// address of registered class name.
						 SZPUBLICNAME,					// address of window name string
						 wstyle,						// window style
						 CW_USEDEFAULT,					// horizontal position of window
						 CW_USEDEFAULT,					// vertical position of window
						 UI_WIN_DX,						// window width
						 UI_WIN_DY,						// window height
						 NULL,							// parent handle is NULL since it's a main window.
						 NULL,							// menu name defined in resource (NULL if no menu or already defined in the Class).
						 handle_app,					// handle of application instance
						 NULL); 						// address of window-creation data

	if (hh==NULL)
	{
		MessageBox(NULL,"Failed to create window...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}
	ShowWindow(hh,SW_SHOW);				//Display the window.
	UpdateWindow(hh);					//Send WM_PAINT.
	/*---------------------------------------------------------------------------*/
	/*                             Messages Loop.                                */
	/*---------------------------------------------------------------------------*/
	while (GetMessage(&msg,NULL,0,0))	//Get Message if any.
	{
		TranslateMessage(&msg);			//Translate the virtuel keys event.
		DispatchMessage(&msg);			//DispatchMessage to the related window.
	}

	//here you can make last uninitialization and release
	return (int)(msg.wParam);
}
