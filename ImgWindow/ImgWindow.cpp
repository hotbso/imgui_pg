/*
 * ImgWindow.cpp
 *
 * Integration for dear imgui into X-Plane.
 *
 * Copyright (C) 2018,2020, Christopher Collins
 * Copyright (C) 2026, Holger Teutsch
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/

#if IBM
#include <windows.h>
#elif APL
#include <Carbon/Carbon.h>
#endif

#include "ImgWindow.h"

#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <XPLMPanelGraphics.h>

#include "log_msg.h"

// size of "frame" around a resizable window, by which its size can be changed
constexpr int WND_RESIZE_LEFT_WIDTH     = 15;
constexpr int WND_RESIZE_TOP_WIDTH      =  5;
constexpr int WND_RESIZE_RIGHT_WIDTH    = 15;
constexpr int WND_RESIZE_BOTTOM_WIDTH   = 15;

static XPLMDataRef		gVrEnabledRef			= nullptr;
static XPLMDataRef		gFrameRatePeriodRef     = nullptr;

static ImGuiKey TranslateXPLMKeyToImGui(unsigned char inVirtualKey) {
    switch (inVirtualKey) {
        case XPLM_VK_TAB: return ImGuiKey_Tab;
        case XPLM_VK_LEFT: return ImGuiKey_LeftArrow;
        case XPLM_VK_RIGHT: return ImGuiKey_RightArrow;
        case XPLM_VK_UP: return ImGuiKey_UpArrow;
        case XPLM_VK_DOWN: return ImGuiKey_DownArrow;
        case XPLM_VK_PRIOR: return ImGuiKey_PageUp;
        case XPLM_VK_NEXT: return ImGuiKey_PageDown;
        case XPLM_VK_HOME: return ImGuiKey_Home;
        case XPLM_VK_END: return ImGuiKey_End;
        case XPLM_VK_INSERT: return ImGuiKey_Insert;
        case XPLM_VK_DELETE: return ImGuiKey_Delete;
        case XPLM_VK_BACK: return ImGuiKey_Backspace;
        case XPLM_VK_SPACE: return ImGuiKey_Space;
        case XPLM_VK_RETURN: return ImGuiKey_Enter;
        case XPLM_VK_ESCAPE: return ImGuiKey_Escape;
        case XPLM_VK_ENTER: return ImGuiKey_KeypadEnter;
        case XPLM_VK_A: return ImGuiKey_A;
        case XPLM_VK_C: return ImGuiKey_C;
        case XPLM_VK_V: return ImGuiKey_V;
        case XPLM_VK_X: return ImGuiKey_X;
        case XPLM_VK_Y: return ImGuiKey_Y;
        case XPLM_VK_Z: return ImGuiKey_Z;
        case XPLM_VK_NUMPAD0: return ImGuiKey_Keypad0;
        case XPLM_VK_NUMPAD1: return ImGuiKey_Keypad1;
        case XPLM_VK_NUMPAD2: return ImGuiKey_Keypad2;
        case XPLM_VK_NUMPAD3: return ImGuiKey_Keypad3;
        case XPLM_VK_NUMPAD4: return ImGuiKey_Keypad4;
        case XPLM_VK_NUMPAD5: return ImGuiKey_Keypad5;
        case XPLM_VK_NUMPAD6: return ImGuiKey_Keypad6;
        case XPLM_VK_NUMPAD7: return ImGuiKey_Keypad7;
        case XPLM_VK_NUMPAD8: return ImGuiKey_Keypad8;
        case XPLM_VK_NUMPAD9: return ImGuiKey_Keypad9;
        case XPLM_VK_0: return ImGuiKey_0;
        case XPLM_VK_1: return ImGuiKey_1;
        case XPLM_VK_2: return ImGuiKey_2;
        case XPLM_VK_3: return ImGuiKey_3;
        case XPLM_VK_4: return ImGuiKey_4;
        case XPLM_VK_5: return ImGuiKey_5;
        case XPLM_VK_6: return ImGuiKey_6;
        case XPLM_VK_7: return ImGuiKey_7;
        case XPLM_VK_8: return ImGuiKey_8;
        case XPLM_VK_9: return ImGuiKey_9;
    }
    return ImGuiKey_None;
}

ImgWindow::ImgWindow(
	int left,
	int top,
	int right,
	int bottom,
	XPLMWindowDecoration decoration,
	XPLMWindowLayer layer) :
    mFirstRender(true),
	mPreferredLayer(layer),
    bHandleWndResize(xplm_WindowDecorationSelfDecoratedResizable == decoration)
{
    mImGuiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(mImGuiContext);
	XPLMCreateWindow_t	windowParams = {
		sizeof(windowParams),
		left,
		top,
		right,
		bottom,
		0,
		DrawWindowCB,
		HandleMouseClickCB,
		HandleKeyFuncCB,
		NULL, //HandleCursorFuncCB
		HandleMouseWheelFuncCB,
		reinterpret_cast<void*>(this),
		decoration,
		layer,
		HandleRightClickFuncCB,
        xplm_WindowContentTypePanelGraphics,
        nullptr
	};

    mWindowID = XPLMCreateWindowEx(&windowParams);
    mDrawCalls.reserve(50); // reserve some space to avoid reallocations

    mImGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(mImGuiContext);
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;       // is not compatible with imgWindow, disable imgui.ini file

    // disable window rounding since we're not rendering the frame anyway.
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 0;

    // disable OSX-like keyboard behaviours always - we don't have the keymapping for it.
    io.ConfigMacOSXBehaviors = false;

    // try to inhibit a few resize/move behaviours that won't play nice with our window control.
    io.ConfigWindowsResizeFromEdges = false;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    io.BackendFlags |=
        ImGuiBackendFlags_RendererHasTextures;  // We can honor ImGuiPlatformIO::Textures[] requests during render.
}

ImgWindow::~ImgWindow()
{
	XPLMDestroyWindow(mWindowID);
    LogMsg("mDrawCalls.capacity(): %zu", mDrawCalls.capacity());

    ImGui::SetCurrentContext(mImGuiContext);

    LogMsg("ImgWindow::Finalize: destroying ImGui textures");
    for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
        if (tex->RefCount == 1) {
            tex->SetStatus(ImTextureStatus_WantDestroy);
            UpdateTexture(tex);
        }
    LogMsg("ImgWindow::Finalize: destroying ImGui context %p", (void*)mImGuiContext);
    ImGui::DestroyContext(mImGuiContext);
}

ImGuiIO& ImgWindow::GetImGuiIO() {
    ImGui::SetCurrentContext(mImGuiContext);
    return ImGui::GetIO();
}

void
ImgWindow::GetCurrentWindowGeometry (int& left, int& top, int& right, int& bottom) const
{
    if (IsPoppedOut())
        GetWindowGeometryOS(left, top, right, bottom);
    else if (IsInVR()) {
        left = bottom = 0;
        GetWindowGeometryVR(right, top);
    } else {
        GetWindowGeometry(left, top, right, bottom);
    }
}

void
ImgWindow::SetWindowResizingLimits (int minW, int minH, int maxW, int maxH)
{
    minWidth  = minW;
    minHeight = minH;
    maxWidth  = maxW;
    maxHeight = maxH;
    XPLMSetWindowResizingLimits(mWindowID, minW, minH, maxW, maxH);
}

// static
void ImgWindow::UpdateTexture(ImTextureData* tex) {
    if (tex->Status == ImTextureStatus_WantCreate) {
        const unsigned char *pixels = static_cast<const unsigned char *>(tex->GetPixels());

        void* pg_tex_id = XPLMCreateTexture(pixels, tex->Width, tex->Height);
        tex->SetTexID((ImTextureID)(intptr_t)pg_tex_id);  // specify backend-specific ImTextureID identifier
        tex->SetStatus(ImTextureStatus_OK);
        LogMsg("ImgWindow::UpdateTexture: Created texture %p for ImTextureData %p", pg_tex_id, (void *)tex);
    }

    if (tex->Status == ImTextureStatus_WantUpdates) {
        // I assume update is not supported by X-Plane, so we destroy and recreate the texture instead.
        void* pg_tex_id = (void*)(intptr_t)tex->GetTexID();
        if (pg_tex_id) {
            XPLMDestroyTexture(pg_tex_id);
            LogMsg("ImgWindow::UpdateTexture: Destroyed texture %p for ImTextureData %p", pg_tex_id, (void *)tex);
        }
        const unsigned char *pixels = static_cast<const unsigned char *>(tex->GetPixels());
        pg_tex_id = XPLMCreateTexture(pixels, tex->Width, tex->Height);
        tex->SetTexID((ImTextureID)(intptr_t)pg_tex_id);  // specify backend-specific ImTextureID identifier
        tex->SetStatus(ImTextureStatus_OK);
        LogMsg("ImgWindow::UpdateTexture: Created texture %p for ImTextureData %p", pg_tex_id, (void *)tex);
    }

    if (tex->Status == ImTextureStatus_WantDestroy) {
        void* pg_tex_id = (void*)(intptr_t)tex->GetTexID();
        if (pg_tex_id) {
            XPLMDestroyTexture(pg_tex_id);
            LogMsg("ImgWindow::UpdateTexture: Destroyed texture %p for ImTextureData %p", pg_tex_id, (void *)tex);
        }

        tex->SetTexID(ImTextureID_Invalid);
        tex->SetStatus(ImTextureStatus_Destroyed);
    }
}


// Use panel graphics to render the ImGui draw data.
void ImgWindow::RenderImGui(ImDrawData * draw_data) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer
    // coordinates)
    ImGui::SetCurrentContext(mImGuiContext);
    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplayFramebufferScale.x != 1.0 || io.DisplayFramebufferScale.y != 1.0) {
        draw_data->ScaleClipRects(io.DisplayFramebufferScale);
    }

    if (draw_data->Textures != nullptr)
        for (ImTextureData* tex : *draw_data->Textures)
            if (tex->Status != ImTextureStatus_OK)
                UpdateTexture(tex);

    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        // LogMsg("ImgWindow::RenderImGui: processing draw list %d of %d", n, draw_data->CmdListsCount);
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

        XPLMMesh_t mesh;
        mesh.vertex_count = cmd_list->VtxBuffer.Size;
        mesh.vertices = (const float*)vtx_buffer;
        mesh.index_count = cmd_list->IdxBuffer.Size;
        mesh.indices = idx_buffer;

        int idx_ofs = 0;
        mDrawCalls.clear();
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            XPLMDrawCall_t drc;
            drc.tex_ref = (void*)(intptr_t)pcmd->GetTexID();
            drc.scissors[0] = pcmd->ClipRect.x;
            drc.scissors[1] = pcmd->ClipRect.y;
            drc.scissors[2] = pcmd->ClipRect.z;
            drc.scissors[3] = pcmd->ClipRect.w;
            drc.idx_offset = idx_ofs;
            drc.element_count = pcmd->ElemCount;
            drc.vtx_offset = 0;  // since we're using a single mesh for the entire draw list
            mDrawCalls.push_back(drc);
            idx_ofs += pcmd->ElemCount;
        }

        XPLMDrawCalls(&mesh, mDrawCalls.size(), mDrawCalls.data());
    }
}

void ImgWindow::translateToImguiSpace(int inX, int inY, float& outX, float& outY) {
    outX = static_cast<float>(inX - mLeft);
    if (outX < 0.0f || outX > (float)(mRight - mLeft)) {
        outX = -FLT_MAX;
        outY = -FLT_MAX;
        return;
    }
    outY = static_cast<float>(mTop - inY);
    if (outY < 0.0f || outY > (float)(mTop - mBottom)) {
        outX = -FLT_MAX;
        outY = -FLT_MAX;
        return;
    }
}

void ImgWindow::translateImguiToBoxel(float inX, float inY, int& outX, int& outY) {
    outX = (int)(mLeft + inX);
    outY = (int)(mTop - inY);
}

void ImgWindow::updateImgui() {
    ImGui::SetCurrentContext(mImGuiContext);
    auto& io = ImGui::GetIO();

    // transfer the window geometry to ImGui
    XPLMGetWindowGeometry(mWindowID, &mLeft, &mTop, &mRight, &mBottom);

    float win_width = static_cast<float>(mRight - mLeft);
    float win_height = static_cast<float>(mTop - mBottom);

    // Needed to add this to prevent io.DeltaTime causing a CTD because when X-Plane starts FrameRatePeriod is equal
    // to 0.0f
    float FrameRatePeriod = XPLMGetDataf(gFrameRatePeriodRef);
    if (FrameRatePeriod > 0.0f) {
        io.DeltaTime = XPLMGetDataf(gFrameRatePeriodRef);
    }
    io.DisplaySize = ImVec2(win_width, win_height);
    // in boxels, we're always scale 1, 1.
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2((float)0.0, (float)0.0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(win_width, win_height), ImGuiCond_Always);

    // and construct the window
    ImGui::Begin(
        mWindowTitle.c_str(), nullptr,
        beforeBegin() | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    BuildInterface();
    ImGui::End();

    // finally, handle window focus.
    int hasKeyboardFocus = XPLMHasKeyboardFocus(mWindowID);
    if (io.WantTextInput && !hasKeyboardFocus) {
        XPLMTakeKeyboardFocus(mWindowID);
    } else if (!io.WantTextInput && hasKeyboardFocus) {
        XPLMTakeKeyboardFocus(nullptr);
        // reset keysdown otherwise we'll think any keys used to defocus the keyboard are still down!
        io.ClearInputKeys();
    }

    // X-Plane does not make a reliable callback when the mouse leaves a Window so we query the mouse here and feed
    // it to ImGui.
    int m_x, m_y;
    XPLMGetMouseLocationGlobal(&m_x, &m_y);
    float outX, outY;
    translateToImguiSpace(m_x, m_y, outX, outY);
    io.AddMousePosEvent(outX, outY);

    mFirstRender = false;
}

void ImgWindow::DrawWindowCB(XPLMWindowID /* inWindowID */, void* inRefcon) {
    auto* thisWindow = reinterpret_cast<ImgWindow*>(inRefcon);

    thisWindow->updateImgui();

    ImGui::SetCurrentContext(thisWindow->mImGuiContext);
    ImGui::Render();

    thisWindow->RenderImGui(ImGui::GetDrawData());

    // Give subclasses a chance to do something after all rendering
    thisWindow->afterRendering();

    // Hack: Reset the Backspace key if in VR (see HandleKeyFuncCB for details)
    if (thisWindow->bResetBackspace) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiKey_Backspace, false);
        thisWindow->bResetBackspace = false;
    }
}

