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
#include <SofaImGui/models/TCPTarget.h>
#include <imgui.h>

namespace sofaimgui::windows {

class MoveWindow : public BaseWindow
{
   public:
    MoveWindow(const std::string& name, const bool& isWindowOpen);
    ~MoveWindow() = default;

    void showWindow(const ImGuiWindowFlags &windowFlags);
    void setTCPTarget(std::shared_ptr<models::TCPTarget> TCPTarget) {m_TCPTarget=TCPTarget;}
    void setTCPLimits(int minPosition, int maxPosition, double minOrientation, double maxOrientation);
    void setActuatorsLimits(double min, double max);
    void setActuators(std::vector<sofa::core::BaseData*> actuators) {m_actuators = actuators;}

   protected:

    int m_TCPMinPosition{-500};
    int m_TCPMaxPosition{500};
    double m_TCPMinOrientation{-M_PI};
    double m_TCPMaxOrientation{M_PI};

    double m_actuatorsMin{-500.};
    double m_actuatorsMax{500.};

    std::shared_ptr<models::TCPTarget> m_TCPTarget;
    std::vector<sofa::core::BaseData*> m_actuators;

    bool showSliderInt(const char *name, const char* label1, const char *label2, int* v, const int& min, const int& max, const int &offset, const ImVec4& color);
    bool showSliderInt(const char *name, const char* label1, const char *label2, int* v, const int& min, const int& max, const int &offset);
    bool showSliderDouble(const char *name, const char* label1, const char *label2, double* v, const double& min, const double& max, const ImVec4 &color);
    bool showSliderDouble(const char *name, const char* label1, const char *label2, double* v, const double& min, const double& max);
};

}

