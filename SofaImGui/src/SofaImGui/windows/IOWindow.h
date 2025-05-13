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
#include <SofaImGui/models/IPController.h>

#include <SofaImGui/windows/BaseWindow.h>
#include <SofaImGui/models/SimulationState.h>
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
    std::vector<rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr> m_subscriptions;

    std::map<std::string, std::vector<float>> m_selectedStateToPublish;
    std::map<std::string, bool> m_selectedDigitalOutputToPublish;
    std::map<std::string, std::vector<float>> m_selectedStateToOverwrite;
    std::map<std::string, bool> m_selectedDigitalInput;
    std::map<std::string, float> m_selectedUserInput;

    bool hasSelectedInput() {return !m_selectedStateToOverwrite.empty() || !m_selectedDigitalInput.empty() || !m_selectedUserInput.empty();}
    void clearSelectedInput() {m_selectedStateToOverwrite.clear(); m_selectedDigitalInput.clear(); m_selectedUserInput.clear();}

    void createSubscription(const std::string& topicName)
    {
        rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr subscription;
        subscription = this->create_subscription<std_msgs::msg::Float32MultiArray>(topicName, 10,
                                                                                   [this, topicName](const std_msgs::msg::Float32MultiArray::SharedPtr msg){topicCallback(msg, topicName);});
        this->m_subscriptions.push_back(subscription);
    }

    void createTopics()
    {
        if (!m_selectedStateToPublish.empty() || !m_selectedDigitalOutputToPublish.empty())
        {
            m_publishers.reserve(m_selectedStateToPublish.size() + m_selectedDigitalOutputToPublish.size());
            for (const auto& [key, value] : m_selectedStateToPublish)
            {
                const auto& publisher = create_publisher<std_msgs::msg::Float32MultiArray>(key, 10);
                m_publishers.push_back(publisher);
            }

            for (const auto& [key, value] : m_selectedDigitalOutputToPublish)
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
            m_subscriptions.reserve(m_selectedStateToOverwrite.size()
                                    + m_selectedDigitalInput.size()
                                    + m_selectedUserInput.size());
            for (const auto& [key, value] : m_selectedStateToOverwrite)
            {
                createSubscription(key);
            }
            for (const auto& [key, value] : m_selectedDigitalInput)
            {
                createSubscription(key);
            }
            for (const auto& [key, value] : m_selectedUserInput)
            {
                createSubscription(key);
            }
        }
    }

    void topicCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg, const std::string &topicName)
    {
        std::vector<float> vector;
        vector.reserve(msg->data.size());
        for (auto value: msg->data)
            vector.push_back(value);

        std::map<std::string, std::vector<float>>::iterator it = m_selectedStateToOverwrite.find(topicName);
        if (it != m_selectedStateToOverwrite.end())
            it->second = vector;

        std::map<std::string, float>::iterator itu = m_selectedUserInput.find(topicName);
        if (itu != m_selectedUserInput.end())
            if (!vector.empty())
                itu->second = vector[0];
    }
};
#endif


class SOFAIMGUI_API IOWindow : public BaseWindow
{
   public:
    IOWindow(){}
    IOWindow(const std::string& name, const bool& isWindowOpen);
    ~IOWindow();

    typedef typename sofa::defaulttype::RigidCoord<3, double> RigidCoord;

    void showWindow(sofa::simulation::Node *groot, const ImGuiWindowFlags &windowFlags);

    void animateBeginEvent(sofa::simulation::Node *groot);
    void animateEndEvent(sofa::simulation::Node *groot);
    
    void setIPController(models::IPController::SPtr IPController) {m_IPController=IPController;}
    void setSimulationState(const models::SimulationState &simulationState);

    void addSubscribableData(const std::string& name, sofa::core::BaseData* data);

    void clearWindow() override {m_IPController=nullptr;}

   protected:
    
    models::IPController::SPtr m_IPController;
    std::string m_defaultNodeName = "SofaComplianceRobotics";
    int m_method;
    bool m_isPublishing;
    bool m_isListening;
    bool m_isReadyToPublish;

    bool m_digitalInput[3];
    bool m_digitalOutput[3];

    void init();
    /// Sanitize the input string to match ROS requirements for topic and node name (no spaces, no special characters)
    bool sanitizeName(std::string &name);

    std::vector<models::SimulationState::StateData> m_simulationStateData;
    std::map<std::string, std::vector<float> > m_simulationState;
    std::vector<models::IPController::Actuator> m_actuators;
    std::map<std::string, sofa::core::BaseData*> m_subscribableData;
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


