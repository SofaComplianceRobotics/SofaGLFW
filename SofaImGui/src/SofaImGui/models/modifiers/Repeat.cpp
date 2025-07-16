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

#include <SofaImGui/models/modifiers/Repeat.h>
#include <SofaImGui/models/Track.h>

namespace sofaimgui::models::modifiers {

Repeat::Repeat(const int &iterations,
               const double &endTime,
               const double &startTime,
               const Type &type):  Modifier(endTime - startTime),
                                   m_iterations(iterations),
                                   m_endTime(endTime),
                                   m_startTime(startTime),
                                   m_type(type),
                                   view(*this)
{
    setComment("Repeat");
    checkInterval();
    m_duration = m_endTime - m_startTime;
    m_counts = m_iterations;
}

void Repeat::pushToTrack(std::shared_ptr<models::Track> track)
{
    const auto& actions = track->getActions();
    m_endTime = 0.;
    for (const auto& action: actions)
        m_endTime += action->getDuration();
    Modifier::pushToTrack(track);
}

void Repeat::insertInTrack(std::shared_ptr<models::Track> track, const sofa::Index &modifierIndex)
{
    auto& modifiers = track->getModifiers();
    m_endTime = 0.;
    for (sofa::Index i=0; i<modifierIndex; i++)
        m_endTime += modifiers[i]->getDuration();
    Modifier::insertInTrack(track, modifierIndex);
}

void Repeat::modify(double &time)
{
    if (time + 1e-5 > m_endTime && m_counts > 0)
    {
        time = m_startTime;
        m_counts--;
    }
}

void Repeat::reset()
{
    m_counts = m_iterations;
}

void Repeat::computeDuration()
{
    m_duration = m_endTime - m_startTime;
}

void Repeat::setIterations(const int &iterations)
{
    if (iterations < 0)
        m_iterations = 0;
    else
        m_iterations = iterations;
    checkCounts();
}

void Repeat::setCounts(const int &counts)
{
    if (counts < 0)
        m_counts = 0;
    else
        m_counts = counts;
    checkCounts();
}

void Repeat::setStartTime(const double &startTime)
{
    if (startTime < 0)
        m_startTime = 0;
    else
        m_startTime = startTime;
    checkInterval();
    computeDuration();
}

void Repeat::setEndTime(const double &endTime)
{
    if (endTime < 0)
        m_endTime = 0;
    else
        m_endTime = endTime;
    checkInterval();
    computeDuration();
}

void Repeat::setInterval(const double &startTime, const double &endTime)
{
    m_startTime = startTime;
    m_endTime = endTime;
    checkInterval();
    computeDuration();
}

void Repeat::checkInterval()
{
    if (m_endTime <= m_startTime)
        m_startTime = 0;
}

void Repeat::checkCounts()
{
    if (m_counts > m_iterations)
        m_counts = m_iterations;
}


} // namespace


