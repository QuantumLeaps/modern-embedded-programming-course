//============================================================================
// QWIN GUI facilities for building realistic embedded front panels
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qwin_gui.h"
#include <stdlib.h>

static HWND l_hWnd;
static HDC  l_hDC;

//--------------------------------------------------------------------------
HWND CreateCustDialog(HINSTANCE hInst, int iDlg, HWND hParent,
                      WNDPROC lpfnWndProc, LPCTSTR lpWndClass)
{
    WNDCLASSEX wndclass;

    wndclass.cbSize        = sizeof(wndclass);
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = lpfnWndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = DLGWINDOWEXTRA;
    wndclass.hInstance     = hInst;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = lpWndClass;
    wndclass.hIconSm       = NULL;

    RegisterClassEx(&wndclass);

    l_hWnd = CreateDialog(hInst, MAKEINTRESOURCE(iDlg), hParent, NULL);
    l_hDC = GetDC(l_hWnd); // obtain the DC for the client area of the window

    // NOTE: WM_INITDIALOG provides stimulus for initializing dialog controls.
    // Dialog box procedures typically use this message to initialize controls
    // and carry out any other initialization tasks that affect the appearance
    // of the dialog box.
    //
    SendMessage(l_hWnd, WM_INITDIALOG, (WPARAM)0, (LPARAM)0);

    return l_hWnd;
}

//--------------------------------------------------------------------------
void OwnerDrawnButton_init(OwnerDrawnButton * const me,
                           UINT itemID,
                           HBITMAP hBitmapUp, HBITMAP hBitmapDwn,
                           HCURSOR hCursor)
{
    me->itemID      = itemID;
    me->hBitmapUp   = hBitmapUp;
    me->hBitmapDown = hBitmapDwn;
    me->hCursor     = hCursor;
    me->isDepressed = 0;
}
//............................................................................
void OwnerDrawnButton_xtor(OwnerDrawnButton * const me) {
    DeleteObject(me->hBitmapUp);
    DeleteObject(me->hBitmapDown);
    DeleteObject(me->hCursor);
}
//............................................................................
enum OwnerDrawnButtonAction OwnerDrawnButton_draw(
                               OwnerDrawnButton * const me,
                               LPDRAWITEMSTRUCT lpdis)
{
    enum OwnerDrawnButtonAction ret = BTN_NOACTION;

    if ((lpdis->itemAction & ODA_DRAWENTIRE) != 0U) {
        if (me->hCursor != NULL) {
           SetClassLongPtr(lpdis->hwndItem,
                           GCLP_HCURSOR, (LONG_PTR)me->hCursor);
           me->hCursor = NULL; // mark the cursor set
        }
        DrawBitmap(lpdis->hDC, me->hBitmapUp,
                   lpdis->rcItem.left, lpdis->rcItem.top);
        me->isDepressed = 0;
        ret = BTN_PAINTED;
    }
    else if ((lpdis->itemAction & ODA_SELECT) != 0U) {
        if ((lpdis->itemState & ODS_SELECTED) != 0U) {
            DrawBitmap(lpdis->hDC, me->hBitmapDown,
                       lpdis->rcItem.left, lpdis->rcItem.top);
            me->isDepressed = !0;
            ret = BTN_DEPRESSED;
        }
        else {
            // NOTE: the bitmap for button "UP" look will be
            // drawn in the ODA_DRAWENTIRE action
            //
            me->isDepressed = 0;
            ret = BTN_RELEASED;
        }
    }
    return ret;
}
//............................................................................
void OwnerDrawnButton_set(OwnerDrawnButton * const me, int isDepressed) {
    if (me->isDepressed != isDepressed) {
        HWND hItem = GetDlgItem(l_hWnd, me->itemID);
        me->isDepressed = isDepressed;
        if (isDepressed) {
            DrawBitmap(GetDC(hItem), me->hBitmapDown, 0, 0);
        }
        else {
            DrawBitmap(GetDC(hItem), me->hBitmapUp, 0, 0);
        }
    }
}
//............................................................................
BOOL OwnerDrawnButton_isDepressed(OwnerDrawnButton const * const me) {
    return me->isDepressed;
}

