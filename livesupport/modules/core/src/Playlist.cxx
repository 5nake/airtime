/*------------------------------------------------------------------------------

    Copyright (c) 2004 Media Development Loan Fund
 
    This file is part of the LiveSupport project.
    http://livesupport.campware.org/
    To report bugs, send an e-mail to bugs@campware.org
 
    LiveSupport is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    LiveSupport is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with LiveSupport; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
 
    Author   : $Author: fgerlits $
    Version  : $Revision: 1.18 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/modules/core/src/Playlist.cxx,v $

------------------------------------------------------------------------------*/

/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <sstream>

#include "LiveSupport/Core/Playlist.h"

using namespace boost::posix_time;

using namespace LiveSupport::Core;

/* ===================================================  local data structures */


/* ================================================  local constants & macros */

/*------------------------------------------------------------------------------
 *  The name of the config element for this class
 *----------------------------------------------------------------------------*/
const std::string Playlist::configElementNameStr = "playlist";

/**
 *  The name of the attribute to get the id of the playlist.
 */
static const std::string    idAttrName = "id";

/**
 *  The name of the attribute to get the playlength of the playlist.
 */
static const std::string    playlengthAttrName = "playlength";

/**
 *  The name of playlist element child nodes.
 */
static const std::string    elementListAttrName = "playlistElement";


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Create a playlist object based on an XML element.
 *----------------------------------------------------------------------------*/
void
Playlist :: configure(const xmlpp::Element    & element)
                                            throw (std::invalid_argument)
{
    if (element.get_name() != configElementNameStr) {
        std::string eMsg = "bad configuration element ";
        eMsg += element.get_name();
        throw std::invalid_argument(eMsg);
    }

    const xmlpp::Attribute    * attribute;

    if (!(attribute = element.get_attribute(idAttrName))) {
        std::string eMsg = "missing attribute ";
        eMsg += idAttrName;
        throw std::invalid_argument(eMsg);
    }
    std::stringstream           idStream(attribute->get_value());
    UniqueId::IdType            idValue;
    idStream >> idValue;
    id.reset(new UniqueId(idValue));

    if (!(attribute = element.get_attribute(playlengthAttrName))) {
        std::string eMsg = "missing attribute ";
        eMsg += playlengthAttrName;
        throw std::invalid_argument(eMsg);
    }
    playlength.reset(new time_duration(
                            duration_from_string(attribute->get_value())));

    xmlpp::Node::NodeList  childNodes 
                           = element.get_children(elementListAttrName);
    xmlpp::Node::NodeList::iterator it = childNodes.begin();

    while (it != childNodes.end()) {
        Ptr<PlaylistElement>::Ref  newPlaylistElement(new PlaylistElement);
        const xmlpp::Element       * childElement 
                                   = dynamic_cast<const xmlpp::Element*> (*it);
        newPlaylistElement->configure(*childElement);    // may throw exception
        addPlaylistElement(newPlaylistElement);
        ++it;
    }
    
    isLockedForPlaying = false;
    isLockedForEditing = false;
}


/*------------------------------------------------------------------------------
 *  Add a new playlist element to the playlist.
 *----------------------------------------------------------------------------*/
void
Playlist::addPlaylistElement(Ptr<PlaylistElement>::Ref playlistElement)
                                            throw (std::invalid_argument)
{
    Ptr<const time_duration>::Ref  relativeOffset
                                   = playlistElement->getRelativeOffset();

    if (elementList->find(*relativeOffset) != elementList->end()) {
        std::string eMsg = "two playlist elements at the same relative offset";
        throw std::invalid_argument(eMsg);
    }

    (*elementList)[*relativeOffset] = playlistElement;
}


/*------------------------------------------------------------------------------
 *  Add a new audio clip to the playlist.
 *----------------------------------------------------------------------------*/
