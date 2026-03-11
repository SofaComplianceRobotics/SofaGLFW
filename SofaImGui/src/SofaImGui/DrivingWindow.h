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

constexpr int getDrivingWindowCount() { return 4; }

// Driving window for Robotics Simulation GUI
enum DrivingWindow {
    NONE    = 0,
    MOVE    = 1, // 1. Move Window - For moving the robot using sliders.
    PROGRAM = 2, // 2. Program Window - For programming the robot using a block based timeline.
    IO      = 3  // 3. IO Window - For piloting the robot from input / output data.
};

// Function to get the names of the DrivingWindow enum as strings
inline const char* getDrivingWindowName(DrivingWindow drivingWindow) {
    switch (drivingWindow) {
    case MOVE: return "Move";
    case PROGRAM: return "Program";
    case IO: return "Input/Output";
    default: return "None";
    }
}

// Function to get the description of the Workbench enum as strings
inline const char* getDrivingWindowDescription(DrivingWindow drivingWindow) {
    switch (drivingWindow) {
    case MOVE: return "For moving the robot using sliders.";
    case PROGRAM: return "For programming the robot using a block based timeline.";
    case IO: return "For piloting the robot from input / output data.";
    default: return "None";
    }
}

extern DrivingWindow drivingWindow;

}
