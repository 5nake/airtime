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
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/gLiveSupport/src/MasterPanelWindow.cxx,v $

------------------------------------------------------------------------------*/

/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <iostream>
#include <unicode/msgfmt.h>
#include <gtkmm/label.h>

#include "LiveSupport/Core/TimeConversion.h"
#include "MasterPanelUserInfoWidget.h"
#include "MasterPanelWindow.h"


using namespace LiveSupport::GLiveSupport;

/* ===================================================  local data structures */


/* ================================================  local constants & macros */


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Constructor.
 *----------------------------------------------------------------------------*/
MasterPanelWindow :: MasterPanelWindow (Ptr<GLiveSupport>::Ref    gLiveSupport,
                                        Ptr<ResourceBundle>::Ref  bundle)
                                                                    throw ()
                        : GtkLocalizedObject(bundle)
{
    this->gLiveSupport = gLiveSupport;

    try {
        set_title(*getResourceUstring("windowTitle"));
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
    }

    lsLogoWidget.reset(new Gtk::Label("lsLogo"));
    nowPlayingWidget.reset(new Gtk::Label("now playing"));
    vuMeterWidget.reset(new Gtk::Label("VU meter"));
    nextPlayingWidget.reset(new Gtk::Label("next playing"));
    onAirWidget.reset(new Gtk::Label("on air"));
    radioLogoWidget.reset(new Gtk::Label("radio logo"));
    userInfoWidget.reset(new MasterPanelUserInfoWidget(gLiveSupport, bundle));

    // set up the time label
    timeWidget.reset(new Gtk::Label("time"));

    // set up the layout, which is a button box
    layout.reset(new Gtk::Table());

    // set up the main window, and show everything
    set_border_width(10);
    layout->attach(*lsLogoWidget,      0, 1, 0, 2);
    layout->attach(*timeWidget,        1, 2, 0, 2);
    layout->attach(*nowPlayingWidget,  2, 3, 0, 2);
    layout->attach(*vuMeterWidget,     3, 4, 0, 1);
    layout->attach(*nextPlayingWidget, 3, 4, 1, 2);
    layout->attach(*onAirWidget      , 4, 5, 0, 1);
    layout->attach(*radioLogoWidget  , 5, 6, 0, 1);
    layout->attach(*userInfoWidget   , 4, 6, 1, 2);
    
    add(*layout);

    // show everything
    show_all();

    // set the timer, that will update timeWidget
    setTimer();
}


/*------------------------------------------------------------------------------
 *  Destructor.
 *----------------------------------------------------------------------------*/
MasterPanelWindow :: ~MasterPanelWindow (void)                        throw ()
{
    resetTimer();
}


/*------------------------------------------------------------------------------
 *  Set the timer
 *----------------------------------------------------------------------------*/
void
MasterPanelWindow :: setTimer(void)                                  throw ()
{
    sigc::slot<bool>    slot = sigc::bind(sigc::mem_fun(*this,
                                              &MasterPanelWindow::onUpdateTime),
                                              0);

    // set the timer to active once a second
    timer.reset(new sigc::connection(
                                  Glib::signal_timeout().connect(slot, 1000)));
}


/*------------------------------------------------------------------------------
 *  Clear the timer
 *----------------------------------------------------------------------------*/
void
MasterPanelWindow :: resetTimer(void)                                throw ()
{
    timer->disconnect();
    timer.reset();
}


/*------------------------------------------------------------------------------
 *  Update the timeWidget display, with the current time
 *----------------------------------------------------------------------------*/
bool
MasterPanelWindow :: onUpdateTime(int   dummy)                       throw ()
{
    Ptr<SessionId>::Ref     sessionId = gLiveSupport->getSessionId();

    if (sessionId.get()) {
        Ptr<const ptime>::Ref   now = gLiveSupport->getScheduler()
                                                  ->getSchedulerTime(sessionId);
        
        if (now.get()) {
            time_duration           dayTime = now->time_of_day();
            // get the time of day, only up to a second precision
            time_duration           dayTimeSec(dayTime.hours(),
                                               dayTime.minutes(),
                                               dayTime.seconds(),
                                               0);

            timeWidget->set_text(to_simple_string(dayTimeSec));
        }
    } else {
        timeWidget->set_text("time");
    }

    return true;
}


