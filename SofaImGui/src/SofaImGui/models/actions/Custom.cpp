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

#include "IconsFontAwesome6.h"
#include <SofaImGui/models/actions/Custom.h>
#include <SofaImGui/FooterStatusBar.h>

namespace sofaimgui::models::actions {

Custom::Custom(const double &duration): Action(duration),
    view(*this)
{
    setComment("Custom");
    computeSpeed();
}

void Custom::computeSpeed()
{
    m_speed = fabs(m_startValue - m_endValue) / m_duration;
}

void Custom::computeDuration()
{
    m_duration = fabs(m_startValue - m_endValue) / m_speed;
    checkDuration();
}

std::shared_ptr<Action> Custom::duplicate()
{
    auto custom = std::make_shared<models::actions::Custom>(m_duration);
    return custom;
}

bool Custom::apply(RigidCoord &position, const double &time)
{
    SOFA_UNUSED(position);
    SOFA_UNUSED(time);

    if(m_data && m_data->isValid())
    {
        auto d = m_data->getData();
        double alpha = time / m_duration;
        double value = (1-alpha) * m_startValue + alpha * m_endValue;
        d->getValueTypeInfo()->setScalarValue(d->beginEditVoidPtr(), 0, value);
        d->endEditVoidPtr();
    }

    return false;
}

bool Custom::setData(const std::string& dataPath, sofa::simulation::Node::SPtr groot)
{
    if (groot && !dataPath.empty())
    {
        sofa::core::BaseData* data;
        if (groot->findDataLinkDest(data, "@" + dataPath, nullptr))
        {
            m_data = std::make_shared<guidata::GUIData>(std::make_shared<guidata::OwnedBaseData>(data, false),
                                                        std::make_shared<guidata::OwnedBaseData>(nullptr, false),
                                                        std::make_shared<guidata::OwnedBaseData>(nullptr, false),
                                                        data->getName(),
                                                        guidata::GUIData::DEFAULTGROUP,
                                                        "");
            return true;
        }
        else
        {
            FooterStatusBar::getInstance().setTempMessage("Data path " + dataPath + " does not exist.", FooterStatusBar::MessageType::MWARNING);
        }
    }

    return false;
}

} // namespace
