//
//    Simbrief Hub: A central resource of simbrief data for other plugins
//
//    Copyright (C) 2025, 2026 Holger Teutsch
//
//      based on example code from imgui4xp by William Good
//      published under MIT license, see README.html for details
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
//    USA
//

#include <string>
#include <vector>
#include <memory>

#include "XPLMDisplay.h"
#include "XPLMProcessing.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImgWindow.h"

#include "ui.h"
#include "log_msg.h"

static constexpr int kWinWidth = 400;
static constexpr int kWinHeight = 450;
static constexpr int kWinPad = 75;
static constexpr float kFontSize = 14.0f;

std::unique_ptr<ImgWindow> ui;

std::string pilot_id = "12345";

void CreateUi() {
    int sc_left, sc_top;
    XPLMGetScreenBoundsGlobal(&sc_left, &sc_top, nullptr, nullptr);

    int left = sc_left + kWinPad;
    int right = left + kWinWidth;
    int top = sc_top - kWinPad;
    int bottom = top - kWinHeight;
    ui = std::make_unique<Ui>(left, top, right, bottom);
}

void ImgWindowIni() {
    LogMsg("Initializing Imgui Window...");
    ImgWindow::sFontAtlas = std::make_shared<ImgFontAtlas>();

    // load from X-Plane's default font directory
    if (ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/DejaVuSans.ttf", kFontSize) == nullptr) {
        LogMsg("Failed to load font DejaVuSans from file, falling back to default font");
    }

#if 0
    // Now we merge some icons from the OpenFontsIcons font into the above font
    // (see `imgui/docs/FONTS.txt`)
    ImFontConfig config;
    config.MergeMode = true;

    // We only read very selectively the individual glyphs we are actually using
    // to safe on texture space
    static ImVector<ImWchar> icon_ranges;
    ImFontGlyphRangesBuilder builder;
    // Add all icons that are actually used (they concatenate into one string)
    builder.AddText((const char*)ICON_FA_CHECK);
    builder.BuildRanges(&icon_ranges);

    // Merge the icon font with the text font
    ImgWindow::sFontAtlas->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data,
                                                          fa_solid_900_compressed_size,
                                                          kFontSize,
                                                          &config,
                                                          icon_ranges.Data);

#endif
    LogMsg("Imgui Window initialized");
}

void ImgWindowFini() {
    ui = nullptr; // just in case ...
    ImgWindow::sFontAtlas.reset();
}

///////////////////////////////////////////////////////////////////////////////////////////
Ui::Ui(int left, int top, int right, int bot)
    : ImgWindow(left, top, right, bot, xplm_WindowDecorationRoundRectangle, xplm_WindowLayerFloatingWindows) {

    // is currently not really supported, hopefully with 12.5
    ImGui::GetIO().IniFilename = nullptr; // strdup((user_cfg_dir + "imgui.ini").c_str());

    // Create a flight loop id, but don't schedule it yet
    XPLMCreateFlightLoop_t loop_params = {
        sizeof(loop_params),                      // structSize
        xplm_FlightLoop_Phase_BeforeFlightModel,  // phase
        FlightLoopCb,                             // callbackFunc
        (void*)this,                              // refcon
    };

    flt_id_ = XPLMCreateFlightLoop(&loop_params);

    SetWindowTitle("imgui_pg");
    SetWindowResizingLimits(100, 100, 1024, 1024);
    SetVisible(true);
}

Ui::~Ui() {
    if (flt_id_)
        XPLMDestroyFlightLoop(flt_id_);
}

void Ui::BuildInterface() {
    if (ImGui::TreeNode("Settings")) {
        //--------------------------------------------------
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Pilot ID:");
        ImGui::SameLine();
        if (ImGui::InputText("##pilot_id", &pilot_id, ImGuiInputTextFlags_EnterReturnsTrue)) {
            LogMsg("Pilot ID set to '%s'", pilot_id.c_str());
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TreePop();
    } else {
        ImGui::TextUnformatted("No settings available");
    }
}

// Delayed actions that require FlightLoop context
float Ui::FlightLoopCb(float, float, int, void* inRefcon) {
    LogMsg("FlightLoopCb called with inRefcon=%p", inRefcon);

    //Ui& ui = *reinterpret_cast<Ui*>(inRefcon);

    return 0.0f;
}
