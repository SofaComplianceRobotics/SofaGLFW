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
#include <map>

#include <SofaImGui/windows/BaseWindow.h>
#include <SofaImGui/models/SimulationState.h>
#include <SofaImGui/models/guidata/KinematicsGUIDataManager.h>
#include <SofaImGui/Workbench.h>
#include <SofaImGui/DrivingWindow.h>
#include <imgui.h>

#if SOFAIMGUI_WITH_ROS
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
using namespace std::chrono_literals;
#endif


namespace sofaimgui::windows {

#if SOFAIMGUI_WITH_ROS
class SOFAIMGUI_API ROSNode: public rclcpp::Node
{
   public:
    ROSNode(const std::string& name): Node(name){}
    ~ROSNode() = default;

    std::vector<rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr> m_publishers;
    std::map<std::string, sofa::core::BaseData*> m_selectedDataToPublish;

    std::vector<rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr> m_subscriptions;
    std::map<std::string, sofa::core::BaseData*> m_selectedDataToOverwrite;

    bool hasSelectedOutput() {return !m_selectedDataToPublish.empty();}
    void clearSelectedOutput() {m_selectedDataToPublish.clear();}

    bool hasSelectedInput() {return !m_selectedDataToOverwrite.empty();}
    void clearSelectedInput() {m_selectedDataToOverwrite.clear();}

    void createSubscription(const std::string& topicName)
    {
        rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr subscription;
        subscription = this->create_subscription<std_msgs::msg::Float32MultiArray>(topicName, 10,
                                                                                   [this, topicName](const std_msgs::msg::Float32MultiArray::SharedPtr msg){topicCallback(msg, topicName);});
        this->m_subscriptions.push_back(subscription);
    }

    void createTopics()
    {
        if (!m_selectedDataToPublish.empty())
        {
            m_publishers.reserve(m_selectedDataToPublish.size());
            for (const auto& [key, value] : m_selectedDataToPublish)
            {
                const auto& publisher = create_publisher<std_msgs::msg::Float32MultiArray>(key, 10);
                m_publishers.push_back(publisher);
            }
        }
    }

    void createSubscriptions()
    {
        if (hasSelectedInput())
        {
            m_subscriptions.reserve(m_selectedDataToOverwrite.size());
            for (const auto& [key, value] : m_selectedDataToOverwrite)
                createSubscription(key);
        }
    }

    void topicCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg, const std::string &topicName)
    {
        std::vector<float> vector;
        vector.reserve(msg->data.size());
        for (auto value: msg->data)
            vector.push_back(value);

        std::map<std::string, sofa::core::BaseData*>::iterator it = m_selectedDataToOverwrite.find(topicName);
        if (it != m_selectedDataToOverwrite.end())
        {
            if (it->second)
            {
                auto* data = it->second->getData();
                auto* typeinfo = data->getValueTypeInfo();
                size_t nbValue = typeinfo->size();
                if (vector.size() == nbValue)
                    for (size_t i = 0; i < nbValue; i++)
                        typeinfo->setScalarValue(data, i, vector[i]);
            }
        }
    }
};
#endif


class SOFAIMGUI_API IOWindow : public BaseWindow
{
   public:
    enum Role
    {
		PUBLISH,
		SUBSCRIBE,
        ALL
    };

    IOWindow(){}
    IOWindow(const std::string& name, const bool& isWindowOpen);
    ~IOWindow();

    typedef typename sofa::defaulttype::RigidCoord<3, double> RigidCoord;

    void showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags) override;
    std::string getDescription() override;

    void animateBeginEvent(sofa::simulation::Node *groot);
    void animateEndEvent(sofa::simulation::Node *groot);
    
    // void setKinematicsController(models::KinematicsController::SPtr kinematicsController) {m_kinematicsController=kinematicsController;}

    sofaimgui::models::guidata::GUIData::SPtr addData(const std::string& label,
                                                     const std::pair<sofa::core::BaseData*, bool>& data,
                                                     const std::pair<sofa::core::BaseData*, bool>& min = std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                                                     const std::pair<sofa::core::BaseData*, bool>& max = std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                                                     const std::string& group = models::guidata::GUIData::DEFAULTGROUP,
                                                     const std::string& tooltip = "",
                                                     Role role = Role::ALL);

    void clearWindow() override {/* m_kinematicsController = nullptr;*/ m_selectableData.clear(); }

   protected:
    
    // models::KinematicsController::SPtr m_kinematicsController;
    std::string m_defaultNodeName = "SofaComplianceRobotics";
    int m_method;

    bool m_isReadyToPublish;
    bool m_isPublishing;
    bool m_isListening;

    /// Sanitize the input string to match ROS requirements for topic and node name (no spaces, no special characters)
    bool sanitizeName(std::string &name);

    /// Update ROS output lists with the topics selected from the GUI
    void updateROSOutput();

    /// Update ROS input lists with the topics selected from the GUI
    void updateROSInput();

    bool isDrivingSimulation() {return drivingWindow == DrivingWindow::IO;}

    std::map<std::string, bool> m_publishListboxItems;
    std::map<std::string, bool> m_subcriptionListboxItems;

    std::map<Role, std::map<std::string, sofaimgui::models::guidata::GUIData::SPtr>> m_selectableData; // <Role<label, GUIData::SPtr>> Fed by the user from the python bindings API

    float m_itemWidth;

#if SOFAIMGUI_WITH_ROS
    std::shared_ptr<ROSNode> m_rosnode;

    void showROSWindow();
    void showROSOutput();
    void showROSInput();
    void animateBeginEventROS(sofa::simulation::Node *groot);
    void animateEndEventROS(sofa::simulation::Node *groot);
#endif
};

}


