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

#include <imgui.h>

#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <sofa/gui/common/MouseOperations.h>
#include <SofaImGui/windows/MouseManagerWindow.h>
#include <SofaImGui/widgets/Widgets.h>

namespace sofaimgui::windows {

MouseManagerWindow::MouseManagerWindow(const std::string& name,
                                       const bool& isWindowOpen)
{
    m_defaultIsOpen = false;
    m_name = name;
    m_isOpen = isWindowOpen;
}

std::string MouseManagerWindow::getDescription()
{
    return "Mouse settings, for interaction with the simulation.";
}

void MouseManagerWindow::showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags)
{
    SOFA_UNUSED(baseGUI);
    if (isOpen())
    {
        if (auto* pickHandler = baseGUI->getPickHandler())
        {
            ImGuiIO& io = ImGui::GetIO();
            const ImVec2 defaultSize = ImVec2(io.DisplaySize.x * 0.5, io.DisplaySize.y * 0.3);
            ImGui::SetNextWindowSize(defaultSize, ImGuiCond_Once);
            ImGui::Begin(getName().c_str(), &m_isOpen, windowFlags | ImGuiWindowFlags_NoDocking);

            ImGui::TextDisabled("Mouse interaction with the simulation is enabled by pressing the left shift key.");

            if (ImGui::BeginChild("##MouseLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.33f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
            {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
                ImGui::Text("Left Button:");
                if (ImGui::BeginChild("##MouseLeftSettings", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
                    showMouseSettings(pickHandler, sofa::gui::common::MOUSE_BUTTON::LEFT);
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();

            ImGui::SameLine();

            if (ImGui::BeginChild("##MouseMiddle", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
            {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
                ImGui::Text("Middle Button:");
                if (ImGui::BeginChild("##MouseMiddleSettings", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
                    showMouseSettings(pickHandler, sofa::gui::common::MOUSE_BUTTON::MIDDLE);
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();

            ImGui::SameLine();

            if (ImGui::BeginChild("##MouseRight", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
            {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
                ImGui::Text("Right Button:");
                if (ImGui::BeginChild("##MouseRightSettings", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
                    showMouseSettings(pickHandler, sofa::gui::common::MOUSE_BUTTON::RIGHT);
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();

            ImGui::End();
        }
    }
}

void MouseManagerWindow::showMouseSettings(PickHandler* pickHandler, sofa::gui::common::MOUSE_BUTTON button)
{
    if (!pickHandler)
        return;

    auto* operation = pickHandler->getOperation(button);
    if (!operation)
        return;

    OperationFactory* operationFactory = OperationFactory::getInstance();
    if (!operationFactory)
        return;

    auto* selectedOperation = operationFactory->registry[operation->getId()];
    if (!selectedOperation)
        return;

    std::string currentOperationDescription = selectedOperation->getDescription();

    ImGui::PushID(button);
    if (ImGui::BeginCombo("Operation", currentOperationDescription.c_str()))
    {
        for (const auto& [label, creator] : operationFactory->registry)
        {
            const bool isSelected = operation->getId() == label;

            if (!creator)
                continue;

            if (ImGui::Selectable(creator->getDescription().c_str(), isSelected))
                pickHandler->changeOperation(button, label);

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SetItemTooltip("%s", currentOperationDescription.c_str());
    ImGui::PopID();

    ImGui::Separator();

    ImGui::Indent();

    if (!operation)
        return;

    if (auto* attachOperation = dynamic_cast<AttachOperation*>(operation)) //also valid for ConstraintAttachOperation because of inheritance
    {
        float stiffness = attachOperation->getStiffness();
        if (ImGui::LocalInputFloat("Stiffness", &stiffness))
        {
            stiffness = std::clamp(stiffness, 0.0f, 1000.0f);
            attachOperation->setStiffness(stiffness);
        }

        float arrowSize = attachOperation->getArrowSize();
        if (ImGui::LocalInputFloat("Arrow Size", &arrowSize))
        {
            arrowSize = std::clamp(arrowSize, 0.0f, 10.0f);
            attachOperation->setArrowSize(arrowSize);
        }

        float showFactorSize = attachOperation->getShowFactorSize();
        if (ImGui::LocalInputFloat("Show Factor Size", &showFactorSize))
        {
            showFactorSize = std::clamp(showFactorSize, 1.0f, 5.0f);
            attachOperation->setShowFactorSize(showFactorSize);
        }
    }
    else if (auto* fixOperation = dynamic_cast<FixOperation*>(operation))
    {
        float stiffness = fixOperation->getStiffness();
        if (ImGui::LocalInputFloat("Stiffness", &stiffness))
        {
            stiffness = std::clamp(stiffness, 0.0f, 100000.0f);
            fixOperation->setStiffness(stiffness);
        }
    }
    else if (auto* inciseOperation = dynamic_cast<InciseOperation*>(operation))
    {
        auto incisionMethod = inciseOperation->getIncisionMethod();
        ImGui::Text("Type:");
        if (ImGui::LocalRadioButton("Through segment", incisionMethod == 0))
        {
            inciseOperation->setIncisionMethod(0);
        }
        ImGui::SetItemTooltip("Incise from click to click");
        if (ImGui::LocalRadioButton("Continually", incisionMethod != 0))
        {
            inciseOperation->setIncisionMethod(1);
        }
        ImGui::SetItemTooltip("Incise continually from first click localization");

        ImGui::Text("Distance to snap:");

        int snapingBorderValue = inciseOperation->getSnapingBorderValue();
        ImGui::PushItemWidth(ImGui::GetFrameHeight()*3);
        if (ImGui::InputInt("##DistanceFromBorder", &snapingBorderValue))
        {
            snapingBorderValue = std::clamp(snapingBorderValue, 0, 100);
            inciseOperation->setSnapingBorderValue(snapingBorderValue);
        }
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("From border (in %)");
        ImGui::PopItemWidth();

        int snapingValue = inciseOperation->getSnapingValue();
        ImGui::PushItemWidth(ImGui::GetFrameHeight()*3);
        if (ImGui::InputInt("##DistanceAlongPath", &snapingValue))
        {
            snapingValue = std::clamp(snapingValue, 0, 100);
            inciseOperation->setSnapingValue(snapingValue);
        }
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Along path (in %)");
        ImGui::PopItemWidth();

        bool finishIncision = inciseOperation->getCompleteIncision();
        if (ImGui::LocalCheckBox("Complete incision", &finishIncision))
        {
            inciseOperation->setCompleteIncision(finishIncision);
        }

        bool keepPoint = inciseOperation->getKeepPoint();
        if (ImGui::LocalCheckBox("Keep in memory last incision point", &keepPoint))
        {
            inciseOperation->setKeepPoint(keepPoint);
        }
    }
    else if (auto* topologyOperation = dynamic_cast<TopologyOperation*>(operation))
    {
        {
            auto topologicalOperation = topologyOperation->getTopologicalOperation();
            ImGui::Text("Topological operation:");
            if (ImGui::LocalRadioButton("Remove one element", topologicalOperation == 0))
            {
                topologyOperation->setTopologicalOperation(0);
            }
            if (ImGui::LocalRadioButton("Remove a zone of elements", topologicalOperation != 0))
            {
                topologyOperation->setTopologicalOperation(1);
            }
        }

        static float scale = topologyOperation->getScale();
        if (ImGui::LocalInputFloat("Selector scale", &scale))
        {
            scale = std::clamp(scale, 0.0f, 100.0f);
            topologyOperation->setScale(scale);
        }

        {
            auto volumicMesh = topologyOperation->getVolumicMesh();
            ImGui::Text("Remove area type:");
            if (ImGui::LocalRadioButton("Surface", !volumicMesh))
            {
                topologyOperation->setVolumicMesh(false);
            }
            if (ImGui::LocalRadioButton("Volume", volumicMesh))
            {
                topologyOperation->setVolumicMesh(true);
            }
        }
    }
    else if (auto* sutureOperation = dynamic_cast<AddSutureOperation*>(operation))
    {
        static float stiffness = 10.0f;
        if (ImGui::LocalInputFloat("Stiffness", &stiffness))
        {
            stiffness = std::clamp(stiffness, 0.0f, 1000.0f);
            sutureOperation->setStiffness(stiffness);
        }

        static float damping = 1.0f;
        if (ImGui::LocalInputFloat("Damping", &damping))
        {
            damping = std::clamp(damping, 0.0f, 10.0f);
            sutureOperation->setDamping(damping);
        }
    }
    ImGui::Unindent();
}

} // namespace windows
