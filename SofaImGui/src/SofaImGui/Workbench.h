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

namespace sofaimgui {

inline int getWorkbenchCount() { return 3; }

// Workbench for Robotics Simulation GUI (flags)
enum Workbench {
    SCENE_EDITOR    = 1 << 0, // 1. Scene Editor - For building and editing the scene without simulation or robot connection capabilities.
    SIMULATION_MODE = 1 << 1, // 2. Simulation Mode - For running simulations with a locked scene that cannot be edited. Only the parameters can be adjusted.
    LIVE_CONTROL    = 1 << 2  // 3. Live Control - For connecting to and controlling the real robot with the finalized scene.
};

// Function to get the names of the Workbench enum as strings
inline const char* getWorkbenchName(Workbench workbench) {
    switch (workbench) {
        case SCENE_EDITOR: return "Scene Editor";
        case SIMULATION_MODE: return "Simulation Mode";
        case LIVE_CONTROL: return "Live Control";
        default: return "Unknown";
    }
}

extern Workbench workbench;

}
