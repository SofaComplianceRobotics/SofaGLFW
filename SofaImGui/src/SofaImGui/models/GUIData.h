#pragma once

#include <sofa/core/objectmodel/DDGNode.h>
#include <SofaGLFW/SofaGLFWBaseGUI.h>

namespace sofaimgui::models {

	/**
	 *  \brief OwnedBaseData is a wrapper around BaseData to manage its lifetime.
	 * If isOwner is true, OwnedBaseData will delete the BaseData when it is destroyed.
	 */
	class OwnedBaseData : public sofa::core::objectmodel::DDGNode {
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
	 * It contains additional information such as label, group, tooltip, min and max values.
	 */

	class GUIData {
	protected:
		OwnedBaseData::SPtr data;
		OwnedBaseData::SPtr min;
		OwnedBaseData::SPtr max;
	public:
		typedef std::shared_ptr<GUIData> SPtr;
		constexpr static std::string DEFAULTGROUP = "";
		std::string label;
		std::string group;
		std::string tooltip;

		virtual ~GUIData() {};

		GUIData() : data(nullptr), min(nullptr), max(nullptr) {}

		GUIData(OwnedBaseData::SPtr data, OwnedBaseData::SPtr min, OwnedBaseData::SPtr max, std::string label, std::string group, std::string tooltip)
		{
			this->data = data;
			this->min = min;
			this->max = max;
			this->label = label;
			this->group = group;
			this->tooltip = tooltip;
		}

		sofa::core::BaseData* getData() const { return this->data->getData(); };

		void setData(sofa::core::BaseData* newData, bool isOwner=false)
		{
			this->data->setData(newData, isOwner);
		}
	};

	struct GUIDataEqual {
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