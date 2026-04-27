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

#include <SofaImGui/models/guidata/GUIData.h>

#include <unordered_set>

namespace sofaimgui::models::guidata {

class SOFAIMGUI_API GUIDataManager
{
public:
    typedef std::shared_ptr<GUIDataManager> SPtr;
    virtual GUIData::SPtr addData(const std::string& label,
                                                     const std::pair<sofa::core::BaseData*, bool>& data,
                                                     const std::pair<sofa::core::BaseData*, bool>& min = std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                                                     const std::pair<sofa::core::BaseData*, bool>& max = std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                                                     const std::string& group = GUIData::DEFAULTGROUP,
                                                     const std::string& help = "");
    virtual GUIData::SPtr addGUIData(const GUIData::SPtr& data);
    virtual void removeGUIData(GUIData::SPtr data);
    void clearGUIData() { m_GUIData.clear(); }

protected:

    std::unordered_set<GUIData::SPtr, GUIDataHash, GUIDataEqual> m_GUIData; /// A set of GUIData to use in the window
    std::map<std::string, std::vector<GUIData::SPtr>> m_groupedGUIData; /// A map of grouped GUIData for easier access in the window by group name
};

}
