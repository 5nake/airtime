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
    Version  : $Revision: 1.4 $
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

        CPPUNIT_ASSERT(playlist->getId()->getId() == 1);
        Ptr<const boost::posix_time::time_duration>::Ref  duration
                                                = playlist->getPlaylength();
        CPPUNIT_ASSERT(duration->hours() == 1);
        CPPUNIT_ASSERT(duration->minutes() == 30);
        CPPUNIT_ASSERT(duration->seconds() == 0);
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
    Playlist::const_iterator       it = playlist->begin();
    CPPUNIT_ASSERT(it != playlist->end());
    Ptr<PlaylistElement>::Ref      playlistElement = it->second;
    CPPUNIT_ASSERT(playlistElement->getId()->getId() == 101);
    Ptr<const time_duration>::Ref  relativeOffset 
                                   = playlistElement->getRelativeOffset();
    CPPUNIT_ASSERT(relativeOffset->total_seconds() == 0);
    CPPUNIT_ASSERT(playlistElement->getAudioClipId()->getId() == 10001);

    ++it;
    CPPUNIT_ASSERT(it != playlist->end());
    playlistElement  = it->second;
    CPPUNIT_ASSERT(playlistElement->getId()->getId() == 102);
    relativeOffset   = playlistElement->getRelativeOffset();
    CPPUNIT_ASSERT(relativeOffset->total_seconds() == 60 * 60);
    CPPUNIT_ASSERT(playlistElement->getAudioClipId()->getId() == 10002);
    
    ++it;
    CPPUNIT_ASSERT(it == playlist->end());
}


/*------------------------------------------------------------------------------
 *  Test to see if locking works
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: lockTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    CPPUNIT_ASSERT(playlist->setLockedForEditing(true));
    CPPUNIT_ASSERT(!playlist->setLockedForEditing(true));
    CPPUNIT_ASSERT(playlist->setLockedForEditing(false));

    CPPUNIT_ASSERT(playlist->setLockedForPlaying(true));
    CPPUNIT_ASSERT(!playlist->setLockedForPlaying(true));
    CPPUNIT_ASSERT(playlist->setLockedForPlaying(false));

    CPPUNIT_ASSERT(playlist->setLockedForEditing(true));
    CPPUNIT_ASSERT(playlist->setLockedForPlaying(true));
    CPPUNIT_ASSERT(!playlist->setLockedForEditing(false));
    CPPUNIT_ASSERT(playlist->setLockedForPlaying(false));
    CPPUNIT_ASSERT(!playlist->setLockedForEditing(true));
}


/*------------------------------------------------------------------------------
 *  Test to see if we can add a new audio clip
 *----------------------------------------------------------------------------*/
void
PlaylistTest :: addAudioClipTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    Ptr<UniqueId>::Ref       audioClipId(new UniqueId(20001));
    Ptr<time_duration>::Ref  relativeOffset(new time_duration(0,10,0,0));
                                                // hour, min, sec, frac_sec
    try {
        playlist->addAudioClip(audioClipId, relativeOffset);
    }
    catch (std::invalid_argument &e) {
        string eMsg = "addAudioClip returned with error: ";
        eMsg += e.what(); 
        CPPUNIT_FAIL(eMsg);
    }

    Playlist::const_iterator       it = playlist->begin();
    CPPUNIT_ASSERT(it != playlist->end());

    ++it;
    Ptr<PlaylistElement>::Ref      playlistElement = it->second;
    CPPUNIT_ASSERT(playlistElement->getAudioClipId()->getId() == 20001);

    Ptr<const time_duration>::Ref  otherRelativeOffset 
                                   = playlistElement->getRelativeOffset();
    CPPUNIT_ASSERT(otherRelativeOffset->total_seconds() == 10*60);

    ++it;
    CPPUNIT_ASSERT(it != playlist->end());

    ++it;
    CPPUNIT_ASSERT(it == playlist->end());
}
