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

#include <filesystem>
#include <sofa/core/CategoryLibrary.h>
#include <SofaImGui/windows/BaseWindow.h>

namespace sofaimgui::windows
{

/**
* @brief Components Window.
*
* Displays a window listing all available components along with their categories.
* It allows users to select a component to view its details, including name, description, templates, 
* aliases, namespaces, parents, targets, and data properties.
*/
class SOFAIMGUI_API ComponentsWindow : public BaseWindow
{

public:
    ComponentsWindow(){}
    ComponentsWindow(const std::string& name, const bool& isWindowOpen);
    ~ComponentsWindow()=default;

    void showWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI, const ImGuiWindowFlags &windowFlags) override;

protected:

    std::vector<std::string> m_examplesPaths;
    std::vector<std::filesystem::path> m_selectedComponentExamples;

    void showComponentsList(std::vector<sofa::core::ClassEntry::SPtr> components, sofa::core::ObjectFactory::ClassEntry::SPtr &selectedComponent);
    void showComponentInfo(sofa::core::ClassEntry::SPtr selectedComponent);
    void showComponentData(sofa::core::ObjectFactory::ClassEntry::SPtr selectedComponent);

    void saveFile();

};

} // namespace 
