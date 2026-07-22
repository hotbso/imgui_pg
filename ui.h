//
//    Simbrief Hub: A central resource of simbrief data for other plugins
//
//    Copyright (C) 2025, 2026 Holger Teutsch
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

#pragma once

#include "ImgWindow.h"
#include <vector>

// Our own class defining our own UI
class Ui : public ImgWindow {
    XPLMFlightLoopID flt_id_ = nullptr;
    std::string pilot_id_ = "12345";
    bool checkbox_state_ = 0;
    bool mono_font_enabled_ = false;
    int font_slider_ = 14;

    // Main function: creates the window's UI
    void BuildInterface() override;

    // flight loop callback for delayed actions prohibited in drawloops
    static float FlightLoopCb(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter,
                              void* inRefcon);

   public:
    Ui(int left, int top, int right, int bot);
    ~Ui() override;
};

// Configure and Cleanup
extern void UiFini();
extern void UiLoadFonts();

extern void CreateUi(int delta, std::unique_ptr<ImgWindow>& ui);

extern std::unique_ptr<ImgWindow> ui, ui1;
