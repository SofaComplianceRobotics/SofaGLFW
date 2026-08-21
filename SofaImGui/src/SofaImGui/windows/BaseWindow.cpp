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

BaseWindow::BaseWindow()
{
    m_enabledWorkbenches = Workbench::LIVE_CONTROL | Workbench::SCENE_EDITOR | Workbench::SIMULATION_MODE;
    m_defaultWorkbenches = Workbench::LIVE_CONTROL | Workbench::SCENE_EDITOR | Workbench::SIMULATION_MODE;
}

BaseWindow::BaseWindow(std::string name)
    : BaseWindow()
{
    m_name = name;
}

void BaseWindow::showWindow(ImGuiWindowFlags windowFlags)
{
    beforeShowWindow();

    if (isOpen())
    {
        if (m_firstTime)
        {
            registerAndLoadWindowSettings();
            m_firstTime = false;
        }

        if (ImGui::Begin(getLabel().c_str(), &isOpen(), windowFlags | m_windowFlags))
        {
            if (!isEnabledInWorkbench())
            {
                showInfoMessage((getDescription() + " Disabled in the active workbench.").c_str());
                ImGui::BeginDisabled();
            }

            internalShowWindow();

            if (!isEnabledInWorkbench())
                ImGui::EndDisabled();
        }

        auto& windowSettings = WindowsSettings::getInstance();
        for (auto it=m_registeredSettings.begin(); it!=m_registeredSettings.end(); it++)
        {
            switch (it->second.second) {
            case WindowsSettings::SettingType::LONG:
                windowSettings.setSetting(m_name.c_str(), it->first.c_str(), *(long*)(it->second.first));
                break;
            case WindowsSettings::SettingType::DOUBLE:
                windowSettings.setSetting(m_name.c_str(), it->first.c_str(), *(double*)(it->second.first));
                break;
            case WindowsSettings::SettingType::BOOL:
                windowSettings.setSetting(m_name.c_str(), it->first.c_str(), *(bool*)(it->second.first));
                break;
            case WindowsSettings::SettingType::STRING:
                windowSettings.setSetting(m_name.c_str(), it->first.c_str(), *(std::string*)(it->second.first));
                break;
            default:
                break;
            }
        }

        ImGui::End();
    }

    afterShowWindow();
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
    clearGUIData();
    clear();
}

bool& BaseWindow::isOpen()
{
    return m_isOpen[workbench];
}

bool& BaseWindow::isOpen(const Workbench& wb)
{
    return m_isOpen[wb];
}

void BaseWindow::setOpen(const bool &isOpen)
{
    m_isOpen[workbench]=isOpen;
}

void BaseWindow::setOpen(const Workbench& wb, const bool &isOpen)
{
    m_isOpen[wb]=isOpen;
}

bool BaseWindow::isEnabledInWorkbench()
{
    return (m_enabledWorkbenches & workbench);
}

bool BaseWindow::isEnabledInWorkbench(const Workbench &wb)
{
    return (m_enabledWorkbenches & wb);
}

bool BaseWindow::isDefaultWorkbench(const Workbench &wb)
{
    return (m_defaultWorkbenches & m_enabledWorkbenches & wb);
}

void BaseWindow::showInfoMessage(const char* message)
{
    ImGui::BeginDisabled();
    ImGui::Text(ICON_FA_CIRCLE_INFO);
    ImGui::SameLine();
    ImGui::TextWrapped("%s", message);
    ImGui::EndDisabled();
}

void BaseWindow::dropGUIData()
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_DATAWIDGET"))
        {
            sofa::core::objectmodel::BaseData* data = static_cast<sofa::core::objectmodel::BaseData*>(payload->Data);
            if (data)
            {
                addData(data->getName(),
                        std::pair<sofa::core::BaseData*, bool>(data, false),
                        std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                        std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                        data->getOwner()? data->getOwner()->getPathName(): models::guidata::GUIData::DEFAULTGROUP,
                        data->getHelp());
            }
        }
        ImGui::EndDragDropTarget();
    }
}

}
