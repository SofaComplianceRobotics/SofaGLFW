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
#include <SofaImGui/models/guidata/AccessoryFeatureGUIData.h>
#include <SoftRobots.Inverse/component/solver/QPInverseProblemSolver.h>

namespace sofaimgui::models::guidata {

class SOFAIMGUI_API KinematicsGUIDataManager: GUIDataManager
{
public:

    typedef std::shared_ptr<KinematicsGUIDataManager> SPtr;

    KinematicsGUIDataManager(){}
    ~KinematicsGUIDataManager() = default;

    void clear();

    void setInverseProblemSolver(softrobotsinverse::solver::QPInverseProblemSolver::SPtr solver);

    bool isSolverInDirectMode() {return m_isSolverInDirectMode;}
    void switchSolverMode() {m_isSolverInDirectMode = !m_isSolverInDirectMode;}

    void addTCP(const std::string &label, softrobots::behavior::SoftRobotsBaseConstraint::SPtr effector,
                const std::pair<sofa::core::BaseData*, bool>& min,
                const std::pair<sofa::core::BaseData*, bool>& max,
                const std::string& group,
                const std::string& help,
                const double& minRotation,
                const double& maxRotation);
    void addActuator(const std::string &label, softrobots::behavior::SoftRobotsBaseConstraint::SPtr actuator,
                     const std::pair<sofa::core::BaseData*, bool>& min,
                     const std::pair<sofa::core::BaseData*, bool>& max,
                     const std::string& group,
                     const std::string& help);
    void addAccessoryFeature(const std::string &accessoryLabel,
                             const std::string &featureLabel,
                             const std::pair<sofa::core::BaseData*, bool> &data,
                             const std::pair<sofa::core::BaseData*, bool>& min,
                             const std::pair<sofa::core::BaseData*, bool>& max);

    bool hasInverseProblemSolver();
    bool hasTCP();
    bool hasInverseProblemSolverAndTCP();
    bool hasActuator();
    bool hasAccessory();

    EffectorGUIData::SPtr getTCPGUIData(const sofa::Index& index=0); // Temp: for the moment we handle only one TCP

    softrobotsinverse::solver::QPInverseProblemSolver::SPtr getInverseProblemSolver() {return m_inverseProblemSolver;}
    const std::vector<EffectorGUIData::SPtr>& getTCPs() {return m_effectorsGUIData;}
    const std::vector<ActuatorGUIData::SPtr>& getActuators() {return m_actuatorsGUIData;}
    const std::map<std::string, std::vector<AccessoryFeatureGUIData::SPtr>>& getAccessories() {return m_accessoriesFeatureGUIData;}

    void setBaseGUI(sofaglfw::SofaGLFWBaseGUI* baseGUI) {m_baseGUI=baseGUI;}
    sofa::simulation::Node::SPtr getRootNode() {return m_baseGUI->getRootNode();}

protected:

    sofaglfw::SofaGLFWBaseGUI* m_baseGUI{nullptr};
    softrobotsinverse::solver::QPInverseProblemSolver::SPtr m_inverseProblemSolver{nullptr};
    std::vector<ActuatorGUIData::SPtr> m_actuatorsGUIData;
    std::vector<EffectorGUIData::SPtr> m_effectorsGUIData;
    std::map<std::string, std::vector<AccessoryFeatureGUIData::SPtr>> m_accessoriesFeatureGUIData;

    bool m_isSolverInDirectMode{false};
};

}
