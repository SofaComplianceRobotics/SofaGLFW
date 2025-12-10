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
#include <sofa/component/visual/BaseCamera.h>
#include <sofa/core/visual/VisualParams.h>

#include <SofaImGui/menus/ViewMenu.h>
#include <SofaImGui/models/View.h>
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
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    if (ImGui::BeginMenu("View"))
    {
        sofa::component::visual::BaseCamera::SPtr camera;
        const auto& groot = m_baseGUI->getRootNode();
        groot->get(camera);

        ImGui::PopStyleColor();

        addViewport();

        ImGui::Separator();

        addCenterCamera(camera);
        addAlignCamera(camera);
        addOrthographic(camera);

        ImGui::Separator();

        addSaveCamera();
        addRestoreCamera();

        ImGui::Separator();

        addSaveScreenShot(fboSize, texture);

        ImGui::Separator();

        addFullScreen();

        ImGui::EndMenu();
    }
    else
    {
        ImGui::PopStyleColor();
    }
}

void ViewMenu::addViewport()
{
    if (ImGui::BeginMenu("Show"))
    {
        models::addViewportViewMenu(m_baseGUI);

        ImGui::EndMenu();
    }
}

void ViewMenu::addAlignCamera(sofa::component::visual::BaseCamera::SPtr camera)
{
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

void ViewMenu::addOrthographic(sofa::component::visual::BaseCamera::SPtr camera)
{
    bool ortho = (camera->getCameraType() == sofa::core::visual::VisualParams::ORTHOGRAPHIC_TYPE);
    if (ImGui::MenuItem((ortho)?"Perspective":"Orthographic"))
    {
        camera->setCameraType((!ortho)? sofa::core::visual::VisualParams::ORTHOGRAPHIC_TYPE: sofa::core::visual::VisualParams::PERSPECTIVE_TYPE);
    }
}

void ViewMenu::addFullScreen()
{
    if (ImGui::MenuItem("Full Screen", "F11"))
    {
        m_baseGUI->switchFullScreen();
    }
}

void ViewMenu::addCenterCamera(sofa::component::visual::BaseCamera::SPtr camera)
{
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

void ViewMenu::addSaveCamera()
{
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

void ViewMenu::addRestoreCamera()
{
    const std::string viewFileName = m_baseGUI->getFilename() + ".view";
    bool fileExists = sofa::helper::system::FileSystem::exists(viewFileName);
    ImGui::BeginDisabled(!fileExists);
    if (ImGui::MenuItem("Restore View"))
    {
        SofaGLFWWindow::resetSimulationView(m_baseGUI);
    }
    ImGui::EndDisabled();
}

void ViewMenu::addSaveScreenShot(const std::pair<unsigned int, unsigned int>& fboSize,
                                 const GLuint& texture)
{
    if (ImGui::MenuItem("Save Screenshot..."))
    {
        std::string screenshotPath = sofa::gui::common::BaseGUI::getScreenshotDirectoryPath();
        nfdchar_t *outPath;
        std::array<nfdfilteritem_t, 1> filterItem{ {{"Image", "jpg,png"}} };

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
