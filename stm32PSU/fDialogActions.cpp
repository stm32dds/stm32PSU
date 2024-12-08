#include "main.h"

char* ii_gcvt(float t)
{
    static CHAR szChDlgTmp[_CVTBUFSIZE];
    CHAR szTmp[_CVTBUFSIZE];
    if(t >=10 )
        _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), t, 5);
    if(10 > t >= 1)
        _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), t, 4);
    if(1 > t >= 0.1)
        _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), t, 3);
    if (0.1 > t >= 0.01)
    {
        _gcvt_s(szTmp, sizeof(szTmp), (t*10), 4);
        szChDlgTmp[0] = '0'; 
        szChDlgTmp[1] = '.';
        szChDlgTmp[2] = '0';
        szChDlgTmp[3] = szTmp[2];
        szChDlgTmp[4] = szTmp[3];
        szChDlgTmp[5] = 0x00;
    }
    if (t < 0.01)
    {
        _gcvt_s(szTmp, sizeof(szTmp), (t * 100), 5);
        szChDlgTmp[0] = '0';
        szChDlgTmp[1] = '.';
        szChDlgTmp[2] = '0';
        szChDlgTmp[3] = '0';
        szChDlgTmp[4] = szTmp[2];
        szChDlgTmp[5] = 0x00;
    }
    return szChDlgTmp;
}

void onEnterInEditCtrl(HWND hDlg, float* uSP, float* iSP)
{
    HWND hFocused = GetFocus();
    CHAR tmpBuff[10];
    float tmpFloat;
    int txtLen;
    if (hFocused == GetDlgItem(hDlg, IDC_DISP_USP))
    {
        GetDlgItemTextA(hDlg, IDC_DISP_USP, tmpBuff, 7);
        tmpFloat = atof(tmpBuff);
        if ((U_SP_MIN <= tmpFloat) && (tmpFloat <= U_SP_MAX))
        {
            SetDlgItemTextA(hDlg, IDC_DISP_USP, ii_gcvt(tmpFloat));
            SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_SETPOS, TRUE, (int)(tmpFloat * 10));
            txtLen = SendDlgItemMessageW(hDlg, IDC_DISP_USP, WM_GETTEXTLENGTH, 0, 0);
            SendDlgItemMessageW(hDlg, IDC_DISP_USP, EM_SETSEL, txtLen, txtLen);
            *uSP = tmpFloat;
        }
        //MessageBox(hDlg, L"Voltage field focused", L"Info", MB_OK);
    }
    if (hFocused == GetDlgItem(hDlg, IDC_DISP_ISP))
    {
        GetDlgItemTextA(hDlg, IDC_DISP_ISP, tmpBuff, 6);
        tmpFloat = atof(tmpBuff);
        if ((I_SP_MIN <= tmpFloat) && (tmpFloat <= I_SP_MAX))
        {
            SetDlgItemTextA(hDlg, IDC_DISP_ISP, ii_gcvt(tmpFloat));
            SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_SETPOS, TRUE, (int)(tmpFloat * 50));
            txtLen = SendDlgItemMessageW(hDlg, IDC_DISP_ISP, WM_GETTEXTLENGTH, 0, 0);
            SendDlgItemMessageW(hDlg, IDC_DISP_ISP, EM_SETSEL, txtLen, txtLen);
            *iSP = tmpFloat;
        }
        // MessageBox(hDlg, L"Current field focused", L"Info", MB_OK);
    }
}


float onUspTrckBar(HWND hDlg, WPARAM wParam, float uSP)
{
    int pos = SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_GETPOS, 0, 0);
    if (((LOWORD(wParam)) == TB_TOP) || ((LOWORD(wParam)) == TB_BOTTOM))
    {// Catch <Home> or <End> keys - no position change
        pos = (int)(uSP * 10);
        SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_SETPOS, TRUE, pos);
    }
    else
    {// All other trakbar actions
        uSP = ((float)pos) / 10;
    }
    SetDlgItemTextA(hDlg, IDC_DISP_USP, ii_gcvt(uSP));
    return uSP;
}

