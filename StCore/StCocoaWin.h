/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#ifndef __StCocoaWin_h_
#define __StCocoaWin_h_

#ifdef __OBJC__

    #include <StTemplates/StRect.h>
    #import <Cocoa/Cocoa.h>

    class StWindowImpl;

    @interface StCocoaWin : NSWindow
    {
        StWindowImpl* myStWin; //!< pointer to StWindowImpl instance
    }

        /**
         * Main constructor.
         * @param theStWin (StWindowImpl* ) - StWindowImpl instance to send events;
         * @param theRect (StRectI_t )      - window content placement, in normal units;
         * @param theWinStyle (NSUInteger ) - window style;
         * @return initialized instance on success or NULL otherwise.
         */
        - (id ) initWithStWin: (StWindowImpl* ) theStWin
                         rect: (StRectI_t )     theRect
                    styleMask: (NSUInteger )    theWinStyle;

        /**
         * Recall to [super close].
         */
        - (void ) forceClose;

        /**
         * Update cursor icon.
         */
        - (void ) doResetCursors: (id ) theSender;

    @end
#else
    class StCocoaWin;
#endif

#endif // __StCocoaWin_h_
#endif // __APPLE__