void
Playlist::addAudioClip(Ptr<AudioClip>::Ref      audioClip,
                       Ptr<time_duration>::Ref  relativeOffset,
                       Ptr<FadeInfo>::Ref       fadeInfo)
                                            throw (std::invalid_argument)
{
    if (elementList->find(*relativeOffset) != elementList->end()) {
        std::string eMsg = "two playlist elements at the same relative offset";
        throw std::invalid_argument(eMsg);
    }

    Ptr<PlaylistElement>::Ref  playlistElement(new PlaylistElement(
                                   relativeOffset, audioClip, fadeInfo));
    (*elementList)[*relativeOffset] = playlistElement;
}


/*------------------------------------------------------------------------------
 *  Add a new sub-playlist to the playlist.
 *----------------------------------------------------------------------------*/
void
Playlist::addPlaylist(Ptr<Playlist>::Ref       playlist,
                      Ptr<time_duration>::Ref  relativeOffset,
                      Ptr<FadeInfo>::Ref       fadeInfo)
                                            throw (std::invalid_argument)
{
    if (elementList->find(*relativeOffset) != elementList->end()) {
        std::string eMsg = "two playlist elements at the same relative offset";
        throw std::invalid_argument(eMsg);
    }

    Ptr<PlaylistElement>::Ref  playlistElement(new PlaylistElement(
                                   relativeOffset, playlist, fadeInfo));
    (*elementList)[*relativeOffset] = playlistElement;
}


/*------------------------------------------------------------------------------
 *  Change the fade in / fade out info of a playlist element.
 *----------------------------------------------------------------------------*/
void
Playlist::setFadeInfo(Ptr<time_duration>::Ref  relativeOffset,
                      Ptr<FadeInfo>::Ref       fadeInfo)
                                            throw (std::invalid_argument)
{
    PlaylistElementListType::iterator it = elementList->find(*relativeOffset);

    if (it == elementList->end()) {
        std::string eMsg = "no playlist element at this relative offset";
        throw std::invalid_argument(eMsg);
    }

    it->second->setFadeInfo(fadeInfo);
}


/*------------------------------------------------------------------------------
 *  Remove an audio clip from the playlist.

CHANGE references of THIS TO REFER TO removePlaylistElement()

 *----------------------------------------------------------------------------*/
void
Playlist::removeAudioClip(Ptr<const time_duration>::Ref  relativeOffset)
                                            throw (std::invalid_argument)
{
    // this returns the number of elements found and erased
    if (!elementList->erase(*relativeOffset)) {
        std::string eMsg = "no audio clip at the specified relative offset";
        throw std::invalid_argument(eMsg);
    }
}


/*------------------------------------------------------------------------------
 *  Remove a playlist element from the playlist.
 *----------------------------------------------------------------------------*/
void
Playlist::removePlaylistElement(Ptr<const time_duration>::Ref  relativeOffset)
                                            throw (std::invalid_argument)
{
    // this returns the number of elements found and erased
    if (!elementList->erase(*relativeOffset)) {
        std::string eMsg = "no playlist element found "
                           "at the specified relative offset";
        throw std::invalid_argument(eMsg);
    }
}


/*------------------------------------------------------------------------------
 *  Lock or unlock the playlist for editing.
 *----------------------------------------------------------------------------*/
bool
Playlist::setLockedForEditing(const bool lockStatus)
                                            throw ()
{
    if (lockStatus == true) {
        if (isLockedForPlaying || isLockedForEditing) {
            return false;
        }
        else {
            isLockedForEditing = true;
            return true;
        }
    }
    else {
        if (isLockedForPlaying) {
            return false;
        }
        else {
            isLockedForEditing = false;
            return true;                    // returns true also if playlist
        }                                   // was already unlocked!
    }
}


/*------------------------------------------------------------------------------
 *  Lock or unlock the playlist for playing.
 *----------------------------------------------------------------------------*/
