/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
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

    class StWindowImpl;

    @interface StCocoaView : NSOpenGLView
    {
        StWindowImpl* myStWin;       //!< pointer to StWindowImpl instance
        NSDictionary* myFullScrOpts; //!< options to switch into fullscreen mode
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
         */
        - (void ) goToFullscreen;

        /**
         * Switch into windowed state.
         */
        - (void ) goToWindowed;

    @end
#else
    class StCocoaView;
#endif

#endif // __StCocoaView_h_
#endif // __APPLE__
