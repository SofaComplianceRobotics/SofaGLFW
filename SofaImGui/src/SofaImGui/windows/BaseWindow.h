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
#include <SofaImGui/models/guidata/GUIDataManager.h>

#include <SofaImGui/config.h>
#include <SofaImGui/windows/WindowsSettings.h>
#include <imgui.h>

namespace sofaimgui::windows {

class SOFAIMGUI_API BaseWindow: sofaimgui::models::guidata::GUIDataManager
{
   public:
    BaseWindow();
    BaseWindow(std::string name);
    ~BaseWindow() = default;

    void setBaseGUI(sofaglfw::SofaGLFWBaseGUI* baseGUI) { m_baseGUI = baseGUI; }

    /// This is called before loading / reloading a simulation.
    void clearWindow();

    /// Implements the drawing of the window
    void showWindow(ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None);

    /// Every window must implement this method, give a description of the window
    /// Will be displayed as a tooltip
    virtual std::string getDescription() = 0;

    /// Implementation on end init
    virtual void onEndInit(){}

    /// Get the name of the window
    std::string getName() const;

    /// Get the label of the window (name to display in the tab). We add spaces for aesthetic reason
    std::string& getLabel();

    /// Set the user choice to open the window or not, for the active workbench.
    void setOpen(const bool &isOpen);

    /// Set the user choice to open the window or not, for the given workbench.
    void setOpen(const Workbench& wb, const bool &isOpen);

    /// Does the user choose to open the window or not, in the active workbench.
    bool& isOpen();

    /// Does the user choose to open the window or not, in the given workbench
    bool& isOpen(const Workbench& wb);

    /// Returns true if the window is enabled in the active workbench
    bool isEnabledInWorkbench();

    /// Returns true if the window is enabled in the given workbench
    bool isEnabledInWorkbench(const Workbench& wb);

    /// Returns true if the window is open by default in the given workbench
    bool isDefaultWorkbench(const Workbench& wb);

    using models::guidata::GUIDataManager::addData;
    using models::guidata::GUIDataManager::addGUIData;
    using models::guidata::GUIDataManager::removeGUIData;

    /// The window may have nothing to display. It should override this method with the corresponding checks.
    /// For example: the PlottingWindow needs data to plot, if none are given, the window is disabled.
    virtual bool isEnabledByState() {return true;}

   protected:

    virtual void beforeShowWindow() {};
    virtual void internalShowWindow() {};
    virtual void afterShowWindow() {};

    /// Called once, the first time we draw the window.
    virtual void registerAndLoadWindowSettings() {};

    /// The window may have addional thing to clear. It should override this method with the corresponding cleaning.
    virtual void clear() {}

    /// Structured message display (info icon + message)
    void showInfoMessage(const char* message);

    /// Load and register a setting.
    /// The setting should be already initialized with the default value.
    template <typename type>
    void registerAndLoadSetting(const std::string& settingName, type& setting, const WindowsSettings::SettingType& settingType)
    {
        m_registeredSettings[settingName] = std::pair<void*, WindowsSettings::SettingType>(&setting, settingType); // Register setting to be saved
        setting = WindowsSettings::getInstance().getSetting(m_name.c_str(), settingName.c_str(), setting); // Load setting
    }
    void dropGUIData();

    using models::guidata::GUIDataManager::m_GUIData;
    using models::guidata::GUIDataManager::m_groupedGUIData;

    sofaglfw::SofaGLFWBaseGUI* m_baseGUI{nullptr};

    std::map<Workbench, bool> m_isOpen; /// The user choice to open the window or not
    std::string m_name = "Window"; /// The name of the window
    std::string m_labelname; /// The label of the window
    int m_enabledWorkbenches;
    int m_defaultWorkbenches;

    ImGuiWindowFlags m_windowFlags = ImGuiWindowFlags_None;

   private:

    bool m_firstTime{true};
    std::map<std::string, std::pair<void*, WindowsSettings::SettingType>> m_registeredSettings;
};
}
