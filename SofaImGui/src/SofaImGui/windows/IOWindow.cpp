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

#include <SofaImGui/windows/IOWindow.h>
#include <SofaImGui/widgets/Widgets.h>
#include <GUIColors.h>

#include <IconsFontAwesome6.h>

#include <sofa/core/behavior/BaseMechanicalState.h>
#include <SofaImGui/FooterStatusBar.h>

#include <regex>

#if SOFAIMGUI_WITH_ROS
#include <rclcpp/node.hpp>
#include <rmw/validate_node_name.h>
#endif


namespace sofaimgui::windows {

IOWindow::IOWindow(const std::string& name, const bool& isWindowOpen, models::guidata::KinematicsGUIDataManager::SPtr kinematicsGUIDataManager)
{
    m_workbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;

    m_defaultIsOpen = false;
    m_name = name;
    m_isOpen = isWindowOpen;
    m_kinematicsGUIDataManager = kinematicsGUIDataManager;

#if SOFAIMGUI_WITH_ROS
    rclcpp::init(0, nullptr);
    m_rosnode = std::make_shared<ROSNode>(m_defaultNodeName);
#endif
}

IOWindow::~IOWindow()
{
#if SOFAIMGUI_WITH_ROS
    rclcpp::shutdown();
#endif
}

std::string IOWindow::getDescription()
{
    return "Input / output operations of data.";
}

bool IOWindow::sanitizeName(std::string &name)
{
    const std::string input = name;

    // Sanitize the input string to match ROS requirements for topic and node name
    // https://design.ros2.org/articles/topic_and_service_names.html
    // Name must not contain characters other than alphanumerics, '_', '~', '{', or '}'
    // Comment: no sanitizer function provided by ROS for the moment 2025

    if (name.empty())
        name = "noname";

    name = std::regex_replace(name, std::regex("[^a-zA-Z0-9_~{}/]+"), "");

    // may start with a tilde (~), the private namespace substitution character
    // must separate a tilde (~) from the rest of the name with a forward slash (/), i.e. ~/foo not ~foo
    name = std::regex_replace(name, std::regex("(?!^~\/)~"), "");

    // must not start with a numeric character ([0-9])
    name = std::regex_replace(name, std::regex("^[0-9]*"), "");

    // must not end with a forward slash (/)
    name = std::regex_replace(name, std::regex("\/$"), "");

    // must not contain any number of repeated forward slashes (/)
    name = std::regex_replace(name, std::regex("\/{2,}"), "");

    // must not contain any number of repeated underscore (_)
    name = std::regex_replace(name, std::regex("_{2,}"), "");

    return input != name;
}

void IOWindow::showWindow(const ImGuiWindowFlags &windowFlags)
{
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
        {
            if (!isEnabledInWorkbench())
            {
                showInfoMessage("This window is used for input/output operations of data. It is disabled in the active workbench.");
                ImGui::BeginDisabled();
            }

            m_itemWidth = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 4;

            static const char* items[]{
#if SOFAIMGUI_WITH_ROS
                                       "ROS",
#endif
                                       "None"
            };

            ImGui::Spacing();
            ImGui::Indent();
            ImGui::Text("Method:");
            ImGui::PushItemWidth(m_itemWidth);
            ImGui::LocalCombo("##ComboMethod", &m_method, items, IM_ARRAYSIZE(items));
            ImGui::PopItemWidth();
            ImGui::Spacing();
            ImGui::Unindent();

#if SOFAIMGUI_WITH_ROS
            if (m_method == 0) // ROS
                showROSWindow();
#endif

            if (!isEnabledInWorkbench())
                ImGui::EndDisabled();
        }
        ImGui::End();
    }
}

