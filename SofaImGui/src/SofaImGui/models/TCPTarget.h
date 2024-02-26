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

#include <sofa/type/Vec.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <SofaGLFW/SofaGLFWBaseGUI.h>

namespace sofaimgui::models {

class TCPTarget
{
   typedef sofa::defaulttype::RigidCoord<3, float> RigidCoord;

   public:

    enum TCPTargetType {
        LINEAR
    };

    TCPTarget(sofa::simulation::Node* groot);
    ~TCPTarget() = default;

    void init(sofa::simulation::Node* groot);

    const RigidCoord& getInitPosition();

    RigidCoord getPosition();
    void getPosition(int &x, int &y, int &z, float &rx, float &ry, float &rz);

    void setPosition(const RigidCoord& position);
    void setPosition(const int &x, const int &y, const int &z, const float &rx, const float &ry, const float &rz);


   protected:

    sofa::core::behavior::BaseMechanicalState* m_state;
    sofa::simulation::Node* m_groot;
    RigidCoord m_initPosition;
};

} // namespace

