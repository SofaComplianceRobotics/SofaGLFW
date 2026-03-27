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

#include <SofaImGui/models/guidata/GUIDataManager.h>

namespace sofaimgui::models::guidata {


GUIData::SPtr GUIDataManager::addData(const std::string& label,
                                     const std::pair<sofa::core::BaseData*, bool>& data,
                                     const std::pair<sofa::core::BaseData*, bool>& min,
                                     const std::pair<sofa::core::BaseData*, bool>& max,
                                     const std::string& group,
                                     const std::string& help)
{
    if(data.first)
    {
        OwnedBaseData::SPtr newdata = std::make_shared<OwnedBaseData>(data.first, data.second);
        OwnedBaseData::SPtr newmin = std::make_shared<OwnedBaseData>(min.first, min.second);
        OwnedBaseData::SPtr newmax = std::make_shared<OwnedBaseData>(max.first, max.second);

        auto guiDataPtr = std::make_shared<GUIData>(newdata, newmin, newmax, label, group, help);

        return addGUIData(guiDataPtr);
    }

    return nullptr;
}

GUIData::SPtr GUIDataManager::addGUIData(const GUIData::SPtr& guidata)
{
    // Check if already in the set
    if (m_GUIData.find(guidata) != m_GUIData.end())
    {
        return nullptr;
    }

    auto inserted = m_GUIData.insert(guidata);
    if (inserted.second) // Check if the insertion was successful
    {

        m_groupedGUIData[guidata.get()->group].push_back(*inserted.first);
        return *inserted.first;
    }
    return nullptr;
}

void GUIDataManager::removeGUIData(GUIData::SPtr data)
{
    if (data)
    {
        m_GUIData.erase(data);
        std::vector<GUIData::SPtr>& group = m_groupedGUIData[data->group];
        auto it = std::find(group.begin(), group.end(), data);
        if(it != group.end())
            group.erase(it);
    }
}

}
