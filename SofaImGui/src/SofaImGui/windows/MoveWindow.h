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
#include <SofaImGui/models/guidata/KinematicsGUIDataManager.h>
#include <SofaImGui/widgets/MovePad.h>
#include <SofaImGui/DrivingWindow.h>
#include <imgui.h>

namespace sofaimgui::windows {

class SOFAIMGUI_API MoveWindow : public BaseWindow
{
   public:
    MoveWindow(const std::string& name, const bool& isWindowOpen, models::guidata::KinematicsGUIDataManager& kinematicsGUIDataManager);
    ~MoveWindow() = default;

    void showWindow(const ImGuiWindowFlags &windowFlags) override;
    std::string getDescription() override;

    enum MoveType {
        PAD,
        SLIDERS
    };
    MoveType m_moveType;

   protected:

    models::guidata::KinematicsGUIDataManager m_kinematicsGUIDataManager;

    double m_x;
    double m_y;
    double m_z;
    double m_rx;
    double m_ry;
    double m_rz;
    
    bool m_freeRoll{true};
    bool m_freePitch{true};
    bool m_freeYaw{true};

    ImGui::MovePad m_movePad;

    bool enabled() override {return m_kinematicsGUIDataManager.hasInverseProblemSolverAndTCP() || m_kinematicsGUIDataManager.hasActuator();}

    bool showSliderDouble(const char *name, const char* label1, const char *label2, double* v, const double& min, const double& max, const ImVec4 &color);
    bool showSliderDouble(const char *name, const char* label1, const char *label2, double* v, const double& min, const double& max);
    void showOptions();
    void showWeightOption(const int &index);
    void showPad();
    bool showVerticalTab(const std::string& label, const std::string& tooltip, const bool &active);
    bool isDrivingSimulation() {return drivingWindow == DrivingWindow::MOVE;}
};

}


