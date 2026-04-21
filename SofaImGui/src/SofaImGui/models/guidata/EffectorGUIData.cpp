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
    validState = false;

    if (effector)
    {
        sofa::core::behavior::BaseMechanicalState* TCPTargetMeca = nullptr;

        if (auto effectorIndices = effector->findData("indices"))
            indices.getData()->copyValueFrom(effectorIndices->getData());
        else
            return;

        if (auto effectorWeights = effector->findData("weight"))
            weights = std::make_shared<OwnedBaseData>(effectorWeights, true);
        else
            return;

        if (auto effectorDirection = effector->findData("useDirections"))
            useDirections = std::make_shared<OwnedBaseData>(effectorDirection, true);
        else
            return;

        auto effectorGoal = effector->findData("effectorGoal");
        if (effectorGoal)
        {
            if (auto parent = effectorGoal->getParent())
                TCPTargetMeca = dynamic_cast<sofa::core::behavior::BaseMechanicalState*>(parent->getOwner());
            else
            {
                if (auto context = effector->getContext())
                {
                    if (auto groot = dynamic_cast<Node*>(context->getRootContext()))
                    {
                        if (Node::SPtr guiNode = groot->getChild(sofaglfw::SofaGLFWBaseGUI::getGUINodeName()))
                        {
                            Node::SPtr targetNode = sofa::core::objectmodel::New<Node>(label);
                            guiNode->addChild(targetNode);
                            int templateSize = effectorGoal->getValueTypeInfo()->size();

                            if (templateSize == sofa::defaulttype::Rigid3Types::coord_total_size)
                                TCPTargetMeca = sofa::core::objectmodel::New<sofa::component::statecontainer::MechanicalObject<sofa::defaulttype::Rigid3Types>>().get();
                            // else if (templateSize == sofa::defaulttype::Vec3Types::coord_total_size)
                            //     TCPTargetMeca = sofa::core::objectmodel::New<sofa::component::statecontainer::MechanicalObject<sofa::defaulttype::Vec3Types>>().get();
                            // else if (templateSize == sofa::defaulttype::Vec2Types::coord_total_size)
                            //     TCPTargetMeca = sofa::core::objectmodel::New<sofa::component::statecontainer::MechanicalObject<sofa::defaulttype::Vec2Types>>().get();
                            // else if (templateSize == sofa::defaulttype::Vec1Types::coord_total_size)
                            //     TCPTargetMeca = sofa::core::objectmodel::New<sofa::component::statecontainer::MechanicalObject<sofa::defaulttype::Vec1Types>>().get();

                            if (TCPTargetMeca)
                            {
                                targetNode->addObject(TCPTargetMeca);

                                auto position = TCPTargetMeca->findData("position");
                                if (auto effectorMechanical = context->getMechanicalState())
                                    position->copyValueFrom(effectorMechanical->findData("position"));
                                effectorGoal->setParent(position->getLinkPath());
                                target = std::make_shared<OwnedBaseData>(position, true);
                                sofa::core::BaseData* positionInit = position->getNewInstance();
                                positionInit->copyValueFrom(position);
                                targetInit = std::make_shared<OwnedBaseData>(positionInit, false);
                            }
                            else
                                return;
                        }
                        else
                            return;
                    }
                    else
                        return;
                }
                else
                    return;
            }
        }
        else
            return;
    }
    else
        return;

    validState = true;
}


sofa::defaulttype::RigidCoord<3, double> EffectorGUIData::getTCPPosition()
{
    sofa::Data<RigidCoord> position;
    position.getData()->copyValueFrom(data->getData());
    return position.getValue();
}

const sofa::defaulttype::RigidCoord<3, double>& EffectorGUIData::getTCPTargetInitPosition()
{
    sofa::Data<RigidCoord> dposition;
    dposition.getData()->copyValueFrom(targetInit->getData());
    return dposition.getValue();
}

sofa::defaulttype::RigidCoord<3, double> EffectorGUIData::getTCPTargetPosition()
{
    sofa::Data<RigidCoord> dposition;
    dposition.getData()->copyValueFrom(target->getData());
    return dposition.getValue();
}

void EffectorGUIData::getTCPTargetPosition(double &x, double &y, double &z, double &rx, double &ry, double &rz)
{
    sofa::Data<RigidCoord> dposition;
    dposition.getData()->copyValueFrom(target->getData());
    RigidCoord position = sofa::helper::getReadAccessor(dposition);
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
    target->getData()->copyValueFrom(sofa::Data<RigidCoord>(position).getData());
}

void EffectorGUIData::setTCPTargetPosition(const double &x, const double &y, const double &z, const double &rx, const double &ry, const double &rz)
{
    sofa::type::Vec3 rotation(rx, ry, rz);
    sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>::createQuaterFromEuler(rotation);
    RigidCoord position(sofa::type::Vec3(x, y, z), q);
    target->getData()->copyValueFrom(sofa::Data<RigidCoord>(position).getData());
}

void EffectorGUIData::setFreeInRotation(const bool &freeRoll, const bool &freePitch, const bool &freeYaw)
{
    if(hasRotation())
    {
        sofa::Data<sofa::type::Vec<RigidDeriv::total_size, bool>> duseDirections;
        duseDirections.getData()->copyValueFrom(useDirections->getData());
        auto d = sofa::helper::getWriteAccessor(duseDirections);
        d[3] = freeRoll;
        d[4] = freePitch;
        d[5] = freeYaw;
        useDirections->getData()->copyValueFrom(duseDirections.getData());
    }
}
}
