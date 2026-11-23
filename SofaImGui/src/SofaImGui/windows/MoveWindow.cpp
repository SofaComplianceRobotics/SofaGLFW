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

#include <SofaImGui/widgets/DataWidget.h>
#include <sofa/type/Quat.h>

#include <imgui_internal.h>
#include <IconsFontAwesome6.h>

#include <SofaImGui/windows/MoveWindow.h>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/FooterStatusBar.h>
#include <SofaImGui/Workbench.h>
#include <GUIColors.h>
#include <SofaImGui/windows/WindowsSettingsName.h>

namespace sofaimgui::windows {

MoveWindow::MoveWindow(const std::string& name,
                       models::guidata::KinematicsGUIDataManager::SPtr kinematicsGUIDataManager)
    : BaseWindow(name)
{
    m_enabledWorkbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;
    m_kinematicsGUIDataManager = kinematicsGUIDataManager;
    m_moveType = MoveType::SLIDERS;
}

void MoveWindow::setTCPSliders(const double& x, const double& y, const double& z, const double& rx, const double& ry, const double& rz)
{
    m_x=x; m_y=y; m_z=z;
    m_rx=rx; m_ry=ry; m_rz=rz;
}

void MoveWindow::clampTCPSliders(double& x, double& y, double& z, double& rx, double& ry, double& rz)
{
    models::guidata::EffectorGUIData::SPtr TCPGUIData = m_kinematicsGUIDataManager->getTCPGUIData();
    const auto &initPosition = TCPGUIData->getTCPTargetInitPosition();
    const double& min = TCPGUIData->getMin();
    const double& max = TCPGUIData->getMax();
    x = std::max(x, min + initPosition[0]); x = std::min(x, max + initPosition[0]);
    y = std::max(y, min + initPosition[1]); y = std::min(y, max + initPosition[1]);
    z = std::max(z, min + initPosition[2]); z = std::min(z, max + initPosition[2]);
    rx = std::max(rx, min); rx = std::min(rx, max);
    ry = std::max(ry, min); ry = std::min(ry, max);
    rz = std::max(rz, min); rz = std::min(rz, max);
}

void MoveWindow::manageTCPSlidersInLocalFrame(const bool& valueChanged)
{
    static bool pressed = false;
    if (valueChanged || pressed)
    {
        models::guidata::EffectorGUIData::SPtr TCPGUIData = m_kinematicsGUIDataManager->getTCPGUIData();
        static bool firstTime = true;

        if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && firstTime)
        {
            m_globalTCPPosition = TCPGUIData->getTCPTargetPosition();
            firstTime = false;
            pressed = true;
        }
        else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            m_globalTCPPosition = TCPGUIData->getTCPTargetPosition();
            setTCPSliders(0.,0.,0.,0.,0.,0.); // We are in relative control
            firstTime = true;
            pressed = false;
        }
    }
}

void MoveWindow::localToGlobalFrame(double& x, double& y, double& z, double& rx, double& ry, double& rz)
{
    sofa::defaulttype::RigidCoord<3, SReal> p_local(sofa::type::Vec3(m_x, m_y, m_z), sofa::type::Quat<SReal>::createQuaterFromEuler(sofa::type::Vec3(m_rx, m_ry, m_rz)));
    sofa::defaulttype::RigidCoord<3, SReal> result;
    result.getCenter() = m_globalTCPPosition.getOrientation().rotate(p_local.getCenter()) + m_globalTCPPosition.getCenter();
    result.getOrientation() = m_globalTCPPosition.getOrientation() * p_local.getOrientation() ;

    sofa::type::Vec3 euler_result = result.getOrientation().toEulerVector();
    x=result[0]; y=result[1]; z=result[2];
    rx=euler_result[0]; ry=euler_result[1]; rz=euler_result[2];
}

std::string MoveWindow::getDescription()
{
    return "Move the target of a robot's tool center position (TCP), its actuators, or accessories.";
}

void MoveWindow::onEndSimulationLoad()
{
    if (m_kinematicsGUIDataManager->hasInverseProblemSolverAndTCP())
        m_kinematicsGUIDataManager->getTCPGUIData()->getFreeInRotation(m_freeRoll, m_freePitch, m_freeYaw);
}

