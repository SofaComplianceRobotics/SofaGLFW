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
        this->data = data;
        if (this->data)
        {
            this->isOwner = isOwner;

            if (!isOwner)
            {
                this->data->addOutput(this);
                this->addInput(this->data);
            }
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
    void setIsOwner(bool owner) { isOwner = owner; }

    void doDelInput(sofa::core::objectmodel::DDGNode* node) override
    {
        DDGNode::delInput(node);
        data = nullptr;

    };

    void update() override
    {
        cleanDirty();
        for (DDGNode* input : inputs)
        {
            input->updateIfDirty();
        }
    }

};


/**
 *  \brief GUIData is a wrapper around BaseData to be used in ImGui widgets.
 * It contains additional information such as label, group, help, min and max values.
 */
class SOFAIMGUI_API GUIData
{
protected:
    OwnedBaseData::SPtr data;
    OwnedBaseData::SPtr min;
    OwnedBaseData::SPtr max;
public:
    typedef std::shared_ptr<GUIData> SPtr;
    constexpr static std::string DEFAULTGROUP = "";
    std::string label;
    std::string group;
    std::string help;

    virtual ~GUIData() {};

    GUIData() : data(nullptr), min(nullptr), max(nullptr) {}

    GUIData(OwnedBaseData::SPtr data, OwnedBaseData::SPtr min, OwnedBaseData::SPtr max, std::string label, std::string group, std::string help)
    {
        this->data = data;
        this->min = min;
        this->max = max;
        this->label = label;
        this->group = group;
        this->help = help;
    }

    sofa::core::BaseData* getData() const { return data? data->getData(): nullptr; };

    void setData(sofa::core::BaseData* newData, bool isOwner=false)
    {
        if (data)
            data->setData(newData, isOwner);
        else
            data = std::make_shared<OwnedBaseData>(newData, isOwner);
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