int ImgWindow::HandleMouseClickCB(XPLMWindowID /* inWindowID */, int x, int y, XPLMMouseStatus inMouse,
                                    void* inRefcon) {
    LogMsg("ImgWindow::HandleMouseClickCB: x=%d, y=%d, inMouse=%d", x, y, (int)inMouse);
    auto* thisWindow = reinterpret_cast<ImgWindow*>(inRefcon);
    return thisWindow->HandleMouseClickGeneric(x, y, inMouse, 0);
}

int ImgWindow::HandleMouseClickGeneric(int x, int y, XPLMMouseStatus inMouse, int button) {
    ImGui::SetCurrentContext(mImGuiContext);
    ImGuiIO& io = ImGui::GetIO();

    // Tell ImGui the mous position relative to the window
    float outX, outY;
    translateToImguiSpace(x, y, outX, outY);
    io.AddMousePosEvent(outX, outY);
    const int loc_x = int(outX);  // local x, relative to top/left corner
    const int loc_y = int(outY);
    const int dx = x - lastMouseDragX;  // dragged how far since last down/drag event?
    const int dy = y - lastMouseDragY;

    bool shift{}, ctrl{};

#if IBM  // Windows
    shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    ctrl  = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    //LogMsg("HandleMouseClickGeneric: x=%d, y=%d, inMouse=%d, button=%d, loc_x=%d, loc_y=%d, dx=%d, dy=%d, ShiftPressed=%d, CtrlPressed=%d",
    //        x, y, (int)inMouse, button, loc_x, loc_y, dx, dy, shift, ctrl);
#elif APL // macOS
    UInt32 modifiers = GetCurrentKeyModifiers();
    shift = (modifiers & shiftKey) != 0;
    ctrl  = (modifiers & controlKey) != 0;
#elif LIN
    #warning "Linux: HandleMouseClickGeneric: Sorry, Shift/Ctrl detection not implemented, no multiselection possible!"
#endif

    switch (inMouse) {
        case xplm_MouseDrag:
            io.AddMouseButtonEvent(button, true);

            // Any kind of self-dragging/resizing only happens with a floating window in the sim
            if (button == 0 &&    // left button
                IsInsideSim() &&  // floating window in sim
                dragWhat &&       // and if there actually _is_ dragging
                (dx != 0 || dy != 0)) {
                // shall we drag the entire window?
                if (dragWhat.wnd) {
                    mLeft += dx;  // move the wdinow
                    mRight += dx;
                    mTop += dy;
                    mBottom += dy;
                } else {
                    // do we need to handle window resize?
                    if (dragWhat.left)
                        mLeft += dx;
                    if (dragWhat.top)
                        mTop += dy;
                    if (dragWhat.right)
                        mRight += dx;
                    if (dragWhat.bottom)
                        mBottom += dy;

                    // Make sure resizing limits are honored
                    if (mRight - mLeft < minWidth) {
                        if (dragWhat.left)
                            mLeft = mRight - minWidth;
                        else
                            mRight = mLeft + minWidth;
                    }
                    if (mRight - mLeft > maxWidth) {
                        if (dragWhat.left)
                            mLeft = mRight - maxWidth;
                        else
                            mRight = mLeft + maxWidth;
                    }
                    if (mTop - mBottom < minHeight) {
                        if (dragWhat.top)
                            mTop = mBottom + minHeight;
                        else
                            mBottom = mTop - minHeight;
                    }
                    if (mTop - mBottom > maxHeight) {
                        if (dragWhat.top)
                            mTop = mBottom + maxHeight;
                        else
                            mBottom = mTop - maxHeight;
                    }
                    // FIXME: If we had to apply resizing restricitons, then mouse and window frame will now be out
                    // of synch
                }

                // Change window geometry
                SetWindowGeometry(mLeft, mTop, mRight, mBottom);
                // now that the window has moved under the mouse we need to update relative mouse pos
                float newOutX, newOutY;
                translateToImguiSpace(x, y, newOutX, newOutY);
                io.AddMousePosEvent(newOutX, newOutY);
                // Update the last handled position
                lastMouseDragX = x;
                lastMouseDragY = y;
            }
            break;

        case xplm_MouseDown:
            if (ctrl)
                io.AddKeyEvent(ImGuiMod_Ctrl, true);
            if (shift)
                io.AddKeyEvent(ImGuiMod_Shift, true);

            io.AddMouseButtonEvent(button, true);

            // Which part of the window would we drag, if any?
            dragWhat.clear();
            if (button == 0 &&             // left button
                IsInsideSim() &&           // floating window in simulator
                loc_x >= 0 && loc_y >= 0)  // valid local position
            {
                // shall we drag the entire window?
                if (IsInsideWindowDragArea(loc_x, loc_y)) {
                    dragWhat.wnd = true;
                }
                // do we need to handle window resize?
                else if (bHandleWndResize) {
                    dragWhat.left = loc_x <= WND_RESIZE_LEFT_WIDTH;
                    dragWhat.top = loc_y <= WND_RESIZE_TOP_WIDTH;
                    dragWhat.right = loc_x >= (mRight - mLeft) - WND_RESIZE_RIGHT_WIDTH;
                    dragWhat.bottom = loc_y >= (mTop - mBottom) - WND_RESIZE_BOTTOM_WIDTH;
                }
                // Anything to drag?
                if (dragWhat) {
                    // Remember pos in case of dragging
                    lastMouseDragX = x;
                    lastMouseDragY = y;
                }
            }
            break;

        case xplm_MouseUp:
            io.AddMouseButtonEvent(button, false);
            lastMouseDragX = lastMouseDragY = -1;
            dragWhat.clear();
            io.AddKeyEvent(ImGuiMod_Ctrl, false);
            io.AddKeyEvent(ImGuiMod_Shift, false);
            break;
        default:
            // dunno!
            break;
    }

    return 1;
}

