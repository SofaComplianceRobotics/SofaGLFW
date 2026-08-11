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

#include <string>
#include <SimpleIni.h>

// Forward declarations
namespace sofaimgui { class ImGuiGUIEngine; }
namespace sofaimgui::windows { class BaseWindow; }

namespace sofaimgui::windows {

class WindowsSettings
{
    friend ImGuiGUIEngine;
    friend BaseWindow;

public:
    enum SettingType {
        LONG,
        DOUBLE,
        BOOL,
        STRING
    };

protected:
    static WindowsSettings &getInstance();

    template <typename type>
    type getSetting(const char* _windowName, const char* settingName, const type& defaultValue)
    {
        std::string windowName = "Window.";
        windowName += _windowName;

        type value;
        _getSetting(windowName.c_str(), settingName, defaultValue, value);
        return value;
    }

    template <typename type>
    void setSetting(const char* _windowName, const char* settingName, const type& value)
    {
        std::string windowName = "Window.";
        windowName += _windowName;

        _setSetting(windowName.c_str(), settingName, value);
    }

    CSimpleIniA& getIniWindowsSettings() {return iniWindowsSettings;}

private:
    CSimpleIniA iniWindowsSettings;

    inline void _getSetting(const char* windowName, const char* settingName, const double& defaultValue, double& value){value = iniWindowsSettings.GetDoubleValue(windowName, settingName, defaultValue);}
    inline void _getSetting(const char* windowName, const char* settingName, const bool& defaultValue, bool& value){value = iniWindowsSettings.GetBoolValue(windowName, settingName, defaultValue);}
    inline void _getSetting(const char* windowName, const char* settingName, const long& defaultValue, long& value){value = iniWindowsSettings.GetLongValue(windowName, settingName, defaultValue);}
    inline void _getSetting(const char* windowName, const char* settingName, const std::string& defaultValue, std::string& value){value = std::string(iniWindowsSettings.GetValue(windowName, settingName, defaultValue.c_str()));}

    inline void _setSetting(const char* windowName, const char* settingName, const double& value){iniWindowsSettings.SetDoubleValue(windowName, settingName, value);}
    inline void _setSetting(const char* windowName, const char* settingName, const bool& value){iniWindowsSettings.SetBoolValue(windowName, settingName, value);}
    inline void _setSetting(const char* windowName, const char* settingName, const long& value){iniWindowsSettings.SetLongValue(windowName, settingName, value);}
    inline void _setSetting(const char* windowName, const char* settingName, const std::string& value){iniWindowsSettings.SetValue(windowName, settingName, value.c_str());}

};

}
