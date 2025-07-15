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

#include <SofaImGui/models/actions/Action.h>
#include <SofaImGui/models/Track.h>

namespace sofaimgui::models::actions
{

void Action::pushToTrack(std::shared_ptr<models::Track> track)
{
    auto& actions = track->getActions();
    actions.push_back(shared_from_this());
}

void Action::insertInTrack(std::shared_ptr<models::Track> track, const sofa::Index &actionIndex)
{
    auto& actions = track->getActions();
    if (actionIndex < actions.size())
        actions.insert(actions.begin() + actionIndex, shared_from_this());
    else
        pushToTrack(track);
}

void Action::deleteFromTrack(std::shared_ptr<models::Track> track, const sofa::Index &actionIndex)
{
    auto& actions = track->getActions();
    if (actionIndex < actions.size())
        actions.erase(actions.begin() + actionIndex);
    else
        dmsg_error("Track") << "actionIndex";
}

} // namespace