bool
Playlist::setLockedForPlaying(const bool lockStatus)
                                            throw ()
{
    if (lockStatus == true) {
        if (isLockedForPlaying) {
            return false;
        }
        else {
            isLockedForPlaying = true;      // preserve LockedForEditing state
            return true;
        }
    }
    else {
        isLockedForPlaying = false;         // restore LockedForEditing state;
        return true;                        // returns true also if playlist
    }                                       // was already unlocked!
}


/*------------------------------------------------------------------------------
 *  Validate the playlist.
 *----------------------------------------------------------------------------*/
bool
Playlist::valid(void)                    throw ()
{
    Ptr<time_duration>::Ref     runningTime(new time_duration(0,0,0,0));
    Ptr<PlaylistElement>::Ref   playlistElement;
    Ptr<AudioClip>::Ref         audioClip;
    Ptr<Playlist>::Ref          playlist;

    PlaylistElementListType::const_iterator  it = elementList->begin();
    while (it != elementList->end()) {
        playlistElement = it->second;
        if (*runningTime != *(playlistElement->getRelativeOffset())) {
            return false;
        }
        if (playlistElement->getType() == PlaylistElement::AudioClipType) {
            audioClip = playlistElement->getAudioClip();
            *runningTime += *(audioClip->getPlaylength());
        }
        else if (playlistElement->getType() == PlaylistElement::PlaylistType) {
            playlist = playlistElement->getPlaylist();
            if (!playlist->valid()) {
                return false;
            }
            *runningTime += *(playlist->getPlaylength());
        }
        else {                  // this should never happen
            return false;
        }
        ++it;
    }
    playlength = runningTime;   // fix playlength, if everything else is OK
    return true;
}


/*------------------------------------------------------------------------------
 *  Create a saved copy of the playlist.
 *----------------------------------------------------------------------------*/
void
Playlist::createSavedCopy(void)          throw ()
{
    savedCopy = Ptr<Playlist>::Ref(new Playlist);

    savedCopy->id                 = this->id;
    savedCopy->playlength         = this->playlength;
    savedCopy->isLockedForPlaying = this->isLockedForPlaying;
    savedCopy->isLockedForEditing = this->isLockedForEditing;

    // note: we create a new copy of the playlist element map, but not of the
    //   individual playlist elements, which (i think) are immutable
    savedCopy->elementList.reset(new PlaylistElementListType(*elementList));

    savedCopy->savedCopy.reset();
}


/*------------------------------------------------------------------------------
 *  Revert to a saved copy of the playlist.
 *----------------------------------------------------------------------------*/
void
Playlist::revertToSavedCopy(void)        throw (std::invalid_argument)
{
    if (savedCopy == 0) {
        throw (std::invalid_argument("playlist has no saved copy"));
    }

    this->id                      = savedCopy->id;
    this->playlength              = savedCopy->playlength;
    this->isLockedForPlaying      = savedCopy->isLockedForPlaying;
    this->isLockedForEditing      = savedCopy->isLockedForEditing;
    this->elementList             = savedCopy->elementList;

    savedCopy.reset();
}


/*------------------------------------------------------------------------------
 *  Return the value of a metadata field.
 *----------------------------------------------------------------------------*/
Ptr<Glib::ustring>::Ref
Playlist :: getMetadata(const string &key, const string &ns) const
                                                throw ()
{
    std::string                   completeKey = key + ns;
    metadataType::const_iterator  it = metadata.find(completeKey);

    if (it != metadata.end()) {
        Ptr<Glib::ustring>::Ref data(new Glib::ustring(*it->second));
        return data;
    }
    else {
        Ptr<Glib::ustring>::Ref nullPointer;
        return nullPointer;
    }
}


/*------------------------------------------------------------------------------
 *  Set the value of a metadata field.
 *----------------------------------------------------------------------------*/
void
Playlist :: setMetadata(Ptr<const Glib::ustring>::Ref value, 
                        const string &key, const string &ns)
                                                throw ()
{
    std::string     completeKey = key + ns;
    metadata[completeKey] = value;
}

