/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#if (defined(__APPLE__))

#ifndef __StCocoaView_h_
#define __StCocoaView_h_

#ifdef __OBJC__

    #import <Cocoa/Cocoa.h>

    #include <StCore/StEvent.h>

    class StWindowImpl;

ST_DISABLE_DEPRECATION_WARNINGS
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
        bool          myIsLionOS;
    }
ST_ENABLE_DEPRECATION_WARNINGS

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