void MoveWindow::registerAndLoadWindowSettings()
{
    registerAndLoadWindowSetting(WS_MOVE_LOCALFRAME, m_ws_localframe, WindowsSettings::BOOL);
}

void MoveWindow::internalShowWindow()
{
    if (isEnabledByState())
    {
        if (m_kinematicsGUIDataManager->hasInverseProblemSolverAndTCP())
        {
            models::guidata::EffectorGUIData::SPtr TCPGUIData = m_kinematicsGUIDataManager->getTCPGUIData();

            static bool firstTime = true;
            static ImGuiTreeNodeFlags orientationDefaultOpen = ImGuiTreeNodeFlags_None;
            if (firstTime)
            {
                firstTime = false;
                const double& min = TCPGUIData->getMin();
                const double& max = TCPGUIData->getMax();

                if (!m_freeRoll || !m_freePitch || !m_freeYaw)
                    orientationDefaultOpen = ImGuiTreeNodeFlags_DefaultOpen;

                m_movePad = sofaimgui::widgets::MovePad("##MovePad", "X", "Z", "Y",
                                                       &m_x, &m_z, &m_y,
                                                       min, max,
                                                       min, max,
                                                       min, max);
            }

            ImGui::Spacing();

            { // Local frame
                if(sofaimgui::widgets::ToggleButton("Local frame", &m_ws_localframe))
                {
                    if (m_ws_localframe)
                    {
                        m_globalTCPPosition = TCPGUIData->getTCPTargetPosition();
                        setTCPSliders(0.,0.,0.,0.,0.,0.); // We switch to relative control
                    }
                }

                ImGui::SetItemTooltip("Either to move the TCP in the local or global frame.");
                ImGui::SameLine();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Local frame");

                if (m_ws_localframe)
                {
                    double x = m_x; double y = m_y; double z = m_z;
                    double rx = m_rx; double ry = m_ry; double rz = m_rz;
                    localToGlobalFrame(x, y, z, rx, ry, rz);
                    clampTCPSliders(x, y, z, rx, ry, rz);
                    ImGui::SameLine();
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextDisabled("(X:%.1f, Y:%.1f, Z:%.1f, R:%.1f, P:%.1f, Y:%.1f)", x, y, z, rx, ry, rz);
                }
            }

            if(isDrivingSimulation() && !m_ws_localframe)
                TCPGUIData->getTCPTargetPosition(m_x, m_y, m_z, m_rx, m_ry, m_rz);

            // TCP Target Position
            if (ImGui::CollapsingHeader((TCPGUIData->getLabel() + " Position").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
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
                    const double& min = TCPGUIData->getMin();
                    const double& max = TCPGUIData->getMax();
                    bool valueChanged = false;

                    if (m_moveType == MoveType::PAD)
                    {
                        m_movePad.setBounds("X", m_ws_localframe? min: min + initPosition[0], m_ws_localframe? max: max + initPosition[0]);
                        m_movePad.setBounds("Y", m_ws_localframe? min: min + initPosition[1], m_ws_localframe? max: max + initPosition[1]);
                        m_movePad.setBounds("Z", m_ws_localframe? min: min + initPosition[2], m_ws_localframe? max: max + initPosition[2]);
                        valueChanged |= showPad();
                    }
                    else if (m_moveType == MoveType::SLIDERS)
                    {
                        ImGui::Indent();
                        valueChanged |= showSliderDouble("X", "##XSlider", "##XInput", &m_x, m_ws_localframe? min: min + initPosition[0], m_ws_localframe? max: max + initPosition[0], COLOR_RED);
                        ImGui::Spacing();
                        valueChanged |= showSliderDouble("Y", "##YSlider", "##YInput", &m_y, m_ws_localframe? min: min + initPosition[1], m_ws_localframe? max: max + initPosition[1], COLOR_GREEN);
                        ImGui::Spacing();
                        valueChanged |= showSliderDouble("Z", "##ZSlider", "##ZInput", &m_z, m_ws_localframe? min: min + initPosition[2], m_ws_localframe? max: max + initPosition[2], COLOR_BLUE);
                        ImGui::Unindent();
                    }

                    if (m_ws_localframe)
                        manageTCPSlidersInLocalFrame(valueChanged);

                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                }
            }

            TCPGUIData->setFreeInRotation(m_freeRoll, m_freePitch, m_freeYaw);

            // TCP Target Orientation
            if (TCPGUIData->hasRotation() && sofaimgui::widgets::BeginCollapsingHeader((TCPGUIData->getLabel() + " Orientation").c_str(), ImGuiTreeNodeFlags_AllowOverlap | orientationDefaultOpen))
            {
                ImGui::SameLine();

                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.x); // Set position to right of the line

                bool openOptions = false;
                ImGui::PushStyleColor(ImGuiCol_Button, COLOR_TRANSPARENT);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_TRANSPARENT);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_TRANSPARENT);
                if (sofaimgui::widgets::Button(ICON_FA_BARS))
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

                bool valueChanged = false;
                if (m_freeRoll)
                    ImGui::BeginDisabled();
                valueChanged |= showSliderDouble("R", "##RSlider", "##RInput", &m_rx, TCPGUIData->getMinRotation(), TCPGUIData->getMaxRotation(), COLOR_RED);
                if (m_freeRoll)
                    ImGui::EndDisabled();

                ImGui::Spacing();

                if (m_freePitch)
                    ImGui::BeginDisabled();
                valueChanged |= showSliderDouble("P", "##PSlider", "##PInput", &m_ry, TCPGUIData->getMinRotation(), TCPGUIData->getMaxRotation(), COLOR_GREEN);
                if (m_freePitch)
                    ImGui::EndDisabled();

                ImGui::Spacing();

                if (m_freeYaw)
                    ImGui::BeginDisabled();
                valueChanged |= showSliderDouble("Y", "##YawSlider", "##YawInput", &m_rz, TCPGUIData->getMinRotation(), TCPGUIData->getMaxRotation(), COLOR_BLUE);
                if (m_freeYaw)
                    ImGui::EndDisabled();

                if (m_ws_localframe)
                    manageTCPSlidersInLocalFrame(valueChanged);

                sofaimgui::widgets::EndCollapsingHeader();
            }

            // Apply user changes to TCP target
            if (isDrivingSimulation())
            {
                sofa::type::Quat<SReal> q = TCPGUIData->getTCPPosition().getOrientation();
                sofa::type::Vec3 rotation = q.toEulerVector();

                double x = m_x; double y = m_y; double z = m_z;
                double rx = m_rx; double ry = m_ry; double rz = m_rz;

                if (m_ws_localframe)
                {
                    localToGlobalFrame(x, y, z, rx, ry, rz);
                    clampTCPSliders(x, y, z, rx, ry, rz);
                }

                TCPGUIData->setTCPTargetPosition(x, y, z,
                                                 m_freeRoll? rotation[0]: rx,
                                                 m_freePitch? rotation[1]: ry,
                                                 m_freeYaw? rotation[2]: rz);
            }
        }

        // Actuators
        if (m_kinematicsGUIDataManager->hasActuator())
        {
            const auto& actuatorsGUIData = m_kinematicsGUIDataManager->getActuators();

            if (sofaimgui::widgets::BeginCollapsingHeader("Actuators", ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool solveInverseProblem = true;
                for (auto actuatorGUIData: actuatorsGUIData)
                {
                    std::string name = actuatorGUIData->getLabel();
                    if (actuatorGUIData->getMin() < actuatorGUIData->getMax())
                    {
                        for (sofa::Index index=0; index<actuatorGUIData->getSize(); index++)
                        {
                            double value = actuatorGUIData->getValue(index);

                            if (actuatorGUIData->getSize() > 1)
                                name += std::to_string(index);

                            if (showSliderDouble(name.c_str(),
                                                 ("##Slider" + name).c_str(),
                                                 ("##Input" + name).c_str(),
                                                 &value,
                                                 actuatorGUIData->getMin(), actuatorGUIData->getMax(),
                                                 ImGui::GetColorU32(ImGuiCol_Text)))
                            {
                                actuatorGUIData->setValue(index, value);
                                solveInverseProblem = false;
                            }
                        }
                    }
                }

                if (m_kinematicsGUIDataManager
                    && m_kinematicsGUIDataManager->hasInverseProblemSolver()
                    && !solveInverseProblem
                    && isDrivingSimulation())
                    m_kinematicsGUIDataManager->switchSolverMode();

                sofaimgui::widgets::EndCollapsingHeader();
            }
        }

        // Accessories
        if (m_kinematicsGUIDataManager->hasAccessory())
        {
            if (sofaimgui::widgets::BeginCollapsingHeader("Accessories", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const auto& accessoriesGUIData = m_kinematicsGUIDataManager->getAccessories();

                for (auto [group, accessories]: accessoriesGUIData)
                {
                    ImGui::TextDisabled("%s", group.c_str());
                    ImGui::Indent();

                    for (auto accessory: accessories)
                    {
                        if (accessory && accessory->isValid())
                        {
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("%s", accessory->getFeatureLabel().c_str());
                            ImGui::SameLine();
                            sofaimgui::widgets::showWidget(*accessory->getData(), accessory->getDataMin(), accessory->getDataMax());
                        }
                    }

                    ImGui::Unindent();
                }
                sofaimgui::widgets::EndCollapsingHeader();
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

bool MoveWindow::showSliderDouble(const char* name, const char* label1, const char *label2, double* v, const double& min, const double& max, const ImU32& color)
{
    bool hasValueChanged = false;

    ImGui::AlignTextToFramePadding();

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text("%s", name);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    float inputWidth = ImGui::CalcTextSize("-100000,00").x + ImGui::GetFrameHeight() / 2 + ImGui::GetStyle().FramePadding.x;
    float sliderWidth = ImGui::GetContentRegionAvail().x - inputWidth;

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
    ImGui::PushItemWidth(sliderWidth);
    if (ImGui::SliderScalar(label1, ImGuiDataType_Double, v, &min, &max, "%0.2f", ImGuiSliderFlags_NoInput))
        hasValueChanged=true;
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    double step = max - min;

    if (sofaimgui::widgets::InputDouble(label2, v, powf(10.0f, floorf(log10f(step * 0.01))), step * 0.1))
        hasValueChanged=true;

    return hasValueChanged;
}

void MoveWindow::showOptions()
{
    { // Free all
        bool freeAll = m_freeRoll && m_freePitch && m_freeYaw;
        if (sofaimgui::widgets::CheckBox("Free all", &freeAll))
        {
            m_freeRoll = freeAll;
            m_freePitch = freeAll;
            m_freeYaw = freeAll;
        }
    }

    float indentWidth = ImGui::GetFrameHeightWithSpacing() * 0.5 + ImGui::GetStyle().ItemSpacing.x;
    ImGui::Indent(indentWidth);

    { // Roll Pitch Yaw
        sofaimgui::widgets::CheckBox("Roll", &m_freeRoll);

        if (m_freeRoll)
            ImGui::BeginDisabled();
        showWeightOption(3);
        if (m_freeRoll)
            ImGui::EndDisabled();

        sofaimgui::widgets::CheckBox("Pitch", &m_freePitch);

        if (m_freePitch)
            ImGui::BeginDisabled();
        showWeightOption(4);
        if (m_freePitch)
            ImGui::EndDisabled();

        sofaimgui::widgets::CheckBox("Yaw", &m_freeYaw);

        if (m_freeYaw)
            ImGui::BeginDisabled();
        showWeightOption(5);
        if (m_freeYaw)
            ImGui::EndDisabled();
    }

    ImGui::Unindent(indentWidth);
}

void MoveWindow::showWeightOption(const int &index)
{
    float indentWidth = ImGui::GetFrameHeightWithSpacing() * 0.5 + ImGui::GetStyle().ItemSpacing.x;
    ImGui::Indent(indentWidth);

    ImGui::AlignTextToFramePadding();
    models::guidata::EffectorGUIData::SPtr TCPGUIData = m_kinematicsGUIDataManager->getTCPGUIData();
    double w = TCPGUIData->getWeight(index);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Weight");
    ImGui::SameLine();
    ImGui::PushID(index);
    if (sofaimgui::widgets::InputDouble("##Input ", &w))
        TCPGUIData->setWeight(index, w);
    ImGui::PopID();

    ImGui::Unindent(indentWidth);
}

bool MoveWindow::showPad()
{
    return m_movePad.showPad(m_baseGUI);
}

bool MoveWindow::showVerticalTab(const std::string& label, const std::string& tooltip, const bool& active)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetStyle().FrameRounding/2.);
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_Tab));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_TabHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_TabActive));

    bool clicked = false;

    if (active)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TabActive));
    }

    if (sofaimgui::widgets::Button(label.c_str()))
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

