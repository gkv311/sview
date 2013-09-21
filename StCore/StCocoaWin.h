/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
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
