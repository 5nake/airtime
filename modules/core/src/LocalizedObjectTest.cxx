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
    Version  : $Revision: 1.1 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/modules/core/src/LocalizedObjectTest.cxx,v $

------------------------------------------------------------------------------*/

/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <string>
#include <iostream>
#include <unicode/resbund.h>

#include "LiveSupport/Core/LocalizedObject.h"
#include "LocalizedObjectTest.h"


using namespace LiveSupport::Core;

/* ===================================================  local data structures */


/* ================================================  local constants & macros */

CPPUNIT_TEST_SUITE_REGISTRATION(LocalizedObjectTest);


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Set up the test environment
 *----------------------------------------------------------------------------*/
void
LocalizedObjectTest :: setUp(void)                         throw ()
{
}


/*------------------------------------------------------------------------------
 *  Clean up the test environment
 *----------------------------------------------------------------------------*/
void
LocalizedObjectTest :: tearDown(void)                      throw ()
{
}


/*------------------------------------------------------------------------------
 *  A simple smoke test.
 *----------------------------------------------------------------------------*/
void
LocalizedObjectTest :: simpleTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    UErrorCode                status = U_ZERO_ERROR;
    Ptr<ResourceBundle>::Ref  bundle(new ResourceBundle("./tmp/" PACKAGE_NAME,
                                                        "root",
                                                        status));
    CPPUNIT_ASSERT(U_SUCCESS(status));

    try {
        Ptr<LocalizedObject>::Ref   locObj(new LocalizedObject(bundle));
        Ptr<LocalizedObject>::Ref   section1(new LocalizedObject(
                                                locObj->getBundle("section1")));
        Ptr<UnicodeString>::Ref     foo = section1->getResourceString("foo");
        CPPUNIT_ASSERT(foo->compare("foo") == 0);
    } catch (std::invalid_argument &e) {
        CPPUNIT_FAIL(e.what());
    }
}


/*------------------------------------------------------------------------------
 *  A test to see different locales and fallback to work.
 *----------------------------------------------------------------------------*/
void
LocalizedObjectTest :: fallbackTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    UErrorCode                status = U_ZERO_ERROR;
    Ptr<ResourceBundle>::Ref  rootBundle;
    Ptr<ResourceBundle>::Ref  enBundle;

    rootBundle.reset(new ResourceBundle("./tmp/" PACKAGE_NAME, "root", status));
    CPPUNIT_ASSERT(U_SUCCESS(status));
    enBundle.reset(new ResourceBundle("./tmp/" PACKAGE_NAME, "en", status));
    CPPUNIT_ASSERT(U_SUCCESS(status));

    // first, see the root bundle
    try {
        Ptr<LocalizedObject>::Ref   locObj(new LocalizedObject(rootBundle));
        Ptr<LocalizedObject>::Ref   section1(new LocalizedObject(
                                                locObj->getBundle("section1")));
        Ptr<UnicodeString>::Ref     foo = section1->getResourceString("foo");
        CPPUNIT_ASSERT(foo->compare("foo") == 0);
        Ptr<UnicodeString>::Ref     bar = section1->getResourceString("bar");
        CPPUNIT_ASSERT(bar->compare("bar") == 0);
    } catch (std::invalid_argument &e) {
        CPPUNIT_FAIL(e.what());
    }

    // now, the en bundle
    try {
        Ptr<LocalizedObject>::Ref   locObj(new LocalizedObject(enBundle));
        Ptr<LocalizedObject>::Ref   section1(new LocalizedObject(
                                                locObj->getBundle("section1")));
        Ptr<UnicodeString>::Ref     foo = section1->getResourceString("foo");
        CPPUNIT_ASSERT(foo->compare("fou") == 0);
        Ptr<UnicodeString>::Ref     bar = section1->getResourceString("bar");
        CPPUNIT_ASSERT(bar->compare("bar") == 0);
    } catch (std::invalid_argument &e) {
        CPPUNIT_FAIL(e.what());
    }
}


/*------------------------------------------------------------------------------
 *  A test to see funny unicode characters to work.
 *----------------------------------------------------------------------------*/
void
LocalizedObjectTest :: unicodeTest(void)
                                                throw (CPPUNIT_NS::Exception)
{
    UErrorCode                status = U_ZERO_ERROR;
    Ptr<ResourceBundle>::Ref  huBundle;
    Ptr<ResourceBundle>::Ref  jpBundle;

    huBundle.reset(new ResourceBundle("./tmp/" PACKAGE_NAME, "hu", status));
    CPPUNIT_ASSERT(U_SUCCESS(status));
    try {
        Ptr<LocalizedObject>::Ref   locObj(new LocalizedObject(huBundle));
        Ptr<LocalizedObject>::Ref   section1(new LocalizedObject(
                                                locObj->getBundle("section1")));
        Ptr<UnicodeString>::Ref     foo = section1->getResourceString("foo");
        CPPUNIT_ASSERT(foo->charAt(0) == 0x0066);  // 'f'
        CPPUNIT_ASSERT(foo->charAt(1) == 0x00fa);  // 'u' with acute
        Ptr<UnicodeString>::Ref     bar = section1->getResourceString("bar");
        CPPUNIT_ASSERT(bar->charAt(0) == 0x0062);  // 'b'
        CPPUNIT_ASSERT(bar->charAt(1) == 0x00e1);  // 'a' with acute
        CPPUNIT_ASSERT(bar->charAt(2) == 0x0072);  // 'r'
    } catch (std::invalid_argument &e) {
        CPPUNIT_FAIL(e.what());
    }

    jpBundle.reset(new ResourceBundle("./tmp/" PACKAGE_NAME, "jp", status));
    CPPUNIT_ASSERT(U_SUCCESS(status));
    try {
        Ptr<LocalizedObject>::Ref   locObj(new LocalizedObject(jpBundle));
        Ptr<LocalizedObject>::Ref   section1(new LocalizedObject(
                                                locObj->getBundle("section1")));
        Ptr<UnicodeString>::Ref     foo = section1->getResourceString("foo");
        CPPUNIT_ASSERT(foo->charAt(0) == 0x3075);  // hiragana fu
        Ptr<UnicodeString>::Ref     bar = section1->getResourceString("bar");
        CPPUNIT_ASSERT(bar->charAt(0) == 0x3070);  // hiragana ba
        CPPUNIT_ASSERT(bar->charAt(1) == 0x308b);  // hiragana ru
    } catch (std::invalid_argument &e) {
        CPPUNIT_FAIL(e.what());
    }
}

