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
XPLMCommandRef toggle_cmdr = nullptr;

void ToggleUi() {
    LogMsg("imgui_pg: toggle UI");
    if (ui == nullptr)
        CreateUi();
    else if (!ui->GetVisible()) // was closed by user, so we can just show it again
        ui->SetVisible(true);
    else
        ui = nullptr;
}

void MenuHandler(void* /*in_menu_ref*/, void* in_item_ref) {
    if (reinterpret_cast<intptr_t>(in_item_ref) == 0) {
        ToggleUi();
    }
}

int ToggleUiCmdCb(XPLMCommandRef /*in_command*/, XPLMCommandPhase in_phase, void* /*in_refcon*/) {
    if (in_phase == xplm_CommandBegin) {
        ToggleUi();
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

    toggle_cmdr = XPLMCreateCommand("imgui_pg/toggle_ui", "Toggle imgui_pg UI");
    XPLMRegisterCommandHandler(toggle_cmdr, ToggleUiCmdCb,
                               /*in_before=*/1, /*in_refcon=*/nullptr);

    int plugins_menu_item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "imgui_pg", nullptr, 1);
    g_menu_id = XPLMCreateMenu("imgui_pg", XPLMFindPluginsMenu(), plugins_menu_item, MenuHandler, nullptr);
    XPLMAppendMenuItem(g_menu_id, "Toggle UI", reinterpret_cast<void*>(static_cast<intptr_t>(0)), 1);

    ImgWindowIni();
    return 1;
}

PLUGIN_API void XPluginStop() {
    ui = nullptr;  // just in case ...
    XPLMUnregisterCommandHandler(toggle_cmdr, ToggleUiCmdCb,
                                 /*in_before=*/1, /*in_refcon=*/nullptr);
    XPLMDestroyMenu(g_menu_id);
    ImgWindowFini();
    LogMsg("imgui_pg: stopped");
}

PLUGIN_API int XPluginEnable() { return 1; }

PLUGIN_API void XPluginDisable() {
    ui = nullptr;  // just in case ...
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID /*in_from*/, int /*in_msg*/, void* /*in_param*/) {}
