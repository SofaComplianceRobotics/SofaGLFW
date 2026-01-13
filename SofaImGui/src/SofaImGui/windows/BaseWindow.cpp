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

void BaseWindow::setDrivingTCPTarget(const bool &isDrivingSimulation)
{
    m_isDrivingSimulation = isDrivingSimulation;
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

bool BaseWindow::isDrivingSimulation()
{
    return m_isDrivingSimulation;
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

void BaseWindow::addGUIData(std::shared_ptr<sofa::core::BaseData> data,
                            std::shared_ptr<sofa::core::BaseData> min,
                            std::shared_ptr<sofa::core::BaseData> max,
                            const std::string& label,
                            const std::string& group,
                            const std::string& tooltip)
{
    if(data)
    {
        GUIData guiData;
        guiData.data = data;
        guiData.min = min;
        guiData.max = max;
        guiData.label = label.empty() ? data.get()->getName() : label;
        guiData.group = group;
        guiData.tooltip = tooltip;

        m_GUIData.insert(guiData);
    }
}

void BaseWindow::removeGUIData(GUIData& data)
{
    m_GUIData.erase(data);
}

}
