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
#pragma once

#include <memory>

#include <SofaImGui/models/actions/Action.h>
#include <SofaImGui/models/actions/Move.h>
#include <SofaImGui/models/actions/StartMove.h>
#include <SofaImGui/models/guidata/KinematicsGUIDataManager.h>

#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <sofa/core/objectmodel/DataFileName.h>
#include <SofaImGui/config.h>


namespace sofaimgui::models {

class Track
{
    typedef sofa::defaulttype::RigidCoord<3, double> RigidCoord;

   public:

    typedef std::shared_ptr<Track> SPtr;

    Track(models::guidata::KinematicsGUIDataManager::SPtr kinematicsGUIDataManager);
    Track(std::shared_ptr<actions::StartMove> startMove);
    ~Track() = default;

    void clear();

    std::shared_ptr<actions::StartMove> getStartMove() {return m_startmove;}

    std::vector<actions::Action::SPtr>& getActions() {return m_actions;}
    actions::Action::SPtr getAction(const sofa::Index& actionIndex) {return m_actions[actionIndex];}
    sofa::Index getActionIndex(actions::Action::SPtr action);

    void updateNextMoveInitialPoint(const sofa::Index &actionIndex, const RigidCoord &initialPoint);

    std::shared_ptr<actions::Move> getPreviousMove(const sofa::Index &actionIndex);
    std::shared_ptr<actions::Move> getNextMove(const sofa::Index &actionIndex);

    void swapActions(const sofa::Index& actionIndex1, const sofa::Index& actionIndex2);

    bool hasSelectedActions() {return m_selectedActions.first != -1;}
    bool isActionSelected(const sofa::Index &actionIndex);
    void setActionSelected(const sofa::Index &index);
    void clearSelectedActions();

    void group();
    void ungroup(const int& actionIndex);
    bool canGroup(const int& actionIndex);
    bool canUngroup(const int& actionIndex);
    bool isInGroup(const int& actionIndex, bool strictly = false);

    int getGroupLength(actions::Action::SPtr action) {return m_groups.contains(action)? m_groups[action]: 0;}

   protected:

    std::shared_ptr<actions::StartMove> m_startmove;
    std::vector<actions::Action::SPtr> m_actions;
    std::map<actions::Action::SPtr, int> m_groups; /// Map of groups: key = first action sptr, value = length of the group

    std::pair<int, int> m_selectedActions{-1,-1};

    bool canGroupSelectedActions();
    bool canUngroupSelectedActions();
    bool isInGroup(const int& actionIndex, std::pair<actions::Action::SPtr, int> group, bool strictly = false);
};

} // namespace


