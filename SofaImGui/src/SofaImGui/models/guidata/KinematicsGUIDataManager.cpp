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

#include <SofaImGui/models/guidata/KinematicsGUIDataManager.h>

namespace sofaimgui::models::guidata {

void KinematicsGUIDataManager::clear()
{
    m_inverseProblemSolver = nullptr;
    m_effectorsGUIData.clear();
    m_actuatorsGUIData.clear();
}

void KinematicsGUIDataManager::setInverseProblemSolver(softrobotsinverse::solver::QPInverseProblemSolver::SPtr solver)
{
    m_inverseProblemSolver = solver;
}

void KinematicsGUIDataManager::addTCP(const std::string &label,
                                      softrobots::behavior::SoftRobotsBaseConstraint::SPtr effector,
                                      const std::pair<sofa::core::BaseData*, bool>& min,
                                      const std::pair<sofa::core::BaseData*, bool>& max,
                                      const std::string& group,
                                      const std::string& help,
                                      const double& minRotation,
                                      const double& maxRotation)
{
    if (effector)
    {
        auto context = effector->getContext();

        if (context)
        {
            sofa::core::behavior::BaseMechanicalState* TCPMeca = context->getMechanicalState();

            if (TCPMeca)
            {
                EffectorGUIData::SPtr guiDataPtr = std::make_shared<EffectorGUIData>(std::make_shared<OwnedBaseData>(TCPMeca->findData("position"), false),
                                                                                     std::make_shared<OwnedBaseData>(min.first, min.second),
                                                                                     std::make_shared<OwnedBaseData>(max.first, max.second),
                                                                                     label,
                                                                                     group,
                                                                                     help,
                                                                                     effector,
                                                                                     minRotation,
                                                                                     maxRotation);
                if (guiDataPtr && guiDataPtr->validState)
                    m_effectorsGUIData[KinematicsSection::TCP].push_back(guiDataPtr);
                else
                    msg_error("addTCP") << "Something went wrong. Expects a valid PositionEffector component as the second argument.";
            }
            else
                msg_error("addTCP") << "No MechanicalObject found in the context.";
        }
        else
            msg_error("addTCP") << "The PositionEffector has no context.";
    }
    else
        msg_error("addTCP") << "Expects a PositionEffector component as the second argument.";
}

void KinematicsGUIDataManager::addActuator(const std::string &label,
                                           softrobots::behavior::SoftRobotsBaseConstraint::SPtr actuator,
                                           const std::pair<sofa::core::BaseData*, bool>& min,
                                           const std::pair<sofa::core::BaseData*, bool>& max,
                                           const std::string& group,
                                           const std::string& help)
{
    if (actuator)
    {
        auto guiDataPtr = std::make_shared<ActuatorGUIData>(std::make_shared<OwnedBaseData>(actuator->getDelta().getData(), false),
                                                            std::make_shared<OwnedBaseData>(min.first, min.second),
                                                            std::make_shared<OwnedBaseData>(max.first, max.second),
                                                            label,
                                                            group,
                                                            help,
                                                            std::make_shared<OwnedBaseData>(actuator->d_constraintIndex.getData(), false),
                                                            actuator->getNbLines(),
                                                            1);

        if (guiDataPtr && guiDataPtr->validState)
        {
            m_actuatorsGUIData[KinematicsSection::ACTUATOR].push_back(guiDataPtr);
        }
        else
            msg_error("addActuator") << "Something went wrong. Expects a valid Actuator component as the second parameter.";
    }
    else
        msg_error("addActuator") << "Expects an Actuator component as the second parameter.";
}

void KinematicsGUIDataManager::addAccessoryComponent(const std::string &accessoryLabel,
                                                     const std::string &componentLabel,
                                                     softrobots::behavior::SoftRobotsBaseConstraint::SPtr constraint,
                                                     const std::pair<sofa::core::BaseData*, bool>& min,
                                                     const std::pair<sofa::core::BaseData*, bool>& max,
                                                     const std::string& group,
                                                     const std::string& help)
{
    // if (constraint)
    // {
    //     if (constraint->m_constraintType == softrobots::behavior::SoftRobotsBaseConstraint::ACTUATOR)
    //     {
    //         auto added = addData(accessoryLabel, data, min, max, group, help);
    //         m_actuatorsGUIData[KinematicsSection::ACCESSORY].insert(added);
    //     }
    //     else if (constraint->m_constraintType == softrobots::behavior::SoftRobotsBaseConstraint::EFFECTOR)
    //     {
    //         auto added = addData(accessoryLabel, data, min, max, group, help);
    //         m_effectorsGUIData[KinematicsSection::ACCESSORY].insert(added);
    //     }
    // }
}

bool KinematicsGUIDataManager::hasInverseProblemSolver()
{
    if (m_inverseProblemSolver == nullptr)
        return false;

    return m_inverseProblemSolver->getContext() != sofa::core::objectmodel::BaseContext::getDefault();
}

bool KinematicsGUIDataManager::hasTCP()
{
    return m_effectorsGUIData.contains(KinematicsSection::TCP);
}

bool KinematicsGUIDataManager::hasInverseProblemSolverAndTCP()
{
    return hasInverseProblemSolver() && hasTCP();
}

bool KinematicsGUIDataManager::hasActuator()
{
    return m_actuatorsGUIData.contains(KinematicsSection::ACTUATOR);
}

bool KinematicsGUIDataManager::hasAccessory()
{
    return m_effectorsGUIData.contains(KinematicsSection::ACCESSORY) || m_actuatorsGUIData.contains(KinematicsSection::ACCESSORY);
}

EffectorGUIData::SPtr KinematicsGUIDataManager::getTCPGUIData(const sofa::Index& index)
{
    if (!hasTCP())
        return nullptr;

    auto TCPsGUIData = m_effectorsGUIData[KinematicsSection::TCP];
    if (TCPsGUIData.size() > index)
        return m_effectorsGUIData[KinematicsSection::TCP][index];

    return nullptr;
}

}
