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
    Version  : $Revision: 1.3 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/gLiveSupport/src/MasterPanelUserInfoWidget.cxx,v $

------------------------------------------------------------------------------*/

/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <iostream>
#include <unicode/msgfmt.h>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>

#include "LiveSupport/Core/TimeConversion.h"
#include "LoginWindow.h"
#include "MasterPanelUserInfoWidget.h"


using namespace LiveSupport::GLiveSupport;

/* ===================================================  local data structures */


/* ================================================  local constants & macros */


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Constructor.
 *----------------------------------------------------------------------------*/
MasterPanelUserInfoWidget :: MasterPanelUserInfoWidget (
                                        Ptr<GLiveSupport>::Ref    gLiveSupport,
                                        Ptr<ResourceBundle>::Ref  bundle)
                                                                    throw ()
                        : LocalizedObject(bundle)
{
    this->gLiveSupport = gLiveSupport;
    loggedIn           = false;

    logInOutButton.reset(new Gtk::Button());
    logInOutSignalConnection =
                logInOutButton->signal_clicked().connect(sigc::mem_fun(*this,
                            &MasterPanelUserInfoWidget::onLoginButtonClicked));

    userInfoLabel.reset(new Gtk::Label());

    changeLanguage(bundle);

    // set up the main window, and show everything
    attach(*logInOutButton,    0, 1, 0, 1);
    attach(*userInfoLabel,     1, 2, 0, 1);
    
    // show everything
    show_all();
}


/*------------------------------------------------------------------------------
 *  Destructor.
 *----------------------------------------------------------------------------*/
MasterPanelUserInfoWidget :: ~MasterPanelUserInfoWidget (void)      throw ()
{
}


/*------------------------------------------------------------------------------
 *  Event handler for the logout button getting clicked.
 *----------------------------------------------------------------------------*/
void
MasterPanelUserInfoWidget :: onLogoutButtonClicked (void)           throw ()
{
    gLiveSupport->logout();

    loggedIn = false;
    login.reset();

    Ptr<Glib::ustring>::Ref     notLoggedInMsg;
    Ptr<Glib::ustring>::Ref     loginButtonLabel;

    try {
        loginButtonLabel = getResourceUstring("loginButtonLabel");
        notLoggedInMsg   = getResourceUstring("notLoggedInMsg");
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    userInfoLabel->set_label(*notLoggedInMsg);

    // change the logout button to a login button
    logInOutButton->set_label(*loginButtonLabel);
    logInOutSignalConnection.disconnect();
    logInOutSignalConnection =
                logInOutButton->signal_clicked().connect(sigc::mem_fun(*this,
                            &MasterPanelUserInfoWidget::onLoginButtonClicked));

    // show only the anonymous UI
    gLiveSupport->showAnonymousUI();
}


/*------------------------------------------------------------------------------
 *  Event handler for the login button getting clicked.
 *----------------------------------------------------------------------------*/
void
MasterPanelUserInfoWidget :: onLoginButtonClicked (void)            throw ()
{
    Ptr<ResourceBundle>::Ref    loginBundle;
    try {
        loginBundle       = getBundle("loginWindow");
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    Ptr<LoginWindow>::Ref       loginWindow(new LoginWindow(gLiveSupport,
                                                            loginBundle));

    Gtk::Main::run(*loginWindow);

    Ptr<const Glib::ustring>::Ref     password = loginWindow->getPassword();

    login    = loginWindow->getLogin();
    loggedIn = gLiveSupport->login(login->raw(), password->raw());

    if (loggedIn) {
        updateStrings();

        // change the login button to a logout button
        logInOutSignalConnection.disconnect();
        logInOutSignalConnection =
                logInOutButton->signal_clicked().connect(sigc::mem_fun(*this,
                            &MasterPanelUserInfoWidget::onLogoutButtonClicked));

        Ptr<const std::string>::Ref   locale = loginWindow->getSelectedLocale();
        if (locale->size() > 0) {
            gLiveSupport->changeLanguage(locale);
        } else {
            // TODO: get and set default locale for user
        }
        gLiveSupport->showLoggedInUI();
    }
}


/*------------------------------------------------------------------------------
 *  Change the language of the panel
 *----------------------------------------------------------------------------*/
void
MasterPanelUserInfoWidget :: changeLanguage(Ptr<ResourceBundle>::Ref    bundle)
                                                                    throw ()
{
    setBundle(bundle);
    updateStrings();
}


/*------------------------------------------------------------------------------
 *  Update the strings in the widget.
 *----------------------------------------------------------------------------*/
void
MasterPanelUserInfoWidget :: updateStrings(void)
                                                  throw (std::invalid_argument)
{
    Ptr<Glib::ustring>::Ref     loggedInMsg;
    Ptr<Glib::ustring>::Ref     loginButtonLabel;

    if (!loggedIn) {
        loginButtonLabel = getResourceUstring("loginButtonLabel");
        loggedInMsg      = getResourceUstring("notLoggedInMsg");
    } else {
        Ptr<UnicodeString>::Ref uLogin = ustringToUnicodeString(login);
        Formattable             arguments[] = { *uLogin };
        loggedInMsg = formatMessageUstring("loggedInMsg", arguments, 1);

        loginButtonLabel = getResourceUstring("logoutButtonLabel");
    }

    userInfoLabel->set_label(*loggedInMsg);
    logInOutButton->set_label(*loginButtonLabel);
}


