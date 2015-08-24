/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#if (defined(__APPLE__))

#ifndef __StCocoaView_h_
#define __StCocoaView_h_

#ifdef __OBJC__

    #import <Cocoa/Cocoa.h>

    #include <StCore/StEvent.h>

    class StWindowImpl;

    @interface StCocoaView : NSOpenGLView
    {
        StWindowImpl* myStWin;        //!< pointer to StWindowImpl instance
        NSCursor*     myBlankCursor;  //!< иlank cursor
        NSDictionary* myFullScrOpts;  //!< options to switch into fullscreen mode
        StEvent       myStEvent;      //!< temporary variable to generate events
        NSRect        myRectWindowed; //!< remember window position in windowed mode
        NSUInteger    myWinStyle;     //!< remember window style in windowed mode
        bool          myIsFullscreen; //!< cached fullscreen state
        @public
        bool          myToHideCursor;
    }

        /**
         * Main constructor.
         * @param theStWin (StWindowImpl* ) - StWindowImpl instance to send events;
         * @param theNsWin (NSWindow* )     - parent window;
         * @return initialized instance on success or NULL otherwise.
         */
        - (id ) initWithStWin: (StWindowImpl* ) theStWin
                        nsWin: (NSWindow* )     theNsWin;

        /**
         * Switch into fullscreen state.
         * This method should be called from main thread!
         */
        - (void ) goToFullscreen: (id ) theSender;

        /**
         * Switch into windowed state.
         * This method should be called from main thread!
         */
        - (void ) goToWindowed: (id ) theSender;

        /**
         * Update window caption.
         * This method should be called from main thread!
         */
        - (void ) updateTitle: (id ) theSender;

    @end
#else
    class StCocoaView;
#endif

#endif // __StCocoaView_h_
#endif // __APPLE__
