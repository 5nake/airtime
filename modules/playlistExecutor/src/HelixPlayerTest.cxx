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
 
 
    Author   : $Author: maroy $
    Version  : $Revision: 1.5 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/modules/playlistExecutor/src/Attic/HelixPlayerTest.cxx,v $

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

#include "LiveSupport/Core/TimeConversion.h"

#include "HelixPlayer.h"
#include "HelixPlayerTest.h"


using namespace LiveSupport::PlaylistExecutor;

/* ===================================================  local data structures */


/* ================================================  local constants & macros */

CPPUNIT_TEST_SUITE_REGISTRATION(HelixPlayerTest);

/**
 *  The name of the configuration file for the Helix player.
 */
static const std::string configFileName = "etc/helixPlayer.xml";


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Set up the test environment
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: setUp(void)                         throw ()
{
    try {
        Ptr<xmlpp::DomParser>::Ref  parser(
                                    new xmlpp::DomParser(configFileName, true));
        const xmlpp::Document * document = parser->get_document();
        const xmlpp::Element  * root     = document->get_root_node();

        helixPlayer.reset(new HelixPlayer());
        helixPlayer->configure(*root);

    } catch (std::invalid_argument &e) {
        std::cerr << "semantic error in configuration file" << std::endl;
    } catch (xmlpp::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}


/*------------------------------------------------------------------------------
 *  Clean up the test environment
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: tearDown(void)                      throw ()
{
    helixPlayer.reset();
}


/*------------------------------------------------------------------------------
 *  Test to see if the HelixPlayer engine can be started and stopped
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: firstTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    try {
        helixPlayer->initialize();
        CPPUNIT_ASSERT(!helixPlayer->isPlaying());
        helixPlayer->deInitialize();
    } catch (std::exception &e) {
        CPPUNIT_FAIL("failed to initialize or de-initialize HelixPlayer");
    }
}


/*------------------------------------------------------------------------------
 *  Play something simple
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: simplePlayTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    Ptr<time_duration>::Ref     sleepT(new time_duration(microseconds(10)));

    helixPlayer->initialize();
    helixPlayer->playThis("file:var/test.mp3");
    CPPUNIT_ASSERT(!helixPlayer->isPlaying());
    helixPlayer->start();
    CPPUNIT_ASSERT(helixPlayer->isPlaying());
    while (helixPlayer->isPlaying()) {
        TimeConversion::sleep(sleepT);
    }
    CPPUNIT_ASSERT(!helixPlayer->isPlaying());
    helixPlayer->deInitialize();
}


/*------------------------------------------------------------------------------
 *  Check for error conditions
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: checkErrorConditions(void)
                                                throw (CPPUNIT_NS::Exception)
{
    helixPlayer->initialize();

    bool    gotException;

    CPPUNIT_ASSERT(!helixPlayer->isPlaying());

    gotException = false;
    try {
        helixPlayer->start();
    } catch (std::logic_error &e) {
        gotException = true;
    }
    CPPUNIT_ASSERT(gotException);

    gotException = false;
    try {
        helixPlayer->stop();
    } catch (std::logic_error &e) {
        gotException = true;
    }
    CPPUNIT_ASSERT(gotException);

    gotException = false;
    try {
        helixPlayer->playThis("totally/bad/URL");
    } catch (std::invalid_argument &e) {
        gotException = true;
    }
    CPPUNIT_ASSERT(gotException);

    gotException = false;
    try {
        helixPlayer->start();
    } catch (std::logic_error &e) {
        gotException = true;
    }
    CPPUNIT_ASSERT(gotException);

    helixPlayer->deInitialize();
}


/*------------------------------------------------------------------------------
 *  Test different SMIL file features
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: smilTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    Ptr<time_duration>::Ref     sleepT(new time_duration(microseconds(10)));

    helixPlayer->initialize();

    helixPlayer->playThis("file:var/simpleSmil.smil");
    CPPUNIT_ASSERT(!helixPlayer->isPlaying());
    helixPlayer->start();
    CPPUNIT_ASSERT(helixPlayer->isPlaying());
    while (helixPlayer->isPlaying()) {
        TimeConversion::sleep(sleepT);
    }
    CPPUNIT_ASSERT(!helixPlayer->isPlaying());
}


/*------------------------------------------------------------------------------
 *  Play a specific file.
 *  Assumes that the player was already initialized.
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: playFile(const std::string   & fileName)
                                                throw (CPPUNIT_NS::Exception)
{
    Ptr<time_duration>::Ref     sleepT(new time_duration(microseconds(10)));

    helixPlayer->playThis(fileName.c_str());
    CPPUNIT_ASSERT(!helixPlayer->isPlaying());
    helixPlayer->start();
    CPPUNIT_ASSERT(helixPlayer->isPlaying());
    while (helixPlayer->isPlaying()) {
        TimeConversion::sleep(sleepT);
    }
    CPPUNIT_ASSERT(!helixPlayer->isPlaying());
}


/*------------------------------------------------------------------------------
 *  Test parallel play
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: smilParallelTest0(void)
                                                throw (CPPUNIT_NS::Exception)
{
    helixPlayer->initialize();
    playFile("file:var/parallel-0.smil");
}


/*------------------------------------------------------------------------------
 *  Test parallel play
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: smilParallelTest1(void)
                                                throw (CPPUNIT_NS::Exception)
{
    helixPlayer->initialize();
    playFile("file:var/parallel-1.smil");
}


/*------------------------------------------------------------------------------
 *  Test parallel play
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: smilParallelTest2(void)
                                                throw (CPPUNIT_NS::Exception)
{
    helixPlayer->initialize();
    playFile("file:var/parallel-2.smil");
}


/*------------------------------------------------------------------------------
 *  Test parallel play
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: smilParallelTest3(void)
                                                throw (CPPUNIT_NS::Exception)
{
    helixPlayer->initialize();
    playFile("file:var/parallel-3.smil");
}


/*------------------------------------------------------------------------------
 *  Test parallel play
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: smilParallelTest4(void)
                                                throw (CPPUNIT_NS::Exception)
{
    helixPlayer->initialize();
    playFile("file:var/parallel-4.smil");
}


/*------------------------------------------------------------------------------
 *  Test different SMIL file features
 *----------------------------------------------------------------------------*/
void
HelixPlayerTest :: smilSoundAnimationTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    Ptr<time_duration>::Ref     sleepT(new time_duration(microseconds(10)));

    helixPlayer->initialize();

    /* TODO: there is a bug with sound level animation, it causes a segfault
             see https://bugs.helixcommunity.org/show_bug.cgi?id=3310
    */
    helixPlayer->playThis("file:var/animateSound.smil");
    CPPUNIT_ASSERT(!helixPlayer->isPlaying());
    helixPlayer->start();
    CPPUNIT_ASSERT(helixPlayer->isPlaying());
    while (helixPlayer->isPlaying()) {
        TimeConversion::sleep(sleepT);
    }
    CPPUNIT_ASSERT(!helixPlayer->isPlaying());

    helixPlayer->deInitialize();
}


