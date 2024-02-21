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

#include <SofaImGui/models/Track.h>
#include <memory>
#include <string>
#include <vector>
#include <sofa/core/objectmodel/DataFileName.h>

namespace sofaimgui::models {

class Program
{
   public:
    Program()
    {
        std::shared_ptr<models::Track> track = std::make_shared<models::Track>();
        addTrack(track);
    }
    ~Program() = default;

    void importProgram(const std::string& filename);
    void exportProgram(const std::string &filename);

    const std::vector<std::shared_ptr<Track>>& getTracks() {return m_tracks;}
    int getNbTracks() {return m_tracks.size();}

    void addTrack(std::shared_ptr<Track> track) {m_tracks.push_back(track);}
    void removeTrack(const sofa::Index &index) {m_tracks.erase(m_tracks.begin() + index);}

    void addAction(const std::shared_ptr<Action> &action, const sofa::Index &trackID);
    void removeAction(const sofa::Index &actionID, const sofa::Index &trackID);

    void interpolate(const float& time);

    void clear();

   protected:

    bool checkExtension(const std::string &filename);
    std::vector<std::shared_ptr<Track>> m_tracks;

};

} // namespace


