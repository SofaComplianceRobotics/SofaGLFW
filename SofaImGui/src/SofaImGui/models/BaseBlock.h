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

#include <sofa/defaulttype/RigidTypes.h>
#include <SofaImGui/config.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <string>

namespace sofaimgui::models {
    class Track;
}

namespace sofaimgui::models {

class BaseBlock
{
   public:

    typedef std::shared_ptr<BaseBlock> SPtr;

    inline static const int COMMENTSIZE = 18;
    inline static const double DEFAULTDURATION = 1.;

    BaseBlock(const double& duration=DEFAULTDURATION):
        m_duration(duration)
    {
    }

    virtual ~BaseBlock() = default;
    virtual BaseBlock::SPtr duplicate() = 0;

    const double& getDuration() {return m_duration;}
    virtual void setDuration(const double& duration) {m_duration = duration;}

    virtual void pushToTrack(std::shared_ptr<models::Track> track) = 0;
    virtual void insertInTrack(std::shared_ptr<models::Track> track, const sofa::Index &actionIndex) = 0;
    virtual void deleteFromTrack(std::shared_ptr<models::Track> track, const sofa::Index &actionIndex) = 0;
    virtual void swapWith(BaseBlock::SPtr action) = 0;

    void setComment(const char* comment) {strncpy(m_comment, comment, COMMENTSIZE); m_comment[COMMENTSIZE-1]='\0';}
    void getComment(char* comment) {strncpy(comment, m_comment, COMMENTSIZE); comment[COMMENTSIZE-1]='\0';}

    char* getComment() {return m_comment;}

   protected:

    double m_duration;
    char m_comment[COMMENTSIZE];

    class BaseBlockView
    {
       public:
        bool showBlock(const std::string &label, const ImVec2 &size, const bool &isSelected=false)
        {
            SOFA_UNUSED(isSelected);
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            float x = window->DC.CursorStartPos.x ;
            float y = window->DC.CursorStartPos.y ;

            bool hasValuesChanged = showBlockInternal(label, size, isSelected);

            window->DC.CursorStartPos.x = x;
            window->DC.CursorStartPos.y = y;

            return hasValuesChanged;
        }
       protected:
        virtual bool showBlockInternal(const std::string &, const ImVec2 &, const bool & = false) {return false;}
    };
    BaseBlockView view;

   public :

    virtual BaseBlockView* getView() {return &view;}
};

} // namespace


