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
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/scheduler/src/RemoveFromScheduleMethod.cxx,v $

------------------------------------------------------------------------------*/

/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#else
#error need time.h
#endif


#include <string>

#include "ScheduleInterface.h"
#include "ScheduleFactory.h"
#include "XmlRpcTools.h"

#include "RemoveFromScheduleMethod.h"


using namespace LiveSupport;
using namespace LiveSupport::Core;

using namespace LiveSupport::Scheduler;

/* ===================================================  local data structures */


/* ================================================  local constants & macros */

/*------------------------------------------------------------------------------
 *  The name of this XML-RPC method.
 *----------------------------------------------------------------------------*/
const std::string RemoveFromScheduleMethod::methodName = "removeFromSchedule";


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Construct the method and register it right away.
 *----------------------------------------------------------------------------*/
RemoveFromScheduleMethod :: RemoveFromScheduleMethod (
                        Ptr<XmlRpc::XmlRpcServer>::Ref xmlRpcServer)   throw()
    : XmlRpc::XmlRpcServerMethod(methodName, xmlRpcServer.get())
{
}


/*------------------------------------------------------------------------------
 *  Execute the remove from schedule XML-RPC function call.
 *----------------------------------------------------------------------------*/
void
RemoveFromScheduleMethod :: execute(XmlRpc::XmlRpcValue  & parameters,
                                    XmlRpc::XmlRpcValue  & returnValue)
                                                                    throw ()
{
    try {
        if (!parameters.valid()) {
            // TODO: mark error
            returnValue = XmlRpc::XmlRpcValue(false);
            return;
        }

        Ptr<UniqueId>::Ref  entryId 
                          = XmlRpcTools::extractScheduleEntryId(parameters[0]);

        Ptr<ScheduleFactory>::Ref   sf = ScheduleFactory::getInstance();
        Ptr<ScheduleInterface>::Ref schedule = sf->getSchedule();

        if (!schedule->scheduleEntryExists(entryId)) {
            // TODO: mark error;
            returnValue = XmlRpc::XmlRpcValue(false);
            return;
        }

        schedule->removeFromSchedule(entryId);

    } catch (std::invalid_argument &e) {
        // TODO: mark error
        returnValue = XmlRpc::XmlRpcValue(false);
        return;
    }

    returnValue = XmlRpc::XmlRpcValue(true);
}

