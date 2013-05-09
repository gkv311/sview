/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StTests program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StTests program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __APPLE__

#ifndef __StTestResponder_h_
#define __StTestResponder_h_

#include <StThreads/StThread.h>

#import <Cocoa/Cocoa.h>

/**
 * Main Cocoa application responder.
 */
@interface StTestResponder : NSObject <NSApplicationDelegate>
    {
    }

        /// public interface

    + (StTestResponder* ) sharedInstance;

    /**
     * Default constructor.
     */
    - (id ) init;

        /// private methods

    /**
     * Dummy method for thread-safety Cocoa initialization.
     */
    + (void ) doDummyThread: (id ) theParam;

@end

#endif // __StTestResponder_h_
#endif // __APPLE__
