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
    Version  : $Revision: 1.17 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/modules/core/include/LiveSupport/Core/Playlist.h,v $

------------------------------------------------------------------------------*/
#ifndef LiveSupport_Core_Playlist_h
#define LiveSupport_Core_Playlist_h

#ifndef __cplusplus
#error This is a C++ include file
#endif


/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <map>
#include <stdexcept>
#include <libxml++/libxml++.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "LiveSupport/Core/Ptr.h"
#include "LiveSupport/Core/UniqueId.h"
#include "LiveSupport/Core/Configurable.h"
#include "LiveSupport/Core/PlaylistElement.h"


namespace LiveSupport {
namespace Core {

using namespace boost::posix_time;

/* ================================================================ constants */


/* =================================================================== macros */


/* =============================================================== data types */

/**
 *  A class representing playlist.
 *  Playlist are containers for AudioClips, with the additional
 *  information of when and how each audio clip is played inside
 *  the playlist.
 *
 *  This object has to be configured with an XML configuration element
 *  called playlist. This may look like the following:
 *
 *  <pre><code>
 *  &lt;playlist id="1" playlength="00:18:30.000000" &gt;
 *      &lt;playlistElement&gt; ... &lt;/playlistElement&gt;
 *      ...
 *  &lt;/playlist&gt;
 *  </code></pre>
 *
 *  For detais of the playlistElement element, see the documentation 
 *  for the PlaylistElement class.
 *
 *  The DTD for the above element is:
 *
 *  <pre><code>
 *  &lt;!ELEMENT playlist (playlistElement*) &gt;
 *  &lt;!ATTLIST playlist  id           NMTOKEN    #REQUIRED  &gt;
 *  &lt;!ATTLIST playlist  playlength   NMTOKEN    #REQUIRED  &gt;
 *  </code></pre>
 *
 *  @author  $Author: fgerlits $
 *  @version $Revision: 1.17 $
 */
class Playlist : public Configurable
{
    private:
        /**
         *  The name of the configuration XML elmenent used by Playlist.
         */
        static const std::string    configElementNameStr;

        /**
         *  The unique id of the playlist.
         */
        Ptr<UniqueId>::Ref          id;

        /**
         *  The playling length of the playlist.
         */
        Ptr<time_duration>::Ref     playlength;

        /**
         *  The uri of the SMIL file generated from this playlist (if any).
         */
        Ptr<const std::string>::Ref uri;

        /**
         *  The token given to this playlist by the storage server.
         */
        Ptr<const std::string>::Ref token;

        /**
         *  Flag set if playlist is currently playing.
         */
        bool                        isLockedForPlaying;

        /**
         *  Flag set if playlist is currently being edited.
         */
        bool                        isLockedForEditing;

        /**
         *  A map type for storing the playlist elements associated with 
         *  this playlist, indexed by their relative offsets.
         */
        typedef std::map<const time_duration, Ptr<PlaylistElement>::Ref>
                                                     PlaylistElementListType;

        /**
         *  The list of playlist elements for this playlist.
         */
        Ptr<PlaylistElementListType>::Ref  elementList;

        /**
         *  Add a new playlist element to the playlist.
         *
         *  @param playlistElement the new playlist element to be added
         *  @exception std::invalid_argument if the playlist already contains
         *             a playlist element with the same relative offset
         */
        void
        addPlaylistElement(Ptr<PlaylistElement>::Ref playlistElement)
                                                throw (std::invalid_argument);

        /**
         *  A saved copy of this playlist.
         */
        Ptr<Playlist>::Ref          savedCopy;


    public:
        /**
         *  Default constructor.
         */
        Playlist(void)                          throw ()
        {
            elementList.reset(new PlaylistElementListType);
            this->isLockedForPlaying = false;
            this->isLockedForEditing = false;
        }

