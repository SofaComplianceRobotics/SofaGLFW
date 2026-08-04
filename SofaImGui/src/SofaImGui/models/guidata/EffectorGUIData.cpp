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


#include <SofaImGui/models/guidata/EffectorGUIData.h>


namespace sofaimgui::models::guidata
{

void EffectorGUIData::initFromEffector(softrobots::behavior::SoftRobotsBaseConstraint::SPtr effector)
{
    if (effector)
    {
        sofa::core::behavior::BaseMechanicalState::SPtr TCPTargetBaseMeca = nullptr;

        if (auto effectorIndices = effector->findData("indices"))
            indices = std::make_shared<OwnedBaseData>(effectorIndices, false);
        else
            validState = false;

        if (auto effectorWeights = effector->findData("weight"))
            weights = std::make_shared<OwnedBaseData>(effectorWeights, false);
        else
            validState = false;

        if (auto effectorDirection = effector->findData("useDirections"))
            useDirections = std::make_shared<OwnedBaseData>(effectorDirection, false);
        else
            validState = false;

        auto effectorGoal = effector->findData("effectorGoal");
        if (effectorGoal)
        {
            if (auto parent = effectorGoal->getParent())
            {
                TCPTargetBaseMeca = dynamic_cast<sofa::core::behavior::BaseMechanicalState*>(parent->getOwner());
            }
            else
            {
                msg_error("GUI") << "Effector should have an effectorGoal";
                validState = false;
            }

            if (validState)
            {
                sofa::core::BaseData* position = TCPTargetBaseMeca->findData("position");
                target = std::make_shared<OwnedBaseData>(position, false);
                sofa::core::BaseData* positionInit = position->getNewInstance();
                positionInit->copyValueFrom(position);
                targetInit = std::make_shared<OwnedBaseData>(positionInit, true);
            }
        }
        else
            validState = false;
    }
    else
        validState = false;
}

sofa::Index EffectorGUIData::getEffectorIndex(const sofa::Index& index)
{
    return indices->getData()->getValueTypeInfo()->getIntegerValue(indices->getData()->getValueVoidPtr(), index);
}

sofa::defaulttype::Rigid3Types::Coord EffectorGUIData::getTCPPosition()
{
    auto dposition = static_cast<sofa::Data<VecCoord>*>(data->getData());
    return dposition->getValue()[getEffectorIndex(0)];
}

sofa::defaulttype::Rigid3Types::Coord EffectorGUIData::getTCPTargetInitPosition()
{
    auto dposition = static_cast<sofa::Data<VecCoord>*>(targetInit->getData());
    return dposition->getValue()[getEffectorIndex(0)];
}

sofa::defaulttype::Rigid3Types::Coord EffectorGUIData::getTCPTargetPosition()
{
    auto dposition = static_cast<sofa::Data<VecCoord>*>(target->getData());
    return dposition->getValue()[getEffectorIndex(0)];
}

void EffectorGUIData::getTCPTargetPosition(double &x, double &y, double &z, double &rx, double &ry, double &rz)
{
    auto dposition = static_cast<sofa::Data<VecCoord>*>(target->getData());
    RigidCoord position = sofa::helper::getReadAccessor(*dposition)[getEffectorIndex(0)];
    x = position[0];
    y = position[1];
    z = position[2];

    sofa::type::Quat<SReal> q(position[3], position[4], position[5], position[6]);
    sofa::type::Vec3 rotation = q.toEulerVector();
    rx = rotation[0];
    ry = rotation[1];
    rz = rotation[2];
}

void EffectorGUIData::setTCPTargetPosition(const RigidCoord& position)
{
    sofa::Data<VecCoord> vposition(sofa::type::vector<RigidCoord>(1, position));
    target->getData()->copyValueFrom(vposition.getData());
}

void EffectorGUIData::setTCPTargetPosition(const double &x, const double &y, const double &z, const double &rx, const double &ry, const double &rz)
{
    sofa::type::Vec3 rotation(rx, ry, rz);
    sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>::createQuaterFromEuler(rotation);
    RigidCoord position(sofa::type::Vec3(x, y, z), q);
    sofa::Data<VecCoord> vposition(sofa::type::vector<RigidCoord>(1, position));
    target->getData()->copyValueFrom(vposition.getData());
}

bool EffectorGUIData::hasRotation()
{
    return useDirections->getData()->getValueTypeInfo()->size()==RigidDeriv::total_size;
}

void EffectorGUIData::setFreeInRotation(const bool &freeRoll, const bool &freePitch, const bool &freeYaw)
{
    if(hasRotation())
    {
        auto duseDirections = static_cast<sofa::Data<sofa::type::Vec<RigidDeriv::total_size, bool>>*>(useDirections->getData());
        auto d = sofa::helper::getWriteAccessor(*duseDirections);
        d[3] = !freeRoll;
        d[4] = !freePitch;
        d[5] = !freeYaw;
    }
}

double EffectorGUIData::getWeight(const sofa::Index& index)
{
    return weights->getData()->getValueTypeInfo()->getScalarValue(weights->getData()->getValueVoidPtr(), index);
}

void EffectorGUIData::setWeight(const sofa::Index& index, const double& w)
{
    weights->getData()->getValueTypeInfo()->setScalarValue(weights->getData()->beginEditVoidPtr(), index, w);
    weights->getData()->endEditVoidPtr();
}

}
