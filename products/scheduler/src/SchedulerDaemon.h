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
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/scheduler/src/SchedulerDaemon.h,v $

------------------------------------------------------------------------------*/
#ifndef SchedulerDaemon_h
#define SchedulerDaemon_h

#ifndef __cplusplus
#error This is a C++ include file
#endif


/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#else
#error "Need sys/types.h"
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#error "Need unistd.h"
#endif

#include <string>
#include <stdexcept>
#include <libxml++/libxml++.h>
#include <XmlRpc.h>

#include "LiveSupport/Core/Ptr.h"
#include "LiveSupport/Core/Installable.h"
#include "LiveSupport/Core/Configurable.h"
#include "LiveSupport/PlaylistExecutor/AudioPlayerInterface.h"
#include "LiveSupport/EventScheduler/EventScheduler.h"
#include "GetVersionMethod.h"
#include "UploadPlaylistMethod.h"
#include "DisplayScheduleMethod.h"
#include "DisplayPlaylistMethod.h"
#include "RemoveFromScheduleMethod.h"
#include "RescheduleMethod.h"
#include "AddAudioClipToPlaylistMethod.h"
#include "OpenPlaylistForEditingMethod.h"
#include "XmlRpcDaemon.h"


namespace LiveSupport {
namespace Scheduler {

using namespace LiveSupport;
using namespace LiveSupport::Core;
using namespace LiveSupport::PlaylistExecutor;

/* ================================================================ constants */


/* =================================================================== macros */


/* =============================================================== data types */

/**
 *  Scheduler daemon main class.
 *  This class is responsible for starting, running and stopping the
 *  Scheduler daemon.
 *
 *  The SchedulerDaemon has to configured by an XML element called
 *  scheduler. This element contains configuration elements for the
 *  compontents used by the scheduler. The configuration file looks
 *  like the following:
 *
 *  <pre><code>
 *  &lt;scheduler&gt;
 *      &lt;connectionManagerFactory&gt;
 *          ...
 *      &lt;/connectionManagerFactory&gt;
 *      &lt;storageClientFactory&gt;
 *          ...
 *      &lt;/storageClientFactory&gt;
 *      &lt;scheduleFactory&gt;
 *          ...
 *      &lt;/scheduleFactory&gt;
 *      &lt;xmlRpcDaemon&gt;
 *          ...
 *      &lt;/xmlRpcDaemon&gt;
 *  &lt;/scheduler&gt;
 *  </code></pre>
 *
 *  For details on the included elements, see the corresponding documentation
 *  for XmlRpcDaemon, StorageClientFactory, ConnectionManagerFactory
 *  and ScheduleFactory.
 *
 *  The DTD for the above element is the following:
 *
 *  <pre><code>
 *  &lt;!ELEMENT scheduler (connectionManagerFactory,storageClientFactory,
 *                          scheduleFactory,xmlRpcDaemon) &gt;
 *  </code></pre>
 *
 *  @author  $Author: maroy $
 *  @version $Revision: 1.10 $
 *  @see ConnectionManagerFactory
 *  @see StorageClientFactory
 *  @see ScheduleFactory
 *  @see XmlRpcDaemon
 */
class SchedulerDaemon : public Installable,
                        public Configurable,
                        public XmlRpcDaemon
{
    private:

        /**
         *  The singleton instance of the scheduler daemon.
         */
        static Ptr<SchedulerDaemon>::Ref    schedulerDaemon;

        /**
         *  The event scheduler.
         */
        Ptr<EventScheduler::EventScheduler>::Ref        eventScheduler;

        /**
         *  The audio player.
         */
        Ptr<AudioPlayerInterface>::Ref      audioPlayer;

        /**
         *  The getVersion the daemon is providing.
         */
        Ptr<GetVersionMethod>::Ref          getVersionMethod;

        /**
         *  The uploadPlaylistMethod the daemon is providing.
         */
        Ptr<UploadPlaylistMethod>::Ref      uploadPlaylistMethod;

        /**
         *  The displayScheduleMethod the daemon is providing.
         */
        Ptr<DisplayScheduleMethod>::Ref     displayScheduleMethod;

        /**
         *  The displayPlaylistMethod the daemon is providing.
         */
        Ptr<DisplayPlaylistMethod>::Ref     displayPlaylistMethod;

        /**
         *  The removeFromScheduleMethod the daemon is providing.
         */
        Ptr<RemoveFromScheduleMethod>::Ref  removeFromScheduleMethod;

        /**
         *  The rescheduleMethod the daemon is providing.
         */
        Ptr<RescheduleMethod>::Ref  rescheduleMethod;

        /**
         *  The addAudioClipToPlaylistMethod the daemon is providing.
         */
        Ptr<AddAudioClipToPlaylistMethod>::Ref  addAudioClipToPlaylistMethod;

        /**
         *  The openPlaylistForEditingMethod the daemon is providing.
         */
        Ptr<OpenPlaylistForEditingMethod>::Ref  openPlaylistForEditingMethod;

        /**
         *  Default constructor.
         */
        SchedulerDaemon (void)                          throw ();


    protected:

        /**
         *  Register your XML-RPC functions by implementing this function.
         */
        virtual void
        registerXmlRpcFunctions(Ptr<XmlRpc::XmlRpcServer>::Ref  xmlRpcServer)
                                                    throw (std::logic_error);

        /**
         *  Execute any calls when the daemon is starting up.
         *  All resources allocated here should be freed up in shutdown().
         *
         *  @see #shutdown
         */
        virtual void
        startup (void)                                      throw ();


    public:

        /**
         *  Virtual destructor.
         */
        virtual
        ~SchedulerDaemon(void)                          throw ()
        {
        }

        /**
         *  Return a pointer to the singleton instance of SchedulerDaemon.
         *
         *  @return a pointer to the singleton instance of SchedulerDaemon
         */
        static Ptr<SchedulerDaemon>::Ref
        getInstance (void)                              throw ();

        /**
         *  Configure the scheduler daemon based on the XML element
         *  supplied.
         *
         *  @param element the XML element to configure the scheduler
         *                 daemon from.
         *  @exception std::invalid_argument if the supplied XML element
         *             contains bad configuraiton information
         *  @exception std::logic_error if the scheduler daemon has already
         *             been configured.
         */
        void
        configure(const xmlpp::Element    & element)
                                                throw (std::invalid_argument,
                                                       std::logic_error);

        /**
         *  Install the component.
         *  This step involves creating the environment in which the component
         *  will run. This may be creation of coniguration files,
         *  database tables, etc.
         *
         *  @exception std::exception on installation problems.
         */
        virtual void
        install(void)                           throw (std::exception);

        /**
         *  Uninstall the component.
         *  Removes all the resources created in the install step.
         *
         *  @exception std::exception on unistallation problems.
         */
        virtual void
        uninstall(void)                         throw (std::exception);

        /**
         *  Shut down the daemon.
         *  This function is public only because the signal handler
         *  needs visibility to this function, which will call it.
         *  A call to stop() will trigger a signal that will call shutdown().
         *
         *  @exception std::logic_error if the daemon has not
         *             yet been configured.
         */
        virtual void
        shutdown (void)                             throw (std::logic_error);
};


/* ================================================= external data structures */


/* ====================================================== function prototypes */


} // namespace Scheduler
} // namespace LiveSupport

#endif // SchedulerDaemon_h

