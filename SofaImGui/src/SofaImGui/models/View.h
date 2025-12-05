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

#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <sofa/simulation/Node.h>


namespace sofaimgui::models {

void addViewportViewMenu(sofaglfw::SofaGLFWBaseGUI *baseGUI);

void showGrid(sofaglfw::SofaGLFWBaseGUI* baseGUI, const bool& show, const float& squareSize, const float& thickness, const sofa::type::RGBAColor& color);
void showBoundingBox(sofaglfw::SofaGLFWBaseGUI* baseGUI, const bool& show);
void showOriginFrame(sofaglfw::SofaGLFWBaseGUI* baseGUI, const bool& show);

} // namespace