        /**
         *  Create a playlist by specifying all details.
         *  This is used for testing purposes.
         *
         *  @param id the id of the playlist.
         *  @param playlength the playing length of the playlist.
         *  @param uri the location of the SMIL file representing this
         *             playlist (optional)
         */
        Playlist(Ptr<UniqueId>::Ref         id,
                 Ptr<time_duration>::Ref    playlength,
                 Ptr<std::string>::Ref      uri = Ptr<std::string>::Ref())
                                                throw ()
        {
            this->id         = id;
            this->playlength = playlength;
            this->uri        = uri;
            elementList.reset(new PlaylistElementListType);
            this->isLockedForPlaying = false;
            this->isLockedForEditing = false;
        }

        /**
         *  A virtual destructor, as this class has virtual functions.
         */
        virtual
        ~Playlist(void)                         throw ()
        {
        }

        /**
         *  Return the name of the XML element this object expects
         *  to be sent to a call to configure().
         *  
         *  @return the name of the expected XML configuration element.
         */
        static const std::string
        getConfigElementName(void)               throw ()
        {
            return configElementNameStr;
        }

        /**
         *  Configure the object based on the XML element supplied.
         *  The supplied element is expected to be of the name
         *  returned by configElementName().
         *
         *  @param element the XML element to configure the object from.
         *  @exception std::invalid_argument if the supplied XML element
         *             contains bad configuraiton information
         */
        virtual void
        configure(const xmlpp::Element    & element)
                                                 throw (std::invalid_argument);

        /**
         *  Return the id of the playlist.
         *
         *  @return the unique id of the playlist.
         */
        Ptr<UniqueId>::Ref
        getId(void) const                        throw ()
        {
            return id;
        }

        /**
         *  Return the total playing length for this playlist.
         *
         *  @return the playling length of this playlist, in microseconds.
         */
        Ptr<time_duration>::Ref
        getPlaylength(void) const                throw ()
        {
            return playlength;
        }

        /**
         *  Return the URI of the SMIL file generated from this playlist.
         *
         *  @return the uri of the playlist.
         */
        Ptr<const std::string>::Ref
        getUri(void) const                                  throw ()
        {
            return uri;
        }

        /**
         *  Set the URI of the SMIL file generated from this playlist.
         *
         */
        void
        setUri(Ptr<const std::string>::Ref uri)             throw ()
        {
            this->uri = uri;
        }

        /**
         *  Return the token given to this playlist by the storage server.
         *
         *  @return the uri of the playlist.
         */
        Ptr<const std::string>::Ref
        getToken(void) const                                throw ()
        {
            return token;
        }

        /**
         *  Set the token given to this playlist by the storage server.
         *
         */
        void
        setToken(Ptr<const std::string>::Ref token)         throw ()
        {
            this->token = token;
        }

        /**
         *  Test whether the playlist is locked for editing or playing.
         *
         *  @return true if the playlist is currently being edited or
         *      played; false otherwise
         */
        bool
        isLocked() const                         throw ()
        {
            return isLockedForEditing || isLockedForPlaying;
        }

        /**
         *  Test whether the playlist is currently available for editing.
         *
         *  @return true if the playlist is available, false otherwise
         */
        bool
        canBeEdited() const                      throw ()
        {
            return isLockedForEditing && !isLockedForPlaying;
        }

        /**
         *  Lock or unlock the playlist for editing.
         *
         *  @return true if successfully obtained or released lock;
         *          false otherwise.
         */
        bool
        setLockedForEditing(const bool lockStatus)
                                                throw ();

        /**
         *  Lock or unlock the playlist for playing.
         *
         *  @return true if successfully obtained or released lock;
         *          false otherwise.
         */
        bool
        setLockedForPlaying(const bool lockStatus)
                                                throw ();

        /**
         *  The iterator type for this class.
         *
         */
        typedef PlaylistElementListType::const_iterator  const_iterator;

        /**
         *  Get an iterator pointing to the first playlist element.
         */
        const_iterator
        begin() const                           throw ()
        {
            return elementList->begin();
        }

        /**
         *  Get an iterator pointing to one after the last playlist element.
         */
        const_iterator
        end() const                             throw ()
        {
            return elementList->end();
        }

