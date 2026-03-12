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

#include <sofa/type/Quat.h>

#include <imgui_internal.h>
#include <IconsFontAwesome6.h>

#include <SofaImGui/windows/MoveWindow.h>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/FooterStatusBar.h>
#include <SofaImGui/Workbench.h>

namespace sofaimgui::windows {

MoveWindow::MoveWindow(const std::string& name,
                         const bool& isWindowOpen)
{
    m_workbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;

    m_defaultIsOpen = true;
    m_name = name;
    m_isOpen = isWindowOpen;
    m_moveType = MoveType::SLIDERS;

    m_movePad = ImGui::MovePad("##MovePad", "X", "Z", "Y",
                                &m_x, &m_z, &m_y,
                                &m_TCPMinPosition, &m_TCPMaxPosition,
                                &m_TCPMinPosition, &m_TCPMaxPosition,
                                &m_TCPMinPosition, &m_TCPMaxPosition);
}

std::string MoveWindow::getDescription()
{
    return "Move the target of a robot's tool center position (TCP), its actuators, or accessories.";
}

void MoveWindow::clearWindow()
{
    m_IPController = nullptr;
    m_accessories.clear();
    m_actuators.clear();
}

void MoveWindow::setTCPDescriptions(const std::string &positionDescription, const std::string &rotationDescription)
{
    m_TCPPositionDescription = positionDescription;
    m_TCPRotationDescription = rotationDescription;
}

void MoveWindow::setTCPLimits(float minPosition, float maxPosition, double minOrientation, double maxOrientation)
{
    m_TCPMinPosition = minPosition;
    m_TCPMaxPosition = maxPosition;
    m_TCPMinOrientation = minOrientation;
    m_TCPMaxOrientation = maxOrientation;
}

void MoveWindow::setActuatorsDescriptions(const std::string &description)
{
    m_actuatorsDescription = description;
}

void MoveWindow::setActuatorsLimits(const double &min, const double &max)
{
    if (m_actuators.empty())
    {
        FooterStatusBar::getInstance().setTempMessage("Calling setActuatorsLimits() without any actuators set. Won't proceed."
                                                      "To fix this warning you can call setActuators() before calling setActuatorsLimits(). ", FooterStatusBar::MWARNING);
    }

    for (auto &actuator: m_actuators)
    {
        actuator.max = max;
        actuator.min = min;
    }
}

void MoveWindow::setActuatorLimits(const sofa::Index &id, const double &min, const double &max)
{
    if (id < m_actuators.size())
    {
        m_actuators[id].max = max;
        m_actuators[id].min = min;
    }
    else
    {
        FooterStatusBar::getInstance().setTempMessage("Calling setActuatorLimits() with 'id' greater than the number of actuators. Won't proceed."
                                                      "To fix this warning give a correct 'id' number.", FooterStatusBar::MWARNING);
    }
}

void MoveWindow::showWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI, const ImGuiWindowFlags &windowFlags)
{
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
        {
            if (enabled())
            {
                if (m_IPController != nullptr)
                {
                    ImGui::Spacing();

                    if(isDrivingSimulation())
                        m_IPController->getTCPTargetPosition(m_x, m_y, m_z, m_rx, m_ry, m_rz);

                    if (ImGui::CollapsingHeader(m_TCPPositionDescription.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        { // Vertical tabs (buttons)
                            ImGui::BeginChild("##MethodButtonsArea", ImVec2(ImGui::GetFrameHeight() * 1.5, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar);

                            if (showVerticalTab(ICON_FA_SLIDERS, "Sliders", m_moveType == MoveType::SLIDERS))
                                m_moveType = MoveType::SLIDERS;
                            if (showVerticalTab(ICON_FA_TABLE_CELLS_LARGE, "Pad", m_moveType == MoveType::PAD))
                                m_moveType = MoveType::PAD;

                            ImGui::EndChild();
                        }

                        ImGui::SameLine();

                        { // Method area (sliders or pad)
                            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
                            ImGui::BeginChild("##MethodArea", ImVec2(ImGui::GetContentRegionAvail().x, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
                            const auto &initPosition = m_IPController->getTCPTargetInitPosition();

                            if (m_moveType == MoveType::PAD)
                            {
                                m_movePad.setBounds("X", m_TCPMinPosition + initPosition[0], m_TCPMaxPosition + initPosition[0]);
                                m_movePad.setBounds("Y", m_TCPMinPosition + initPosition[1], m_TCPMaxPosition + initPosition[1]);
                                m_movePad.setBounds("Z", m_TCPMinPosition + initPosition[2], m_TCPMaxPosition + initPosition[2]);
                                showPad(baseGUI);
                            }
                            else if (m_moveType == MoveType::SLIDERS)
                            {
                                ImGui::Indent();
                                showSliderDouble("X", "##XSlider", "##XInput", &m_x, m_TCPMinPosition + initPosition[0], m_TCPMaxPosition + initPosition[0], ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                                ImGui::Spacing();
                                showSliderDouble("Y", "##YSlider", "##YInput", &m_y, m_TCPMinPosition + initPosition[1], m_TCPMaxPosition + initPosition[1], ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                                ImGui::Spacing();
                                showSliderDouble("Z", "##ZSlider", "##ZInput", &m_z, m_TCPMinPosition + initPosition[2], m_TCPMaxPosition + initPosition[2], ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
                                ImGui::Unindent();
                            }
                            ImGui::EndChild();
                            ImGui::PopStyleColor();
                        }
                    }

                    m_IPController->setFreeInRotation(m_freeRoll, m_freePitch, m_freeYaw);

                    if (m_IPController->hasRotationEffector() && ImGui::LocalBeginCollapsingHeader(m_TCPRotationDescription.c_str(), ImGuiTreeNodeFlags_AllowOverlap))
                    {
                        ImGui::SameLine();

                        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.x); // Set position to right of the line

                        bool openOptions = false;
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_Header));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_Header));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_Header));
                        if (ImGui::Button(ICON_FA_BARS, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
                            openOptions = true;
                        ImGui::PopStyleColor(3);

                        if (openOptions)
                        {
                            ImGui::OpenPopup("##RotationOptions");
                        }

                        if (ImGui::BeginPopup("##RotationOptions"))
                        {
                            showOptions();
                            ImGui::EndPopup();
                        }

                        if (m_freeRoll)
                            ImGui::BeginDisabled();
                        showSliderDouble("R", "##RSlider", "##RInput", &m_rx, m_TCPMinOrientation, m_TCPMaxOrientation, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        if (m_freeRoll)
                            ImGui::EndDisabled();

                        ImGui::Spacing();

                        if (m_freePitch)
                            ImGui::BeginDisabled();
                        showSliderDouble("P", "##PSlider", "##PInput", &m_ry, m_TCPMinOrientation, m_TCPMaxOrientation, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                        if (m_freePitch)
                            ImGui::EndDisabled();

                        ImGui::Spacing();

                        if (m_freeYaw)
                            ImGui::BeginDisabled();
                        showSliderDouble("Y", "##YawSlider", "##YawInput", &m_rz, m_TCPMinOrientation, m_TCPMaxOrientation, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
                        if (m_freeYaw)
                            ImGui::EndDisabled();

                        ImGui::LocalEndCollapsingHeader();
                    }

                    if (isDrivingSimulation())
                    {
                        sofa::type::Quat<SReal> q = m_IPController->getTCPPosition().getOrientation();
                        sofa::type::Vec3 rotation = q.toEulerVector();
                        m_IPController->setTCPTargetPosition(m_x, m_y, m_z,
                                                             m_freeRoll? rotation[0]: m_rx,
                                                             m_freePitch? rotation[1]: m_ry,
                                                             m_freeYaw? rotation[2]: m_rz);
                    }
                }

                if (!m_actuators.empty())
                {
                    if (ImGui::LocalBeginCollapsingHeader(m_actuatorsDescription.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (!isEnabledInWorkbench())
                        {
                            showInfoMessage("This section is disabled in the active workbench.");
                            ImGui::BeginDisabled();
                        }

                        int nbActuators = m_actuators.size();
                        bool solveInverseProblem = true;
                        for (int i=0; i<nbActuators; i++)
                        {
                            std::string name = "M" + std::to_string(i);

                            auto &actuator = m_actuators[i];

                            if (actuator.min < actuator.max)
                            {
                                auto* typeinfo = actuator.data->getValueTypeInfo();
                                auto* value = actuator.data->getValueVoidPtr();
                                double buffer = typeinfo->getScalarValue(value, 0);
                                bool hasChanged = showSliderDouble(name.c_str(), ("##Slider" + name).c_str(), ("##Input" + name).c_str(), &buffer,
                                                                   actuator.min, actuator.max,
                                                                   ImVec4(0, 0, 0, 0));
                                if (hasChanged)
                                {
                                    actuator.data->read(std::to_string(buffer));
                                    solveInverseProblem = false;
                                }
                                actuator.value=buffer;
                            }
                        }
                        if (m_IPController && !solveInverseProblem && isDrivingSimulation())
                        {
                            // TODO: don't solve the inverse problem since we'll overwrite the solution
                            m_IPController->applyActuatorsForce(m_actuators);
                        }

                        if (!isEnabledInWorkbench())
                            ImGui::EndDisabled();

                        ImGui::LocalEndCollapsingHeader();
                    }
                }

                if (!m_accessories.empty())
                {
                    if (ImGui::LocalBeginCollapsingHeader("Accessories", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        for (auto& accessory: m_accessories)
                        {
                            std::string name = accessory.description;

                            auto* typeinfo = accessory.data->getValueTypeInfo();
                            auto* value = accessory.data->getValueVoidPtr();
                            double buffer = typeinfo->getScalarValue(value, 0);
                            bool hasChanged = showSliderDouble(name.c_str(),
                                                               ("##Slider" + name).c_str(),
                                                               ("##Input" + name).c_str(),
                                                               &buffer, accessory.min, accessory.max,
                                                               ImVec4(0, 0, 0, 0));
                            if (hasChanged && isDrivingSimulation())
                            {
                                accessory.data->read(std::to_string(buffer));
                            }
                        }
                        ImGui::LocalEndCollapsingHeader();
                    }
                }
            }
            else
            {
                showInfoMessage("This window is used to move the target of a robot's tool center position (TCP), or actuators, using sliders. "
                               "The scene is missing elements for this window to work properly. "
                               );
            }
        }
        ImGui::End();
    }
}

bool MoveWindow::showSliderDouble(const char* name, const char* label1, const char *label2, double* v, const double& min, const double& max, const ImVec4& color)
{
    ImGui::AlignTextToFramePadding();
    ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
    pos.y += ImGui::GetFrameHeight() / 4.;
    ImVec2 size(1.0f, ImGui::GetFrameHeight() / 2.);
    ImGui::GetWindowDrawList()->AddRectFilled(pos,
                                              ImVec2(pos.x + size.x, pos.y + size.y),
                                              ImGui::GetColorU32(color), ImGuiStyleVar_FrameRounding);
    ImGui::Spacing();
    ImGui::SameLine();

    return showSliderDouble(name, label1, label2, v, min, max);
}

bool MoveWindow::showSliderDouble(const char* name, const char* label1, const char *label2, double* v, const double& min, const double& max)
{
    bool hasValueChanged = false;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", name);
    ImGui::SameLine();
    float inputWidth = ImGui::CalcTextSize("-100000,00").x + ImGui::GetFrameHeight() / 2 + ImGui::GetStyle().ItemSpacing.x * 2;
    float sliderWidth = ImGui::GetWindowWidth() - inputWidth - ImGui::CalcTextSize(name).x - ImGui::GetStyle().FramePadding.x * 3 - ImGui::GetStyle().IndentSpacing - ImGui::GetStyle().ScrollbarSize;

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
    ImGui::PushItemWidth(sliderWidth);
    if (ImGui::SliderScalar(label1, ImGuiDataType_Double, v, &min, &max, "%0.2f", ImGuiSliderFlags_NoInput))
        hasValueChanged=true;
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    double step = max - min;

    if (ImGui::LocalInputDouble(label2, v, powf(10.0f, floorf(log10f(step * 0.01))), step * 0.1))
        hasValueChanged=true;

    return hasValueChanged;
}

void MoveWindow::showOptions()
{
    if (ImGui::BeginTable("##option", 2, ImGuiTableFlags_None))
    {
        ImGui::TableNextColumn();
        ImGui::LocalCheckBox("Free roll", &m_freeRoll);
        if (m_freeRoll)
            ImGui::BeginDisabled();
        ImGui::TableNextColumn();
        showWeightOption(0);
        if (m_freeRoll)
            ImGui::EndDisabled();

        ImGui::TableNextColumn();
        ImGui::LocalCheckBox("Free pitch", &m_freePitch);
        if (m_freePitch)
            ImGui::BeginDisabled();
        ImGui::TableNextColumn();
        showWeightOption(1);
        if (m_freePitch)
            ImGui::EndDisabled();

        ImGui::TableNextColumn();
        ImGui::LocalCheckBox("Free yaw", &m_freeYaw);
        if (m_freeYaw)
            ImGui::BeginDisabled();
        ImGui::TableNextColumn();
        showWeightOption(2);
        if (m_freeYaw)
            ImGui::EndDisabled();

        ImGui::EndTable();
    }
}

void MoveWindow::showWeightOption(const int &i)
{
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    auto* weight = m_IPController->getRotationWeight();
    double w = weight[i];
    ImGui::AlignTextToFramePadding();
    ImGui::Text("weight");
    ImGui::SameLine();
    ImGui::PushID(i);
    ImGui::LocalInputDouble("##Input ", &w, 0, 0);
    ImGui::PopID();
    weight[i] = w;
}

void MoveWindow::showPad(sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    m_movePad.showPad(baseGUI);
}

bool MoveWindow::showVerticalTab(const std::string& label, const std::string& tooltip, const bool& active)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetStyle().FrameRounding/2.);
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_Tab));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_TabHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_TabActive));

    const ImVec2 buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
    bool clicked = false;

    if (active)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TabActive));
    }

    if (ImGui::Button(label.c_str(), buttonSize))
        clicked = true;
    ImGui::SetItemTooltip("%s", tooltip.c_str());

    if (active)
    {
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 2.f);
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    return clicked;
}
}

