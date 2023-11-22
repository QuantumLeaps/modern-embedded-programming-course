/*******************************************************************
* HelloWin.c -- Displays "Hello, Windows" in client area
*               (c) Charles Petzold, 1988
*
* Adapted for "Modern Embedded Systems Programming" video course
* Miro Samek, 2020
*******************************************************************/
#include <windows.h> // Windows API

LRESULT CALLBACK WndProc(HWND me, UINT sig,
                         WPARAM wParam, LPARAM lParam);

#define WIN_HANDLED   ((LRESULT)0)

/*----------------------------------------------------------------
* The main entry point to the Windows apllication
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PTSTR szCmdLine, int iShowCmd)
{
    WNDCLASSEX wnd; // instance of the Window class ('wnd' object)
    HWND hwnd;  // "handle" to the window object
    int status; // status of the processing to report back to Windows

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(szCmdLine);

    // set attributes of the 'wnd' object
    wnd.cbSize = sizeof(wnd);
    wnd.style = CS_HREDRAW | CS_VREDRAW;
    wnd.cbClsExtra = 0;
    wnd.cbWndExtra = 0;
    wnd.hInstance = hInstance;
    wnd.hIcon = NULL; // LoadIcon(NULL, IDI_APPLICATION);
    wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
    wnd.hbrBackground = GetStockObject(WHITE_BRUSH);
    wnd.lpszMenuName = NULL;
    wnd.hIconSm = NULL;
    wnd.lpszClassName = "HelloWin";

    // set "virtual" function of the 'wnd' object
    wnd.lpfnWndProc = &WndProc; // attach the "window proc"

    // register the Window Class with Windows
    RegisterClassEx(&wnd);

    // constructor of the 'wnd' class
    hwnd = CreateWindow(
        wnd.lpszClassName,    // window class name
        "Hello, Windows!",    // window caption
        WS_OVERLAPPEDWINDOW,  // window style
        CW_USEDEFAULT,        // initial x position
        CW_USEDEFAULT,        // initial y position
        300,                  // initial x size
        200,                  // initial y size
        NULL,                 // parent window handle
        NULL,                 // window menu handle
        hInstance,            // program instance handle
        NULL);                // creation parameters

    ShowWindow(hwnd, iShowCmd);
    UpdateWindow(hwnd);

    // event loop ("message pump")
    while (1) {
        MSG msg; // message object to receive

        // generically WAIT for any message to arrive in the queue
        status = GetMessage(&msg, NULL, 0, 0);

        if (status == 0) { // message NOT to be processed?
            status = msg.wParam;
            break; // terminate the event loop
        }

        // dispatch to the appropriate "window proc"
        DispatchMessage(&msg);
    }

    return status; // return to Windows with the status of processing
}

/*----------------------------------------------------------------
* The "windows proc" registered for the main window
* of this application
*/
LRESULT CALLBACK WndProc(HWND me,
                         UINT sig, WPARAM wParam, LPARAM lParam)
{
    static int wm_keydown_ctr   = 0; // counter incremented in WM_KEYDOWN
    static int wm_mousemove_ctr = 0; // counter incremented in WM_MOUSEMOVE
    static char const* led_text = "OFF";

    LRESULT status; // status of the processing to report back to Windows

    switch (sig) {  // signal of the message
        case WM_CREATE: { // window was created
            status = WIN_HANDLED; // report event handled
            break;
        }
        case WM_DESTROY: { // windows is about to be destroyed
            PostQuitMessage(0);
            status = WIN_HANDLED; // report event handled
            break;
        }
        case WM_PAINT: { // window needs to be repainted
            PAINTSTRUCT ps;
            HDC hdc;
            RECT rect;
            char cBuffer[100];

            wsprintf(cBuffer, "KEYBOARD=%3d, MOUSE=%3d, LED=%s ",
                (wm_keydown_ctr % 1000),
                (wm_mousemove_ctr % 1000),
                led_text);

            // painting performed between BeginPain() and EndPaint()...
            hdc = BeginPaint(me, &ps);
            GetClientRect(me, &rect);
            DrawText(hdc, cBuffer, -1, &rect,
                     DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            EndPaint(me, &ps);

            status = WIN_HANDLED; // report event handled
            break;
        }
        case WM_TIMER: {
            led_text = "OFF"; // BSP_ledRedOff();
            InvalidateRect(me, NULL, FALSE); // force re-paining of the window
            KillTimer(me, wParam); // stop the timer
            status = WIN_HANDLED; // report event handled
            break;
        }
        case WM_KEYDOWN: { // keyboard key has been pressed
            ++wm_keydown_ctr;

            led_text = "RED"; // BSP_ledRedOn();
            InvalidateRect(me, NULL, FALSE); // force re-paining of the window
            SetTimer(me, 1, 200, NULL); // 200ms timer interval
            status = WIN_HANDLED; // report event handled
            break;
        }
        case WM_MOUSEMOVE: { // mouse has been moved over the window
            ++wm_mousemove_ctr;
            InvalidateRect(me, NULL, FALSE); // force re-paining of the window
            status = WIN_HANDLED; // report event handled
            break;
        }
        default: { // default behavior (characteristic "look and feel")
            // report the status of default processing
            status = DefWindowProc(me, sig, wParam, lParam);
            break;
        }
    }

    return status; // return to Windows with the status of processing
}
