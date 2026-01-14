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

sofaimgui::models::GUIData::SPtr BaseWindow::addData(const std::string& label, 
                                                    const std::pair<sofa::core::BaseData*, bool>& data,
                                                    const std::pair<sofa::core::BaseData*, bool>& min,
	                                                const std::pair<sofa::core::BaseData*, bool>& max,
                                                    const std::string& group,
                                                    const std::string& tooltip)
{
    if(data.first)
    {
        sofaimgui::models::OwnedBaseData::SPtr newdata = std::make_shared<sofaimgui::models::OwnedBaseData>(data.first, data.second);
        sofaimgui::models::OwnedBaseData::SPtr newmin = std::make_shared<sofaimgui::models::OwnedBaseData>(min.first, min.second);
        sofaimgui::models::OwnedBaseData::SPtr newmax = std::make_shared<sofaimgui::models::OwnedBaseData>(max.first, max.second);

		auto guiDataPtr = std::make_shared<sofaimgui::models::GUIData>(newdata, newmin, newmax, label, group, tooltip);

        return addGUIData(guiDataPtr);
    }

	return nullptr;
}

sofaimgui::models::GUIData::SPtr sofaimgui::windows::BaseWindow::addGUIData(const sofaimgui::models::GUIData::SPtr& guidata)
{
	// Check if already in the set
	if (m_GUIData.find(guidata) != m_GUIData.end())
	{
		return nullptr;
	}

    auto inserted = m_GUIData.insert(guidata);
    if (inserted.second) // Check if the insertion was successful
    {
        
		m_groupedGUIData[guidata.get()->group].push_back(*inserted.first);
        return *inserted.first;
    }
    return nullptr;
}

void BaseWindow::removeGUIData(sofaimgui::models::GUIData::SPtr data)
{
    if (data)
    {
        m_GUIData.erase(data);
	    std::vector<sofaimgui::models::GUIData::SPtr>& group = m_groupedGUIData[data->group];
		auto it = std::find(group.begin(), group.end(), data);
        if(it != group.end())
		    group.erase(it);
    }
}

}