void ImgWindow::HandleKeyFuncCB(XPLMWindowID /*inWindowID*/, char inKey, XPLMKeyFlags inFlags, char inVirtualKey,
                                void* inRefcon, int blosingFocus) {
    LogMsg("ImgWindow::HandleKeyFuncCB: inKey=%d, inFlags=%08x, inVirtualKey=%d, blosingFocus=%d", (unsigned)inKey, (unsigned)inFlags,
           (unsigned)inVirtualKey, blosingFocus);
    auto* thisWindow = reinterpret_cast<ImgWindow*>(inRefcon);
    ImGui::SetCurrentContext(thisWindow->mImGuiContext);
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        // Loosing focus? That's not exactly something ImGui allows us to do...
        // we try convincing ImGui to let it go by sending an [Esc] key
        if (blosingFocus) {
            io.AddKeyEvent(ImGuiKey_Escape, true);
            io.AddKeyEvent(ImGuiKey_Escape, false);
        } else {
            // Hack for the Backspace key in VR:
            // Apparently, the virtual VR keyboard sends both the Up and the Down
            // event within the same drawing cycle, which would overwrite
            // io.KeyDown[XPLM_VK_BACK] with false again before we could pass on true.
            // Also see
            // https://forums.x-plane.org/index.php?/forums/topic/147139-dear-imgui-x-plane/&do=findComment&comment=2032062
            // though I am following a different solution:
            // So we ignore the "up" event (release key) here, and do the actual
            // release only after the next drawing cycle (flag bResetBackspace).
            // (And this little delay doesn't hurt in non-VR either, so we don't even test for VR.)

            // If Backspace is _released_ ...
            if (inVirtualKey == XPLM_VK_BACK && !(inFlags & xplm_DownFlag)) {
                thisWindow->bResetBackspace = true;  // have it reset only later in DrawWindowCB
            } else {
                // in all normal cases: save the up/down flag as it comes from XP
                ImGuiKey key = TranslateXPLMKeyToImGui(static_cast<unsigned char>(inVirtualKey));
                if (key != ImGuiKey_None) {
                    io.AddKeyEvent(key, (inFlags & xplm_DownFlag) == xplm_DownFlag);
                }
            }
            io.AddKeyEvent(ImGuiMod_Shift, (inFlags & xplm_ShiftFlag) == xplm_ShiftFlag);
            io.AddKeyEvent(ImGuiMod_Alt, (inFlags & xplm_OptionAltFlag) == xplm_OptionAltFlag);
            io.AddKeyEvent(ImGuiMod_Ctrl, (inFlags & xplm_ControlFlag) == xplm_ControlFlag);

            // inKey will only includes printable characters,
            // but also those created with key combinations like @ or {}
            if ((inFlags & xplm_DownFlag) == xplm_DownFlag && inKey > '\0') {
                char smallStr[2] = {inKey, 0};
                io.AddInputCharactersUTF8(smallStr);
            }
        }
    }
}

