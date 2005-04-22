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
    Version  : $Revision: 1.6 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/gLiveSupport/src/SearchWindow.cxx,v $

------------------------------------------------------------------------------*/

/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <iostream>
#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "LiveSupport/Widgets/WidgetFactory.h"
#include "LiveSupport/Widgets/Notebook.h"
#include "LiveSupport/Widgets/Button.h"
#include "LiveSupport/Widgets/AdvancedSearchEntry.h"
#include "LiveSupport/Widgets/ZebraTreeView.h"
#include "SearchWindow.h"


using namespace Glib;
using namespace boost::posix_time;

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
SearchWindow :: SearchWindow (Ptr<GLiveSupport>::Ref      gLiveSupport,
                              Ptr<ResourceBundle>::Ref    bundle)
                                                                throw ()
          : WhiteWindow(WidgetFactory::searchWindowTitleImage,
                        Colors::White,
                        WidgetFactory::getInstance()->getWhiteWindowCorners()),
            LocalizedObject(bundle)
{
    this->gLiveSupport = gLiveSupport;
    treeModel = Gtk::ListStore::create(modelColumns);

    Gtk::Box *      simpleSearchView = constructSimpleSearchView();
    Gtk::Box *      advancedSearchView = constructAdvancedSearchView();
    Gtk::Box *      browseView = constructBrowseView();

    Notebook *      views = Gtk::manage(new Notebook);
    try {
        views->appendPage(*simpleSearchView,   *getResourceUstring(
                                                        "simpleSearchTab"));
        views->appendPage(*advancedSearchView, *getResourceUstring(
                                                        "advancedSearchTab"));
        views->appendPage(*browseView,         *getResourceUstring(
                                                        "browseTab"));
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    add(*views);    

    // show
    set_name("searchWindow");
    set_default_size(450, 250);
    set_modal(false);
    property_window_position().set_value(Gtk::WIN_POS_NONE);
    
//  showContents();
    show_all_children();
}


/*------------------------------------------------------------------------------
 *  Destructor.
 *----------------------------------------------------------------------------*/
SearchWindow :: ~SearchWindow (void)                            throw ()
{
}


/*------------------------------------------------------------------------------
 *  Construct the simple search view.
 *----------------------------------------------------------------------------*/
Gtk::VBox*
SearchWindow :: constructSimpleSearchView(void)                 throw ()
{
    Ptr<WidgetFactory>::Ref     wf = WidgetFactory::getInstance();

    // set up the entry box
    simpleSearchEntry = Gtk::manage(wf->createEntryBin());
    
    Button *        searchButton;
    try {
        searchButton = Gtk::manage(wf->createButton(
                                    *getResourceUstring("searchButtonLabel") ));
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    simpleSearchEntry->signal_activate().connect(sigc::mem_fun(
                                    *this, &SearchWindow::onSimpleSearch ));
    searchButton->signal_clicked().connect(sigc::mem_fun(
                                    *this, &SearchWindow::onSimpleSearch ));

    Gtk::HBox *         entryBox = Gtk::manage(new Gtk::HBox);
    entryBox->pack_start(*simpleSearchEntry,    Gtk::PACK_SHRINK,  5);
    entryBox->pack_start(*searchButton,         Gtk::PACK_SHRINK,  5);

    // set up the search results display
    ZebraTreeView *     searchResults = constructSearchResults();
    
    // make a new box and pack the main components into it
    Gtk::VBox *         view = Gtk::manage(new Gtk::VBox);
    view->pack_start(*entryBox,         Gtk::PACK_EXPAND_WIDGET, 5);
    view->pack_start(*searchResults,    Gtk::PACK_EXPAND_WIDGET,  5);
    return view;
}


/*------------------------------------------------------------------------------
 *  Construct the advanced search view.
 *----------------------------------------------------------------------------*/
Gtk::VBox*
SearchWindow :: constructAdvancedSearchView(void)               throw ()
{
    Ptr<WidgetFactory>::Ref     wf = WidgetFactory::getInstance();

    // the three main components of the window    
    advancedSearchEntry = Gtk::manage(new AdvancedSearchEntry(getBundle()));
    Gtk::Box *          searchButtonBox = Gtk::manage(new Gtk::HButtonBox(
                                                        Gtk::BUTTONBOX_END ));
    ZebraTreeView *     searchResults   = constructSearchResults();
    
    // set up the callback function for the entry field
    advancedSearchEntry->connectCallback(sigc::mem_fun(
                                    *this, &SearchWindow::onAdvancedSearch ));
    
    // set up the search button box
    try {
        Button *        searchButton = Gtk::manage(wf->createButton(
                                    *getResourceUstring("searchButtonLabel") ));
        searchButton->signal_clicked().connect(sigc::mem_fun(
                                    *this, &SearchWindow::onAdvancedSearch ));
        searchButtonBox->pack_start(*searchButton, Gtk::PACK_SHRINK, 5);

    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }
    
    // make a new box and pack the main components into it
    Gtk::VBox *     view = Gtk::manage(new Gtk::VBox);
    view->pack_start(*advancedSearchEntry,    Gtk::PACK_SHRINK,         5);
    view->pack_start(*searchButtonBox,        Gtk::PACK_SHRINK,         5);
    view->pack_start(*searchResults,          Gtk::PACK_EXPAND_WIDGET,  5);
    
    return view;
}


/*------------------------------------------------------------------------------
 *  Construct the browse view.
 *----------------------------------------------------------------------------*/
Gtk::VBox*
SearchWindow :: constructBrowseView(void)                       throw ()
{
    Ptr<WidgetFactory>::Ref     wf = WidgetFactory::getInstance();

    Gtk::VBox *     view = Gtk::manage(new Gtk::VBox);
    return view;
}


/*------------------------------------------------------------------------------
 *  Construct the search results display.
 *----------------------------------------------------------------------------*/
ZebraTreeView*
SearchWindow :: constructSearchResults(void)                    throw ()
{
    Ptr<WidgetFactory>::Ref     wf = WidgetFactory::getInstance();
    
    ZebraTreeView *     searchResults = Gtk::manage(wf->createTreeView(
                                                                treeModel ));

    // add the TreeView's view columns
    try {
        searchResults->appendColumn(*getResourceUstring("typeColumnLabel"),
                               modelColumns.typeColumn);
        searchResults->appendColumn(*getResourceUstring("titleColumnLabel"),
                               modelColumns.titleColumn);
        searchResults->appendColumn(*getResourceUstring("creatorColumnLabel"),
                               modelColumns.creatorColumn);
        searchResults->appendColumn(*getResourceUstring("lengthColumnLabel"),
                               modelColumns.lengthColumn);
    } catch (std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    // color the rows blue and gray
    searchResults->setCellDataFunction();
    
    return searchResults;
}


/*------------------------------------------------------------------------------
 *  Event handler for the simple Search button getting clicked.
 *----------------------------------------------------------------------------*/
void
SearchWindow :: onSimpleSearch(void)                            throw ()
{
    Glib::ustring               value = simpleSearchEntry->get_text();
    
    Ptr<SearchCriteria>::Ref    criteria(new SearchCriteria("all", "or"));
    criteria->addCondition("dc:title",   "partial", value);    // id3v2 Title
    criteria->addCondition("dc:creator", "partial", value);    // id3v2 Artist
    criteria->addCondition("dc:source",  "partial", value);    // id3v2 Album
    
    onSearch(criteria);
}


/*------------------------------------------------------------------------------
 *  Event handler for the advanced Search button getting clicked.
 *----------------------------------------------------------------------------*/
void
SearchWindow :: onAdvancedSearch(void)                          throw ()
{
    onSearch(advancedSearchEntry->getSearchCriteria());
}


/*------------------------------------------------------------------------------
 *  Do the searching.
 *----------------------------------------------------------------------------*/
void
SearchWindow :: onSearch(Ptr<SearchCriteria>::Ref   criteria)
                                                                throw ()
{
    Ptr<std::list<Ptr<Playable>::Ref> >::Ref
            searchResults = gLiveSupport->search(criteria);
    std::list<Ptr<Playable>::Ref>::const_iterator it;

    treeModel->clear();
    int     rowNumber = 0;
    
    for (it = searchResults->begin(); it != searchResults->end(); 
                                                        ++it, ++rowNumber) {
        Ptr<Playable>::Ref      playable = *it;
        Gtk::TreeModel::Row     row = *treeModel->append();
        
        row[modelColumns.rowNumberColumn]   = rowNumber;

        row[modelColumns.playableColumn]    = playable;
        
        switch (playable->getType()) {
            case Playable::AudioClipType:
                row[modelColumns.typeColumn]  = "audioclip";
                break;
            case Playable::PlaylistType:
                row[modelColumns.typeColumn]  = "playlist";
                break;
            default:
                break;
        }

        Ptr<const Glib::ustring>::Ref   title   = playable->getTitle();
        row[modelColumns.titleColumn]           = title ? *title : "";

        Ptr<Glib::ustring>::Ref creator = playable->getMetadata("dc:creator");
        row[modelColumns.creatorColumn] = creator ? *creator : "";

        Ptr<time_duration>::Ref length = playable->getPlaylength();
        row[modelColumns.lengthColumn] = length ? to_simple_string(*length)
                                                : "";
    }
}

