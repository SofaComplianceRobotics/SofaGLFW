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
                       const bool& isWindowOpen,
                       models::guidata::KinematicsGUIDataManager& kinematicsGUIDataManager)
{
    m_workbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;

    m_defaultIsOpen = true;
    m_name = name;
    m_isOpen = isWindowOpen;
    m_kinematicsGUIDataManager = kinematicsGUIDataManager;
    m_moveType = MoveType::SLIDERS;
}

std::string MoveWindow::getDescription()
{
    return "Move the target of a robot's tool center position (TCP), its actuators, or accessories.";
}

void MoveWindow::showWindow(const ImGuiWindowFlags &windowFlags)
{
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
        {
            if (enabled())
            {
                if (m_kinematicsGUIDataManager.hasInverseProblemSolverAndTCP())
                {
                    models::guidata::EffectorGUIData::SPtr TCPGUIData = m_kinematicsGUIDataManager.getTCPGUIData();

                    static bool firstTime = true;
                    if (firstTime)
                    {
                        firstTime = false;
                        double min = TCPGUIData->getMin();
                        double max = TCPGUIData->getMax();

                        m_movePad = ImGui::MovePad("##MovePad", "X", "Z", "Y",
                                                   &m_x, &m_z, &m_y,
                                                   min, max,
                                                   min, max,
                                                   min, max);
                    }

                    ImGui::Spacing();

                    if(isDrivingSimulation())
                        TCPGUIData->getTCPTargetPosition(m_x, m_y, m_z, m_rx, m_ry, m_rz);

                    if (ImGui::CollapsingHeader(TCPGUIData->label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
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
                            const auto &initPosition = TCPGUIData->getTCPTargetInitPosition();
                            double min = TCPGUIData->getMin();
                            double max = TCPGUIData->getMax();

                            if (m_moveType == MoveType::PAD)
                            {
                                m_movePad.setBounds("X", min + initPosition[0], max + initPosition[0]);
                                m_movePad.setBounds("Y", min + initPosition[1], max + initPosition[1]);
                                m_movePad.setBounds("Z", min + initPosition[2], max + initPosition[2]);
                                showPad();
                            }
                            else if (m_moveType == MoveType::SLIDERS)
                            {
                                ImGui::Indent();
                                showSliderDouble("X", "##XSlider", "##XInput", &m_x, min + initPosition[0], max + initPosition[0], ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                                ImGui::Spacing();
                                showSliderDouble("Y", "##YSlider", "##YInput", &m_y, min + initPosition[1], max + initPosition[1], ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                                ImGui::Spacing();
                                showSliderDouble("Z", "##ZSlider", "##ZInput", &m_z, min + initPosition[2], max + initPosition[2], ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
                                ImGui::Unindent();
                            }
                            ImGui::EndChild();
                            ImGui::PopStyleColor();
                        }
                    }

                    TCPGUIData->setFreeInRotation(m_freeRoll, m_freePitch, m_freeYaw);

                    if (TCPGUIData->hasRotation() && ImGui::LocalBeginCollapsingHeader("TODO Rotation", ImGuiTreeNodeFlags_AllowOverlap))
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
                        showSliderDouble("R", "##RSlider", "##RInput", &m_rx, -3.14, 3.14, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); //TODO real min max
                        if (m_freeRoll)
                            ImGui::EndDisabled();

                        ImGui::Spacing();

                        if (m_freePitch)
                            ImGui::BeginDisabled();
                        showSliderDouble("P", "##PSlider", "##PInput", &m_ry, -3.14, 3.14, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                        if (m_freePitch)
                            ImGui::EndDisabled();

                        ImGui::Spacing();

                        if (m_freeYaw)
                            ImGui::BeginDisabled();
                        showSliderDouble("Y", "##YawSlider", "##YawInput", &m_rz, -3.14, 3.14, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
                        if (m_freeYaw)
                            ImGui::EndDisabled();

                        ImGui::LocalEndCollapsingHeader();
                    }

                    if (isDrivingSimulation())
                    {
                        sofa::type::Quat<SReal> q = TCPGUIData->getTCPPosition().getOrientation();
                        sofa::type::Vec3 rotation = q.toEulerVector();
                        TCPGUIData->setTCPTargetPosition(m_x, m_y, m_z,
                                                         m_freeRoll? rotation[0]: m_rx,
                                                         m_freePitch? rotation[1]: m_ry,
                                                         m_freeYaw? rotation[2]: m_rz);
                    }
                }

                if (m_kinematicsGUIDataManager.hasActuator())
                {
                //     const auto& actuatorsGUIData = m_kinematicsGUIDataManager.getActuators();

                //     if (ImGui::LocalBeginCollapsingHeader(m_actuatorsDescription.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                //     {
                //         if (!isEnabledInWorkbench())
                //         {
                //             showInfoMessage("This section is disabled in the active workbench.");
                //             ImGui::BeginDisabled();
                //         }

                //         int nbActuators = m_actuators.size();
                //         bool solveInverseProblem = true;
                //         for (int i=0; i<nbActuators; i++)
                //         {
                //             std::string name = "M" + std::to_string(i);

                //             auto &actuator = m_actuators[i];

                //             if (actuator.min < actuator.max)
                //             {
                //                 auto* typeinfo = actuator.data->getValueTypeInfo();
                //                 auto* value = actuator.data->getValueVoidPtr();
                //                 double buffer = typeinfo->getScalarValue(value, 0);
                //                 bool hasChanged = showSliderDouble(name.c_str(), ("##Slider" + name).c_str(), ("##Input" + name).c_str(), &buffer,
                //                                                    actuator.min, actuator.max,
                //                                                    ImVec4(0, 0, 0, 0));
                //                 if (hasChanged)
                //                 {
                //                     actuator.data->read(std::to_string(buffer));
                //                     solveInverseProblem = false;
                //                 }
                //                 actuator.value=buffer;
                //             }
                //         }

                //         if (m_kinematicsGUIDataManager && !solveInverseProblem && isDrivingSimulation())
                //         {
                //             // TODO: don't solve the inverse problem since we'll overwrite the solution
                //             m_kinematicsGUIDataManager->applyActuatorsForce(m_actuators);
                //         }

                //         if (!isEnabledInWorkbench())
                //             ImGui::EndDisabled();

                //         ImGui::LocalEndCollapsingHeader();
                //     }
                }

                if (m_kinematicsGUIDataManager.hasAccessoryComponent())
                {
                    if (ImGui::LocalBeginCollapsingHeader("Accessories", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                //         for (auto& accessory: m_accessories)
                //         {
                //             std::string name = accessory.description;

                //             auto* typeinfo = accessory.data->getValueTypeInfo();
                //             auto* value = accessory.data->getValueVoidPtr();
                //             double buffer = typeinfo->getScalarValue(value, 0);
                //             bool hasChanged = showSliderDouble(name.c_str(),
                //                                                ("##Slider" + name).c_str(),
                //                                                ("##Input" + name).c_str(),
                //                                                &buffer, accessory.min, accessory.max,
                //                                                ImVec4(0, 0, 0, 0));

                //             if (hasChanged && isDrivingSimulation())
                //             {
                //                 accessory.data->read(std::to_string(buffer));
                //             }
                //         }
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

void MoveWindow::showWeightOption(const int &index)
{
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    models::guidata::EffectorGUIData::SPtr TCPGUIData = m_kinematicsGUIDataManager.getTCPGUIData();
    double w = TCPGUIData->getWeight(index);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("weight");
    ImGui::SameLine();
    ImGui::PushID(index);
    ImGui::LocalInputDouble("##Input ", &w, 0, 0);
    ImGui::PopID();
    TCPGUIData->setWeight(index, w);
}

void MoveWindow::showPad()
{
    m_movePad.showPad(m_baseGUI);
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

