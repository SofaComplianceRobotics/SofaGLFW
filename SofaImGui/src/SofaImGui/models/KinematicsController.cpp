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
#include <sofa/simulation/AnimateBeginEvent.h>
#include <sofa/simulation/AnimateEndEvent.h>
#include <sofa/core/ObjectFactory.h>

namespace sofaimgui::models {

KinematicsController::KinematicsController(models::guidata::KinematicsGUIDataManager::SPtr kinematicsGUIDataManager)
    : m_kinematicsGUIDataManager(kinematicsGUIDataManager)
{
    setName("KinematicsController");
    this->f_listening.setValue(true);
}

void KinematicsController::handleEvent(sofa::core::objectmodel::Event *event)
{
    if (m_kinematicsGUIDataManager == nullptr
        || !m_kinematicsGUIDataManager->hasInverseProblemSolverAndTCP()
        || !m_kinematicsGUIDataManager->hasActuator())
        return;

    // AnimateBeginEvent
    if (sofa::simulation::AnimateBeginEvent::checkEventType(event)
        && m_kinematicsGUIDataManager->isSolverInDirectMode())
    {
        auto solver = m_kinematicsGUIDataManager->getInverseProblemSolver();
        if (solver)
        {
            auto mode = sofa::helper::getWriteAccessor(solver->d_mode);
            mode->setSelectedItem(1);
        }
    }

    // SolveConstraintSystemEndEvent
    if (sofa::simulation::SolveConstraintSystemEndEvent::checkEventType(event)
        && m_kinematicsGUIDataManager->isSolverInDirectMode())
    {
        const auto& problem = m_kinematicsGUIDataManager->getInverseProblemSolver()->getConstraintProblem();
        softrobotsinverse::solver::module::QPInverseProblem* inverseProblem = dynamic_cast<softrobotsinverse::solver::module::QPInverseProblem*>(problem);

        if (problem && inverseProblem)
        {
            auto& lambda = problem->f;
            auto& w = problem->W;
            auto& dfree = problem->dFree;

            softrobotsinverse::solver::module::QPInverseProblem::QPConstraintLists* qpCLists = inverseProblem->getQPConstraintLists();
            softrobotsinverse::solver::module::QPInverseProblemImpl::QPSystem* qpSystem = inverseProblem->getQPSystem();

            if (qpSystem)
            {
                const size_t nbActuatorRows = qpCLists->actuatorRowIds.size();
                const size_t nbEffectorRows = qpCLists->effectorRowIds.size();
                const size_t nbSensorRows   = qpCLists->sensorRowIds.size();
                const size_t nbContactRows  = qpCLists->contactRowIds.size();
                const size_t nbEqualityRows = qpCLists->equalityRowIds.size();
                const size_t nbRows = nbEffectorRows + nbActuatorRows + nbContactRows + nbSensorRows + nbEqualityRows;
                qpSystem->delta.resize(nbRows);

                auto actuatorsGUIData = m_kinematicsGUIDataManager->getActuators();

                // TODO solve the direct kinematics. And handle contacts.
                // Add a direct solver to the QPInverseProblemSolver
                std::vector<double> d(actuatorsGUIData.size());
                for (auto actuator: actuatorsGUIData)
                {
                    if (actuator && actuator->isValid())
                        lambda[actuator->getIndexInProblem()] = 0;
                }

                for (size_t i=0; i<10; i++)
                {
                    int j=0;
                    for (auto a1: actuatorsGUIData)
                    {
                        if (a1 && a1->isValid())
                        {
                            if (a1->getValueTypeId() == 0)
                            {
                                lambda[a1->getIndexInProblem()] = a1->getValue(0);
                            }
                            else
                            {
                                d[j] = dfree[a1->getIndexInProblem()];
                                for(auto a2: actuatorsGUIData)
                                {
                                    if (a2 && a2->isValid())
                                        d[j] += w[a1->getIndexInProblem()][a2->getIndexInProblem()] * lambda[a2->getIndexInProblem()];
                                }
                                lambda[a1->getIndexInProblem()] -= (d[j]-a1->getValue(0)) / w[a1->getIndexInProblem()][a1->getIndexInProblem()];
                                j++;
                            }
                        }
                    }
                }

                for(size_t i=0; i<nbRows; i++)
                {
                    qpSystem->delta[i] = dfree[i];
                    for(size_t j=0; j<nbRows; j++)
                        qpSystem->delta[i] += lambda[j]*w[i][j];
                }

                inverseProblem->sendResults();
            }
        }
    }

    // AnimateEndEvent
    if (sofa::simulation::AnimateEndEvent::checkEventType(event)
        && m_kinematicsGUIDataManager->isSolverInDirectMode())
    {
        auto TCPGUIData = m_kinematicsGUIDataManager->getTCPGUIData();
        if (TCPGUIData)
        {
            const auto &TCPPosition = TCPGUIData->getTCPPosition();
            TCPGUIData->setTCPTargetPosition(TCPPosition);
        }
        m_kinematicsGUIDataManager->switchSolverMode();

        auto solver = m_kinematicsGUIDataManager->getInverseProblemSolver();
        if (solver)
        {
            auto mode = sofa::helper::getWriteAccessor(solver->d_mode);
            mode->setSelectedItem(0);
        }
    }
}

} // namespace


