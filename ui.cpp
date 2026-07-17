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

#include "fa-solid-900.inc"
#include "IconsFontAwesome5.h"

static constexpr int kWinWidth = 400;
static constexpr int kWinHeight = 450;
static constexpr int kWinPad = 75;
static constexpr float kFontSize = 14.0f;

std::unique_ptr<ImgWindow> ui, ui1;

ImFont* std_font, *mono_font, *symbol_font;

void CreateUi(int delta, std::unique_ptr<ImgWindow>& ui) {
    int sc_left, sc_top;
    XPLMGetScreenBoundsGlobal(&sc_left, &sc_top, nullptr, nullptr);

    int left = sc_left + kWinPad;
    int right = left + kWinWidth;
    int top = sc_top - kWinPad;
    int bottom = top - kWinHeight;
    ui = std::make_unique<Ui>(left + delta, top, right + delta, bottom);
}

void UiLoadFonts() {
    ImFontAtlas* atlas = ImgWindow::GetSharedFontAtlas();
    if (!atlas) {
        LogMsg("ImgWindowLoadFonts: shared font atlas is not initialized");
        return;
    }

    // load from X-Plane's default font directory
    std_font = atlas->AddFontFromFileTTF("./Resources/fonts/DejaVuSans.ttf");
    if (std_font == nullptr) {
        LogMsg("Failed to load font DejaVuSans from file, falling back to default font");
    }

    mono_font = atlas->AddFontFromFileTTF("./Resources/fonts/DejaVuSansMono.ttf");
    if (mono_font == nullptr) {
        LogMsg("Failed to load font DejaVuSansMono from file, falling back to default font");
    }

    symbol_font =
        atlas->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size);
}

void UiFini() {
    ui = nullptr; // just in case ...
    ui1 = nullptr;
    ImgWindow::Finalize();
    LogMsg("Imgui Window finalized");
}

///////////////////////////////////////////////////////////////////////////////////////////
Ui::Ui(int left, int top, int right, int bot)
    : ImgWindow(left, top, right, bot) {
    // Create a flight loop id, but don't schedule it yet
    XPLMCreateFlightLoop_t loop_params = {
        sizeof(loop_params),                      // structSize
        xplm_FlightLoop_Phase_BeforeFlightModel,  // phase
        FlightLoopCb,                             // callbackFunc
        (void*)this,                              // refcon
    };

    ImGuiStyle& style = ImGui::GetStyle();
    style.FontSizeBase = kFontSize;

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
        if (ImGui::InputText("##pilot_id_", &pilot_id_, ImGuiInputTextFlags_EnterReturnsTrue)) {
            LogMsg("Pilot ID set to '%s'", pilot_id_.c_str());
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TreePop();
        ImGui::Checkbox("Enable feature X", &checkbox_state_);
        if (checkbox_state_) {
            ImGui::TextUnformatted("Checkbox state: ");
            ImGui::SameLine();
            ImGui::PushFont(symbol_font, 0.0f);
            ImGui::TextUnformatted((const char*)ICON_FA_CHECK);
            ImGui::PopFont();
        }
    } else {
        ImGui::TextUnformatted("No settings available");
    }

    if (ImGui::SliderInt("Font size", &font_slider_, 8, 32)) {
        LogMsg("Font size set to %d", font_slider_);
    }

    if (ImGui::Checkbox("Use monospace font", &mono_font_enabled_)) {
        LogMsg("Monospace font %s", mono_font_enabled_ ? "enabled" : "disabled");
    }
    if (mono_font_enabled_ )
        ImGui::PushFont(mono_font, 0.0f);

    ImGui::PushFont(NULL, static_cast<float>(font_slider_));
    ImGui::TextUnformatted("This text is displayed in the selected font size.");
    ImGui::PopFont();

    if (mono_font_enabled_)
        ImGui::PopFont();
}

// Delayed actions that require FlightLoop context
float Ui::FlightLoopCb(float, float, int, void* inRefcon) {
    LogMsg("FlightLoopCb called with inRefcon=%p", inRefcon);

    //Ui& ui = *reinterpret_cast<Ui*>(inRefcon);

    return 0.0f;
}
