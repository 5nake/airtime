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
    Version  : $Revision: 1.10 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/gLiveSupport/src/GLiveSupport.h,v $

------------------------------------------------------------------------------*/
#ifndef GLiveSupport_h
#define GLiveSupport_h

#ifndef __cplusplus
#error This is a C++ include file
#endif


/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <string>
#include <map>
#include <boost/enable_shared_from_this.hpp>
#include <unicode/resbund.h>

#include "LiveSupport/Core/Ptr.h"
#include "LiveSupport/Core/LocalizedConfigurable.h"
#include "LiveSupport/Authentication/AuthenticationClientInterface.h"
#include "LiveSupport/Storage/StorageClientInterface.h"
#include "LiveSupport/SchedulerClient/SchedulerClientInterface.h"
#include "LiveSupport/PlaylistExecutor/AudioPlayerInterface.h"

namespace LiveSupport {
namespace GLiveSupport {

using namespace LiveSupport::Core;
using namespace LiveSupport::SchedulerClient;
using namespace LiveSupport::Authentication;
using namespace LiveSupport::Storage;
using namespace LiveSupport::PlaylistExecutor;

/* ================================================================ constants */


/* =================================================================== macros */


/* =============================================================== data types */


class MasterPanelWindow;


/**
 *  The main application object for the gLiveSupport GUI.
 *
 *  The configuration file for this object is based on configuration
 *  elements used by the object. The DTD for the root element of the
 *  configuration file is:
 *
 *  <pre><code>
 *  <!ELEMENT gLiveSupport                (resourceBundle,
 *                                         authenticationClientFactory,
 *                                         storageClientFactory,
 *                                         schedulerClientFactory) >
 *  </code></pre>
 *
 *  For a description of the <code>resourceBundle</code>,
 *  <code>authenticationClientFactory</code>,
 *  <code>storageClientFactory</code> and
 *  <code>schedulerClientFactory</code> elements see their
 *  respective documentation.
 *
 *  @author $Author: maroy $
 *  @version $Revision: 1.10 $
 *  @see LocalizedObject#getBundle(const xmlpp::Element &)
 *  @see AuthenticationClientFactory
 *  @see StorageClientFactory
 *  @see SchedulerClientFactory
 */
class GLiveSupport : public LocalizedConfigurable,
                     public boost::enable_shared_from_this<GLiveSupport>
{
    public:
        /**
         *  A type for the map of supported languages.
         *  This is an STL map, containing const std::string as keys, which
         *  are the locale names of supported langauges. The values are
         *  Ptr<const UnicodeString>::Ref, the names of the languages.
         */
        typedef std::map<const std::string,
                         Ptr<const UnicodeString>::Ref>     LanguageMap;


    private:
        /**
         *  The name of the configuration XML elmenent used by Playlist.
         */
        static const std::string    configElementNameStr;

        /**
         *  The authentication client used by the application.
         */
        Ptr<AuthenticationClientInterface>::Ref     authentication;

        /**
         *  The storage client used by the application.
         */
        Ptr<StorageClientInterface>::Ref            storage;

        /**
         *  The scheduler client, used to access the scheduler daemon.
         */
        Ptr<SchedulerClientInterface>::Ref          scheduler;

        /**
         *  The audio player.
         */
        Ptr<AudioPlayerInterface>::Ref              audioPlayer;

        /**
         *  The session id for the user.
         */
        Ptr<SessionId>::Ref         sessionId;

        /**
         *  The map of supported languages.
         */
        Ptr<LanguageMap>::Ref       supportedLanguages;

        /**
         *  The master panel window.
         */
        Ptr<MasterPanelWindow>::Ref  masterPanel;

        /**
         *  Read a supportedLanguages configuration element,
         *  and fill the supportedLanguages map with its contents.
         *
         *  @param element a supportedLanguages element
         *  @exception std::invalid_argument if the supplied XML element
         *             is wrong
         */
        void
        configSupportedLanguages(const xmlpp::Element    & element)
                                                throw (std::invalid_argument);


    public:
        /**
         *  Constructor.
         */
        GLiveSupport(void)                                  throw ()
        {
        }

        /**
         *  Virtual destructor.
         */
        virtual
        ~GLiveSupport(void)                                  throw ()
        {
            if (audioPlayer.get()) {
                audioPlayer->deInitialize();
            }
        }

        /**
         *  Return the name of the XML element this object expects
         *  to be sent to a call to configure().
         *  
         *  @return the name of the expected XML configuration element.
         */
        static const std::string
        getConfigElementName(void)                          throw ()
        {
            return configElementNameStr;
        }

        /**
         *  Configure the scheduler daemon based on the XML element
         *  supplied.
         *
         *  @param element the XML element to configure the scheduler
         *                 daemon from.
         *  @exception std::invalid_argument if the supplied XML element
         *             contains bad configuraiton information
         *  @exception std::logic_error if the object has already
         *             been configured.
         */
        void
        configure(const xmlpp::Element    & element)
                                                throw (std::invalid_argument,
                                                       std::logic_error);

        /**
         *  Show the main window, and run the application.
         *  This call will only return after the main window has been closed.
         */
        void
        show(void)                                          throw ();

        /**
         *  Change the language of the application.
         *
         *  @param locale the new locale of the appliction.
         */
        void
        changeLanguage(Ptr<const std::string>::Ref  locale)     throw ();

        /**
         *  Perform authentication for the user of the application.
         *  As a result, the user will be authenticated for later
         *  actions, that need authentication.
         *  The user has to be logged out later.
         *
         *  @param login the login ID of the user
         *  @param password the password of the user
         *  @return true if the authentication was successful,
         *          false otherwise
         *  @see #logout
         */
        bool
        login(const std::string & login,
              const std::string & password)          throw ();

        /**
         *  Return the session id for the user.
         *
         *  @return the session id for the user, if he has been already
         *          successfully authenticated with a call to login(),
         *          and not yet logged out.
         *          otherwise, a reference to 0.
         *  @see #login
         */
        Ptr<SessionId>::Ref
        getSessionId(void) const                            throw ()
        {
            return sessionId;
        }

        /**
         *  Log out the user, if he has been authenticated previously
         *  with a successful call to login().
         *
         *  @see #logout
         */
        void
        logout(void)                                    throw ();

        /**
         *  Accessor function to the scheduler client held by this object.
         *
         *  @return the scheduler client held by this object.
         */
        Ptr<SchedulerClientInterface>::Ref
        getScheduler(void)                              throw ()
        {
            return scheduler;
        }

        /**
         *  Accessor function to the storage client held by this object.
         *
         *  @return the storage client held by this object.
         */
        Ptr<StorageClientInterface>::Ref
        getStorage(void)                                throw ()
        {
            return storage;
        }

        /**
         *  Get the map of supported languages.
         *
         *  @return the map of supported languages.
         */
        Ptr<const LanguageMap>::Ref
        getSupportedLanguages(void) const               throw ()
        {
            return supportedLanguages;
        }

        /**
         *  Show the UI components that are visible when no one is logged in.
         */
        void
        showAnonymousUI(void)                                   throw ();

        /**
         *  Show the UI components that are visible when someone is logged in.
         */
        void
        showLoggedInUI(void)                                    throw ();

        /**
         *  Upload a file to the storage.
         *
         *  @param title the title of the audio clip.
         *  @param fileName the full filename of the audio clip.
         *  @exception StorageException on upload failures.
         */
        void
        uploadFile(Ptr<const Glib::ustring>::Ref    title,
                   Ptr<const std::string>::Ref      fileName)
                                                    throw (StorageException);

};

/* ================================================= external data structures */


/* ====================================================== function prototypes */


} // namespace GLiveSupport
} // namespace LiveSupport

#endif // GLiveSupport_h

