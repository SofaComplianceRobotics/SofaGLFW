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
#include <SofaImGui/models/modifiers/Repeat.h>


namespace sofaimgui::models {

Track::Track(models::IPController::SPtr IPController)
    : m_IPController(IPController)
{
    m_startmove = std::make_shared<models::actions::StartMove>(m_IPController->getTCPTargetInitPosition(),
                                                               m_IPController->getTCPTargetInitPosition(),
                                                               0.5,
                                                               m_IPController,
                                                               true);
}

Track::Track(models::IPController::SPtr IPController,
             std::shared_ptr<actions::StartMove> startMove)
    : m_IPController(IPController)
    , m_startmove(startMove)
{
}

void Track::clear()
{
    m_actions.clear();
    m_modifiers.clear();
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
    if (actionIndex + 1==m_actions.size() || m_actions.empty())
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

} // namespace