//--------------------------------------------------------------------------
void GraphicDisplay_init(GraphicDisplay * const me,
                         UINT width,  UINT height,
                         UINT itemID, BYTE const bgColor[3])
{
    BITMAPINFO bi24BitInfo;
    RECT rect;

    me->src_width  = width;
    me->src_height = height;

    me->hItem      = GetDlgItem(l_hWnd, itemID);
    me->dst_hDC    = GetDC(me->hItem);
    GetWindowRect(me->hItem, &rect);
    me->dst_width  = rect.right - rect.left;
    me->dst_height = rect.bottom - rect.top;

    me->bgColor[0] = bgColor[0];
    me->bgColor[1] = bgColor[1];
    me->bgColor[2] = bgColor[2];

    bi24BitInfo.bmiHeader.biBitCount    = 3U*8U;  // 3 RGB bytes
    bi24BitInfo.bmiHeader.biCompression = BI_RGB; // RGB color
    bi24BitInfo.bmiHeader.biPlanes      = 1U;
    bi24BitInfo.bmiHeader.biSize        = sizeof(bi24BitInfo.bmiHeader);
    bi24BitInfo.bmiHeader.biWidth       = me->src_width;
    bi24BitInfo.bmiHeader.biHeight      = me->src_height;

    me->src_hDC = CreateCompatibleDC(me->dst_hDC);
    me->hBitmap = CreateDIBSection(me->src_hDC, &bi24BitInfo, DIB_RGB_COLORS,
                                   (void **)&me->bits, 0, 0);
    SelectObject(me->src_hDC, me->hBitmap);

    GraphicDisplay_clear(me);
    GraphicDisplay_redraw(me);
}
//............................................................................
void GraphicDisplay_xtor(GraphicDisplay * const me) {
    DeleteDC(me->src_hDC);
    DeleteObject(me->hBitmap);
    OutputDebugString("GraphicDisplay_xtor\n");
}
//............................................................................
void GraphicDisplay_clear(GraphicDisplay * const me) {
    UINT n;
    BYTE r = me->bgColor[0];
    BYTE g = me->bgColor[1];
    BYTE b = me->bgColor[2];
    BYTE *bits = me->bits;

    for (n = me->src_width * me->src_height; n != 0U; --n, bits += 3) {
        bits[0] = b;
        bits[1] = g;
        bits[2] = r;
    }
}
//............................................................................
void GraphicDisplay_redraw(GraphicDisplay * const me) {
    StretchBlt(me->dst_hDC, 0, 0, me->dst_width, me->dst_height,
               me->src_hDC, 0, 0, me->src_width, me->src_height, SRCCOPY);
}

// SegmentDisplay ----------------------------------------------------------
void SegmentDisplay_init(SegmentDisplay * const me,
                         UINT segmentNum, UINT bitmapNum)
{
    UINT n;

    me->hSegment = (HWND *)calloc(segmentNum, sizeof(HWND));
    me->segmentNum = segmentNum;
    for (n = 0U; n < segmentNum; ++n) {
        me->hSegment[n] = NULL;
    }
    me->hBitmap = (HBITMAP *)calloc(bitmapNum, sizeof(HBITMAP));
    me->bitmapNum = bitmapNum;
    for (n = 0U; n < bitmapNum; ++n) {
        me->hBitmap[n] = NULL;
    }
}
//............................................................................
void SegmentDisplay_xtor(SegmentDisplay * const me) {
    UINT n;

    for (n = 0U; n < me->segmentNum; ++n) {
        DeleteObject(me->hSegment[n]);
    }
    free(me->hSegment);

    for (n = 0U; n < me->bitmapNum; ++n) {
        DeleteObject(me->hBitmap[n]);
    }
    free(me->hBitmap);
}
//............................................................................
BOOL SegmentDisplay_initSegment(SegmentDisplay * const me,
                                UINT segmentNum, UINT segmentID)
{
    if (segmentNum < me->segmentNum) {
        me->hSegment[segmentNum] = GetDlgItem(l_hWnd, segmentID);
        return me->hSegment[segmentNum] != NULL;
    }
    else {
        return FALSE;
    }
}
//............................................................................
BOOL SegmentDisplay_initBitmap(SegmentDisplay * const me,
                               UINT bitmapNum, HBITMAP hBitmap)
{
    if ((bitmapNum < me->bitmapNum) && (hBitmap != NULL)) {
        me->hBitmap[bitmapNum] = hBitmap;
        return TRUE;
    }
    else {
        return FALSE;
    }
}
//............................................................................
BOOL SegmentDisplay_setSegment(SegmentDisplay * const me,
                               UINT segmentNum, UINT bitmapNum)
{
    if ((segmentNum < me->segmentNum) && (bitmapNum < me->bitmapNum)) {
        SendMessage(me->hSegment[segmentNum], STM_SETIMAGE,
                IMAGE_BITMAP,
                (LPARAM)me->hBitmap[bitmapNum]);
        return TRUE;
    }
    else {
        return FALSE;
    }
}

//--------------------------------------------------------------------------
// DrawBitmap() function adapted from the book "Programming Windows" by
// Charles Petzold.
//
void DrawBitmap(HDC hdc, HBITMAP hBitmap,
                int xStart, int yStart)
{
    BITMAP bm;
    POINT  ptSize, ptOrg;
    HDC    hdcMem = CreateCompatibleDC(hdc);

    SelectObject(hdcMem, hBitmap);
    SetMapMode(hdcMem, GetMapMode(hdc));

    GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm);
    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    DPtoLP(hdc, &ptSize, 1);

    ptOrg.x = 0;
    ptOrg.y = 0;
    DPtoLP(hdcMem, &ptOrg, 1);

    BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y,
           hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
    DeleteDC(hdcMem);
}


