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
    Version  : $Revision: 1.5 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/scheduler/src/Attic/XmlRpcToolsTest.cxx,v $

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
#include <XmlRpcValue.h>

#include "LiveSupport/Db/ConnectionManagerFactory.h"
#include "LiveSupport/Storage/StorageClientFactory.h"
#include "XmlRpcTools.h"
#include "XmlRpcToolsTest.h"


using namespace LiveSupport::Db;
using namespace LiveSupport::Storage;
using namespace LiveSupport::Scheduler;

using namespace std;
using namespace XmlRpc;

/* ===================================================  local data structures */


/* ================================================  local constants & macros */

CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcToolsTest);

/**
 *  The name of the configuration file for the playlist.
 */
const std::string configFileName = "etc/playlist.xml";


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Configure a Configurable with an XML file.
 *----------------------------------------------------------------------------*/
void
XmlRpcToolsTest :: configure(
            Ptr<Configurable>::Ref      configurable,
            const std::string           fileName)
                                                throw (CPPUNIT_NS::Exception)
{
    try {
        Ptr<xmlpp::DomParser>::Ref  parser(
                                    new xmlpp::DomParser(configFileName, true));
        const xmlpp::Document * document = parser->get_document();
        const xmlpp::Element  * root     = document->get_root_node();

        configurable->configure(*root);
    } catch (std::invalid_argument &e) {
        CPPUNIT_FAIL("semantic error in configuration file");
    } catch (xmlpp::exception &e) {
        CPPUNIT_FAIL("error parsing configuration file");
    }
}

                      
/*------------------------------------------------------------------------------
 *  Set up the test environment
 *----------------------------------------------------------------------------*/
void
XmlRpcToolsTest :: setUp(void)                         throw ()
{
}


/*------------------------------------------------------------------------------
 *  Clean up the test environment
 *----------------------------------------------------------------------------*/
void
XmlRpcToolsTest :: tearDown(void)                      throw ()
{
}


/*------------------------------------------------------------------------------
 *  Just a very simple smoke test
 *----------------------------------------------------------------------------*/
void
XmlRpcToolsTest :: firstTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    XmlRpcValue                xmlRpcPlaylist;
    XmlRpcValue                xmlRpcAudioClip;
    Ptr<Playlist>::Ref         playlist = Ptr<Playlist>::Ref(new Playlist);
    Ptr<const AudioClip>::Ref  audioClip;

    // set up a playlist instance
    configure(playlist, configFileName);
    audioClip = playlist->begin()->second->getAudioClip();

    // run the packing methods
    XmlRpcTools :: playlistToXmlRpcValue(playlist, xmlRpcPlaylist);
    XmlRpcTools :: audioClipToXmlRpcValue(audioClip, xmlRpcAudioClip);

    CPPUNIT_ASSERT(int(xmlRpcPlaylist["id"]) == 1);
    CPPUNIT_ASSERT(int(xmlRpcPlaylist["playlength"]) == 90 * 60);

    CPPUNIT_ASSERT(int(xmlRpcAudioClip["id"]) == 0x10001);
    CPPUNIT_ASSERT(int(xmlRpcAudioClip["playlength"]) == 60 * 60);

    XmlRpcValue              xmlRpcPlaylistId;
    Ptr<UniqueId>::Ref       playlistId;
    Ptr<UniqueId>::Ref       audioClipId;
    Ptr<time_duration>::Ref  relativeOffset;

    int                 playlistIdNum = rand();
    int                 audioClipIdNum = rand();
    int                 relativeOffsetNum = rand();

    xmlRpcPlaylistId["playlistId"] = playlistIdNum;
    xmlRpcPlaylistId["audioClipId"] = audioClipIdNum;
    xmlRpcPlaylistId["relativeOffset"] = relativeOffsetNum;

    // run the unpacking methods
    try {
        playlistId     = XmlRpcTools::extractPlaylistId(xmlRpcPlaylistId);
        audioClipId    = XmlRpcTools::extractAudioClipId(xmlRpcPlaylistId);
        relativeOffset = XmlRpcTools::extractRelativeOffset(xmlRpcPlaylistId);
    }
    catch (std::invalid_argument &e) {
        CPPUNIT_FAIL(e.what());
    }

    CPPUNIT_ASSERT((int)(playlistId->getId())  == playlistIdNum);
    CPPUNIT_ASSERT((int)(audioClipId->getId()) == audioClipIdNum);
    CPPUNIT_ASSERT((int)(relativeOffset->total_seconds()) 
                                               == relativeOffsetNum);
}


/*------------------------------------------------------------------------------
 *  Testing markError()
 *----------------------------------------------------------------------------*/
void
XmlRpcToolsTest :: errorTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    XmlRpcValue  xmlRpcValue;

    try {
        XmlRpcTools :: markError(42, "this is an error", xmlRpcValue);
        CPPUNIT_FAIL("did not throw exception in markError()");
    }
    catch (XmlRpc::XmlRpcException &e) {
        CPPUNIT_ASSERT(e.getCode() == 42);
        CPPUNIT_ASSERT(e.getMessage() == "this is an error");
    }
}

