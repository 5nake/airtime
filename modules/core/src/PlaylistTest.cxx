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
    Version  : $Revision: 1.19 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/modules/core/src/PlaylistTest.cxx,v $

------------------------------------------------------------------------------*/

/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#error "Need unistd.h"
#endif


#include <string>
#include <iostream>

#include "LiveSupport/Core/Playlist.h"
#include "PlaylistTest.h"


using namespace std;
using namespace LiveSupport::Core;


/* ===================================================  local data structures */


/* ================================================  local constants & macros */

CPPUNIT_TEST_SUITE_REGISTRATION(PlaylistTest);

/**
 *  The name of the configuration file for the playlist.
 */
static const std::string configFileName = "etc/playlist.xml";


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Set up the test environment
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: setUp(void)                         throw ()
{
    playlist.reset(new Playlist);
    try {
        Ptr<xmlpp::DomParser>::Ref  parser(
                                    new xmlpp::DomParser(configFileName, true));
        const xmlpp::Document * document = parser->get_document();
        const xmlpp::Element  * root     = document->get_root_node();

        playlist->configure(*root);

    } catch (std::invalid_argument &e) {
        CPPUNIT_FAIL("semantic error in configuration file");
    } catch (xmlpp::exception &e) {
        CPPUNIT_FAIL("error parsing configuration file");
    }
}


