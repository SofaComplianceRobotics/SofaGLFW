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

#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <SofaImGui/Workbench.h>
#include <string>

#include <sofa/simulation/Node.h>
#include <SofaImGui/config.h>
#include <imgui.h>
#include <SimpleIni.h>


namespace sofaimgui::windows {

class WindowsSettings
{
public:
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

protected:
    CSimpleIniA iniWindowsSettings;

    inline void _getSetting(const char* windowName, const char* settingName, const double& defaultValue, double& value){value = iniWindowsSettings.GetDoubleValue(windowName, settingName, defaultValue);}
    inline void _getSetting(const char* windowName, const char* settingName, const bool& defaultValue, bool& value){value = iniWindowsSettings.GetBoolValue(windowName, settingName, defaultValue);}
    inline void _getSetting(const char* windowName, const char* settingName, const int& defaultValue, int& value){value = iniWindowsSettings.GetLongValue(windowName, settingName, defaultValue);}
    inline void _getSetting(const char* windowName, const char* settingName, const std::string& defaultValue, std::string& value){value = std::string(iniWindowsSettings.GetValue(windowName, settingName, defaultValue.c_str()));}

    inline void _setSetting(const char* windowName, const char* settingName, const double& value){iniWindowsSettings.SetDoubleValue(windowName, settingName, value);}
    inline void _setSetting(const char* windowName, const char* settingName, const bool& value){iniWindowsSettings.SetBoolValue(windowName, settingName, value);}
    inline void _setSetting(const char* windowName, const char* settingName, const int& value){iniWindowsSettings.SetLongValue(windowName, settingName, value);}
    inline void _setSetting(const char* windowName, const char* settingName, const std::string& value){iniWindowsSettings.SetValue(windowName, settingName, value.c_str());}

};

class SOFAIMGUI_API BaseWindow
{
   public:
    struct GUIData {
        std::shared_ptr<sofa::core::BaseData> data;
        std::shared_ptr<sofa::core::BaseData> min;
        std::shared_ptr<sofa::core::BaseData> max;
        std::string group;
        std::string label;
        std::string tooltip;
    };


    struct GUIDataCompare {
        bool operator()(const GUIData& a, const GUIData& b) const { return a.data.get() > b.data.get(); }
    };

    BaseWindow();
    ~BaseWindow() = default;

    /// Implements the drawing of the window
    virtual void showWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI, const ImGuiWindowFlags &windowFlags);

    /// Every window must implement this method, give a description of the window
    /// Will be displayed as a tooltip
    virtual std::string getDescription() = 0;

    /// Set the window as able to drive the robot in simulation.
    virtual void setDrivingTCPTarget(const bool &isDrivingSimulation);

    /// This is called before loading / reloading a simulation.
    virtual void clearWindow() {}

    /// Get the name of the window
    std::string getName() const;

    /// Get the label of the window (name to display in the tab). We add spaces for aesthetic reason
    std::string& getLabel();

    /// Does the window have tools to drive the robot in simulation.
    bool isDrivingSimulation();

    /// Set the user choice to open the window or not.
    void setOpen(const bool &isOpen);

    /// Does the user choose to open the window or not.
    bool& isOpen();

    /// The default open state when there is no project file
    const bool& getDefaultIsOpen();

    /// Returns true if the window is enabled in the current workbench
    bool isEnabledInWorkbench();

    virtual void addGUIData(std::shared_ptr<sofa::core::BaseData> data,
                            std::shared_ptr<sofa::core::BaseData> min = nullptr,
                            std::shared_ptr<sofa::core::BaseData> max = nullptr,
                            const std::string& label = "",
                            const std::string& group = "",
                            const std::string& tooltip = "");
    virtual void removeGUIData(GUIData& data);
    void clearGUIData() { m_GUIData.clear(); }

   protected:

    /// The window may have nothing to display. It should override this method with the corresponding checks.
    /// For example: the PlottingWindow needs data to plot, if none are given, the window is disabled.
    virtual bool enabled() {return true;}

    /// Structured message display (info icon + message)
    void showInfoMessage(const char* message);

    bool m_isOpen{false}; /// The user choice to open the window or not
    std::string m_name = "Window"; /// The name of the window
    std::string m_labelname; /// The label of the window
    bool m_isDrivingSimulation{false}; /// Does the window have tools to drive the robot in simulation
    bool m_defaultIsOpen{false}; /// The default open state when there is no project file
    int m_workbenches;

    std::set<GUIData, GUIDataCompare> m_GUIData; /// A set of GUIData to use in the window
};

}

