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

#include <sofa/type/Material.h>
#include <SofaImGui/windows/BaseWindow.h>
#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <SofaImGui/Workbench.h>
#include <imgui.h>

namespace sofaimgui::windows {

class SOFAIMGUI_API SceneGraphWindow : public BaseWindow
{
public:
    SceneGraphWindow(const std::string& name, const bool& isWindowOpen);
    ~SceneGraphWindow() = default;

    void showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags) override;
    std::string getDescription() override;
    void clearWindow() override;

protected:

    std::set<std::pair<sofa::simulation::Node*, bool>> m_openedNodePopups;
    std::set<std::pair<sofa::core::objectmodel::BaseObject*, bool>> m_openedComponentPopups;
    std::set<sofa::simulation::Node*> m_openedNodes;
    std::set<sofa::core::objectmodel::BaseObject*> m_openedComponents;
    std::set<sofa::core::objectmodel::Base::SPtr> m_selection;

    sofa::type::Material m_highlightMaterial;

    int m_modifyingRow{-1};

    bool m_renaming{false};
    bool m_renamingTreeOpen{false};
    sofa::core::objectmodel::Base* m_renamingObject{nullptr};

    bool m_showSearch = false;
    bool m_showFiltered = false;
    bool m_showFilteredWarning = false;
    bool m_showFilteredError = false;
    bool m_showFilteredInfo = false;

    void showGraph(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags,
                   std::set<sofa::core::objectmodel::BaseObject*>& componentToOpen,
                   std::set<sofa::simulation::Node *> &nodeToOpen,
                   std::set<std::pair<sofa::core::objectmodel::BaseObject*, bool>>& componentToOpenContextMenu,
                   std::set<std::pair<sofa::simulation::Node *, bool> > &nodeToOpenContextMenu);
    void showNodeComponents(sofaglfw::SofaGLFWBaseGUI *baseGUI,
                            sofa::simulation::Node* node,
                            const ImGuiTextFilter &filter,
                            const bool& expandAll, const bool&collapseAll,
                            std::set<sofa::core::objectmodel::BaseObject*>& componentToOpen,
                            std::set<std::pair<sofa::core::objectmodel::BaseObject*, bool>>& componentToOpenContextMenu);
    bool showComponentWindow(sofa::core::objectmodel::BaseObject* component, const ImGuiWindowFlags &windowsFlags);
    bool showNodeWindow(sofa::simulation::Node* node, const ImGuiWindowFlags &windowsFlags);

    void addComponentDocTextLinkOpenURL(sofa::core::objectmodel::BaseObject *component);

    void addGroupTab(const std::map<std::string, std::vector<sofa::core::BaseData*> >& groupMap);
    void addLinksTab(const sofa::core::objectmodel::Base::VecLink& links);
    void addMessagesTab(const std::deque<sofa::helper::logging::Message> &messages, const std::string& name, const std::string &icon);
    void addInfosTab(sofa::simulation::Node* node);
    void addNodeContextMenu(sofa::simulation::Node *node);
    void addComponentContextMenu(sofa::core::objectmodel::BaseObject*component);
    void addBaseContextMenu(sofa::core::objectmodel::Base *object);

    std::string getComponentIconAlert(sofa::core::objectmodel::BaseObject* object, ImVec4& objectColor, std::string& icon);
    void updateSelection(sofa::core::objectmodel::Base::SPtr object);

    void highlightOglModels(sofa::simulation::Node *node);
    void resetOglModels(sofa::simulation::Node *node);

    bool showTemplate(sofa::core::objectmodel::BaseObject *object, sofa::simulation::Node *node);
    bool showName(sofa::core::objectmodel::Base *object, const std::string icon, const std::string name, ImGuiTreeNodeFlags objectFlags = ImGuiTreeNodeFlags_None);

    bool showAddNodeButton(sofa::simulation::Node *node);

    bool showRemoveNodeButton(sofa::simulation::Node *parent, sofa::simulation::Node *node);
    bool showRemoveComponentButton(sofa::simulation::Node *parent, sofa::core::objectmodel::BaseObject *component);
};

}