/*------------------------------------------------------------------------------
 *  Clean up the test environment
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: tearDown(void)                      throw ()
{
}


/*------------------------------------------------------------------------------
 *  Test to see if the singleton Hello object is accessible
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: firstTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    CPPUNIT_ASSERT(playlist->getId()->getId() == 0x1);
    Ptr<const boost::posix_time::time_duration>::Ref  duration
                                            = playlist->getPlaylength();
    CPPUNIT_ASSERT(duration->total_seconds() == 34);

    CPPUNIT_ASSERT(playlist->valid());
    CPPUNIT_ASSERT(*playlist->getXmlString() ==
"<playlist id=\"0000000000000001\" playlength=\"00:00:34\" title=\"\">\n"
"<playlistElement id=\"0000000000000101\" relativeOffset=\"00:00:00\">\n"
"<audioClip id=\"0000000000010001\" playlength=\"00:00:11\" title=\"one\"/>\n"
"</playlistElement>\n"
"<playlistElement id=\"0000000000000102\" relativeOffset=\"00:00:11\">\n"
"<audioClip id=\"0000000000010002\" playlength=\"00:00:12\" title=\"two\"/>\n"
"<fadeInfo id=\"0000000000009901\" fadeIn=\"00:00:02\" "
                                  "fadeOut=\"00:00:01.500000\"/>\n"
"</playlistElement>\n"
"<playlistElement id=\"0000000000000103\" relativeOffset=\"00:00:23\">\n"
"<playlist id=\"0000000000000002\" playlength=\"00:00:11\" title=\"\">\n"
"<playlistElement id=\"0000000000000111\" relativeOffset=\"00:00:00\">\n"
"<audioClip id=\"0000000000010003\" playlength=\"00:00:11\" title=\"three\"/>\n"
"</playlistElement>\n"
"</playlist>\n"
"</playlistElement>\n"
"</playlist>");

    Playlist::const_iterator        it = playlist->begin();
    CPPUNIT_ASSERT(it != playlist->end());
    Ptr<PlaylistElement>::Ref       playlistElement = it->second;
    CPPUNIT_ASSERT(playlistElement->getId()->getId() == 0x101);
    Ptr<const time_duration>::Ref   relativeOffset 
                                    = playlistElement->getRelativeOffset();
    CPPUNIT_ASSERT(relativeOffset->total_seconds()   == 0);
    CPPUNIT_ASSERT(playlistElement->getType() 
                                    == PlaylistElement::AudioClipType);
    CPPUNIT_ASSERT(playlistElement->getAudioClip()->getId()->getId() 
                                                     == 0x10001);

    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    playlistElement  = it->second;
    CPPUNIT_ASSERT(playlistElement->getId()->getId() == 0x102);
    relativeOffset   = playlistElement->getRelativeOffset();
    CPPUNIT_ASSERT(relativeOffset->total_seconds()   == 11);
    CPPUNIT_ASSERT(playlistElement->getType() 
                                    == PlaylistElement::AudioClipType);
    CPPUNIT_ASSERT(playlistElement->getAudioClip()->getId()->getId() 
                                                     == 0x10002);
    
    ++it;
//    CPPUNIT_ASSERT(it == playlist->end());
}


/*------------------------------------------------------------------------------
 *  Test to see if locking works
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: lockTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    CPPUNIT_ASSERT(!playlist->isLocked());
    CPPUNIT_ASSERT(!playlist->canBeEdited());

    CPPUNIT_ASSERT(playlist->setLockedForEditing(true));
    CPPUNIT_ASSERT(playlist->isLocked());
    CPPUNIT_ASSERT(playlist->canBeEdited());
    CPPUNIT_ASSERT(!playlist->setLockedForEditing(true));

    CPPUNIT_ASSERT(playlist->setLockedForPlaying(true));
    CPPUNIT_ASSERT(playlist->isLocked());
    CPPUNIT_ASSERT(!playlist->canBeEdited());
    CPPUNIT_ASSERT(!playlist->setLockedForEditing(true));
    CPPUNIT_ASSERT(!playlist->setLockedForEditing(false));
    CPPUNIT_ASSERT(!playlist->setLockedForPlaying(true));

    CPPUNIT_ASSERT(playlist->setLockedForPlaying(false));
    CPPUNIT_ASSERT(playlist->isLocked());
    CPPUNIT_ASSERT(playlist->canBeEdited());
    CPPUNIT_ASSERT(playlist->setLockedForEditing(false));
}


/*------------------------------------------------------------------------------
 *  Test to see if we can add or remove an audio clip
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: audioClipTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    Ptr<UniqueId>::Ref       clipId(new UniqueId("20001"));
    Ptr<time_duration>::Ref  clipLength(new time_duration(0,30,0,0));
    Ptr<AudioClip>::Ref      audioClip(new AudioClip(clipId, clipLength));

    Ptr<time_duration>::Ref  relativeOffset(new time_duration(0,10,0,0));
                                                // hour, min, sec, frac_sec
    try {
        playlist->addAudioClip(audioClip, relativeOffset);
    }
    catch (std::invalid_argument &e) {
        string eMsg = "addAudioClip returned with error: ";
        eMsg += e.what(); 
        CPPUNIT_FAIL(eMsg);
    }

    CPPUNIT_ASSERT(playlist->getPlaylength());
    CPPUNIT_ASSERT(*playlist->getPlaylength() == *relativeOffset + *clipLength);

    CPPUNIT_ASSERT(!playlist->valid());    // big gap in playlist

    Playlist::const_iterator       it = playlist->begin();
    CPPUNIT_ASSERT(it != playlist->end());

    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    
    Ptr<PlaylistElement>::Ref   playlistElement = it->second;
    CPPUNIT_ASSERT(playlistElement->getType() 
                                == PlaylistElement::AudioClipType);
    CPPUNIT_ASSERT(playlistElement->getAudioClip()->getId()->getId()
                                                             == 0x20001);

    Ptr<const time_duration>::Ref  otherRelativeOffset 
                                   = playlistElement->getRelativeOffset();
    CPPUNIT_ASSERT(otherRelativeOffset->total_seconds() == 10*60);

    ++it;
    CPPUNIT_ASSERT(it == playlist->end());

    try {
        playlist->removePlaylistElement(relativeOffset);
    }
    catch (std::invalid_argument &e) {
        string eMsg = "removePlaylistElement returned with error: ";
        eMsg += e.what(); 
        CPPUNIT_FAIL(eMsg);
    }

    it = playlist->begin();
    CPPUNIT_ASSERT(it != playlist->end());
    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    ++it;
    CPPUNIT_ASSERT(it == playlist->end());

    Ptr<const time_duration>::Ref  phonyRelativeOffset(
                                   new time_duration(0,0,1,0));
    try {
        playlist->removePlaylistElement(phonyRelativeOffset);
        CPPUNIT_FAIL("removePlaylistElement allowed to remove "
                     "non-existent audio clip");
    }
    catch (std::invalid_argument &e) {
    }
}


/*------------------------------------------------------------------------------
 *  Test the "save/revert to current state" mechanism
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: savedCopyTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    try {
        playlist->revertToSavedCopy();
        CPPUNIT_FAIL("allowed to revert to non-existent state");
    }
    catch (std::invalid_argument &e) {
    }

    playlist->createSavedCopy();
    playlist->removePlaylistElement(Ptr<time_duration>::Ref(
                              new time_duration(0,0,0,0)));
    playlist->removePlaylistElement(Ptr<time_duration>::Ref(
                              new time_duration(0,0,11,0)));
    playlist->removePlaylistElement(Ptr<time_duration>::Ref(
                              new time_duration(0,0,23,0)));
    CPPUNIT_ASSERT(playlist->begin() == playlist->end());

    try {
        playlist->revertToSavedCopy();
    }
    catch (std::logic_error &e) {
        CPPUNIT_FAIL("could not revert to saved state");
    }
    
    Playlist::const_iterator  it = playlist->begin();
    CPPUNIT_ASSERT(it != playlist->end());
    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    Ptr<PlaylistElement>::Ref   playlistElement = it->second;
    CPPUNIT_ASSERT(playlistElement->getType() 
                                    == PlaylistElement::AudioClipType);
    CPPUNIT_ASSERT(playlistElement->getAudioClip()->getId()->getId() 
                                    == 0x10002);
    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    ++it;
    CPPUNIT_ASSERT(it == playlist->end());

    playlist->deleteSavedCopy();
    try {
        playlist->revertToSavedCopy();
        CPPUNIT_FAIL("allowed to revert to deleted state");
    }
    catch (std::logic_error &e) {
    }
}


/*------------------------------------------------------------------------------
 *  Test to see if we can add a fade info
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: fadeInfoTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    Playlist::const_iterator       it = playlist->begin();
    CPPUNIT_ASSERT(it != playlist->end());
    Ptr<PlaylistElement>::Ref      playlistElement = it->second;
    CPPUNIT_ASSERT(playlistElement->getFadeInfo().get() == 0);

    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    playlistElement  = it->second;
    CPPUNIT_ASSERT(playlistElement->getFadeInfo()->getFadeIn()
                                  ->total_milliseconds() == 2000);
    CPPUNIT_ASSERT(playlistElement->getFadeInfo()->getFadeOut()
                                  ->total_milliseconds() == 1500);
    
    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    ++it;
    CPPUNIT_ASSERT(it == playlist->end());

    Ptr<time_duration>::Ref fadeIn (new time_duration(0,0,3,200000));
    Ptr<time_duration>::Ref fadeOut(new time_duration(0,0,4,0));
    Ptr<FadeInfo>::Ref      fadeInfo(new FadeInfo(fadeIn, fadeOut));

    Ptr<time_duration>::Ref relativeOffset (new time_duration(0,0,0,0));
    try {
        playlist->setFadeInfo(relativeOffset, fadeInfo);
    }
    catch (std::invalid_argument &e) {
        CPPUNIT_FAIL("could not add new fade info");
    }

    relativeOffset.reset(new time_duration(0,0,11,0));
    try {
        playlist->setFadeInfo(relativeOffset, fadeInfo);
    }
    catch (std::invalid_argument &e) {
        CPPUNIT_FAIL("could not update fade info");
    }

    it = playlist->begin();
    playlistElement  = it->second;
    CPPUNIT_ASSERT(playlistElement->getFadeInfo()->getFadeIn()
                                  ->total_milliseconds() == 3200);
    CPPUNIT_ASSERT(playlistElement->getFadeInfo()->getFadeOut()
                                  ->total_milliseconds() == 4000);
    ++it;
    playlistElement  = it->second;
    CPPUNIT_ASSERT(playlistElement->getFadeInfo()->getFadeIn()
                                  ->total_milliseconds() == 3200);
    CPPUNIT_ASSERT(playlistElement->getFadeInfo()->getFadeOut()
                                  ->total_milliseconds() == 4000);

    relativeOffset.reset(new time_duration(0,0,7,0));

    try {
        playlist->setFadeInfo(relativeOffset, fadeInfo);
        CPPUNIT_FAIL("allowed to set fade info for non-existent element");
    }
    catch (std::invalid_argument &e) {
    }
}


/*------------------------------------------------------------------------------
 *  Test conversion to and from Playable
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: conversionTest(void)
                                                throw (CPPUNIT_NS::Exception)
{    
    CPPUNIT_ASSERT(playlist.use_count() == 1);

    Ptr<Playable>::Ref      playable = playlist;
    CPPUNIT_ASSERT(playable->getType() == Playable::PlaylistType);
    CPPUNIT_ASSERT(playlist.use_count() == 2);
    
    Ptr<Playlist>::Ref      otherPlaylist = playable->getPlaylist();
    CPPUNIT_ASSERT(otherPlaylist == playlist);
    CPPUNIT_ASSERT(playlist.use_count() == 3);

    Ptr<AudioClip>::Ref     audioClip = playable->getAudioClip();
    CPPUNIT_ASSERT(!audioClip);
}

