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

#include <SofaImGui/models/guidata/GUIDataManager.h>


namespace sofaimgui::models::guidata
{

class ActuatorGUIData: public GUIData
{
public:
    typedef std::shared_ptr<ActuatorGUIData> SPtr;

    ActuatorGUIData(OwnedBaseData::SPtr data,
                    OwnedBaseData::SPtr min,
                    OwnedBaseData::SPtr max,
                    std::string label,
                    std::string group,
                    std::string help,
                    OwnedBaseData::SPtr _indexInProblem,
                    sofa::Size _size,
                    int _valueType)
        : GUIData(data, min, max, label, group, help)
        , m_size(_size)
        , m_indexInProblem(_indexInProblem)
    {
        m_valueType.setSelectedItem(_valueType);
    }


    double getValue(const sofa::Index &index);
    void setValue(const sofa::Index &index, const double& value);

    sofa::Size getSize() {return m_size;}
    sofa::Index getIndexInProblem();
    sofa::Index getValueTypeId() {return m_valueType.getSelectedId();}

protected:

    sofa::Size m_size;
    OwnedBaseData::SPtr m_indexInProblem;
    sofa::helper::OptionsGroup m_valueType{"lambda", "delta"};
};

}
