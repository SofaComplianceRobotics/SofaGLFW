/******************************************************************************
 *                 SOFA, Simulation Open-Framework Architecture                *
 *                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
 *                                                                             *
 * This Track is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the Free  *
 * Software Foundation; either version 2 of the License, or (at your option)   *
 * any later version.                                                          *
 *                                                                             *
 * This Track is distributed in the hope that it will be useful, but WITHOUT *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
 * more details.                                                               *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with this Track. If not, see <http://www.gnu.org/licenses/>.              *
 *******************************************************************************
 * Authors: The SOFA Team and external contributors (see Authors.txt)          *
 *                                                                             *
 * Contact information: contact@sofa-framework.org                             *
 ******************************************************************************/

#include "imgui.h"

#include <sofa/component/visual/LineAxis.h>
#include <sofa/component/visual/VisualBoundingBox.h>
#include <sofa/component/visual/VisualStyle.h>

#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/models/View.h>
#include <SofaImGui/FooterStatusBar.h>

#include <SofaGLFW/SofaGLFWWindow.h>



namespace sofaimgui::models {
using sofaglfw::SofaGLFWWindow;

void addViewportViewMenu(sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    if (ImGui::BeginMenu("Grid"))
    {
        static bool show01 = false;
        if (ImGui::LocalCheckBox(std::string("Square size: " + SofaGLFWWindow::GridSquareSize::getString(SofaGLFWWindow::GridSquareSize::DOTONE)).c_str(), &show01))
            showGrid(baseGUI, show01, SofaGLFWWindow::GridSquareSize::DOTONE, 1.f, sofa::type::RGBAColor::fromFloat(0.5f, 0.5f, 0.5f, 0.5f));
        static bool show1 = false;
        if (ImGui::LocalCheckBox(std::string("Square size: " + SofaGLFWWindow::GridSquareSize::getString(SofaGLFWWindow::GridSquareSize::ONE)).c_str(), &show1))
            showGrid(baseGUI, show1, SofaGLFWWindow::GridSquareSize::ONE, 1.f, sofa::type::RGBAColor::fromFloat(0.5f, 0.5f, 0.5f, 0.75f));
        static bool show10 = false;
        if (ImGui::LocalCheckBox(std::string("Square size: " + SofaGLFWWindow::GridSquareSize::getString(SofaGLFWWindow::GridSquareSize::TEN)).c_str(), &show10))
            showGrid(baseGUI, show10, SofaGLFWWindow::GridSquareSize::TEN, 1.f, sofa::type::RGBAColor::fromFloat(0.5f, 0.5f, 0.5f, 1.f));
        static bool show100 = false;
        if (ImGui::LocalCheckBox(std::string("Square size: " + SofaGLFWWindow::GridSquareSize::getString(SofaGLFWWindow::GridSquareSize::HUNDRED)).c_str(), &show100))
            showGrid(baseGUI, show100, SofaGLFWWindow::GridSquareSize::HUNDRED, 2.f, sofa::type::RGBAColor::fromFloat(0.5f, 0.5f, 0.5f, 1.f));

        ImGui::EndMenu();
    }

    {
        static bool show = false;
        if (ImGui::LocalCheckBox("Origin Frame", &show))
            showOriginFrame(baseGUI, show);
        ImGui::SetItemTooltip("Show / hide");
    }

    {
        static bool show = false;
        if (ImGui::LocalCheckBox("Bounding Box", &show))
            showBoundingBox(baseGUI, show);
        ImGui::SetItemTooltip("Show / hide");
    }

    ImGui::Separator();

    sofa::component::visual::VisualStyle::SPtr visualStyle = nullptr;
    const auto& groot = baseGUI->getRootNode();
    groot->get(visualStyle);
    if (visualStyle)
    {
        auto& displayFlags = sofa::helper::getWriteAccessor(visualStyle->d_displayFlags).wref();

        {
            const bool initialValue = displayFlags.getShowVisualModels();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Visual Models", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowVisualModels(changeableValue);
            }
        }

        {
            const bool initialValue = displayFlags.getShowBehaviorModels();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Behavior Models", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowBehaviorModels(changeableValue);
            }
        }

        {
            const bool initialValue = displayFlags.getShowForceFields();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Force Fields", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowForceFields(changeableValue);
            }
        }

        {
            const bool initialValue = displayFlags.getShowInteractionForceFields();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Interaction Force Fields", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowInteractionForceFields(changeableValue);
            }
        }

        {
            const bool initialValue = displayFlags.getShowCollisionModels();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Collision Models", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowCollisionModels(changeableValue);
            }
        }

        {
            const bool initialValue = displayFlags.getShowBoundingCollisionModels();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Bounding Collision Models", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowBoundingCollisionModels(changeableValue);
            }
        }

        ImGui::Separator();

        {
            const bool initialValue = displayFlags.getShowMappings();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Mappings", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowMappings(changeableValue);
            }
        }

        {
            const bool initialValue = displayFlags.getShowMechanicalMappings();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Mechanical Mappings", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowMechanicalMappings(changeableValue);
            }
        }

        ImGui::Separator();

        {
            const bool initialValue = displayFlags.getShowWireFrame();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Wire Frame", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowWireFrame(changeableValue);
            }
        }

        {
            const bool initialValue = displayFlags.getShowNormals();
            bool changeableValue = initialValue;
            ImGui::LocalCheckBox("Normals", &changeableValue);
            ImGui::SetItemTooltip("Show / hide");
            if (changeableValue != initialValue)
            {
                displayFlags.setShowNormals(changeableValue);
            }
        }
    }
}

