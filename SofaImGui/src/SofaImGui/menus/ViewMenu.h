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
#pragma once

#include <SofaImGui/config.h>
#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <imgui.h>


namespace sofaimgui::menus {

class ViewMenu
{
   public:
    ViewMenu(sofaglfw::SofaGLFWBaseGUI *baseGUI);
    ~ViewMenu();

    void addMenu(const std::pair<unsigned int, unsigned int>& fboSize,
                 const GLuint &texture);
    sofaglfw::SofaGLFWBaseGUI * m_baseGUI;

    void addShowIn3DViewMenu();
    void addShowIn3DViewMenuItems();
    void addAlignCameraMenu(sofa::component::visual::BaseCamera::SPtr camera);
    void addOrthographicMenuItem(sofa::component::visual::BaseCamera::SPtr camera);
    void addCenterCameraMenuItems(sofa::component::visual::BaseCamera::SPtr camera);

    void addSaveCameraMenuItem();
    void addRestoreCameraMenuItem();
    void addSaveScreenShotMenuItem(const std::pair<unsigned int, unsigned int>& fboSize,
                                   const GLuint& texture);

    void addFullScreenMenuItem();

   protected:

    void showGrid(const bool& show, const float &squareSize, const float &thickness, const sofa::type::RGBAColor& color);
    void showOriginFrame(const bool& show);
    void showBoundingBox(const bool& show);
};

}