int ImgWindow::HandleMouseWheelFuncCB(XPLMWindowID /*inWindowID*/, int x, int y, int wheel, int clicks,
                                        void* inRefcon) {
    auto* thisWindow = reinterpret_cast<ImgWindow*>(inRefcon);
    ImGui::SetCurrentContext(thisWindow->mImGuiContext);
    ImGuiIO& io = ImGui::GetIO();

    float outX, outY;
    thisWindow->translateToImguiSpace(x, y, outX, outY);
    io.AddMousePosEvent(outX, outY);
    switch (wheel) {
        case 0:
            io.AddMouseWheelEvent(0.0f, static_cast<float>(clicks));
            break;
        case 1:
            io.AddMouseWheelEvent(static_cast<float>(clicks), 0.0f);
            break;
        default:
            // unknown wheel
            break;
    }
    return 1;
}

int ImgWindow::HandleRightClickFuncCB(XPLMWindowID /* inWindowID */, int x, int y, XPLMMouseStatus inMouse,
                                        void* inRefcon) {
    auto* thisWindow = reinterpret_cast<ImgWindow*>(inRefcon);
    return thisWindow->HandleMouseClickGeneric(x, y, inMouse, 1);
}

void ImgWindow::SetWindowTitle(const std::string& title) {
    mWindowTitle = title;
    XPLMSetWindowTitle(mWindowID, mWindowTitle.c_str());
}

