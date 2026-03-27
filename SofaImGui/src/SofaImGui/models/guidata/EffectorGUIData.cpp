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
        auto context = effector->getContext();

        if (context)
        {
            auto groot = dynamic_cast<Node*>(context->getRootContext());
            if (groot)
            {
                Node::SPtr guiNode = groot->getChild(sofaglfw::SofaGLFWBaseGUI::getGUINodeName());
                if (!guiNode)
                {
                    guiNode = sofa::core::objectmodel::New<Node>(sofaglfw::SofaGLFWBaseGUI::getGUINodeName());
                    groot->addChild(guiNode);
                }


                auto positionEffector = dynamic_cast<softrobotsinverse::constraint::PositionEffector<sofa::defaulttype::Rigid3Types>*>(effector.get());

                if (positionEffector)
                {
                    sofa::core::behavior::BaseMechanicalState* TCPTargetMeca = nullptr;
                    indexInMechanical = positionEffector->d_indices.getValue()[0];
                    auto parent = positionEffector->findData("effectorGoal")->getParent();
                    if (parent)
                    {
                        TCPTargetMeca = dynamic_cast<sofa::component::statecontainer::MechanicalObject<sofa::defaulttype::Rigid3Types>*>(parent->getOwner());
                    }
                    else
                    {
                        Node::SPtr targetNode = sofa::core::objectmodel::New<Node>(label);
                        guiNode->addChild(targetNode);
                        TCPTargetMeca = sofa::core::objectmodel::New<sofa::component::statecontainer::MechanicalObject<sofa::defaulttype::Rigid3Types>>().get();
                        targetNode->addObject(TCPTargetMeca);
                        positionEffector->d_effectorGoal.setParent(TCPTargetMeca->findData("position")->getLinkPath());
                    }

                    if (TCPTargetMeca)
                        target = std::make_shared<OwnedBaseData>(TCPTargetMeca->findData("position"), false);
                }
            }
        }
    }
}

}
