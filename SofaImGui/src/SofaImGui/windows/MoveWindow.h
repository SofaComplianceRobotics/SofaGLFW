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
#include <imgui.h>

namespace sofaimgui::windows {

class MoveWindow : public BaseWindow
{
   public:
    MoveWindow(const std::string& name, const bool& isWindowOpen);
    ~MoveWindow() = default;

    using BaseWindow::m_name;
    using BaseWindow::m_isWindowOpen;

    void showWindow(sofa::simulation::Node *groot, const ImGuiWindowFlags &windowFlags);

   protected:
    void addSliderInt(const char *name, const char* label1, const char *label2, int* v, const ImVec4& color);
    void addSliderFloat(const char *name, const char* label1, const char *label2, float* v, const ImVec4 &color);

    void getTarget(sofa::simulation::Node* groot, int &x, int &y, int &z, float &rx, float &ry, float &rz);
    void setTarget(sofa::simulation::Node* groot, const int &x, const int &y, const int &z, const float &rx, const float &ry, const float &rz);
};

}

