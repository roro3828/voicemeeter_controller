#include"VoicemeeterRemote.h"
#include<iostream>
#include<windows.h>
#include"serial.h"

static char uninstDirKey[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
#define INSTALLER_UNINST_KEY	"VB:Voicemeeter {17359A74-1236-5467}"

void RemoveNameInPath(char * szPath)
{
	long ll;
	ll=(long)strlen(szPath);
	while ((ll>0) && (szPath[ll]!='\\')) ll--;
	if (szPath[ll] == '\\') szPath[ll]=0;
}

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
	rep=RegOpenKeyExA(HKEY_LOCAL_MACHINE,szKey,0, KEY_READ, &hkResult);
	if (rep != ERROR_SUCCESS)
	{
		// if not present we consider running in 64bit mode and force to read 32bit registry
		rep=RegOpenKeyExA(HKEY_LOCAL_MACHINE,szKey,0, KEY_READ | KEY_WOW64_32KEY, &hkResult); 
	}
	if (rep != ERROR_SUCCESS) return FALSE;
	// read uninstall profram path
	rep=RegQueryValueExA(hkResult,"UninstallString",0,&pptype,(unsigned char *)sss,&nnsize);
	RegCloseKey(hkResult);
	
	if (pptype != REG_SZ) return FALSE;
	if (rep != ERROR_SUCCESS) return FALSE;
	// remove name to get the path only
	RemoveNameInPath(sss);
	if (nnsize>512) nnsize=512;
	strncpy(szDir,sss,nnsize);
	return TRUE;
}

int main(){
    char szDLLName[1024];
    if(!RegistryGetVoicemeeterFolder(szDLLName)){
        return -100;
    }

    if(sizeof(void*)==8){
        strcat(szDLLName,"\\VoicemeeterRemote64.dll");
    }
    else{
        strcat(szDLLName,"\\VoicemeeterRemote.dll");
    }

    std::cout<<szDLLName<<std::endl;

    HMODULE G_H_Module=LoadLibraryA(szDLLName);
    if(G_H_Module==NULL){
        return -101;
    }
    T_VBVMR_INTERFACE iVMR;

    iVMR.VBVMR_Login=(T_VBVMR_Login)GetProcAddress(G_H_Module,"VBVMR_Login");
    iVMR.VBVMR_Logout=(T_VBVMR_Logout)GetProcAddress(G_H_Module,"VBVMR_Logout");
    iVMR.VBVMR_SetParameterFloat=(T_VBVMR_SetParameterFloat)GetProcAddress(G_H_Module,"VBVMR_SetParameterFloat");
	iVMR.VBVMR_RunVoicemeeter=(T_VBVMR_RunVoicemeeter)GetProcAddress(G_H_Module,"VBVMR_RunVoicemeeter");

    int rep=iVMR.VBVMR_Login();
    if(rep<0){
        return -102;
    }
    if(rep==1){
		iVMR.VBVMR_RunVoicemeeter(3);
		Sleep(1000);
    }

    for(int i=-600;i<=0;i++){
        float val=((float)i)/10.0f;
        iVMR.VBVMR_SetParameterFloat("Strip[7].gain",val);
        std::cout<<val<<std::endl;
        Sleep(1);
    }

    iVMR.VBVMR_SetParameterFloat("Strip[0].gain",0);
    Sleep(10);
    iVMR.VBVMR_Logout();

    std::cout<<"Logout"<<std::endl;

    return 0;

}