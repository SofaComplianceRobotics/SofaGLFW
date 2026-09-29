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

#include <SofaImGui/models/guidata/GUIData.h>
#include <SofaImGui/models/actions/Action.h>
#include <SofaImGui/config.h>


namespace sofaimgui::models::actions {

class Custom : public Action
{
    typedef sofa::defaulttype::RigidCoord<3, double> RigidCoord;

public:

    Custom(const double& duration = Action::DEFAULTDURATION);
    ~Custom() = default;

    std::shared_ptr<Action> duplicate() override;
    bool apply(RigidCoord &position, const double &time) override;
    void computeDuration() override;
    void computeSpeed() override;

    guidata::GUIData::SPtr getData() {return m_data;}
    bool setData(const std::string& dataPath, sofa::simulation::Node::SPtr groot);

    double getStartValue() {return m_startValue;}
    void setStartValue(const double& startValue) {m_startValue=startValue;}

    double getEndValue() {return m_endValue;}
    void setEndValue(const double& endValue) {m_endValue=endValue;}

protected:

    guidata::GUIData::SPtr m_data{nullptr};
    double m_startValue{0.f};
    double m_endValue{1.f};

    class CustomView : public ActionView
    {
    public:
        CustomView(Custom &_custom) : custom(_custom) {}
        bool showBlock(const std::string &label,
                       const ImVec2 &size) override;

    protected:
        Custom &custom;
    };
    CustomView view;

public :

    ActionView* getView() override {return &view;}
};

} // namespace