models::guidata::GUIData::SPtr IOWindow::addData(const std::string& label,
                                                    const std::pair<sofa::core::BaseData*, bool>& data,
                                                    const std::pair<sofa::core::BaseData*, bool>& min,
                                                    const std::pair<sofa::core::BaseData*, bool>& max,
                                                    const std::string& group,
                                                    const std::string& tooltip,
                                                    Role role)
{
    auto newdData = BaseWindow::addData(label, data, min, max, group, tooltip);
	if (role == Role::ALL)
	{
        m_selectableData[Role::PUBLISH][label] = newdData;
        m_selectableData[Role::SUBSCRIBE][label] = newdData;
	}
    else
        m_selectableData[role][label] = newdData;
    return newdData;
}

void IOWindow::animateBeginEvent(sofa::simulation::Node *groot)
{
    SOFA_UNUSED(groot);

#if SOFAIMGUI_WITH_ROS
    if (m_method == 0) // ROS
    {
        animateBeginEventROS(groot);
    }
#endif
}

void IOWindow::animateEndEvent(sofa::simulation::Node *groot)
{
    SOFA_UNUSED(groot);
#if SOFAIMGUI_WITH_ROS
    if (m_method == 0) // ROS
        animateEndEventROS(groot);
#endif
}

#if SOFAIMGUI_WITH_ROS

void IOWindow::showROSWindow()
{
    // When publishing/listening, make the section's title blink
    static float pulseDuration = 0;
    pulseDuration += ImGui::GetIO().DeltaTime;
    float pulse = pulseDuration / 2.0f;
    if (pulse > 2)
        pulseDuration = 0;

    ImVec4 color = ImColor(COLOR_GREEN);
    color.w = 0.75f + 0.25f * sin(pulse * 2 * 3.1415);

    { // Show output section
        ImGui::PushStyleColor(ImGuiCol_Text, (m_isPublishing)? color: ImGui::GetStyle().Colors[ImGuiCol_Text]);
        if (ImGui::LocalBeginCollapsingHeader("Output", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::PopStyleColor();
            showROSOutput();
            ImGui::LocalEndCollapsingHeader();
        }
        else
        {
            ImGui::PopStyleColor();
        }
    }

    { // Show input section
        ImGui::PushStyleColor(ImGuiCol_Text, (m_isListening)? color: ImGui::GetStyle().Colors[ImGuiCol_Text]);
        if (ImGui::LocalBeginCollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::PopStyleColor();
            showROSInput();
            ImGui::LocalEndCollapsingHeader();
        }
        else
        {
            ImGui::PopStyleColor();
        }
    }
}

void IOWindow::showROSOutput()
{
    if (m_isPublishing)
    {
        ImGui::BeginDisabled();
    }

    { // Node name
        static bool validNodeName = true;
        static char nodeBuf[30];

        ImGui::PushItemWidth(m_itemWidth);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
        bool hasNodeNameChanged = ImGui::InputTextWithHint("##NodePublishers", "Enter a node name", nodeBuf, 30, ImGuiInputTextFlags_CharsNoBlank);
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();
        ImGui::SetItemTooltip("Default is %s", m_defaultNodeName.c_str());

        static int nameCheckResult;
        if (hasNodeNameChanged)
        {
            size_t index;
            [[maybe_unused]] auto result = rmw_validate_node_name(nodeBuf, &nameCheckResult, &index);
            if(nameCheckResult == 0)
            {
                m_rosnode = std::make_shared<ROSNode>(nodeBuf);
                validNodeName = true;
            }
            else
            {
                if (nameCheckResult == 1) // empty buf
                {
                    m_rosnode = std::make_shared<ROSNode>(m_defaultNodeName);
                    validNodeName = true;
                }
                else
                    validNodeName = false;
            }
        }

        if (!validNodeName)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, COLOR_RED);
            ImGui::Text("%s", rmw_node_name_validation_result_string(nameCheckResult));
            ImGui::PopStyleColor();
        }

        m_isReadyToPublish = validNodeName;
    }

    { // List box
        static bool publishFirstTime = true;
        ImGui::Text("Select what to publish:");
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);

        bool updateROSData = false;
        if (ImGui::BeginListBox("##StatePublish", ImVec2(m_itemWidth, 0)))
        {
            // User-defined data
            for (const auto& [key, value] : m_selectableData[Role::PUBLISH])
            {
                if (publishFirstTime)
                {
                    m_publishListboxItems[key] = false;
                }

                ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
                color.w = 1.0;
                ImGui::PushStyleColor(ImGuiCol_FrameBg, color);
                if (ImGui::LocalCheckBox(key.c_str(), &m_publishListboxItems[key]))
                    updateROSData = true;
                ImGui::PopStyleColor();
            }

            ImGui::EndListBox();
        }
        publishFirstTime = false;
        ImGui::PopStyleVar();

        if (updateROSData)
            updateROSOutput();
    }

    if (m_isPublishing)
    {
        ImGui::EndDisabled();
    }

    { // Publishing button
        // Disable the button if nothing is selected
        bool nothingSelected = !m_rosnode->hasSelectedOutput();
        if (nothingSelected)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::LocalToggleButton("PublishersListening", &m_isPublishing))
        {
            if(m_isPublishing)
            {
                m_rosnode->createTopics();
            }
            else
            {
                m_rosnode->m_publishers.clear();
            }
        }
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(m_isPublishing? "Publishing" : "Publish");

        if (nothingSelected)
        {
            ImGui::EndDisabled();
        }
    }
}

