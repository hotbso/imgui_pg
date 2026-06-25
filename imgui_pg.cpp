// Copyright 2026 hotbso
// SPDX-License-Identifier: MIT

#include <cstdint>
#include <cstring>

#include "XPLMMenus.h"
#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMDisplay.h"
#include "XPLMPanelGraphics.h"

#include "log_msg.h"

const char* log_msg_prefix = "imgui_pg: ";

XPLMMenuID g_menu_id = nullptr;
XPLMCommandRef g_open_cmd = nullptr;

static constexpr int kWinWidth = 400;
static constexpr int kWinHeight = 450;
static constexpr int kWinPad = 75;
//static constexpr int kWinX = 100;
//static constexpr int kWinY = 100;
//static constexpr float kFontSize = 14.0f;

void DrawWindowCb(XPLMWindowID win_id, void* inRefcon) {
    //LogMsg("DrawWindowCb: win_id=%p, inRefcon=%p", win_id, inRefcon);
    uint32_t red = XPLMMakeColor(1.0f, 0, 0, 1.0f);
    uint32_t green = XPLMMakeColor(0, 1.0f, 0, 1.0f);

    XPLMVertexColor_t vertices[] = {
        {0, 5, green},
        {kWinWidth, 5, green},

        {0, 0, red},
        {kWinWidth, kWinHeight, red},

        {0, kWinHeight, green},
        {kWinWidth, 0, green},
    };

    int n_vertex = sizeof(vertices) / sizeof(vertices[0]);

    int left, top;
    XPLMGetWindowGeometry(win_id, &left, &top, nullptr, nullptr);
    //LogMsg("DrawWindowCb: left=%d, top=%d", left, top);
    for (auto& v : vertices) {
        v.x += left;
        v.y = top - v.y;
    }

    XPLMLinesc(vertices, n_vertex);
}

int MouseCb(XPLMWindowID win_id, int x, int y, XPLMMouseStatus inMouse, void* inRefcon) {
    //LogMsg("MouseCb: win_id=%p, x=%d, y=%d, inMouse=%d, inRefcon=%p", win_id, x, y, inMouse, inRefcon);
    return 1;
}

int MouseWheelCb(XPLMWindowID win_id, int x, int y, int wheel, int clicks, void* inRefcon) {
    //LogMsg("MouseWheelCb: win_id=%p, x=%d, y=%d, wheel=%d, clicks=%d, inRefcon=%p", win_id, x, y, wheel, clicks, inRefcon);
    return 1;
}

XPLMCursorStatus CursorCb(XPLMWindowID win_id, int x, int y, void* inRefcon) {
    //LogMsg("CursorCb: win_id=%p, x=%d, y=%d, inRefcon=%p", win_id, x, y, inRefcon);
    return xplm_CursorDefault;
}
void KeyboardCb(XPLMWindowID win_id, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void* inRefcon, int losingFocus) {
    LogMsg("KeyboardCb: win_id=%p, inKey=%c, inFlags=%d, inVirtualKey=%c, inRefcon=%p, losingFocus=%d", win_id, inKey, inFlags,
           inVirtualKey, inRefcon, losingFocus);
}


static XPLMWindowID window_id = nullptr;


static XPLMCreateWindow_t window_proto = {
    sizeof(XPLMCreateWindow_t),
    0, 0, 0, 0,  // left, top, right, bottom
    true,                   // visible
    DrawWindowCb,
    MouseCb,
    KeyboardCb,
    CursorCb,
    MouseWheelCb,
    (void *)0x123abc,   // refcon
    xplm_WindowDecorationRoundRectangle,
    xplm_WindowLayerFloatingWindows,
    MouseCb,  // right click
    xplm_WindowContentTypePanelGraphics,
    nullptr,  // browser navigation callback
};

void CreateUi() {
    int sc_left, sc_top;
    XPLMGetScreenBoundsGlobal(&sc_left, &sc_top, nullptr, nullptr);

    int left = sc_left + kWinPad;
    int right = left + kWinWidth;
    int top = sc_top - kWinPad;
    int bottom = top - kWinHeight;

    window_proto.left = left;
    window_proto.right = right;
    window_proto.top = top;
    window_proto.bottom = bottom;
    window_id = XPLMCreateWindowEx(&window_proto);
    LogMsg("CreateUi: window_id=%p, left=%d, right=%d, top=%d, bottom=%d", window_id, left, right, top, bottom);
}


void OnOpen() {
  XPLMDebugString("imgui_pg: open\n");
  if (window_id == nullptr) {
      CreateUi();
  }
}

void MenuHandler(void* /*in_menu_ref*/, void* in_item_ref) {
    if (reinterpret_cast<intptr_t>(in_item_ref) == 0) {
        OnOpen();
    }
}

int OpenCmdHandler(XPLMCommandRef /*in_command*/, XPLMCommandPhase in_phase, void* /*in_refcon*/) {
    if (in_phase == xplm_CommandBegin) {
        OnOpen();
    }
    return 1;  // Pass command to other handlers.
}

// XPluginStart, XPluginStop, XPluginEnable, XPluginDisable, and
// XPluginReceiveMessage are the five required X-Plane plugin entry points.

// Buffer sizes are fixed at 256 bytes by the X-Plane SDK contract.
static constexpr int kXplmStringMax = 256;

PLUGIN_API int XPluginStart(char* out_name, char* out_sig, char* out_desc) {
    std::strncpy(out_name, "imgui_pg", kXplmStringMax);
    std::strncpy(out_sig, "hotbso.imgui_pg", kXplmStringMax);
    std::strncpy(out_desc, "imgui with XPDSK panel graphics", kXplmStringMax);

    g_open_cmd = XPLMCreateCommand("imgui_pg/open", "Open imgui_pg");
    XPLMRegisterCommandHandler(g_open_cmd, OpenCmdHandler,
                               /*in_before=*/1, /*in_refcon=*/nullptr);

    int plugins_menu_item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "imgui_pg", nullptr, 1);
    g_menu_id = XPLMCreateMenu("imgui_pg", XPLMFindPluginsMenu(), plugins_menu_item, MenuHandler, nullptr);
    XPLMAppendMenuItem(g_menu_id, "Open", reinterpret_cast<void*>(static_cast<intptr_t>(0)), 1);

    return 1;
}

PLUGIN_API void XPluginStop() {
    XPLMUnregisterCommandHandler(g_open_cmd, OpenCmdHandler,
                                 /*in_before=*/1, /*in_refcon=*/nullptr);
    XPLMDestroyMenu(g_menu_id);
}

PLUGIN_API int XPluginEnable() {
    LogMsg("XPluginEnable: window_id=%p\n", window_id);
    return 1;
}

PLUGIN_API void XPluginDisable() {}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID /*in_from*/, int /*in_msg*/, void* /*in_param*/) {}