        /**
         *  Get an iterator pointing to a playlist element at a given
         *  relative offset.
         *  @param relativeOffset (a pointer to) the relative offset where
         *                        the playlist element is.
         *  @return a constant iterator to the playlist element if it exists,
         *          or playlist->end() if it does not.
         */
        const_iterator
        find(Ptr<const time_duration>::Ref relativeOffset) const
                                                throw ()
        {
            return elementList->find(*relativeOffset);
        }

        /**
         *  Add a new audio clip to the playlist.
         *
         *  @param audioClip the new audio clip to be added
         *  @param relativeOffset the start of the audio clip, relative
         *             to the start of the playlist
         *  @param fadeInfo the fade in / fade out info (optional)
         *  @exception std::invalid_argument if the playlist already contains
         *             a playlist element with the same relative offset
         */
         void
         addAudioClip(Ptr<AudioClip>::Ref      audioClip,
                      Ptr<time_duration>::Ref  relativeOffset,
                      Ptr<FadeInfo>::Ref       fadeInfo
                                               = Ptr<FadeInfo>::Ref())
                                                throw (std::invalid_argument);

        /**
         *  Add a new sub-playlist to the playlist.
         *
         *  @param playlist the sub-playlist to be added
         *  @param relativeOffset the start of the sub-playlist, relative
         *             to the start of the containing playlist
         *  @param fadeInfo the fade in / fade out info (optional)
         *  @exception std::invalid_argument if the playlist already contains
         *             a playlist element with the same relative offset
         */
         void
         addPlaylist(Ptr<Playlist>::Ref       playlist,
                     Ptr<time_duration>::Ref  relativeOffset,
                     Ptr<FadeInfo>::Ref       fadeInfo
                                               = Ptr<FadeInfo>::Ref())
                                                throw (std::invalid_argument);

        /**
         *  Set the fade in / fade out info for a playlist element.
         *
         *  @param relativeOffset the start of the playlist element, relative
         *             to the start of the playlist
         *  @param fadeInfo the new fade in / fade out info
         *  @exception std::invalid_argument if there is no playlist element
         *             at the given relative offset
         */
         void
         setFadeInfo(Ptr<time_duration>::Ref  relativeOffset,
                     Ptr<FadeInfo>::Ref       fadeInfo)
                                                throw (std::invalid_argument);

        /**
         *  Remove an audio clip from the playlist.
         *  THIS IS OBSOLETE, SUPERSEDED BY removePlaylistElement().
         *  TODO: REMOVE IT AFTER THERE ARE NO MORE REFERENCES TO IT.
         *
         *  @param relativeOffset the start of the audio clip, relative
         *             to the start of the playlist
         *  @exception std::invalid_argument if the playlist does not contain
         *             an audio clip with the specified relative offset
         */
         void
         removeAudioClip(Ptr<const time_duration>::Ref  relativeOffset)
                                                throw (std::invalid_argument);

        /**
         *  Remove a playlist element from the playlist.
         *
         *  @param relativeOffset the start of the playlist element, relative
         *             to the start of the playlist
         *  @exception std::invalid_argument if the playlist does not contain
         *             a playlist element at the specified relative offset
         */
         void
         removePlaylistElement(Ptr<const time_duration>::Ref  relativeOffset)
                                                throw (std::invalid_argument);

        /**
         *  Validate the playlist: check that there are no overlaps or gaps.
         *  If the playlength is the only thing amiss, playlist is considered
         *  valid, and the playlength is fixed.  (Hence no 'const'.)
         */
         bool
         valid(void)                            throw ();


        /**
         *  Create a saved copy of this playlist.  If a saved copy exists
         *  already, it is replaced by the current state.
         */
         void
         createSavedCopy(void)                  throw ();

        /**
         *  Delete the saved copy of the playlist, if exists (or do nothing).
         */
         void
         deleteSavedCopy(void)                  throw ()
         {
             savedCopy.reset();
         }

        /**
         *  Revert to the saved copy of this playlist.  If there is no
         *  saved copy, do nothing and throw an exception.
         */
         void
         revertToSavedCopy(void)                throw (std::logic_error);
};


/* ================================================= external data structures */


/* ====================================================== function prototypes */


} // namespace Core
} // namespace LiveSupport

#endif // LiveSupport_Core_Playlist_h

