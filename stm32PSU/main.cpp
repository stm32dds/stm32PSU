#include "main.h"

#pragma comment (lib, "Setupapi.lib")
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")
#pragma comment(lib, "ComCtl32.lib")

//Dialog variables
HWND hDlg;
HWND hStatus; //Status bar window
HFONT OutputFont;
HFONT SetpointsFont;
HINSTANCE hInstDlg;
float uPV = 0.0;//Process value of voltage
float iPV = 0.0;//Process value of current
float uSP = 0.0;//Voltage Set point
float iSP = 0.0;//Current Set point

//USB Device variables
COMMTIMEOUTS comtimes;
DCB dcb;
HANDLE hCom;
BOOL isConnected = FALSE; // Is Device connected or NOT?
BOOL isOutputPowered = FALSE; //is Device output powered or NOT ?
BOOL cmdPowerON = FALSE; // command to power ON/OFF device output
TCHAR pcCommPort[20] = L"\\\\.\\";
DWORD dwEventMask;
OVERLAPPED oRead; LPOVERLAPPED oR = &oRead; //Overlapped for reading
OVERLAPPED oWrite; LPOVERLAPPED oW = &oWrite; //Overlapped for writing
unsigned __int8 aUSBRxBuffer[9] = { 0,0,0,0,0,0,0,0,0 };

//Thread variable - used to "Pool" serial port for incoming data
HANDLE hThread;
DWORD WINAPI WaitForDataToRead()
{
    DWORD dwLen;

    do { ; } while (isConnected != TRUE);
    do
    {
        WaitCommEvent(hCom, &dwEventMask, oR);
        if (WaitForSingleObject(oR->hEvent, INFINITE) == WAIT_OBJECT_0)
        {
            BOOL bWFret;
            static float old_uPV; //To avoid display refresh-
            static float old_iPV; //when values are same
            static BOOL old_isConnected;//To catch first connection
            static BOOL old_isOutputPowered; 
            do
            {
                bWFret = ReadFile(hCom, aUSBRxBuffer, sizeof(aUSBRxBuffer), &dwLen, oR);
                if (dwLen == sizeof(aUSBRxBuffer))
                {
                    if (aUSBRxBuffer[0] == 0) isOutputPowered = FALSE;
                    else isOutputPowered = TRUE;
                    memcpy(&uPV, &aUSBRxBuffer[1], sizeof(float));
                    memcpy(&iPV, &aUSBRxBuffer[5], sizeof(float));
                    if (uPV != old_uPV)
                    {//To display voltage process value
                        SetDlgItemTextA(hDlg, IDC_DISP_UPV, ii_gcvt(uPV));
                        old_uPV = uPV;
                    }
                    if (iPV != old_iPV) 
                    {//To display current process value
                        SetDlgItemTextA(hDlg, IDC_DISP_IPV, ii_gcvt(iPV));
                        old_iPV = iPV;
                    }
                    if (isOutputPowered != old_isOutputPowered)
                    {//To display device power state changes
                        SetDlgItemTextA(hDlg, IDC_DISP_UPV, ii_gcvt(uPV));
                        SetDlgItemTextA(hDlg, IDC_DISP_IPV, ii_gcvt(iPV));
                        old_isOutputPowered = isOutputPowered;
                    }
                    if (isConnected != old_isConnected)
                    {// Act only once, at connection establishment
                        SetDlgItemTextA(hDlg, IDC_DISP_UPV, ii_gcvt(uPV));
                        SetDlgItemTextA(hDlg, IDC_DISP_IPV, ii_gcvt(iPV));
                        old_isConnected = isConnected;
                    }
                    Sent(hCom, oW, cmdPowerON, uSP, iSP);
                }
                if ((dwLen != sizeof(aUSBRxBuffer))&&(dwLen != 0))
                        SendMessage(hStatus, SB_SETTEXT, 0,
                            (LPARAM)L"!!!BROKEN communication to Device!!!");

            } while (dwLen > 0);
        }
    } while (TRUE);
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) /* more compact, each "case" through a single line, easier on the eyes */
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_ABOUT: onAbout(hDlg, hInstDlg); return TRUE;
        case IDC_CONNECT: if (!isConnected)
            isConnected = onConnect(hDlg, pcCommPort, hCom, hStatus,
                dcb, oR, oW, dwEventMask, comtimes); return TRUE;
        case IDC_POWER: if(isConnected)
                    cmdPowerON = onPower(cmdPowerON); return TRUE;
        case IDC_BTN_U_UP: uSP = onUsp_UP(hDlg, uSP); return TRUE;
        case IDC_BTN_U_DN: uSP = onUsp_DN(hDlg, uSP); return TRUE;
        case IDC_BTN_I_UP: iSP = onIsp_UP(hDlg, iSP); return TRUE;
        case IDC_BTN_I_DN: iSP = onIsp_DN(hDlg, iSP); return TRUE;
        case IDOK: onEnterInEditCtrl(hDlg, &uSP, &iSP); return TRUE;
        }
        break;
    case WM_HSCROLL:
        switch (GetDlgCtrlID((HWND)lParam))
        {
        case IDC_SLIDER_U: uSP = onUspTrckBar(hDlg, wParam, uSP); return TRUE;
        case IDC_SLIDER_I: iSP = onIspTrckBar(hDlg, wParam, iSP); return TRUE;
        }
        break;
    case WM_CLOSE:   onClose(hDlg, hCom, hThread,
        OutputFont, SetpointsFont); return TRUE;
    case WM_INITDIALOG: return TRUE;   
    case WM_DESTROY:  PostQuitMessage(0);  return TRUE;
    case WM_CTLCOLORSTATIC:
        {
        if (isConnected)
            if ((IDC_DISP_UPV == GetDlgCtrlID((HWND)lParam)) ||
                (IDC_DISP_IPV == GetDlgCtrlID((HWND)lParam)))
            {
                HDC hdc = (HDC)wParam;
                //Color is a WORD and it's format is 0x00BBGGRR
                //0x0036B948 -Green, 0x000D0DE2 -Red
                if (isOutputPowered)SetTextColor(hdc, 0x0036B948);
                else SetTextColor(hdc, 0x000D0DE2);
                return (LRESULT)GetStockObject(DC_BRUSH); // return a DC brush.
            }
        }
    }
    return FALSE;
}

int WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE h0, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
 // Main window variable-> HWND hDlg as global to work in WaitForDataToRead()
    MSG msg;
    BOOL ret;
    hInstDlg = hInst;

    InitCommonControls();

    // Create the window.
    // Dialog type windows are not registrated as separate window class
    hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN_DIALOG), 0, DialogProc, 0);
    //Create font for device outputs - uPV&iPV
    OutputFont = CreateFontA(60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0);
    //Create font for device setpints - uSP&iSP
    SetpointsFont = CreateFontA(40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    //Create simple statusbar on main dialog window
    hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hDlg,
        (HMENU)IDC_DIALOG_STATUS, GetModuleHandle(NULL), NULL);
    // write first init message on status bar
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)L"Hi there :)");
    //Load custom, resourse based icon
    SetClassLong(hDlg,          // window handle 
        GCL_HICON,              // changes icon 
        (LONG)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON))
    );
    //Create slider for voltage set point
    DialogBoxW(hInst, MAKEINTRESOURCE(IDC_SLIDER_U), hDlg, DialogProc);
    SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_SETRANGE, TRUE, MAKELONG(0, 300));
    SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_SETPAGESIZE, 0, 1);
    SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_SETTICFREQ, 10, 0);
    SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_SETPOS, TRUE, 0);
    SendDlgItemMessageA(hDlg, IDC_STATIC_SETV, WM_SETFONT, (WPARAM)SetpointsFont, 0);
    //Create slider for current set point
    DialogBoxW(hInst, MAKEINTRESOURCE(IDC_SLIDER_I), hDlg, DialogProc);
    SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_SETRANGE, TRUE, MAKELONG(0, 230));
    SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_SETPAGESIZE, 0, 1);
    SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_SETTICFREQ, 25, 0);
    SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_SETPOS, TRUE, 0);
    SendDlgItemMessageA(hDlg, IDC_STATIC_SETI, WM_SETFONT, (WPARAM)SetpointsFont, 0);
    //Display empty contents on process values controls
    SendDlgItemMessageA(hDlg, IDC_DISP_UPV, WM_SETFONT, (WPARAM) OutputFont, 0);
    SendDlgItemMessageA(hDlg, IDC_DISP_IPV, WM_SETFONT, (WPARAM)OutputFont, 0);
    SendDlgItemMessageA(hDlg, IDC_STATIC_OUT_V, WM_SETFONT, (WPARAM)OutputFont, 0);
    SendDlgItemMessageA(hDlg, IDC_STATIC_OUT_A, WM_SETFONT, (WPARAM)OutputFont, 0);
    SetDlgItemTextA(hDlg, IDC_DISP_UPV, "--.---");
    SetDlgItemTextA(hDlg, IDC_DISP_IPV, "-.---");
    //Create Setpoint values controls
    DialogBoxW(hInst, MAKEINTRESOURCE(IDC_DISP_USP), hDlg, DialogProc);
    DialogBoxW(hInst, MAKEINTRESOURCE(IDC_DISP_ISP), hDlg, DialogProc);
    //Display Zeroes on setpoint values controls
    SendDlgItemMessageA(hDlg, IDC_DISP_USP, WM_SETFONT, (WPARAM)SetpointsFont, 0);
    SendDlgItemMessageA(hDlg, IDC_DISP_USP, EM_SETLIMITTEXT, 6, 0); //max.6 characters
    SendDlgItemMessageA(hDlg, IDC_DISP_ISP, WM_SETFONT, (WPARAM)SetpointsFont, 0);
    SendDlgItemMessageA(hDlg, IDC_DISP_ISP, EM_SETLIMITTEXT, 5, 0); //max.5 characters
    SetDlgItemTextA(hDlg, IDC_DISP_USP, "00.000");
    SetDlgItemTextA(hDlg, IDC_DISP_ISP, "0.000");
//------------------------------------------------------------------------------------

    // Create a new thread which will start at the WaitForDataToRead function
    hThread = CreateThread(NULL, // security attributes ( default if NULL )
        0, // stack SIZE default if 0
        (LPTHREAD_START_ROUTINE)WaitForDataToRead, // Start Address
        NULL, // input data
        0, // creational flag ( start if  0 )
        NULL); // thread ID

    ShowWindow(hDlg, nCmdShow);

    while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
        if (ret == -1)
            return -1;

        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}