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

#include <sofa/defaulttype/RigidTypes.h>

#include <SofaImGui/models/BaseBlock.h>
#include <SofaImGui/config.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace sofaimgui::models {
    class Track;
}

namespace sofaimgui::models::actions {

class Action: public std::enable_shared_from_this< Action >, public BaseBlock
{
    typedef sofa::defaulttype::RigidCoord<3, double> RigidCoord;

   public:

    typedef std::shared_ptr<Action> SPtr;

    using BaseBlock::m_duration;

    Action(const double& duration=DEFAULTDURATION):
           BaseBlock(duration)
    {
       checkDuration();
    }

    virtual ~Action() = default;
    
    virtual bool apply(RigidCoord &/*position*/, const double &/*time*/){return false;}
    virtual void computeDuration(){}
    virtual void computeSpeed(){}

    void setDuration(const double& duration) override
    {
        m_duration = duration;
        checkDuration();
        computeSpeed();
    }

    const double& getSpeed() {return m_speed;}
    virtual void setSpeed(const double& speed)
    {
        m_speed = speed;
        computeDuration();
    }

    void pushToTrack(std::shared_ptr<models::Track> track) override;
    void insertInTrack(std::shared_ptr<models::Track> track, const sofa::Index &actionIndex) override;
    void deleteFromTrack(std::shared_ptr<models::Track> track, const sofa::Index &actionIndex) override;
    void swapWith(models::BaseBlock::SPtr action) override;

   protected:

    double m_minDuration{0.2};
    double m_speed;

    void checkDuration()
    {
        if (m_duration < m_minDuration)
            m_duration = m_minDuration;
    }

    class ActionView: public BaseBlockView
    {
    };
    ActionView view;

   public :

    ActionView* getView() override {return &view;}
};

} // namespace


