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
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/scheduler/src/UpdateFadeInFadeOutMethod.h,v $

------------------------------------------------------------------------------*/
#ifndef UpdateFadeInFadeOutMethod_h
#define UpdateFadeInFadeOutMethod_h

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


namespace LiveSupport {
namespace Scheduler {

using namespace LiveSupport;
using namespace LiveSupport::Core;

/* ================================================================ constants */


/* =================================================================== macros */


/* =============================================================== data types */

/**
 *  An XML-RPC method object to change the fade in / fade out info of an 
 *  audio clip (specified by its relative offset) from a playlist (specified 
 *  by its ID).
 *
 *  The name of the method when called through XML-RPC is 
 *  "updateFadeInFadeOut".
 *
 *  The expected parameter is an XML-RPC structure, with the following
 *  members:
 *  <ul>
 *      <li>playlistId - int - the unique id of the playlist.</li>
 *      <li>relativeOffset - int - the number of seconds between the
 *                start of the playlist and the start of the audio clip
 *                to be removed.</li>
 *      <li>fadeIn  - int - the length (in milliseconds) of the fade in. </li>
 *      <li>fadeOut - int - the length (in milliseconds) of the fade out. </li>
 *  </ul>
 *
 *  In case of an error, an XML-RPC structure is returned, with the following
 *  fields:
 *  <ul>
 *      <li>errorCode - int - the id of the error condition</li>
 *      <li>errorMessage - string - a description of the error</li>
 *  </ul>
 *  The possible error codes are:
 *  <ul>
 *     <li>1601 - invalid argument format </li>
 *     <li>1602 - missing playlist ID argument </li>
 *     <li>1603 - missing relative offset argument </li>
 *     <li>1604 - missing fade in argument </li>
 *     <li>1605 - missing fade out argument </li>
 *     <li>1606 - playlist does not exist </li>
 *     <li>1607 - playlist has not been opened for editing </li>
 *     <li>1608 - no audio clip at the specified relative offset </li>
 *  </ul>
 *  @author  $Author: fgerlits $
 *  @version $Revision: 1.2 $
 */
class UpdateFadeInFadeOutMethod : public XmlRpc::XmlRpcServerMethod
{
    private:
        /**
         *  The name of this method, as it will be registered into the
         *  XML-RPC server.
         */
        static const std::string        methodName;

        /**
         *  The ID of this method for error reporting purposes.
         */
        static const int                errorId;


    public:
        /**
         *  A default constructor, for testing purposes.
         */
        UpdateFadeInFadeOutMethod(void)                             throw ()
                            : XmlRpc::XmlRpcServerMethod(methodName)
        {
        }

        /**
         *  Constuctor that registers the method with the server right away.
         *
         *  @param xmlRpcServer the XML-RPC server to register with.
         */
        UpdateFadeInFadeOutMethod(
                    Ptr<XmlRpc::XmlRpcServer>::Ref xmlRpcServer)
                                                                    throw ();

        /**
         *  Execute the display schedule command on the Scheduler daemon.
         *
         *  @param parameters XML-RPC function call parameters
         *  @param returnValue the return value of the call (out parameter)
         */
        void
        execute(XmlRpc::XmlRpcValue  & parameters,
                XmlRpc::XmlRpcValue  & returnValue)                 throw ();
};


/* ================================================= external data structures */


/* ====================================================== function prototypes */


} // namespace Scheduler
} // namespace LiveSupport

#endif // UpdateFadeInFadeOutMethod_h