void IOWindow::showROSInput()
{
    if (m_isListening)
    {
        ImGui::BeginDisabled();
    }

    // Nodes
    static int nodeID = -1;

    // List of found nodes
    const std::vector<std::string>& nodelist = m_rosnode->get_node_names();
    const size_t nbNodes = nodelist.size();
    std::vector<const char*> nodes;
    nodes.reserve(nbNodes);
    for (size_t i=0; i<nbNodes; i++)
        nodes.push_back(nodelist[i].c_str());

    ImGui::Text("Select a node:");
    ImGui::PushItemWidth(m_itemWidth);
    ImGui::LocalCombo("##NodeSubscription", &nodeID, nodes.data(), nbNodes);
    ImGui::PopItemWidth();

    // List of found topics
    auto topiclist = m_rosnode->get_topic_names_and_types();
    if (nodeID >= 0)
    {
        for (auto it = topiclist.cbegin(); it != topiclist.cend();) // Loop over the found topics
        {
            bool isTopicOnlyPublishedByChosenNode = true;
            const std::vector<rclcpp::TopicEndpointInfo>& publishers = m_rosnode->get_publishers_info_by_topic(it->first.c_str());
            for (const auto& p : publishers)
            {
                if (nodes[nodeID] != "/" + p.node_name())
                {
                    isTopicOnlyPublishedByChosenNode = false;
                }
            }
            if (!isTopicOnlyPublishedByChosenNode)
            {
                topiclist.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }

    { // List box subcriptions
        // Subscription parameters
        static bool subscribeFirstTime = true;
        ImGui::Text("Select data to overwrite:");
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);

        bool updateROSData = false;
        if (ImGui::BeginListBox("##StateSubscription", ImVec2(m_itemWidth, 0)))
        {
            // User defined input
            for (const auto &[name, value] : m_selectableData[Role::SUBSCRIBE])
            {
                if (subscribeFirstTime)
                {
                    m_subcriptionListboxItems[name] = false;
                }

                bool hasMatchingTopic = topiclist.find(name) != topiclist.end();

                if (!hasMatchingTopic)
                {
                    ImGui::BeginDisabled();
                }

                ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
                color.w = 1.0;
                ImGui::PushStyleColor(ImGuiCol_FrameBg, color);
                if (ImGui::LocalCheckBox(name.c_str(), &m_subcriptionListboxItems[name]))
                    updateROSData = true;
                ImGui::PopStyleColor();

                if (!hasMatchingTopic)
                {
                    ImGui::EndDisabled();
                }
            }

            ImGui::EndListBox();
        }
        ImGui::PopStyleVar();
        subscribeFirstTime = false;

        if (updateROSData)
            updateROSInput();
    }

    if (m_isListening)
    {
        ImGui::EndDisabled();
    }

    { // Listening button
        // Disable the button if nothing is selected
        if (!m_rosnode->hasSelectedInput())
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::LocalToggleButton("SubcriptionListening", &m_isListening))
        {
            if (m_isListening)
            {
                m_rosnode->createSubscriptions();
            }
            else
            {
                m_rosnode->m_subscriptions.clear();
            }
        }
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(m_isListening? "Listening" : "Subscribe");

        if (!m_rosnode->hasSelectedInput())
        {
            ImGui::EndDisabled();
        }
    }
}

