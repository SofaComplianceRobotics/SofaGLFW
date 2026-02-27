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

#include <sofa/component/visual/VisualStyle.h>
#include <sofa/component/visual/VisualGrid.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/component/visual/BaseCamera.h>

#include <sofa/component/visual/LineAxis.h>
#include <sofa/component/visual/VisualBoundingBox.h>

#include <SofaImGui/menus/ViewMenu.h>
#include <SofaImGui/FooterStatusBar.h>
#include <SofaGLFW/SofaGLFWWindow.h>

#include <sofa/helper/system/FileSystem.h>
#include <sofa/helper/io/STBImage.h>
#include <sofa/gui/common/BaseGUI.h>

#include <nfd.h>
#include <filesystem>
#include <SofaImGui/Utils.h>
#include <SofaImGui/widgets/Widgets.h>

namespace sofaimgui::menus {

using sofaglfw::SofaGLFWWindow;

ViewMenu::ViewMenu(sofaglfw::SofaGLFWBaseGUI *baseGUI) : m_baseGUI(baseGUI)
{
}

ViewMenu::~ViewMenu()
{
}

void ViewMenu::addMenu(const std::pair<unsigned int, unsigned int>& fboSize,
                       const GLuint& texture)
{
    if (!m_baseGUI)
        return;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    if (ImGui::BeginMenu("View"))
    {
        sofa::component::visual::BaseCamera::SPtr camera;
        const auto& groot = m_baseGUI->getRootNode();
        groot->get(camera);

        ImGui::PopStyleColor();

        addShowIn3DViewMenu();

        ImGui::Separator();

        addCenterCameraMenuItems(camera);
        addAlignCameraMenu(camera);
        addOrthographicMenuItem(camera);

        ImGui::Separator();

        addSaveCameraMenuItem();
        addRestoreCameraMenuItem();

        ImGui::Separator();

        addSaveScreenShotMenuItem(fboSize, texture);

        ImGui::Separator();

        addFullScreenMenuItem();

        ImGui::EndMenu();
    }
    else
    {
        ImGui::PopStyleColor();
    }
}

void ViewMenu::showGrid(const bool& show, const float& squareSize, const float& thickness, const sofa::type::RGBAColor& color)
{
    const auto& groot = m_baseGUI->getRootNode();
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

        auto guiNode = groot->getChild(m_baseGUI->getGUINodeName());
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

    sofaglfw::SofaGLFWWindow::setGridsPlane(m_baseGUI);
}

void ViewMenu::showOriginFrame(const bool& show)
{
    const auto& groot = m_baseGUI->getRootNode();
    if (groot)
    {
        auto guiNode = groot->getChild(m_baseGUI->getGUINodeName());
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

void ViewMenu::showBoundingBox(const bool& show)
{
    const auto& groot = m_baseGUI->getRootNode();
    if (groot)
    {
        auto guiNode = groot->getChild(m_baseGUI->getGUINodeName());
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

void ViewMenu::addShowIn3DViewMenu()
{
    if (ImGui::BeginMenu("Show"))
    {
        addShowIn3DViewMenuItems();
        ImGui::EndMenu();
    }
}

void ViewMenu::addShowIn3DViewMenuItems()
{
    if (!m_baseGUI)
        return;

    const auto& groot = m_baseGUI->getRootNode();

    if (ImGui::BeginMenu("Grid"))
    {
        static bool show01 = false;
        if (ImGui::LocalCheckBox(std::string("Square size: " + SofaGLFWWindow::GridSquareSize::getString(SofaGLFWWindow::GridSquareSize::DOTONE)).c_str(), &show01))
            showGrid(show01, SofaGLFWWindow::GridSquareSize::DOTONE, 1.f, sofa::type::RGBAColor::fromFloat(0.5f, 0.5f, 0.5f, 0.5f));
        static bool show1 = false;
        if (ImGui::LocalCheckBox(std::string("Square size: " + SofaGLFWWindow::GridSquareSize::getString(SofaGLFWWindow::GridSquareSize::ONE)).c_str(), &show1))
            showGrid(show1, SofaGLFWWindow::GridSquareSize::ONE, 1.f, sofa::type::RGBAColor::fromFloat(0.5f, 0.5f, 0.5f, 0.75f));
        static bool show10 = false;
        if (ImGui::LocalCheckBox(std::string("Square size: " + SofaGLFWWindow::GridSquareSize::getString(SofaGLFWWindow::GridSquareSize::TEN)).c_str(), &show10))
            showGrid(show10, SofaGLFWWindow::GridSquareSize::TEN, 1.f, sofa::type::RGBAColor::fromFloat(0.5f, 0.5f, 0.5f, 1.f));
        static bool show100 = false;
        if (ImGui::LocalCheckBox(std::string("Square size: " + SofaGLFWWindow::GridSquareSize::getString(SofaGLFWWindow::GridSquareSize::HUNDRED)).c_str(), &show100))
            showGrid(show100, SofaGLFWWindow::GridSquareSize::HUNDRED, 2.f, sofa::type::RGBAColor::fromFloat(0.5f, 0.5f, 0.5f, 1.f));

        ImGui::EndMenu();
    }

    {
        static bool show = false;
        if (ImGui::LocalCheckBox("Origin Frame", &show))
            showOriginFrame(show);
        ImGui::SetItemTooltip("Show / hide");
    }

    {
        static bool show = false;
        if (ImGui::LocalCheckBox("Bounding Box", &show))
            showBoundingBox(show);
        ImGui::SetItemTooltip("Show / hide");
    }

    ImGui::Separator();

    sofa::component::visual::VisualStyle::SPtr visualStyle = nullptr;
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

void ViewMenu::addAlignCameraMenu(sofa::component::visual::BaseCamera::SPtr camera)
{
    if (!m_baseGUI)
        return;

    if (ImGui::BeginMenu("Align"))
    {
        if (camera)
        {
            if (ImGui::MenuItem("Top", "1"))
            {
                SofaGLFWWindow::alignCamera(m_baseGUI, SofaGLFWWindow::CameraAlignement::TOP);
            }

            if (ImGui::MenuItem("Bottom", "2"))
            {
                SofaGLFWWindow::alignCamera(m_baseGUI, SofaGLFWWindow::CameraAlignement::BOTTOM);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Front", "3"))
            {
                SofaGLFWWindow::alignCamera(m_baseGUI, SofaGLFWWindow::CameraAlignement::FRONT);
            }

            if (ImGui::MenuItem("Back", "4"))
            {
                SofaGLFWWindow::alignCamera(m_baseGUI, SofaGLFWWindow::CameraAlignement::BACK);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Left", "5"))
            {
                SofaGLFWWindow::alignCamera(m_baseGUI, SofaGLFWWindow::CameraAlignement::LEFT);
            }

            if (ImGui::MenuItem("Right", "6"))
            {
                SofaGLFWWindow::alignCamera(m_baseGUI, SofaGLFWWindow::CameraAlignement::RIGHT);
            }
        }
        ImGui::EndMenu();
    }
}

void ViewMenu::addOrthographicMenuItem(sofa::component::visual::BaseCamera::SPtr camera)
{
    bool ortho = (camera->getCameraType() == sofa::core::visual::VisualParams::ORTHOGRAPHIC_TYPE);
    if (ImGui::MenuItem((ortho)?"Perspective":"Orthographic"))
    {
        camera->setCameraType((!ortho)? sofa::core::visual::VisualParams::ORTHOGRAPHIC_TYPE: sofa::core::visual::VisualParams::PERSPECTIVE_TYPE);
    }
}

void ViewMenu::addFullScreenMenuItem()
{
    if (!m_baseGUI)
        return;

    if (ImGui::MenuItem("Full Screen", "F11"))
    {
        m_baseGUI->switchFullScreen();
    }
}

void ViewMenu::addCenterCameraMenuItems(sofa::component::visual::BaseCamera::SPtr camera)
{
    if (!m_baseGUI)
        return;

    const auto& groot = m_baseGUI->getRootNode();
    const auto& bbox = groot->f_bbox.getValue();

    if(!bbox.isValid())
    {
        msg_error_when(!bbox.isValid(), "GUI") << "Global bounding box is invalid: " << bbox;
        return;
    }

    if (ImGui::MenuItem("Fit All", "0"))
    {
        if (camera)
        {
            camera->fitBoundingBox(bbox.minBBox(), bbox.maxBBox());
            auto bbCenter = (bbox.maxBBox() + bbox.minBBox()) * 0.5f;
            camera->d_lookAt.setValue(bbCenter);
        }
    }

    if (ImGui::MenuItem("Center"))
    {
        if (camera)
        {
            auto bbCenter = (bbox.maxBBox() + bbox.minBBox()) * 0.5f;
            camera->d_lookAt.setValue(bbCenter);
        }
    }
}

void ViewMenu::addSaveCameraMenuItem()
{
    if (!m_baseGUI)
        return;

    const std::string viewFileName = m_baseGUI->getFilename() + ".view";
    if (ImGui::MenuItem("Save View"))
    {
        sofa::component::visual::BaseCamera::SPtr camera;
        const auto& groot = m_baseGUI->getRootNode();
        groot->get(camera);
        if (camera)
        {
            if (camera->exportParametersInFile(viewFileName))
            {
                FooterStatusBar::getInstance().setTempMessage("Current camera parameters have been exported to " + viewFileName + ".");
            }
            else
            {
                FooterStatusBar::getInstance().setTempMessage("Could not export camera parameters to " + viewFileName + ".", FooterStatusBar::MERROR);
            }
        }
    }
}

void ViewMenu::addRestoreCameraMenuItem()
{
    if (!m_baseGUI)
        return;

    const std::string viewFileName = m_baseGUI->getFilename() + ".view";
    bool fileExists = sofa::helper::system::FileSystem::exists(viewFileName);
    ImGui::BeginDisabled(!fileExists);
    if (ImGui::MenuItem("Restore View"))
    {
        SofaGLFWWindow::resetSimulationView(m_baseGUI);
    }
    ImGui::EndDisabled();
}

void ViewMenu::addSaveScreenShotMenuItem(const std::pair<unsigned int, unsigned int>& fboSize,
                                         const GLuint& texture)
{
    if (!m_baseGUI)
        return;

    if (ImGui::MenuItem("Save Screenshot..."))
    {
        std::string screenshotPath = sofa::gui::common::BaseGUI::getScreenshotDirectoryPath();
        nfdchar_t *outPath;
        std::array<nfdfilteritem_t, 1> filterItem{{{"Image", "jpg,png"}}};

        // Add the date and time to the filename
        auto now = std::chrono::system_clock::now();
        auto localTime = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;

        auto sceneFilename = m_baseGUI->getFilename();
        if (!sceneFilename.empty())
        {
            std::filesystem::path path(sceneFilename);
            ss << path.filename().replace_extension("").string() << "_" << std::put_time(std::localtime(&localTime), "%F_%H-%M-%S") << ".png";
        } else {
            ss << "screenshot_" << std::put_time(std::localtime(&localTime), "%F_%H-%M-%S") << ".png";
        }
        sceneFilename = ss.str();

        nfdresult_t result = NFD_SaveDialog(&outPath, filterItem.data(), filterItem.size(), screenshotPath.c_str(), sceneFilename.c_str());
        if (result == NFD_OKAY)
        {
            sofa::helper::io::STBImage image;
            image.init(fboSize.first, fboSize.second, 1, 1,
                       sofa::helper::io::Image::DataType::UINT32,
                       sofa::helper::io::Image::ChannelFormat::RGBA);

            glBindTexture(GL_TEXTURE_2D, texture);

            // Read the pixel data from the OpenGL texture
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixels());

            glBindTexture(GL_TEXTURE_2D, 0);

            image.save(outPath, 90);
        }
    }
}

}
