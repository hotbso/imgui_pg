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

constexpr int kScreenWidth = 100;
constexpr int kScreenHeight = 200;
constexpr int kBezelWidth = 200;
constexpr int kBezelHeight = 300;
constexpr int kScreenOffsetX = 50;
constexpr int kScreenOffsetY = 50;

XPLMMenuID g_menu_id = nullptr;
XPLMCommandRef g_open_cmd = nullptr;

void BezelCb(float inAmbiantR, float inAmbiantG, float inAmbiantB, void* inRefcon) {
    LogMsg("BezelCb: inAmbiantR=%f, inAmbiantG=%f, inAmbiantB=%f, inRefcon=%p\n", inAmbiantR, inAmbiantG, inAmbiantB,
           inRefcon);
}

void ScreenCb(void* inRefcon) {
    LogMsg("ScreenCb: inRefcon=%p\n", inRefcon);
    uint32_t red = XPLMMakeColor(1.0f, 0, 0, 1.0f);
    XPLMVertex_t vertices[] = {
        {0, 0},
        {kScreenWidth, 0},
        {kScreenWidth, kScreenHeight},
        {0, kScreenHeight},
    };

    XPLMLines(red, vertices, 4);
}

int MouseCb(int x, int y, XPLMMouseStatus inMouse, void* inRefcon) {
    LogMsg("MouseCb: x=%d, y=%d, inMouse=%d, inRefcon=%p\n", x, y, inMouse, inRefcon);
    return 1;
}

int MouseWheelCb(int x, int y, int wheel, int clicks, void* inRefcon) {
    LogMsg("MouseWheelCb: x=%d, y=%d, wheel=%d, clicks=%d, inRefcon=%p\n", x, y, wheel, clicks, inRefcon);
    return 1;
}

XPLMCursorStatus CursorCb(int x, int y, void* inRefcon) {
    LogMsg("CursorCb: x=%d, y=%d, inRefcon=%p\n", x, y, inRefcon);
    return xplm_CursorDefault;
}
int KeyboardCb(char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void* inRefcon, int losingFocus) {
    LogMsg("KeyboardCb: inKey=%c, inFlags=%d, inVirtualKey=%c, inRefcon=%p, losingFocus=%d\n", inKey, inFlags,
           inVirtualKey, inRefcon, losingFocus);
    return 1;
}

static XPLMCreateAvionics_t avionics_panel_t = {
    sizeof(XPLMCreateAvionics_t),
    kScreenWidth,  // screen w, h
    kScreenHeight,

    kBezelWidth,   // bezel w, h
    kBezelHeight,

    kScreenOffsetX,  // screen offset x, y
    kScreenOffsetY,
    false,
    BezelCb,
    ScreenCb,
    MouseCb,
    MouseCb,
    MouseWheelCb,
    CursorCb,
    MouseCb,
    MouseCb,
    MouseWheelCb,
    CursorCb,
    KeyboardCb,
    nullptr,  // brightnessCallback
    "imgui_pg",
    "imgui with XPDSK panel graphics",
    nullptr,  // refcon
    1,         // native
    0
};

static XPLMAvionicsID panel_id;

void OnOpen() { XPLMDebugString("imgui_pg: open\n"); }

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
    panel_id = XPLMCreateAvionicsEx(&avionics_panel_t);
    LogMsg("XPluginEnable: panel_id=%p\n", panel_id);
    return 1;
}

PLUGIN_API void XPluginDisable() {}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID /*in_from*/, int /*in_msg*/, void* /*in_param*/) {}
