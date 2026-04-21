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

#include <SofaImGui/models/guidata/GUIDataManager.h>
#include <SofaImGui/models/guidata/EffectorGUIData.h>
#include <SofaImGui/models/guidata/ActuatorGUIData.h>
#include <SoftRobots.Inverse/component/solver/QPInverseProblemSolver.h>

namespace sofaimgui::models::guidata {

class SOFAIMGUI_API KinematicsGUIDataManager: GUIDataManager
{
public:

    enum KinematicsSection {
        TCP,
        ACTUATOR,
        ACCESSORY
    };

    KinematicsGUIDataManager(){}
    ~KinematicsGUIDataManager() = default;

    void clear();

    void setInverseProblemSolver(softrobotsinverse::solver::QPInverseProblemSolver::SPtr solver);

    void addTCP(softrobots::behavior::SoftRobotsBaseConstraint::SPtr effector,
                const std::pair<sofa::core::BaseData*, bool>& min,
                const std::pair<sofa::core::BaseData*, bool>& max,
                const std::string& group,
                const std::string& help);
    void addActuator(const std::string &label, softrobots::behavior::SoftRobotsBaseConstraint::SPtr actuator,
                     const std::pair<sofa::core::BaseData*, bool>& min,
                     const std::pair<sofa::core::BaseData*, bool>& max,
                     const std::string& group,
                     const std::string& help);
    void addAccessoryComponent(const std::string &accessoryLabel,
                               const std::string &componentLabel,
                               softrobots::behavior::SoftRobotsBaseConstraint::SPtr constraint,
                               const std::pair<sofa::core::BaseData*, bool>& min,
                               const std::pair<sofa::core::BaseData*, bool>& max,
                               const std::string& group,
                               const std::string& help);

    bool hasInverseProblemSolver(){return m_inverseProblemSolver == nullptr;}
    bool hasTCP(){return m_effectorsGUIData.contains(KinematicsSection::TCP);}
    bool hasInverseProblemSolverAndTCP(){return hasInverseProblemSolver() && hasTCP();}
    bool hasActuator(){return m_actuatorsGUIData.contains(KinematicsSection::ACTUATOR);}
    bool hasAccessoryComponent(){return m_effectorsGUIData.contains(KinematicsSection::ACCESSORY) || m_actuatorsGUIData.contains(KinematicsSection::ACCESSORY);}

    EffectorGUIData::SPtr getEffectorGUIData() {return m_effectorsGUIData[KinematicsSection::TCP][0];} // Temp: for the moment we handle only one TCP

    void setBaseGUI(sofaglfw::SofaGLFWBaseGUI* baseGUI) {m_baseGUI=baseGUI;}
    sofa::simulation::Node::SPtr getRootNode() {return m_baseGUI->getRootNode();}

protected:

    sofaglfw::SofaGLFWBaseGUI* m_baseGUI;
    softrobotsinverse::solver::QPInverseProblemSolver::SPtr m_inverseProblemSolver;
    std::map<KinematicsSection, std::vector<ActuatorGUIData::SPtr>> m_actuatorsGUIData;
    std::map<KinematicsSection, std::vector<EffectorGUIData::SPtr>> m_effectorsGUIData;
};

}
