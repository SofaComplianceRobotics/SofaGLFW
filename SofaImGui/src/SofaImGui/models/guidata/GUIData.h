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

#include <sofa/core/objectmodel/DDGNode.h>
#include <SofaGLFW/SofaGLFWBaseGUI.h>

namespace sofaimgui::models::guidata {

/**
 *  \brief OwnedBaseData is a wrapper around BaseData to manage its lifetime.
 * If isOwner is true, OwnedBaseData will delete the BaseData when it is destroyed.
 */
class SOFAIMGUI_API OwnedBaseData : public sofa::core::objectmodel::DDGNode
{
protected:
    sofa::core::BaseData* data;
    bool isOwner;
public:
    typedef std::shared_ptr<OwnedBaseData> SPtr;
    OwnedBaseData() : sofa::core::objectmodel::DDGNode(), data(nullptr), isOwner(false) {}
    OwnedBaseData(sofa::core::BaseData* data, bool isOwner) : DDGNode(), data(data), isOwner(isOwner)
    {
        if (data && !isOwner)
        {
            this->data->addOutput(this);
            this->addInput(this->data);
        }
    };
    virtual ~OwnedBaseData() {
        if (isOwner && data) {
            delete(data);
            data = nullptr;
        }
    };

    sofa::core::BaseData* getData() const { return data; }
    void setData(sofa::core::BaseData* newData, bool isOwner)
    {
        if (data)
        {
            data->delOutput(this);
            this->delInput(data);
        }
        data = newData;
        if (data)
        {
            this->isOwner = isOwner;
            if (!isOwner)
            {
                this->data->addOutput(this);
                this->addInput(this->data);
            }
        }
    };

    bool getIsOwner() const { return isOwner; }

    void doDelInput(sofa::core::objectmodel::DDGNode* node) override
    {
        data = nullptr;
        DDGNode::doDelInput(node);
    };

    void update() override
    {
        cleanDirty();
        for (DDGNode* input : inputs)
        {
            if (input)
                input->updateIfDirty();
        }
    }

    bool isValid()
    {
        return data != nullptr && data->getData() != nullptr && data->getData()->getValueTypeInfo() != nullptr && data->getData()->getValueVoidPtr() != nullptr;
    }

    bool static isDataValid(OwnedBaseData::SPtr data)
    {
        return data != nullptr && data->getData() != nullptr && data->getData()->getValueTypeInfo() != nullptr && data->getData()->getValueVoidPtr() != nullptr;
    }

};


/**
 *  \brief GUIData is a wrapper around BaseData to be used in ImGui widgets.
 * It contains additional information such as label, group, help, min and max values.
 */
class SOFAIMGUI_API GUIData
{
protected:
    OwnedBaseData::SPtr m_data;
    OwnedBaseData::SPtr m_min;
    OwnedBaseData::SPtr m_max;
    std::string m_label;
    std::string m_group;
    std::string m_help;
    bool m_validState;

public:
    typedef std::shared_ptr<GUIData> SPtr;
    constexpr static std::string DEFAULTGROUP = "";

    virtual ~GUIData() {};

    GUIData() : m_data(nullptr), m_min(nullptr), m_max(nullptr) {}

    GUIData(OwnedBaseData::SPtr data, OwnedBaseData::SPtr min, OwnedBaseData::SPtr max, std::string label, std::string group, std::string help)
    {
        this->m_data = data;
        this->m_min = min;
        this->m_max = max;
        this->m_label = label;
        this->m_group = group;
        this->m_help = help;
        this->m_validState = true;
    }

    sofa::core::BaseData* getData() const { return m_data? m_data->getData(): nullptr; };
    sofa::core::BaseData* getDataMin() const { return m_min? m_min->getData(): nullptr; };
    sofa::core::BaseData* getDataMax() const { return m_max? m_max->getData(): nullptr; };
    const std::string& getLabel() {return m_label;}
    const std::string& getGroup() {return m_group;}
    const std::string& getHelp() {return m_help;}

    double getMin() {return (m_min && m_min->isValid())? m_min->getData()->getValueTypeInfo()->getScalarValue(m_min->getData()->getValueVoidPtr(), 0): std::numeric_limits<float>::min();}
    double getMax() {return (m_max && m_max->isValid())? m_max->getData()->getValueTypeInfo()->getScalarValue(m_max->getData()->getValueVoidPtr(), 0): std::numeric_limits<float>::max();}

    void setData(sofa::core::BaseData* newData, bool isOwner=false)
    {
        if (m_data)
            m_data->setData(newData, isOwner);
        else
            m_data = std::make_shared<OwnedBaseData>(newData, isOwner);
    }

    bool isValid()
    {
        return m_validState && OwnedBaseData::isDataValid(m_data);
    }

    bool static isDataValid(GUIData* data)
    {
        return data != nullptr && data->isValid();
    }
};

struct GUIDataEqual
{
    bool operator()(const GUIData::SPtr a, const GUIData::SPtr b) const { return a->getData() == b->getData(); }
};

struct GUIDataHash
{
    size_t operator()(const GUIData::SPtr data) const
    {
        return std::hash<sofa::core::BaseData*>{}(data->getData());
    }
};

} // namespace sofaimgui::models
