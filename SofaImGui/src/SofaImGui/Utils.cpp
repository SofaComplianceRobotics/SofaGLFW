/******************************************************************************
 *                 SOFA, Utils Open-Framework Architecture                *
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

#include <sofa/core/visual/VisualParams.h>
#include <SofaImGui/menus/ViewMenu.h>
#include <sofa/helper/system/FileSystem.h>
#include <SofaImGui/Utils.h>
#include <sofa/core/behavior/BaseMechanicalState.h>
#include <sofa/gui/common/ArgumentParser.h>
#include <SofaGLFW/SofaGLFWWindow.h>

namespace sofaimgui::Utils {

using sofaimgui::menus::ViewMenu;

static bool withArguments = true;

void loadFile(sofaglfw::SofaGLFWBaseGUI *baseGUI, const bool& reload, const std::string &filePathName)
{
    if (baseGUI && !filePathName.empty() && sofa::helper::system::FileSystem::exists(filePathName))
    {
        sofa::core::sptr<sofa::simulation::Node> groot = baseGUI->getRootNode();
        sofa::simulation::node::unload(groot);
        const std::vector<std::string> sceneArgs = (reload && withArguments)? sofa::gui::common::ArgumentParser::extra_args(): std::vector<std::string>(0);

        groot = sofa::simulation::node::load(filePathName.c_str(), reload, sceneArgs);
        if(!groot)
            groot = sofa::simulation::getSimulation()->createNewGraph("");
        baseGUI->setSimulation(groot, filePathName);
        baseGUI->setWindowTitle(nullptr, std::string("SOFA - " + filePathName).c_str());

        sofa::simulation::node::initRoot(groot.get());
        auto camera = baseGUI->findCamera(groot);
        if (camera)
        {
            const auto& bbox = groot->f_bbox.getValue();
            camera->fitBoundingBox(bbox.minBBox(), bbox.maxBBox());
            baseGUI->changeCamera(camera);
        }
        baseGUI->initVisual();
    }
}

void loadSimulation(sofaglfw::SofaGLFWBaseGUI *baseGUI, const bool& reload, const std::string& filePathName)
{
    if(!reload)
        withArguments = false; // Forget python arguments once the user opens a new simulation from the GUI
    loadFile(baseGUI, reload, filePathName);
    sofaglfw::SofaGLFWWindow::resetSimulationView(baseGUI);
}

} // namespace


