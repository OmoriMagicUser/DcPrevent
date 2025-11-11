#include <Windows.h>
#include <stdio.h>
#include <time.h> 
#include <stdbool.h>
#include <sys/timeb.h>
#include <stdlib.h> 

#define DEBOUNCE 45
#define DBLEEWAY 60

typedef long long MsTime; 
SYSTEMTIME sT; // This is the function for the HH:MM:SS output. 
struct timeb mT; // This is the function for millisecond precision accuracy to track intervals. 

HHOOK mouseHook;
HHOOK keyboardHook;

typedef struct {
    MsTime CurrentDown;
    MsTime LastClick; 
    int CurrentTimeDifference; 
    int LastTimeDifference; 
    int Counter; 
    int ReleaseTime;
    BOOL Blocked; 
    BOOL Pass; 
} buttonStructDown; 

typedef struct {
    MsTime CurrentUp;
    MsTime LastUp; 
    int HoldTime; 
} buttonStructUp; 

buttonStructDown LEFT_DOWN;
buttonStructUp LEFT_UP; 
buttonStructDown RIGHT_DOWN;    // The structs are all Global because they are used in multiple case's. 
buttonStructUp RIGHT_UP; 
buttonStructDown MIDDLE_DOWN;
buttonStructUp MIDDLE_UP;
buttonStructDown DPI_DOWN;

int mouseFunctionDown(buttonStructDown *BtnDn, buttonStructUp *BtnUp, char *ButtonName) {
    GetLocalTime(&sT);
    ftime(&mT);

    BtnDn->CurrentDown = mT.time * 1000 + mT.millitm;
    BtnDn->ReleaseTime = BtnDn->CurrentDown - BtnUp->LastUp;
    BtnDn->CurrentTimeDifference = BtnDn->CurrentDown - BtnDn->LastClick;
    if (BtnDn->Counter <= 0) {
        printf("%s      [%02d:%02d%02d] [First]", ButtonName, sT.wHour, sT.wMinute, sT.wSecond); 
        BtnDn->Counter++;
    } else {
        printf("%s      [%02d:%02d:%02d] [%d] [%d]", ButtonName, sT.wHour, sT.wMinute, sT.wSecond, BtnDn->CurrentTimeDifference, BtnDn->ReleaseTime);
    }
    if ((BtnDn->Blocked) && (BtnDn->LastTimeDifference + BtnDn->CurrentTimeDifference > DBLEEWAY)) {BtnDn->Pass = false;}
    if ((BtnDn->CurrentTimeDifference <= DEBOUNCE) && (BtnDn->Pass)) {
        printf("Click Blocked\n");
        BtnDn->Blocked = true;
        BtnDn->LastTimeDifference = BtnDn->CurrentTimeDifference; 
        BtnDn->LastClick = mT.time * 1000 + mT.millitm; 
        return 1; 
    } else {
        printf("\n");
        BtnDn->Blocked = false; 
        BtnDn->LastClick = mT.time * 1000 + mT.millitm; 
        return 0;
    }
}

int mouseFunctionUp(buttonStructUp *BtnUp, buttonStructDown *BtnDn, char *ButtonName) {
    GetLocalTime(&sT);
    ftime(&mT);
    BtnUp->CurrentUp = mT.time * 1000 + mT.millitm;
    BtnUp->HoldTime = BtnUp->CurrentUp - BtnDn->LastClick;
    printf("%s        [%02d:%02d:%02d] [%d]\n", ButtonName, sT.wHour, sT.wMinute, sT.wSecond, BtnUp->HoldTime);
    BtnUp->LastUp = mT.time * 1000 + mT.millitm; 
    BtnDn->Pass = true;  
    return 0;
}

LRESULT CALLBACK mouseProcess(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >=0 ) {

        switch (wParam)
        {
            case WM_LBUTTONDOWN: { 
                mouseFunctionDown(&LEFT_DOWN, &LEFT_UP, "Left Down ");
                break; 
            }
            case WM_LBUTTONUP: {
                mouseFunctionUp(&LEFT_UP, &LEFT_DOWN, "Left Up "); 
                break; 
            }
            case WM_RBUTTONDOWN: {
                mouseFunctionDown(&RIGHT_DOWN, &RIGHT_UP, "Right Down");
                break;
            }
            case WM_RBUTTONUP: {
                mouseFunctionUp(&RIGHT_UP, &RIGHT_DOWN, "Right Up"); 
                break; 
            }
            case WM_MBUTTONDOWN: {
                mouseFunctionDown(&MIDDLE_DOWN, &MIDDLE_UP, "Middle Down"); 
                break; 
            }
            case WM_MBUTTONUP: {
                mouseFunctionUp(&MIDDLE_UP, &MIDDLE_DOWN, "Middle Up");
                break; 
            }
            case WM_DPICHANGED: {
                break;
            }
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam); 
}

LRESULT CALLBACK keyboardProcess(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam; 

        

    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam); 
}

int main () {

    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProcess, NULL, 0);
    if (mouseHook == NULL) {
        printf("Failed to install mouse hook!\n");
        return 1; 
    }

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProcess, NULL, 0);
    if (keyboardHook == NULL) {
        printf("Failed to install keyboard hook\n");
    }

    printf("Listener sucessfully innitiated\n");

    MSG msg; 
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(mouseHook);
    UnhookWindowsHookEx(keyboardHook);

    return 0; 
}
