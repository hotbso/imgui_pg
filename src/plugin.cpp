// Copyright 2026 hotbso
// SPDX-License-Identifier: MIT

#include <cstdint>
#include <cstring>

#include "XPLMMenus.h"
#include "XPLMPlugin.h"
#include "XPLMUtilities.h"

namespace {

XPLMMenuID g_menu_id = nullptr;
XPLMCommandRef g_open_cmd = nullptr;

void OnOpen() { XPLMDebugString("imgui_pg: open\n"); }

void MenuHandler(void* /*in_menu_ref*/, void* in_item_ref) {
  if (reinterpret_cast<intptr_t>(in_item_ref) == 0) {
    OnOpen();
  }
}

int OpenCmdHandler(XPLMCommandRef /*in_command*/, XPLMCommandPhase in_phase,
                   void* /*in_refcon*/) {
  if (in_phase == xplm_CommandBegin) {
    OnOpen();
  }
  return 1;  // Pass command to other handlers.
}

}  // namespace

extern "C" {

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

  int plugins_menu_item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "imgui_pg",
                                             nullptr, 1);
  g_menu_id =
      XPLMCreateMenu("imgui_pg", XPLMFindPluginsMenu(), plugins_menu_item,
                     MenuHandler, nullptr);
  XPLMAppendMenuItem(g_menu_id, "Open",
                     reinterpret_cast<void*>(static_cast<intptr_t>(0)), 1);

  return 1;
}

PLUGIN_API void XPluginStop() {
  XPLMUnregisterCommandHandler(g_open_cmd, OpenCmdHandler,
                               /*in_before=*/1, /*in_refcon=*/nullptr);
  XPLMDestroyMenu(g_menu_id);
}

PLUGIN_API int XPluginEnable() { return 1; }

PLUGIN_API void XPluginDisable() {}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID /*in_from*/, int /*in_msg*/,
                                      void* /*in_param*/) {}

}  // extern "C"
