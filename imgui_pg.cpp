// Copyright 2026 hotbso
// SPDX-License-Identifier: MIT

#include <cstdint>
#include <cstring>

#include "XPLMMenus.h"
#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMDisplay.h"
#include "XPLMPanelGraphics.h"

#include "ui.h"
#include "log_msg.h"

const char* log_msg_prefix = "imgui_pg: ";

XPLMMenuID g_menu_id = nullptr;
XPLMCommandRef g_open_cmd = nullptr;

#if 0
static constexpr int kWinWidth = 400;
static constexpr int kWinHeight = 450;
static constexpr int kWinPad = 75;

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
#endif

void OnOpen() {
    XPLMDebugString("imgui_pg: open");
    if (ui == nullptr)
        CreateUi();
    else
        ui->SetVisible(true);
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

    ImgWindowIni();
    return 1;
}

PLUGIN_API void XPluginStop() {
    ui = nullptr;  // just in case ...
    XPLMUnregisterCommandHandler(g_open_cmd, OpenCmdHandler,
                                 /*in_before=*/1, /*in_refcon=*/nullptr);
    XPLMDestroyMenu(g_menu_id);
    ImgWindowFini();
}

PLUGIN_API int XPluginEnable() { return 1; }

PLUGIN_API void XPluginDisable() {
    ui = nullptr;  // just in case ...
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID /*in_from*/, int /*in_msg*/, void* /*in_param*/) {}