float onIspTrckBar(HWND hDlg, WPARAM wParam, float iSP)
{
    int pos = SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_GETPOS, 0, 0);
    if (((LOWORD(wParam)) == TB_TOP) || ((LOWORD(wParam)) == TB_BOTTOM))
    {// Catch <Home> or <End> keys - no position change
        pos = (int)(iSP * 50);
        SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_SETPOS, TRUE, pos);
    }
    else
    {// All other trakbar actions
        iSP = ((float)pos) / 50;
    }
    SetDlgItemTextA(hDlg, IDC_DISP_ISP, ii_gcvt(iSP));
    return iSP;
}

float onUsp_UP(HWND hDlg, float uSP)
{
    uSP = uSP + 0.001;
    if (uSP > U_SP_MAX) uSP = U_SP_MAX;
    if (uSP < U_SP_MIN) uSP = U_SP_MIN;
    if (uSP < SP_STEP_MIN) uSP = U_SP_MIN;
    SetDlgItemTextA(hDlg, IDC_DISP_USP, ii_gcvt(uSP));
    SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_SETPOS, TRUE, (int)(uSP*10));
    return uSP;
}

float onUsp_DN(HWND hDlg, float uSP)
{
    uSP = uSP - 0.001;
    if (uSP > U_SP_MAX) uSP = U_SP_MAX;
    if (uSP < U_SP_MIN) uSP = U_SP_MIN;
    if (uSP < SP_STEP_MIN) uSP = U_SP_MIN;
    SetDlgItemTextA(hDlg, IDC_DISP_USP, ii_gcvt(uSP));
    SendDlgItemMessageW(hDlg, IDC_SLIDER_U, TBM_SETPOS, TRUE, (int)(uSP * 10));
    return uSP;
}

float onIsp_UP(HWND hDlg, float iSP)
{
    iSP = iSP + 0.001;
    if (iSP > I_SP_MAX) iSP = I_SP_MAX;
    if (iSP < I_SP_MIN) iSP = I_SP_MIN;
    if (iSP < SP_STEP_MIN) iSP = I_SP_MIN;
    SetDlgItemTextA(hDlg, IDC_DISP_ISP, ii_gcvt(iSP));
    SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_SETPOS, TRUE, (int)(iSP * 50));
    return iSP;
}

float onIsp_DN(HWND hDlg, float iSP)
{
    iSP = iSP - 0.001;
    if (iSP > I_SP_MAX) iSP = I_SP_MAX;
    if (iSP < I_SP_MIN) iSP = I_SP_MIN;
    if (iSP < SP_STEP_MIN) iSP = I_SP_MIN;
    SetDlgItemTextA(hDlg, IDC_DISP_ISP, ii_gcvt(iSP));
    SendDlgItemMessageW(hDlg, IDC_SLIDER_I, TBM_SETPOS, TRUE, (int)(iSP * 50));
    return iSP;
}

BOOL onPower(BOOL cmdPowerON)
{
    if (cmdPowerON) cmdPowerON = FALSE;
    else cmdPowerON = TRUE;
    return cmdPowerON;
}

void onClose(HWND hDlg, HANDLE hCom, 
    HANDLE hThread, HFONT OutputFont, HFONT SetpointsFont)
{
    CloseHandle(hThread); // Close reading thread
    CloseHandle(hCom); //Closes hStatus also
    DeleteObject((HGDIOBJ) OutputFont);
    DeleteObject((HGDIOBJ) SetpointsFont);
    DestroyWindow(hDlg); //Destroys Main Dialog Window
    //& issue WM_DESTROY message to DialogProc

}

// Message handler for "About" box.
INT_PTR CALLBACK AboutDialog(HWND hDlg,
    UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case NM_CLICK:
            ShellExecute(NULL, L"open",
                L"https://www.stm32dds.tk/stm32-psu", NULL, NULL, SW_SHOW);
            return (INT_PTR)TRUE;
        }
    case WM_INITDIALOG:
        SetDlgItemTextA(hDlg, IDC_VERSION,
            (LPCSTR)"0.1"
        );
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Create and Show the box - "About".
void onAbout(HWND hDlg, HINSTANCE hInst)
{
    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, AboutDialog);
}