void IOWindow::updateROSOutput()
{
    m_rosnode->clearSelectedOutput();

    // Simulation data
    for (const auto& [key, guiData] : m_selectableData[Role::PUBLISH])
    {
        if(m_publishListboxItems[key])
        {
            m_rosnode->m_selectedDataToPublish["/" + key] = guiData->getData();
        }
    }
}

void IOWindow::updateROSInput()
{
    m_rosnode->clearSelectedInput();

    // User defined input
    for (const auto &[label, guiData] : m_selectableData[Role::SUBSCRIBE])
    {
        if (guiData)
        {
            std::string dataLabel = label;
            if (dataLabel.find("TCP") != std::string::npos) //TCP target
                std::string dataLabel = "/TCPTarget/Frame";

            if (m_subcriptionListboxItems[dataLabel])
                m_rosnode->m_selectedDataToOverwrite[dataLabel] = guiData->getData();  // default temp value, will be overwritten by chosen topic's callback
        }
    }
}

void IOWindow::animateBeginEventROS(sofa::simulation::Node *groot)
{
    SOFA_UNUSED(groot);

    if (m_isListening)
    {
        rclcpp::spin_some(m_rosnode);  // Create a default single-threaded executor and execute any immediately available work.

        // Overwrite the TCPTarget with ROS input
        for (const auto& [label, guiData]: m_rosnode->m_selectedDataToOverwrite)
        {
            if (guiData)
            {
                sofa::core::BaseData* data = guiData->getData();
                if (data)
                {
                    if (label.find("TCPTarget") != std::string::npos && m_kinematicsGUIDataManager->hasTCP() && isDrivingSimulation())
                    {
                        if (data->getValueTypeInfo()->size() == IOWindow::RigidCoord::total_size)
                        {
                            IOWindow::RigidCoord position;
                            for (size_t i=0; i<IOWindow::RigidCoord::total_size; i++)
                                position[i] = data->getValueTypeInfo()->getScalarValue(data->getValueVoidPtr(), i);

                            m_kinematicsGUIDataManager->getTCPGUIData()->setTCPTargetPosition(position);
                        }
                        else
                        {
                            FooterStatusBar::getInstance().setTempMessage("Wrong size for the data from topic TCPTarget. The expected data structure is [x, y, z, qx, qy, qz, qw].",
                                                                          FooterStatusBar::MessageType::MWARNING);
                        }
                    }
                    else
                    {
                        m_selectableData[Role::SUBSCRIBE][label]->getData()->copyValueFrom(data);
                    }
                }
            }
        }
    }
}

void IOWindow::animateEndEventROS(sofa::simulation::Node *groot)
{
    SOFA_UNUSED(groot);

    if (m_isPublishing)
    {
        for (const auto& publisher : m_rosnode->m_publishers)
        {
            // Copy the data to publish
            auto message = std_msgs::msg::Float32MultiArray();
            const auto& data = m_rosnode->m_selectedDataToPublish[publisher->get_topic_name()];
            auto* typeinfo = data->getValueTypeInfo();
            auto* values = data->getValueVoidPtr();
            size_t nbValue = typeinfo->size();
            std::vector<float> vector(nbValue);

            for (size_t i=0; i<nbValue; i++) // Values
                vector[i]=typeinfo->getScalarValue(values, i);

            message.data.insert(message.data.end(), vector.begin(), vector.end());
            publisher->publish(message);
        }
    }
}

#endif

}

