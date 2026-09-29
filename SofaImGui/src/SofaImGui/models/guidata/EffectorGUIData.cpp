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
            m_indices = std::make_shared<OwnedBaseData>(effectorIndices, false);
        else
            m_validState = false;

        if (auto effectorWeights = effector->findData("weight"))
            m_weights = std::make_shared<OwnedBaseData>(effectorWeights, false);
        else
            m_validState = false;

        if (auto effectorDirection = effector->findData("useDirections"))
            m_useDirections = std::make_shared<OwnedBaseData>(effectorDirection, false);
        else
            m_validState = false;

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
                m_validState = false;
            }

            if (m_validState)
            {
                sofa::core::BaseData* position = TCPTargetBaseMeca->findData("position");
                m_target = std::make_shared<OwnedBaseData>(position, false);
                sofa::core::BaseData* positionInit = position->getNewInstance();
                positionInit->copyValueFrom(position);
                m_targetInit = std::make_shared<OwnedBaseData>(positionInit, true);
            }
        }
        else
            m_validState = false;
    }
    else
        m_validState = false;
}

sofa::Index EffectorGUIData::getEffectorIndex(const sofa::Index& index)
{
    if (!OwnedBaseData::isDataValid(m_indices))
        return 0;

    return m_indices->getData()->getValueTypeInfo()->getIntegerValue(m_indices->getData()->getValueVoidPtr(), index);
}

sofa::defaulttype::Rigid3Types::Coord EffectorGUIData::getTCPPosition()
{
    if (!isValid())
        return sofa::defaulttype::Rigid3Types::Coord();

    auto dposition = static_cast<sofa::Data<VecCoord>*>(m_data->getData());
    return dposition->getValue()[getEffectorIndex(0)];
}

sofa::defaulttype::Rigid3Types::Coord EffectorGUIData::getTCPTargetInitPosition()
{
    if (!OwnedBaseData::isDataValid(m_targetInit))
        return sofa::defaulttype::Rigid3Types::Coord();

    auto dposition = static_cast<sofa::Data<VecCoord>*>(m_targetInit->getData());
    return dposition->getValue()[getEffectorIndex(0)];
}

sofa::defaulttype::Rigid3Types::Coord EffectorGUIData::getTCPTargetPosition()
{
    if (!OwnedBaseData::isDataValid(m_target))
        return sofa::defaulttype::Rigid3Types::Coord();

    auto dposition = static_cast<sofa::Data<VecCoord>*>(m_target->getData());
    return dposition->getValue()[getEffectorIndex(0)];
}

void EffectorGUIData::getTCPTargetPosition(double &x, double &y, double &z, double &rx, double &ry, double &rz)
{
    if (OwnedBaseData::isDataValid(m_target))
    {
        auto dposition = static_cast<sofa::Data<VecCoord>*>(m_target->getData());
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
}

void EffectorGUIData::setTCPTargetPosition(const RigidCoord& position)
{
    if (OwnedBaseData::isDataValid(m_target))
    {
        sofa::Data<VecCoord> vposition(sofa::type::vector<RigidCoord>(1, position));
        m_target->getData()->copyValueFrom(vposition.getData());
    }
}

void EffectorGUIData::setTCPTargetPosition(const double &x, const double &y, const double &z, const double &rx, const double &ry, const double &rz)
{
    if (OwnedBaseData::isDataValid(m_target))
    {
        sofa::type::Vec3 rotation(rx, ry, rz);
        sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>::createQuaterFromEuler(rotation);
        RigidCoord position(sofa::type::Vec3(x, y, z), q);
        sofa::Data<VecCoord> vposition(sofa::type::vector<RigidCoord>(1, position));
        m_target->getData()->copyValueFrom(vposition.getData());
    }
}

bool EffectorGUIData::hasRotation()
{
    return OwnedBaseData::isDataValid(m_useDirections) && m_useDirections->getData()->getValueTypeInfo()->size()==RigidDeriv::total_size;
}

void EffectorGUIData::setFreeInRotation(const bool &freeRoll, const bool &freePitch, const bool &freeYaw)
{
    if(hasRotation())
    {
        auto duseDirections = static_cast<sofa::Data<sofa::type::Vec<RigidDeriv::total_size, bool>>*>(m_useDirections->getData());
        auto d = sofa::helper::getWriteAccessor(*duseDirections);
        d[3] = !freeRoll;
        d[4] = !freePitch;
        d[5] = !freeYaw;
    }
}

double EffectorGUIData::getWeight(const sofa::Index& index)
{
    return OwnedBaseData::isDataValid(m_weights) && m_weights->getData()->getValueTypeInfo()->getScalarValue(m_weights->getData()->getValueVoidPtr(), index);
}

void EffectorGUIData::setWeight(const sofa::Index& index, const double& w)
{
    if (OwnedBaseData::isDataValid(m_weights))
    {
        m_weights->getData()->getValueTypeInfo()->setScalarValue(m_weights->getData()->beginEditVoidPtr(), index, w);
        m_weights->getData()->endEditVoidPtr();
    }
}

}
