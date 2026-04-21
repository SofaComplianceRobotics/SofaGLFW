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

#include <SofaImGui/models/KinematicsController.h>
#include <sofa/core/behavior/BaseMechanicalState.h>
#include <sofa/simulation/events/SolveConstraintSystemEndEvent.h>
#include <sofa/simulation/AnimateEndEvent.h>
#include <sofa/core/ObjectFactory.h>

namespace sofaimgui::models {

KinematicsController::KinematicsController()
{
}

void KinematicsController::applyActuatorsForce()
{
    m_updateSolutionOnSolveEndEvent = true;
}

void KinematicsController::handleEvent(sofa::core::objectmodel::Event *event)
{
    // if (sofa::simulation::SolveConstraintSystemEndEvent::checkEventType(event) && m_updateSolutionOnSolveEndEvent)
    // {
    //     const auto& problem = m_inverseProblemSolver->getConstraintProblem();
    //     softrobotsinverse::solver::module::QPInverseProblem* inverseProblem = dynamic_cast<softrobotsinverse::solver::module::QPInverseProblem*>(problem);

    //     if (inverseProblem)
    //     {
    //         auto& lambda = problem->f;
    //         auto& w = problem->W;
    //         auto& dfree = problem->dFree;

    //         softrobotsinverse::solver::module::QPInverseProblem::QPConstraintLists* qpCLists = inverseProblem->getQPConstraintLists();
    //         softrobotsinverse::solver::module::QPInverseProblemImpl::QPSystem* qpSystem = inverseProblem->getQPSystem();

    //         const size_t nbActuatorRows = qpCLists->actuatorRowIds.size();
    //         const size_t nbEffectorRows = qpCLists->effectorRowIds.size();
    //         const size_t nbSensorRows   = qpCLists->sensorRowIds.size();
    //         const size_t nbContactRows  = qpCLists->contactRowIds.size();
    //         const size_t nbEqualityRows = qpCLists->equalityRowIds.size();
    //         const size_t nbRows = nbEffectorRows + nbActuatorRows + nbContactRows + nbSensorRows + nbEqualityRows;
    //         qpSystem->delta.resize(nbRows);

    //         if (m_actuators[0].valueType.getSelectedId() == 0)
    //         {
    //             for (const auto& actuator: m_actuators)
    //                 lambda[actuator.indexInProblem] = actuator.value;
    //         }
    //         else
    //         {
    //             // TODO solve the direct kinematics. And handle contacts.
    //             // Add a direct solver to the QPInverseProblemSolver
    //             std::vector<double> d(m_actuators.size());
    //             for (const auto& a1: m_actuators)
    //                 lambda[a1.indexInProblem] = 0;

    //             for (size_t i=0; i<10; i++)
    //             {
    //                 int j=0;
    //                 for (const auto& a1: m_actuators)
    //                 {
    //                     d[j] = dfree[a1.indexInProblem];
    //                     for(const auto& a2: m_actuators)
    //                     {
    //                         d[j] += w[a1.indexInProblem][a2.indexInProblem] * lambda[a2.indexInProblem];
    //                     }
    //                     lambda[a1.indexInProblem] -= (d[j]-a1.value) / w[a1.indexInProblem][a1.indexInProblem];
    //                     j++;
    //                 }
    //             }
    //         }

    //         for(size_t i=0; i<nbRows; i++)
    //         {
    //             qpSystem->delta[i] = dfree[i];
    //             for(size_t j=0; j<nbRows; j++)
    //                 qpSystem->delta[i] += lambda[j]*w[i][j];
    //         }

    //         inverseProblem->sendResults();
    //     }
    // }

    // if (sofa::simulation::AnimateEndEvent::checkEventType(event) && m_updateSolutionOnSolveEndEvent)
    // {
    //     setTCPTargetPosition(getTCPPosition());
    //     m_updateSolutionOnSolveEndEvent = false;
    // }
}

} // namespace


