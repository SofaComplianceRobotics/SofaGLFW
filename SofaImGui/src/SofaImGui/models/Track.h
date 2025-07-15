/******************************************************************************
 *                 SOFA, Simulation Open-Framework Architecture                *
 *                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
 *                                                                             *
 * This Track is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the Free  *
 * Software Foundation; either version 2 of the License, or (at your option)   *
 * any later version.                                                          *
 *                                                                             *
 * This Track is distributed in the hope that it will be useful, but WITHOUT *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
 * more details.                                                               *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with this Track. If not, see <http://www.gnu.org/licenses/>.              *
 *******************************************************************************
 * Authors: The SOFA Team and external contributors (see Authors.txt)          *
 *                                                                             *
 * Contact information: contact@sofa-framework.org                             *
 ******************************************************************************/
#pragma once

#include <memory>

#include <SofaImGui/models/modifiers/Modifier.h>
#include <SofaImGui/models/actions/Action.h>
#include <SofaImGui/models/actions/Move.h>
#include <SofaImGui/models/actions/StartMove.h>
#include <SofaImGui/models/IPController.h>

#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <sofa/core/objectmodel/DataFileName.h>
#include <SofaImGui/config.h>


namespace sofaimgui::models {

class Track
{
    typedef sofa::defaulttype::RigidCoord<3, double> RigidCoord;

   public:

    Track(models::IPController::SPtr IPController);
    Track(models::IPController::SPtr IPController, std::shared_ptr<actions::StartMove> startMove);
    ~Track() = default;

    void clear();

    std::shared_ptr<actions::StartMove> getStartMove() {return m_startmove;}

    std::vector<std::shared_ptr<actions::Action>>& getActions() {return m_actions;}
    std::shared_ptr<actions::Action> getAction(const sofa::Index& actionIndex) {return m_actions[actionIndex];}

    const std::vector<std::shared_ptr<modifiers::Modifier>>& getModifiers() {return m_modifiers;}
    std::shared_ptr<modifiers::Modifier> getModifier(const sofa::Index& modifierIndex) {return m_modifiers[modifierIndex];}

    void updateNextMoveInitialPoint(const sofa::Index &actionIndex, const RigidCoord &initialPoint);

    void pushModifier(std::shared_ptr<modifiers::Modifier> modifier);
    void pushRepeat();

    void popModifier();

    void insertModifier(const sofa::Index &modifierIndex, std::shared_ptr<modifiers::Modifier> modifier);
    void insertRepeat(const sofa::Index &modifierIndex);

    void deleteModifier(const sofa::Index &modifierIndex);

    std::shared_ptr<actions::Move> getPreviousMove(const sofa::Index &actionIndex);
    std::shared_ptr<actions::Move> getNextMove(const sofa::Index &actionIndex);

   protected:
    
    models::IPController::SPtr m_IPController;
    std::shared_ptr<actions::StartMove> m_startmove;
    std::vector<std::shared_ptr<actions::Action>> m_actions;
    std::vector<std::shared_ptr<modifiers::Modifier>> m_modifiers;
};

} // namespace


