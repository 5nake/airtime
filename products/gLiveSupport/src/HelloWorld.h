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
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/products/gLiveSupport/src/Attic/HelloWorld.h,v $

------------------------------------------------------------------------------*/
#ifndef HelloWorld_h
#define HelloWorld_h

#ifndef __cplusplus
#error This is a C++ include file
#endif


/* ============================================================ include files */

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#include <gtkmm/button.h>
#include <gtkmm/window.h>

namespace LiveSupport {
namespace GLiveSupport {

/* ================================================================ constants */


/* =================================================================== macros */


/* =============================================================== data types */

/**
 *  A simple window, just saying "Hello, World!"
 *
 *  @author $Author: maroy $
 *  @version $Revision: 1.1 $
 */
class HelloWorld : public Gtk::Window
{

    protected:
        /**
         *  The only button in the window.
         */
        Gtk::Button         button;

        /**
         *  Signal handler for the only button clicked.
         */
        virtual void
        onButtonClicked(void)                           throw ();


    public:
        /**
         *  Constructor.
         */
        HelloWorld(void)                                throw ();

        /**
         *  Virtual destructor.
         */
        virtual
        ~HelloWorld(void)                               throw ();

};

/* ================================================= external data structures */


/* ====================================================== function prototypes */


} // namespace GLiveSupport
} // namespace LiveSupport

#endif // HelloWorld_h

