/******************************************************************************
 *                 SOFA, Simulation Open-Framework Architecture                *
 *                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the Free  *
 * Software Foundation; either version 2 of the License, or (at your option)   *
 * any later version.                                                          *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but WITHOUT *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
 * more details.                                                               *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with this program. If not, see <http://www.gnu.org/licenses/>.              *
 *******************************************************************************
 * Authors: The SOFA Team and external contributors (see Authors.txt)          *
 *                                                                             *
 * Contact information: contact@sofa-framework.org                             *
 ******************************************************************************/
#pragma once

#include <SofaImGui/windows/BaseWindow.h>
#include <SofaImGui/menus/ViewMenu.h>
#include <imgui.h>

namespace sofaimgui::windows {

class SOFAIMGUI_API ViewportWindow : public BaseWindow
{
   public:

    ViewportWindow(const std::string& name);
    ~ViewportWindow() = default;

    void internalShowWindow() override;
    std::string getDescription() override;

    void setTextureID(const ImTextureID& textureID) {m_textureID=textureID;}

    void addCameraButtons();
    bool addAnimateButton(bool *animate, const float &shift_x);
    bool addStepButton();
    bool addReloadButton();
    void addDrivingTabCombo();

    std::pair<float, float> m_windowSize{0., 0.};

    bool isMouseOnViewport() {return m_isMouseOnViewport;}
    bool isFocusOnViewport() {return m_isFocusOnViewport;}

   protected:

    float m_fps{0.f};

    bool m_isMouseOnViewport{false};
    bool m_isFocusOnViewport{false};

    double m_maxPanelItemWidth{0.0};

    ImTextureID m_textureID;

    bool m_ws_orientationGizmoEnabled{false};
    bool m_ws_cameraButtonsCollapsed{true};
    long m_ws_drivingWindow{1};

    void registerAndLoadWindowSettings() override;

    void addSimulationTimeAndFPS();
    void addRecordingStatus(const ImVec4 &red);
    bool checkCamera();
    void addContextMenu(const ImTextureID& texture);
};

}


