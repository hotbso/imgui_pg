//
// ImgWindow.h
//
// Integration for dear imgui into X-Plane.
//
// Copyright (C) 2018,2020 Christopher Collins
// Copyright (C) 2026, Holger Teutsch
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#ifndef IMGWINDOW_H
#define IMGWINDOW_H

#include <climits>
#include <string>
#include <memory>

#include <XPLMDisplay.h>
#include <XPLMProcessing.h>
#include <XPLMPanelGraphics.h>

#include <imgui.h>
#include <queue>

class
ImgWindow {
public:
    static bool Initialize();
    static void Finalize();

    virtual ~ImgWindow();

    void GetWindowGeometry (int& left, int& top, int& right, int& bottom) const;

    void SetWindowGeometry (int left, int top, int right, int bottom);

    void GetWindowGeometryOS (int& left, int& top, int& right, int& bottom) const;

    void SetWindowGeometryOS (int left, int top, int right, int bottom);

    void GetWindowGeometryVR (int& width, int& height) const;

    void SetWindowGeometryVR (int width, int height);

    void GetCurrentWindowGeometry (int& left, int& top, int& right, int& bottom) const;

    void SetWindowResizingLimits (int minW, int minH, int maxW, int maxH);

    virtual void SetVisible(bool inIsVisible);

    bool GetVisible() const;

    bool IsPoppedOut () const;

    bool IsInVR () const;

    bool IsInsideSim () const;

    void SetWindowPositioningMode (XPLMWindowPositioningMode inPosMode,
                                   int                       inMonitorIdx = -1);

    void BringWindowToFront ();

    bool IsWindowInFront () const;

    void SetWindowDragArea (int left=0, int top=0, int right=INT_MAX, int bottom=INT_MAX);

    void ClearWindowDragArea ();

    bool HasWindowDragArea (int* pL = nullptr, int* pT = nullptr,
                            int* pR = nullptr, int* pB = nullptr) const;

    bool IsInsideWindowDragArea (int x, int y) const;

protected:

    bool mFirstRender;

    ImgWindow(
        int left,
        int top,
        int right,
        int bottom,
        ImFontAtlas* shared_font_atlas = nullptr,
        XPLMWindowDecoration decoration = xplm_WindowDecorationRoundRectangle,
        XPLMWindowLayer layer = xplm_WindowLayerFloatingWindows);

    ImgWindow (const ImgWindow&) = delete;
    ImgWindow& operator = (const ImgWindow&) = delete;

    void SetWindowTitle(const std::string &title);

    void moveForVR();

    virtual ImGuiWindowFlags_ beforeBegin() { return ImGuiWindowFlags_None; }

    virtual void BuildInterface() = 0;

    virtual void afterRendering() {}

    virtual bool onShow();

    void SafeDelete();

    XPLMWindowID GetWindowId () const { return mWindowID; }

    ImGuiIO& GetImGuiIO();

private:
    ImFontAtlas* mSharedFontAtlas;
    std::vector<XPLMDrawCall_t> mDrawCalls;
    static void UpdateTexture(ImTextureData* tex);

    static void DrawWindowCB(XPLMWindowID inWindowID, void *inRefcon);

    static int HandleMouseClickCB(
        XPLMWindowID inWindowID,
        int x, int y,
        XPLMMouseStatus inMouse,
        void *inRefcon);

    static void HandleKeyFuncCB(
        XPLMWindowID inWindowID,
        char inKey,
        XPLMKeyFlags inFlags,
        char inVirtualKey,
        void *inRefcon,
        int losingFocus);

    static XPLMCursorStatus HandleCursorFuncCB(
        XPLMWindowID inWindowID,
        int x, int y,
        void *inRefcon);

    static int HandleMouseWheelFuncCB(
        XPLMWindowID inWindowID,
        int x, int y,
        int wheel,
        int clicks,
        void *inRefcon);

    static int HandleRightClickFuncCB(
        XPLMWindowID inWindowID,
        int x, int y,
        XPLMMouseStatus inMouse,
        void *inRefcon);

    static float SelfDestructCallback(float inElapsedSinceLastCall,
                                      float inElapsedTimeSinceLastFlightLoop,
                                      int inCounter,
                                      void *inRefcon);
    static std::queue<ImgWindow *>  sPendingDestruction;
    static XPLMFlightLoopID         sSelfDestructHandler;

    int HandleMouseClickGeneric(
        int x, int y,
        XPLMMouseStatus inMouse,
        int button = 0);

    void RenderImGui(ImDrawData *draw_data);

    void updateImgui();

    void translateImguiToBoxel(float inX, float inY, int &outX, int &outY);

    void translateToImguiSpace(int inX, int inY, float &outX, float &outY);

    std::string mWindowTitle;

    XPLMWindowID mWindowID;
    ImGuiContext *mImGuiContext;

    int mTop;
    int mBottom;
    int mLeft;
    int mRight;

    XPLMWindowLayer mPreferredLayer;

    bool bResetBackspace = false;

    const bool bHandleWndResize;

    int minWidth    = 100;
    int minHeight   = 100;
    int maxWidth    = INT_MAX;
    int maxHeight   = INT_MAX;

    int dragLeft    = -1;
    int dragTop     = -1;
    int dragRight   = -1;
    int dragBottom  = -1;

    int lastMouseDragX  = -1;
    int lastMouseDragY  = -1;

    struct DragTy {
        bool wnd    : 1;
        bool left   : 1;
        bool top    : 1;
        bool right  : 1;
        bool bottom : 1;

        DragTy () { clear(); }
        void clear () { wnd = left = top = right = bottom = false; }
        operator bool() const { return wnd || left || top || right || bottom; }
    } dragWhat;
};

#endif