void showGrid(sofaglfw::SofaGLFWBaseGUI* baseGUI, const bool& show, const float& squareSize, const float& thickness, const sofa::type::RGBAColor& color)
{
    if (!baseGUI)
        return;

    const auto& groot = baseGUI->getRootNode();
    const auto& bbox = groot->f_bbox.getValue();
    if (groot && bbox.isValid())
    {
        auto bboxSize = bbox.maxBBox() - bbox.minBBox();
        auto gridSize = floor(*std::max_element(bboxSize.begin(), bboxSize.end()) * 10); // we choose the grid to be ten times larger than the bounding box
        gridSize = floor(gridSize / squareSize);
        gridSize -= fmod(gridSize, 2); // garanties that the grid is centered wrt the origin
        gridSize *= squareSize; // garanties that the grid square size is the one selected
        if (gridSize / squareSize < 2)
        {
            if (show)
                FooterStatusBar::getInstance().setTempMessage("The selected square size is too large wrt to the bounding box the scene.",
                                                              FooterStatusBar::MWARNING);
            return;
        }

        auto guiNode = groot->getChild(baseGUI->getGUINodeName());
        if (guiNode)
        {
            std::string name = "ViewportGrid" + SofaGLFWWindow::GridSquareSize::getString(squareSize);
            auto grid = guiNode->get<sofa::component::visual::VisualGrid>(name);
            if (!grid)
            {
                auto newGrid = sofa::core::objectmodel::New<sofa::component::visual::VisualGrid>();
                guiNode->addObject(newGrid);
                newGrid->setName(name);
                newGrid->addTag(sofa::core::objectmodel::Tag("createdByGUI"));
                newGrid->d_enable.setValue(show);
                newGrid->d_size.setValue(gridSize);
                newGrid->d_thickness.setValue(thickness);
                newGrid->d_nbSubdiv.setValue(gridSize / squareSize);
                newGrid->d_color.setValue(color);
                newGrid->init();
            }
            else
            {
                grid->d_enable.setValue(show);
            }
        }
    }

    sofaglfw::SofaGLFWWindow::setGridsPlane(baseGUI);
}

void showOriginFrame(sofaglfw::SofaGLFWBaseGUI* baseGUI, const bool& show)
{
    const auto& groot = baseGUI->getRootNode();
    if (groot)
    {
        auto guiNode = groot->getChild(baseGUI->getGUINodeName());
        if (guiNode)
        {
            auto originFrame = guiNode->get<sofa::component::visual::LineAxis>();
            if (!originFrame)
            {
                auto newOriginFrame = sofa::core::objectmodel::New<sofa::component::visual::LineAxis>();
                guiNode->addObject(newOriginFrame);
                newOriginFrame->setName("ViewportOriginFrame");
                newOriginFrame->addTag(sofa::core::objectmodel::Tag("createdByGUI"));
                newOriginFrame->d_enable.setValue(show);
                newOriginFrame->d_infinite.setValue(true);
                newOriginFrame->d_thickness.setValue(2.f);
                newOriginFrame->d_vanishing.setValue(true);
                newOriginFrame->init();
            }
            else
            {
                originFrame->d_enable.setValue(show);
            }
        }
    }
}

void showBoundingBox(sofaglfw::SofaGLFWBaseGUI* baseGUI, const bool& show)
{
    const auto& groot = baseGUI->getRootNode();
    if (groot)
    {
        auto guiNode = groot->getChild(baseGUI->getGUINodeName());
        if (guiNode)
        {
            auto bbox = guiNode->get<sofa::component::visual::VisualBoundingBox>();
            if (!bbox)
            {
                auto newBBox = sofa::core::objectmodel::New<sofa::component::visual::VisualBoundingBox>();
                guiNode->addObject(newBBox);
                newBBox->setName("VisualBoundingBox");
                newBBox->d_enable.setValue(show);
                newBBox->f_bbox.setParent(&groot->f_bbox);
                newBBox->d_color.setValue(sofa::type::RGBAColor::white());
                newBBox->d_thickness.setValue(1.0f);
            }
            else
            {
                bbox->d_enable.setValue(show);
            }
        }
    }
}

} // namespace


