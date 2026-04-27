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

#include <SofaImGui/config.h>
#include <sofa/type/Vec.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <sofa/component/controller/Controller.h>
#include <SofaImGui/models/guidata/KinematicsGUIDataManager.h>

namespace sofaimgui::models {

class SOFAIMGUI_API KinematicsController : public sofa::component::controller::Controller
{
   typedef sofa::defaulttype::RigidCoord<3, double> RigidCoord;

   public:

    SOFA_CLASS(KinematicsController, sofa::component::controller::Controller);

    KinematicsController();
    ~KinematicsController() = default;

    void applyActuatorsForce();
    void handleEvent(sofa::core::objectmodel::Event *event) override;

   protected:

    // models::guidata::KinematicsGUIDataManager m_kinematicsGUIDataManager;
    bool m_updateSolutionOnSolveEndEvent{false};

};

} // namespace


