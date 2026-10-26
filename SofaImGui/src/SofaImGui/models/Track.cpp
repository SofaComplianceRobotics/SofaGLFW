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
#include <SofaImGui/models/Track.h>


namespace sofaimgui::models {

Track::Track(guidata::KinematicsGUIDataManager::SPtr kinematicsGUIDataManager)
{
    guidata::EffectorGUIData::SPtr TCPGUIData = kinematicsGUIDataManager->getTCPGUIData();

    if (TCPGUIData && TCPGUIData->isValid())
        m_startmove = std::make_shared<actions::StartMove>(TCPGUIData->getTCPTargetInitPosition(),
                                                           TCPGUIData->getTCPTargetInitPosition(),
                                                           0.5,
                                                           kinematicsGUIDataManager,
                                                           true);
    else
        m_startmove = std::make_shared<actions::StartMove>(sofa::defaulttype::Rigid3Types::Coord(),
                                                           sofa::defaulttype::Rigid3Types::Coord(),
                                                           0.5,
                                                           kinematicsGUIDataManager,
                                                           true);
}

Track::Track(std::shared_ptr<actions::StartMove> startMove)
    : m_startmove(startMove)
{
}

void Track::clear()
{
    m_actions.clear();
}

sofa::Index Track::getActionIndex(actions::Action::SPtr action)
{
    auto it = std::find(m_actions.begin(), m_actions.end(), action);
    return it - m_actions.begin();
}

std::shared_ptr<actions::Move> Track::getPreviousMove(const sofa::Index &actionIndex)
{
    if (actionIndex==0 || m_actions.empty())
        return nullptr; // no previous move

    for (int i=actionIndex - 1; i>=0; i--)
    {
        std::shared_ptr<actions::Move> previous = std::dynamic_pointer_cast<actions::Move>(m_actions[i]);
        if (previous)
            return previous;
    }

    return nullptr; // no previous move
}

std::shared_ptr<actions::Move> Track::getNextMove(const sofa::Index &actionIndex)
{
    if (actionIndex + 1 == m_actions.size() || m_actions.empty())
        return nullptr; // no next move

    for (size_t i=actionIndex + 1; i<m_actions.size(); i++)
    {
        std::shared_ptr<actions::Move> next = std::dynamic_pointer_cast<actions::Move>(m_actions[i]);
        if (next)
            return next;
    }

    return nullptr; // no next move
}

void Track::updateNextMoveInitialPoint(const sofa::Index &actionIndex, const RigidCoord &initialPoint)
{
    std::shared_ptr<actions::Move> next = getNextMove(actionIndex);
    if (next)
        next->setInitialPoint(initialPoint);
}

void Track::swapActions(const sofa::Index& actionIndex1, const sofa::Index& actionIndex2)
{
    sofa::Size nbActions = m_actions.size();
    if (actionIndex1 <= nbActions - 1 && actionIndex2 <= nbActions - 1)
    {
        iter_swap(m_actions.begin() + actionIndex1, m_actions.begin() + actionIndex2);
        m_actions[actionIndex1]->swapWith(m_actions[actionIndex2]);
    }
}

bool Track::isActionSelected(const sofa::Index& actionIndex)
{
    int i = actionIndex;
    return (m_selectedActions.first == i || (m_selectedActions.first <= i && i <= m_selectedActions.second));
}

void Track::setActionSelected(const sofa::Index &index)
{
    int i = index;
    if ((ImGui::IsKeyDown(ImGuiKey_LeftShift) && m_selectedActions.first != -1 && m_selectedActions.second == -1 && m_selectedActions.first < i)
        || (ImGui::IsKeyDown(ImGuiKey_LeftShift) && m_selectedActions.first != -1 && m_selectedActions.second != -1 && m_selectedActions.second < i)
        )
    {
        m_selectedActions.second = i;
    }
    else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && m_selectedActions.first != -1 && m_selectedActions.second == -1 && m_selectedActions.first > i)
    {
        m_selectedActions.second = m_selectedActions.first;
        m_selectedActions.first = i;
    }
    else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && m_selectedActions.first != -1 && m_selectedActions.second != -1 && m_selectedActions.first > i)
    {
        m_selectedActions.first = i;
    }
    else if(m_selectedActions.first == i)
    {
        clearSelectedActions();
    }
    else
    {
        clearSelectedActions();
        m_selectedActions.first = i;
    }
}

void Track::clearSelectedActions()
{
    m_selectedActions.first = -1;
    m_selectedActions.second = -1;
}

void Track::group()
{
    if (canGroupSelectedActions())
        m_groups[m_actions[m_selectedActions.first]] = m_selectedActions.second - m_selectedActions.first;
}

void Track::ungroup(const int &actionIndex)
{
    for (auto [key, length]: m_groups)
    {
        if (isInGroup(actionIndex, std::pair<actions::Action::SPtr, int>(key, length)))
        {
            m_groups.erase(key);
            return;
        }
    }
}

bool Track::canGroup(const int& actionIndex)
{
    return isActionSelected(actionIndex) && !canUngroupSelectedActions() && canGroupSelectedActions();
}

bool Track::canUngroup(const int& actionIndex)
{
    return isInGroup(actionIndex);
}

bool Track::isInGroup(const int& actionIndex, bool strictly)
{
    for (auto [key, length]: m_groups)
    {
        if (isInGroup(actionIndex, std::pair<actions::Action::SPtr, int>(key, length), strictly))
            return true;
    }
    return false;
}

bool Track::isInGroup(const int& actionIndex, std::pair<actions::Action::SPtr, int> group, bool strictly)
{
    int firstIndex = getActionIndex(group.first);
    int lastIndex = firstIndex + group.second;

    if (strictly)
        return (firstIndex < actionIndex && actionIndex < lastIndex);

    return (firstIndex <= actionIndex && actionIndex <= lastIndex);
}

bool Track::canGroupSelectedActions()
{
    return m_selectedActions.first != -1 && m_selectedActions.second != -1 && m_selectedActions.first < m_selectedActions.second;
}

bool Track::canUngroupSelectedActions()
{
    for (int i = m_selectedActions.first ; i <= m_selectedActions.second ; i++)
        if (isInGroup(i))
            return true;
    return false;
}

} // namespace


