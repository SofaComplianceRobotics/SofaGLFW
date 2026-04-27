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

#include <SofaImGui/models/guidata/GUIDataManager.h>
#include <SoftRobots.Inverse/component/constraint/PositionEffector.h>
#include <sofa/defaulttype/RigidCoord.h>


namespace sofaimgui::models::guidata
{

class EffectorGUIData: public GUIData
{
    // Todo: use template to allow different type of effector
    typedef sofa::defaulttype::Rigid3Types::Coord RigidCoord;
    typedef sofa::defaulttype::Rigid3Types::Deriv RigidDeriv;
    typedef sofa::defaulttype::Rigid3Types::VecCoord VecCoord;
    typedef sofa::defaulttype::Rigid3Types::VecDeriv VecDeriv;

public:
    typedef std::shared_ptr<EffectorGUIData> SPtr;

    EffectorGUIData(OwnedBaseData::SPtr data,
                    OwnedBaseData::SPtr min,
                    OwnedBaseData::SPtr max,
                    std::string label,
                    std::string group,
                    std::string help,
                    softrobots::behavior::SoftRobotsBaseConstraint::SPtr effector)
        : GUIData(data, min, max, label, group, help)
    {
        initFromEffector(effector);
    }

    RigidCoord getTCPTargetInitPosition();

    RigidCoord getTCPTargetPosition();
    void getTCPTargetPosition(double &x, double &y, double &z, double &rx, double &ry, double &rz);

    void setTCPTargetPosition(const RigidCoord& position);
    void setTCPTargetPosition(const double &x, const double &y, const double &z, const double &rx, const double &ry, const double &rz);

    RigidCoord getTCPPosition();

    bool hasRotation() {return useDirections->getData()->getValueTypeInfo()->size()==RigidDeriv::total_size;}
    void setFreeInRotation(const bool &freeRoll, const bool &freePitch, const bool &freeYaw);

    double getWeight(const sofa::Index &index);
    void setWeight(const sofa::Index &index, const double &w);

protected:

    void initFromEffector(softrobots::behavior::SoftRobotsBaseConstraint::SPtr effector);

    sofa::Data<sofa::type::vector<unsigned int>> indices;
    OwnedBaseData::SPtr target{nullptr};
    OwnedBaseData::SPtr targetInit{nullptr};
    OwnedBaseData::SPtr weights{nullptr};
    OwnedBaseData::SPtr useDirections{nullptr};
};

}


