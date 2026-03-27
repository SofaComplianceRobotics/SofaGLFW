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
#include "IconsFontAwesome6.h"
#include <SofaImGui/windows/BaseWindow.h>

namespace sofaimgui::windows {

WindowsSettings &WindowsSettings::getInstance()
{
    static WindowsSettings windowsSettings;
    return windowsSettings;
}

BaseWindow::BaseWindow()
{
    m_workbenches = Workbench::LIVE_CONTROL | Workbench::SCENE_EDITOR | Workbench::SIMULATION_MODE;
}

void BaseWindow::showWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI, const ImGuiWindowFlags &windowFlags)
{
    SOFA_UNUSED(baseGUI);
    SOFA_UNUSED(windowFlags);
}

std::string BaseWindow::getName() const
{
    return m_name;
}

std::string& BaseWindow::getLabel()
{
    m_labelname = "       " + m_name;
    return m_labelname;
}

void BaseWindow::clearWindow()
{
	m_groupedGUIData.clear();
	m_GUIData.clear();
}

bool& BaseWindow::isOpen()
{
    return m_isOpen;
}

void BaseWindow::setOpen(const bool &isOpen)
{
    m_isOpen=isOpen;
}

const bool& BaseWindow::getDefaultIsOpen()
{
    return m_defaultIsOpen;
}

bool BaseWindow::isEnabledInWorkbench()
{
    return (m_workbenches & workbench);
}

void BaseWindow::showInfoMessage(const char* message)
{
    ImGui::BeginDisabled();
    ImGui::Text(ICON_FA_CIRCLE_INFO);
    ImGui::SameLine();
    ImGui::TextWrapped("%s", message);
    ImGui::EndDisabled();
}

}
