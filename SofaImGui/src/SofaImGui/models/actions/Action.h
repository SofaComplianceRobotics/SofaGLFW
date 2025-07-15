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
#include <SofaImGui/config.h>
#include <imgui.h>
#include <string>

namespace sofaimgui::models {
    class Track;
}

namespace sofaimgui::models::actions {

class Action: public std::enable_shared_from_this< Action >
{
    typedef sofa::defaulttype::RigidCoord<3, double> RigidCoord;

   public:

    inline static const int COMMENTSIZE = 18;
    inline static const double DEFAULTDURATION = 1.;

    Action(const double& duration=DEFAULTDURATION):
                                  m_duration(duration)
    {
       checkDuration();
    }

    virtual ~Action() = default;
    virtual std::shared_ptr<Action> duplicate() = 0;
    
    virtual bool apply(RigidCoord &/*position*/, const double &/*time*/){return false;}
    virtual void computeDuration(){}
    virtual void computeSpeed(){}

    const double& getDuration() {return m_duration;}
    virtual void setDuration(const double& duration)
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

    virtual void pushToTrack(std::shared_ptr<models::Track> track);
    virtual void insertInTrack(std::shared_ptr<models::Track> track, const sofa::Index &actionIndex);
    virtual void deleteFromTrack(std::shared_ptr<models::Track> track, const sofa::Index &actionIndex);

    void setComment(const char* comment) {strncpy(m_comment, comment, COMMENTSIZE); m_comment[COMMENTSIZE-1]='\0';}
    void getComment(char* comment) {strncpy(comment, m_comment, COMMENTSIZE); comment[COMMENTSIZE-1]='\0';}

    char* getComment() {return m_comment;}

   protected:

    double m_duration;
    double m_minDuration{0.2};
    double m_speed;
    char m_comment[COMMENTSIZE];

    void checkDuration()
    {
        if (m_duration < m_minDuration)
            m_duration = m_minDuration;
    }

    class ActionView
    {
       public:
        virtual bool showBlock(const std::string &,
                               const ImVec2 &) {return false;}
    };
    ActionView view;

   public :

    virtual ActionView* getView() {return &view;}
};

} // namespace


