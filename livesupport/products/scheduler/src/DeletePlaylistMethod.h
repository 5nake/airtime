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
    Version  : $Revision: 1.2 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/scheduler/src/Attic/DeletePlaylistMethod.h,v $

------------------------------------------------------------------------------*/
#ifndef DeletePlaylistMethod_h
#define DeletePlaylistMethod_h

#ifndef __cplusplus
#error This is a C++ include file
#endif


/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <stdexcept>
#include <string>
#include <XmlRpcServerMethod.h>
#include <XmlRpcValue.h>

#include "LiveSupport/Core/Ptr.h"
#include "LiveSupport/Core/Playlist.h"


namespace LiveSupport {
namespace Scheduler {

using namespace LiveSupport;
using namespace LiveSupport::Core;

/* ================================================================ constants */


/* =================================================================== macros */


/* =============================================================== data types */

/**
 *  An XML-RPC method object to delete a playlist given by its playlist id.
 *
 *  The name of the method when called through XML-RPC is "deletePlaylist".
 *  The expected parameter is an XML-RPC structure, with the following
 *  member:
 *  <ul>
 *      <li>playlistId - int - the unique id of the playlist 
 *                                                   to be deleted.</li>
 *  </ul>
 *
 *  The XML-RPC function returns the XML-RPC value:
 *  <ul>
 *      <li>true if the operation completed successfully,</li>
 *      <li>false if playlist is not found or there was some other error.</li>
 *  </ul>
 *
 *  @author  $Author: fgerlits $
 *  @version $Revision: 1.2 $
 */
class DeletePlaylistMethod : public XmlRpc::XmlRpcServerMethod
{
    private:
        /**
         *  The name of this method, as it will be registered into the
         *  XML-RPC server.
         */
        static const std::string        methodName;

        /**
         *  The name of the playlistId member in the XML-RPC parameter
         *  structure.
         */
        static const std::string        playlistIdName;


    public:
        /**
         *  A default constructor, for testing purposes.
         */
        DeletePlaylistMethod(void)                                 throw ()
                            : XmlRpc::XmlRpcServerMethod(methodName)
        {
        }

        /**
         *  Constuctor that registers the method with the server right away.
         *
         *  @param xmlRpcServer the XML-RPC server to register with.
         */
        DeletePlaylistMethod(
                    Ptr<XmlRpc::XmlRpcServer>::Ref xmlRpcServer)
                                                                    throw ();

        /**
         *  Execute the display schedule command on the Scheduler daemon.
         *
         *  @param parameters XML-RPC function call parameters
         *  @param returnValue the return value of the call (out parameter)
         */
        void
        execute( XmlRpc::XmlRpcValue  & parameters,
                 XmlRpc::XmlRpcValue  & returnValue)            throw ();
};


/* ================================================= external data structures */


/* ====================================================== function prototypes */


} // namespace Scheduler
} // namespace LiveSupport

#endif // DisplayPlaylistMethod_h

