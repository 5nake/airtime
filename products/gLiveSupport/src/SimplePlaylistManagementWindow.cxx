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
    Version  : $Revision: 1.24 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/gLiveSupport/src/SimplePlaylistManagementWindow.cxx,v $

------------------------------------------------------------------------------*/

/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <iostream>
#include <stdexcept>

#include "LiveSupport/Core/TimeConversion.h"

#include "SimplePlaylistManagementWindow.h"


using namespace Glib;

using namespace LiveSupport::Core;
using namespace LiveSupport::Widgets;
using namespace LiveSupport::GLiveSupport;

/* ===================================================  local data structures */


/* ================================================  local constants & macros */


/* ===============================================  local function prototypes */


/* =============================================================  module code */

/*------------------------------------------------------------------------------
 *  Constructor.
 *----------------------------------------------------------------------------*/
SimplePlaylistManagementWindow :: SimplePlaylistManagementWindow (
                                    Ptr<GLiveSupport>::Ref      gLiveSupport,
                                    Ptr<ResourceBundle>::Ref    bundle)
                                                                    throw ()
          : WhiteWindow(WidgetFactory::playlistsWindowTitleImage,
                        Colors::White,
                        WidgetFactory::getInstance()->getWhiteWindowCorners()),
            LocalizedObject(bundle),
            gLiveSupport(gLiveSupport),
            isPlaylistModified(false)
{
    Ptr<WidgetFactory>::Ref     wf = WidgetFactory::getInstance();
    
    try {
        set_title(*getResourceUstring("windowTitle"));
        nameLabel = Gtk::manage(new Gtk::Label(
                                    *getResourceUstring("nameLabel")));
        saveButton = Gtk::manage(wf->createButton(
                                    *getResourceUstring("saveButtonLabel")));
        closeButton = Gtk::manage(wf->createButton(
                                    *getResourceUstring("closeButtonLabel")));
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    nameEntry             = Gtk::manage(wf->createEntryBin());
    nameEntry->signal_changed().connect(sigc::mem_fun(
                    *this, &SimplePlaylistManagementWindow::onTitleEdited ));

    entriesScrolledWindow = Gtk::manage(new Gtk::ScrolledWindow());
    entriesModel          = Gtk::ListStore::create(modelColumns);
    entriesView           = Gtk::manage(wf->createTreeView(entriesModel));

    // set up the entry scrolled window, with the entry treeview inside.
    entriesScrolledWindow->add(*entriesView);
    entriesScrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC,
                                      Gtk::POLICY_AUTOMATIC);

    // Add the TreeView's view columns:
    try {
        entriesView->appendColumn(*getResourceUstring("startColumnLabel"),
                                   modelColumns.startColumn,
                                   60);
        entriesView->appendColumn(*getResourceUstring("titleColumnLabel"),
                                   modelColumns.titleColumn,
                                   200);
        entriesView->appendEditableColumn(
                                  *getResourceUstring("fadeInColumnLabel"),
                                   modelColumns.fadeInColumn,
                                   fadeInColumnId,
                                   60);
        entriesView->appendColumn(*getResourceUstring("lengthColumnLabel"),
                                   modelColumns.lengthColumn,
                                   60);
        entriesView->appendEditableColumn(
                                  *getResourceUstring("fadeOutColumnLabel"),
                                   modelColumns.fadeOutColumn,
                                   fadeOutColumnId,
                                   60);

        statusBar = Gtk::manage(new Gtk::Label(""));
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    entriesView->signalCellEdited().connect(sigc::mem_fun(
                    *this, &SimplePlaylistManagementWindow::onFadeInfoEdited ));

    // construct the "lock fades" check button
    Ptr<Glib::ustring>::Ref     lockFadesCheckButtonLabel;
    try {
        lockFadesCheckButtonLabel = getResourceUstring(
                                                "lockFadesCheckButtonLabel");
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }
    Gtk::CheckButton *  lockFadesCheckButton = Gtk::manage(new Gtk::CheckButton(
                                                *lockFadesCheckButtonLabel ));
    lockFadesCheckButton->set_active(true);
    areFadesLocked = true;
    lockFadesCheckButton->signal_toggled().connect(sigc::mem_fun(
            *this, 
            &SimplePlaylistManagementWindow::onLockFadesCheckButtonClicked ));
        
    // set up the layout
    Gtk::VBox *         mainBox = Gtk::manage(new Gtk::VBox);
    
    Gtk::HBox *         nameBox = Gtk::manage(new Gtk::HBox);
    nameBox->pack_start(*nameLabel, Gtk::PACK_SHRINK, 10);
    Gtk::Alignment *    nameEntryAlignment = Gtk::manage(new Gtk::Alignment(
                                        Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER,
                                        0.7));  // take up 70% of available room
    nameEntryAlignment->add(*nameEntry);
    nameBox->pack_start(*nameEntryAlignment, Gtk::PACK_EXPAND_WIDGET, 5);
    mainBox->pack_start(*nameBox, Gtk::PACK_SHRINK, 5);

    mainBox->pack_start(*entriesScrolledWindow, Gtk::PACK_EXPAND_WIDGET, 5);
    
    mainBox->pack_start(*lockFadesCheckButton, Gtk::PACK_SHRINK, 5);

    Gtk::ButtonBox *    buttonBox = Gtk::manage(new Gtk::HButtonBox(
                                                        Gtk::BUTTONBOX_END, 5));
    buttonBox->pack_start(*saveButton);
    buttonBox->pack_start(*closeButton);
    mainBox->pack_start(*buttonBox, Gtk::PACK_SHRINK, 0);
    
    Gtk::Alignment *    statusBarAlignment = Gtk::manage(new Gtk::Alignment(
                                        Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER,
                                        0.0));  // do not expand the label
    statusBarAlignment->add(*statusBar);
    mainBox->pack_start(*statusBarAlignment, Gtk::PACK_SHRINK, 5);

    add(*mainBox);

    // Register the signal handlers for the buttons
    saveButton->signal_clicked().connect(sigc::mem_fun(*this,
                        &SimplePlaylistManagementWindow::onSaveButtonClicked));
    closeButton->signal_clicked().connect(sigc::mem_fun(*this,
                        &SimplePlaylistManagementWindow::onCloseButtonClicked));

    // show
    set_name("simplePlaylistManagementWindow");
    set_default_size(480, 350);
    set_modal(false);
    property_window_position().set_value(Gtk::WIN_POS_NONE);

    show_all();

    Ptr<Glib::ustring>::Ref     confirmationMessage;
    try {
        confirmationMessage.reset(new Glib::ustring(
                                *getResourceUstring("savePlaylistDialogMsg") ));
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    dialogWindow.reset(new DialogWindow(confirmationMessage,
                                        DialogWindow::cancelButton |
                                        DialogWindow::noButton |
                                        DialogWindow::yesButton,
                                        gLiveSupport->getBundle() ));

    gLiveSupport->signalEditedPlaylistModified().connect(sigc::mem_fun(
            *this, &SimplePlaylistManagementWindow::onPlaylistModified ));
}


/*------------------------------------------------------------------------------
 *  Destructor.
 *----------------------------------------------------------------------------*/
SimplePlaylistManagementWindow :: ~SimplePlaylistManagementWindow (void)
                                                                    throw ()
{
}


/*------------------------------------------------------------------------------
 *  Save the edited playlist.
 *----------------------------------------------------------------------------*/
bool
SimplePlaylistManagementWindow :: savePlaylist (void)               throw ()
{
    try {
        Ptr<Playlist>::Ref              playlist
                                        = gLiveSupport->getEditedPlaylist();
        if (!playlist) {
            return false;
        }

        Ptr<const Glib::ustring>::Ref   title(new Glib::ustring(
                                                    nameEntry->get_text()));
        if (*title == "") {
            statusBar->set_text(*getResourceUstring("emptyTitleErrorMsg"));
            return false;
        }
        
        playlist->setTitle(title);
        gLiveSupport->savePlaylist();
        gLiveSupport->openPlaylistForEditing(playlist->getId());
        isPlaylistModified = false;

        Ptr<Glib::ustring>::Ref statusText = formatMessage(
                                                    "playlistSavedMsg",
                                                    *playlist->getTitle());
        statusBar->set_text(*statusText);
        return true;
        
    } catch (XmlRpcException &e) {
        statusBar->set_text(e.what());
        return false;
    }
}


/*------------------------------------------------------------------------------
 *  Signal handler for the save button getting clicked.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: onSaveButtonClicked(void)         throw ()
{
    savePlaylist();
}


/*------------------------------------------------------------------------------
 *  Cancel the edited playlist (no questions asked).
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: cancelPlaylist(void)        throw ()
{
    gLiveSupport->cancelEditedPlaylist();
    closeWindow();
}


/*------------------------------------------------------------------------------
 *  Signal handler for the close button getting clicked.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: onCloseButtonClicked(void)        throw ()
{
    if (gLiveSupport->getEditedPlaylist()) {
        if (!isPlaylistModified) {
            cancelPlaylist();
        } else {
            DialogWindow::ButtonType    result = dialogWindow->run();
            switch (result) {
                case DialogWindow::noButton:        cancelPlaylist();
                                                    break;

                case DialogWindow::yesButton:       if (savePlaylist()) {
                                                        closeWindow();
                                                    }
                                                    break;

                case DialogWindow::cancelButton:    break;

                default :                           break;
                                        // can happen if window is closed
            }                           //   with Alt-F4 -- treated as cancel
        }
    } else {
        closeWindow();
    }
}


/*------------------------------------------------------------------------------
 *  Clean and close the window.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: closeWindow(void)                 throw ()
{
    statusBar->set_text("");
    nameEntry->set_text("");
    entriesModel->clear();
    isPlaylistModified = false;
    hide();
}


/*------------------------------------------------------------------------------
 *  Signal handler for the "lock fades" check button toggled.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: onLockFadesCheckButtonClicked(void)
                                                                    throw ()
{
    areFadesLocked = !areFadesLocked;
}


/*------------------------------------------------------------------------------
 *  Show the contents of the currently edited playlist.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: showContents(void)                throw ()
{
    Ptr<Playlist>::Ref          playlist;
    Playlist::const_iterator    it;
    Playlist::const_iterator    end;
    int                         rowNumber;

    playlist = gLiveSupport->getEditedPlaylist();
    
    if (playlist) {
        nameEntry->set_text(*playlist->getTitle());
        entriesModel->clear();
        rowNumber = 0;
        for (it = playlist->begin(); it != playlist->end(); ++it) {
            Ptr<PlaylistElement>::Ref  
                                    playlistElem  = it->second;
            Ptr<Playable>::Ref      playable      = playlistElem->getPlayable();
            Gtk::TreeModel::Row     row           = *(entriesModel->append());
    
            row[modelColumns.rowNumberColumn]
                        = rowNumber++;
            row[modelColumns.idColumn]
                        = playable->getId();
            row[modelColumns.startColumn]
                        = *TimeConversion::timeDurationToHhMmSsString(
                                            playlistElem->getRelativeOffset());
            row[modelColumns.titleColumn]
                        = Glib::Markup::escape_text(*playable->getTitle());
            row[modelColumns.lengthColumn]
                        = *TimeConversion::timeDurationToHhMmSsString(
                                            playable->getPlaylength());

            Ptr<FadeInfo>::Ref      fadeInfo = playlistElem->getFadeInfo();
            Ptr<time_duration>::Ref fadeIn, fadeOut;
            if (fadeInfo) {
                fadeIn  = fadeInfo->getFadeIn();
                fadeOut = fadeInfo->getFadeOut();
            }
            row[modelColumns.fadeInColumn]
                        = (fadeIn && fadeIn->total_microseconds() != 0)
                          ? *TimeConversion::timeDurationToHhMmSsString(fadeIn)
                          : "-   ";
            row[modelColumns.fadeOutColumn]
                        = (fadeOut && fadeOut->total_microseconds() != 0)
                          ? *TimeConversion::timeDurationToHhMmSsString(fadeOut)
                          : "-   ";
        }
    }
}


/*------------------------------------------------------------------------------
 *  Signal handler for the fade info being edited.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: onTitleEdited(void)               throw()
{
    Ptr<Playlist>::Ref          playlist = gLiveSupport->getEditedPlaylist();
    if (!playlist) {
        playlist = gLiveSupport->openPlaylistForEditing();
    }
    Ptr<Glib::ustring>::Ref     title(new Glib::ustring(
                                                    nameEntry->get_text()));
    if (*title != *playlist->getTitle()) {
        playlist->setTitle(title);
        isPlaylistModified = true;
    }
    
    showContents();
}


/*------------------------------------------------------------------------------
 *  Signal handler for the fade info being edited.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: onFadeInfoEdited(
                                        const Glib::ustring &  pathString,
                                        int                    columnId,
                                        const Glib::ustring &  newText)
                                                                    throw()
{
    Gtk::TreeModel::Path        path(pathString);
    std::vector<int>            rowNumberVector = path.get_indices();
    int                         rowNumber = rowNumberVector.at(0);
    
    Ptr<time_duration>::Ref     newTime;
    try {
        newTime.reset(new time_duration(duration_from_string(newText)));
    } catch (boost::bad_lexical_cast &e) {
        showContents();         // bad time format; restore previous state
        return;
    }
    
    Ptr<Playlist>::Ref          playlist = gLiveSupport->getEditedPlaylist();
    Playlist::const_iterator    iter = playlist->begin();
    for (int i=0; i<rowNumber; ++i) {
        ++iter;
    }
    // TODO: add an at(n) access function to Playlist
    Ptr<PlaylistElement>::Ref   playlistElement = iter->second;
    
    switch (columnId) {
        case fadeInColumnId :
            setFadeIn(playlistElement, newTime);
            if (areFadesLocked && iter-- != playlist->begin()) {
                Ptr<PlaylistElement>::Ref   prevPlaylistElement = iter->second;
                setFadeOut(prevPlaylistElement, newTime);
            }
            break;
        case fadeOutColumnId :
            setFadeOut(playlistElement, newTime);
            if (areFadesLocked && ++iter != playlist->end()) {
                Ptr<PlaylistElement>::Ref   nextPlaylistElement = iter->second;
                setFadeIn(nextPlaylistElement, newTime);
            }
            break;
        default :
            return;         // should never happen
    }
    
    showContents();
}


/*------------------------------------------------------------------------------
 *  Auxilliary function: set the fade in of a playlist element.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: setFadeIn(
                          Ptr<PlaylistElement>::Ref     playlistElement,
                          Ptr<time_duration>::Ref       newFadeIn)
                                                                    throw()
{
    Ptr<FadeInfo>::Ref          oldFadeInfo = playlistElement->getFadeInfo();
    Ptr<time_duration>::Ref     oldFadeOut;
    if (oldFadeInfo) {
        if (*oldFadeInfo->getFadeIn() == *newFadeIn) {
            return;
        }
        oldFadeOut  = oldFadeInfo->getFadeOut();
    } else {
        oldFadeOut.reset(new time_duration(0,0,0,0));
    }
    Ptr<FadeInfo>::Ref          newFadeInfo(new FadeInfo(
                                                newFadeIn, oldFadeOut ));
    if (isLengthOkay(playlistElement, newFadeInfo)) {
        playlistElement->setFadeInfo(newFadeInfo);
        isPlaylistModified = true;
    }
}


/*------------------------------------------------------------------------------
 *  Auxilliary function: set the fade out of a playlist element.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: setFadeOut(
                            Ptr<PlaylistElement>::Ref     playlistElement,
                            Ptr<time_duration>::Ref       newFadeOut)
                                                                    throw()
{
    Ptr<FadeInfo>::Ref          oldFadeInfo = playlistElement->getFadeInfo();
    Ptr<time_duration>::Ref     oldFadeIn;
    if (oldFadeInfo) {
        if (*oldFadeInfo->getFadeOut() == *newFadeOut) {
            return;
        }
        oldFadeIn = oldFadeInfo->getFadeIn();
    } else {
        oldFadeIn.reset(new time_duration(0,0,0,0));
    }
    Ptr<FadeInfo>::Ref          newFadeInfo(new FadeInfo(
                                                    oldFadeIn, newFadeOut ));
    if (isLengthOkay(playlistElement, newFadeInfo)) {
        playlistElement->setFadeInfo(newFadeInfo);
        isPlaylistModified = true;
    }
}


/*------------------------------------------------------------------------------
 *  Auxilliary function: check that fades are not longer than the whole clip.
 *----------------------------------------------------------------------------*/
inline bool
SimplePlaylistManagementWindow :: isLengthOkay(
                            Ptr<PlaylistElement>::Ref     playlistElement,
                            Ptr<FadeInfo>::Ref            newFadeInfo)
                                                                    throw()
{
    time_duration   totalFades = *newFadeInfo->getFadeIn()
                               + *newFadeInfo->getFadeOut();
    return (totalFades <= *playlistElement->getPlayable()->getPlaylength());
}


/*------------------------------------------------------------------------------
 *  Signal handler for the playlist being modified outside the window.
 *----------------------------------------------------------------------------*/
void
SimplePlaylistManagementWindow :: onPlaylistModified(void)          throw()
{
    isPlaylistModified = true;
}

