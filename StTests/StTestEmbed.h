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

#ifndef __StTestEmbed_h_
#define __StTestEmbed_h_

#include "StTest.h"

#include <StCore/StNativeWin_t.h>
#include <StThreads/StThreads.h>

/**
 * Tests StWindow embedding functionality.
 */
class ST_LOCAL StTestEmbed : public StTest {

        public:

    virtual void perform();

        private:

    /**
     * Creates the native window.
     * @return true on success.
     */
    bool createNative();

    /**
     * Loop for native window.
     */
    void nativeLoop();

    /**
     * Loop for embedded StWindow instance (in dedicated thread).
     */
    void embedAppLoop();

    /**
     * Dedicated thread for embedded StWindow instance.
     */
    static SV_THREAD_FUNCTION embedAppThread(void* thePtr);

        private:

    StNativeWin_t myParent;
    void*         myDisplay;

};

#endif // __StTestEmbed_h_
