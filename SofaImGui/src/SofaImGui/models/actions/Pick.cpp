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

#include <SofaImGui/models/actions/Pick.h>

namespace sofaimgui::models::actions {

bool Pick::gripperInstalled = false;
double Pick::minClosingDistance = 0;
double Pick::maxOpeningDistance = 0;
sofa::core::BaseData* Pick::distance = nullptr;


Pick::Pick(const double &duration, const bool& release, const double &closingDistance, const double &openingDistance)
    : Action(duration)
    , m_release(release)
    , m_closingDistance(closingDistance)
    , m_openingDistance(openingDistance)
    , view(*this)
{
    setComment("Pick");
}

std::shared_ptr<Action> Pick::duplicate()
{
    auto pick = std::make_shared<models::actions::Pick>(m_duration,
                                                        m_release,
                                                        m_closingDistance,
                                                        m_openingDistance);
    return pick;
}


void Pick::setDuration(const double& duration)
{
    m_duration = duration;
    checkDuration();
}

bool Pick::apply(RigidCoord &position, const double &time)
{
    SOFA_UNUSED(position);
    SOFA_UNUSED(time);

    if(gripperInstalled && distance)
    {
        double alpha = time / m_duration;
        if (m_release)
        {
            double dist = alpha * m_openingDistance + (1 - alpha) * m_closingDistance;
            distance->read(std::to_string(dist));
        }
        else
        {
            double dist = alpha * m_closingDistance + (1 - alpha) * m_openingDistance;
            distance->read(std::to_string(dist));
        }
    }

    return false;
}

} // namespace


