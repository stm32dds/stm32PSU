#include <wtypes.h>
#include <string.h>
#include <Windows.h>
#include <winuser.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <strsafe.h>
#include <initguid.h>
#include <SetupAPI.h>
#include <math.h>
#include "resource.h"
#include <math.h>

#define U_SP_MAX 30.0 //definition for max value for U set point
#define U_SP_MIN 0.0
#define I_SP_MAX 4.6 //definition for max value for I set point
#define I_SP_MIN 0.0
#define SP_STEP_MIN 0.001 // minimal step point to truncation

extern char* ii_gcvt(float t);
extern void onEnterInEditCtrl(HWND hDlg, float* uSP, float* iSP);
extern float onUspTrckBar(HWND hDlg, WPARAM wParam, float uSP);
extern float onIspTrckBar(HWND hDlg, WPARAM wParam, float iSP);
extern float onUsp_UP(HWND hDlg, float uSP);
extern float onUsp_DN(HWND hDlg, float uSP);
extern float onIsp_UP(HWND hDlg, float iSP);
extern float onIsp_DN(HWND hDlg, float iSP);
extern BOOL onPower(BOOL cmdPowerON);
extern void Sent(HANDLE hCom, LPOVERLAPPED oW, BOOL cmdPowerON, float uSP, float iSP);
extern void onClose(HWND hDlg, HANDLE hCom,
	HANDLE hThread, HFONT OutputFont, HFONT SetpointsFont);
extern void onAbout(HWND hDlg, HINSTANCE hInst);
extern BOOL onConnect(HWND hDlg, TCHAR* pcCommPort, HANDLE& hCom, HWND hStatus, DCB dcb,
	LPOVERLAPPED oR, LPOVERLAPPED oW, DWORD dwEventMask, COMMTIMEOUTS comtimes);