void ImgWindow::SetVisible(bool inIsVisible) {
    if (inIsVisible)
        moveForVR();
    if (GetVisible() == inIsVisible) {
        // if the state is already correct, no-op.
        return;
    }
    if (inIsVisible) {
        if (!onShow()) {
            // chance to early abort.
            return;
        }
    }
    XPLMSetWindowIsVisible(mWindowID, inIsVisible);
}

void ImgWindow::moveForVR() {
    // if we're trying to display the window, check the state of the VR flag
    // - if we're VR enabled, explicitly move the window to the VR world.
    if (XPLMGetDatai(gVrEnabledRef)) {
        XPLMSetWindowPositioningMode(mWindowID, xplm_WindowVR, 0);
    } else {
        if (IsInVR()) {
            XPLMSetWindowPositioningMode(mWindowID, mPreferredLayer, -1);
        }
    }
}

bool
ImgWindow::GetVisible() const
{
	return XPLMGetWindowIsVisible(mWindowID) != 0;
}


bool
ImgWindow::onShow()
{
	return true;
}

void
ImgWindow::SetWindowDragArea (int left, int top, int right, int bottom)
{
    dragLeft    = left;
    dragTop     = top;
    dragRight   = right;
    dragBottom  = bottom;
}

void
ImgWindow::ClearWindowDragArea ()
{
    dragLeft = dragTop = dragRight = dragBottom = -1;
}

