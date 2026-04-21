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

#include <IconsFontAwesome6.h>
#include <SofaImGui/widgets/ImGuiDataWidget.h>
#include <sofa/core/behavior/BaseMechanicalState.h>
#include <sofa/type/Quat.h>

#include <imgui_internal.h>

#include <string>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/windows/MyRobotWindow.h>
#include <SofaImGui/Robot.h>
#include <SofaImGui/Workbench.h>


namespace sofaimgui::windows {

MyRobotWindow::MyRobotWindow(const std::string& name,
                         const bool& isWindowOpen)
{
    m_workbenches = Workbench::LIVE_CONTROL;

    m_defaultIsOpen = true;
    m_name = name;
    m_isOpen = isWindowOpen;
}

std::string MyRobotWindow::getDescription()
{
    return "Robot's information and settings. "
           "Also provides connection management features.";
}

void MyRobotWindow::clearWindow()
{
    BaseWindow::clearWindow();
	m_sectionedGUIData.clear();
}

bool MyRobotWindow::isInEmptyGroup(const std::string &group)
{
    return models::guidata::GUIData::DEFAULTGROUP.find(group) != std::string::npos;
}

void MyRobotWindow::setAvailablePorts(const std::vector<std::string> &ports)
{
    m_connection.ports.clear();
    m_connection.ports.reserve(ports.size());
    for(auto port: ports)
        m_connection.ports.push_back(port);

    Robot::getInstance().setDetected((m_connection.ports.size() > 0));
}

std::string MyRobotWindow::getSelectedPort()
{
    if (!m_connection.ports.empty())
        return m_connection.ports[m_connection.portId];

    return std::string();
}

MyRobotWindow::Connection& MyRobotWindow::getConnection()
{
    return m_connection;
}

models::guidata::GUIData::SPtr MyRobotWindow::addData(const std::string& label,
                                                        const std::pair<sofa::core::BaseData*, bool>& data,
                                                        const std::pair<sofa::core::BaseData*, bool>& min,
                                                        const std::pair<sofa::core::BaseData*, bool>& max,
                                                        const std::string& group,
                                                        const std::string& help, Section section)
{
    auto added = BaseWindow::addData(label, data, min, max, group, help);
	m_sectionedGUIData[section].insert(added);
	return added;
}

void MyRobotWindow::removeGUIData(models::guidata::GUIData::SPtr guiData)
{
    BaseWindow::removeGUIData(guiData);
	m_sectionedGUIData[Section::INFORMATION].erase(guiData);
	m_sectionedGUIData[Section::SETTINGS].erase(guiData);
}

bool MyRobotWindow::enabled()
{
    return (m_connection.listAvailablePortsCallback || !m_groupedGUIData.empty() || !m_GUIData.empty());
}

void MyRobotWindow::showWindow(const ImGuiWindowFlags &windowFlags)
{
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
        {
            if (enabled())
            {
                ImGui::Spacing();

                if (m_connection.listAvailablePortsCallback)
                { // Connection
                    if (ImGui::LocalBeginCollapsingHeader("Connection", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (!isEnabledInWorkbench())
                        {
                            showInfoMessage("This section is disabled in the active workbench.");
                            ImGui::BeginDisabled();
                        }

                        bool connected = Robot::getInstance().getConnection();

                        ImGui::Text("Available ports:");

                        if(connected)
                            ImGui::BeginDisabled();

                        ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 4);
                        const size_t nbPorts = m_connection.ports.size();
                        std::vector<const char*> ports;
                        ports.reserve(nbPorts);
                        for (size_t i=0; i<nbPorts; i++)
                            ports.push_back(m_connection.ports[i].c_str());
                        ImGui::LocalCombo("##ComboMethod", &m_connection.portId, ports.data(), nbPorts);
                        static bool firstTime = true;
                        if (ImGui::IsItemClicked() || firstTime)
                        {
                            firstTime = false;
                            setAvailablePorts(m_connection.listAvailablePortsCallback());
                        }
                        ImGui::PopItemWidth();

                        if(connected)
                            ImGui::EndDisabled();

                        ImGui::Text("Status:");
                        ImGui::SameLine();

                        ImGui::PushStyleColor(ImGuiCol_Text, (connected)? ImVec4(0.56f, 0.83f, 0.26f, 1.f): ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)); // TODO : color utils
                        ImGui::Text((connected)? "Connected": "Disconnected");
                        ImGui::PopStyleColor();

                        if (!isEnabledInWorkbench())
                            ImGui::EndDisabled();

                        ImGui::LocalEndCollapsingHeader();
                    }
                }

                if (!m_sectionedGUIData.empty())
                {
                    // Information
                    if (ImGui::LocalBeginCollapsingHeader("Information", ImGuiTreeNodeFlags_None))
                    {
                        std::string groups;
                        int k=0;

                        for (auto& itGroup : m_groupedGUIData)
                        {
                            ImGui::PushID(k++);
                            bool firsttime = true;

                            int i=0;
                            for (auto &data : itGroup.second)
                            {
								if(m_sectionedGUIData[Section::INFORMATION].contains(data))
                                {
                                    if (!isInEmptyGroup(data->group) && firsttime)
                                    {
                                        ImGui::TextDisabled("%s", data->group.c_str());
                                        ImGui::Indent();
									    firsttime = false;
                                    }
                                    ImGui::PushID(i++);
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::Text("%s", data->label.c_str());
                                    ImGui::SameLine();
                                    BaseDataWidget::showWidgetAsText(*data->getData());
                                    ImGui::PopID();
                                }
                            }

                            if (!isInEmptyGroup(itGroup.first) && !firsttime)
                                ImGui::Unindent();

                            ImGui::PopID();
                        }

                        ImGui::LocalEndCollapsingHeader();
                    }

                    // Settings
                    if (ImGui::LocalBeginCollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        std::string groups;
                        int k = 0;
                        for (auto& itGroup : m_groupedGUIData)
                        {
                            ImGui::PushID(k++);
                            bool firsttime = true;

                            for (auto& data : itGroup.second)
                            {
                                if (m_sectionedGUIData[Section::SETTINGS].contains(data))
                                {
                                    if (!isInEmptyGroup(data->group) && firsttime)
                                    {
                                        ImGui::TextDisabled("%s", data->group.c_str());
                                        ImGui::Indent();
                                        firsttime = false;
                                    }
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::Text("%s", data->label.c_str());
                                    ImGui::SameLine();
                                    showWidget(*data->getData());
                                }
                            }

                            if (!isInEmptyGroup(itGroup.first) && !firsttime)
                                ImGui::Unindent();

                            ImGui::PopID();
                        }
                        ImGui::LocalEndCollapsingHeader();
                    }
                }
            }
            else
            {
                showInfoMessage("This window is used to display the robot's information and settings. "
                               "It also provides connection management features. However, no information or settings"
                               " have been registered for display, nor is there any connection management available."
                               );
            }
        }
        ImGui::End();
    }
}


bool MyRobotWindow::showSliderDouble(const std::string& name, double* v, const double& min, const double& max, const int nbIndents)
{
    bool hasValueChanged = false;
    float inputWidth = ImGui::CalcTextSize("-100000,00").x + ImGui::GetFrameHeight() / 2 + ImGui::GetStyle().ItemSpacing.x * 2;
    float sliderWidth = ImGui::GetWindowWidth() - inputWidth - ImGui::CalcTextSize(name.c_str()).x - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().IndentSpacing * nbIndents - ImGui::GetStyle().ScrollbarSize;

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
    ImGui::PushItemWidth(sliderWidth);
    if (ImGui::SliderScalar(("##SettingSlider" + name).c_str() , ImGuiDataType_Double, v, &min, &max, "%0.2f", ImGuiSliderFlags_NoInput))
        hasValueChanged=true;
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    double step = max - min;

    if (ImGui::LocalInputDouble(("##SettingInput" + name).c_str(), v, powf(10.0f, floorf(log10f(step * 0.01))), step * 0.1))
        hasValueChanged=true;

    return hasValueChanged;
}

}