bool
ImgWindow::HasWindowDragArea (int* pL, int* pT,
                              int* pR, int* pB) const
{
    // return definition if requested
    if (pL) *pL = dragLeft;
    if (pT) *pT = dragTop;
    if (pR) *pR = dragRight;
    if (pB) *pB = dragBottom;

    // is a valid drag area defined?
    return
    dragLeft  >= 0          && dragTop    >= 0 &&
    dragRight >  dragLeft   && dragBottom >= dragTop;
}

bool
ImgWindow::IsInsideWindowDragArea (int x, int y) const
{
    // values outside the window aren't valid
    if (x == -FLT_MAX || y == -FLT_MAX)
        return false;

    // is a drag area defined in the first place?
    if (!HasWindowDragArea())
        return false;

    // inside the defined drag area?
    return
    dragLeft <= x && x <= dragRight &&
    dragTop  <= y && y <= dragBottom;
}

void
ImgWindow::SafeDelete()
{
	sPendingDestruction.push(this);
	if (sSelfDestructHandler == nullptr) {
        XPLMCreateFlightLoop_t flParams{
            sizeof(flParams),
            xplm_FlightLoop_Phase_BeforeFlightModel,
            &ImgWindow::SelfDestructCallback,
            nullptr,
        };
        sSelfDestructHandler = XPLMCreateFlightLoop(&flParams);
	}
	XPLMScheduleFlightLoop(sSelfDestructHandler, -1, 1);
}

std::queue<ImgWindow *>  ImgWindow::sPendingDestruction;
XPLMFlightLoopID         ImgWindow::sSelfDestructHandler = nullptr;

float
ImgWindow::SelfDestructCallback(float /*inElapsedSinceLastCall*/,
                                float /*inElapsedTimeSinceLastFlightLoop*/,
                                int   /*inCounter*/,
                                void* /*inRefcon*/)
{
    while (!sPendingDestruction.empty()) {
        auto *thisObj = sPendingDestruction.front();
        sPendingDestruction.pop();
        delete thisObj;
    }
    return 0;
}

static bool init_done;
// static
bool ImgWindow::Initialize() {
    if (init_done)
        return true;
    init_done = true;

    gVrEnabledRef = XPLMFindDataRef("sim/graphics/VR/enabled");
    gFrameRatePeriodRef = XPLMFindDataRef("sim/operation/misc/frame_rate_period");
    return true;
}

void ImgWindow::Finalize() {
    // nothing to do for now
}
