/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StQuadBufferCheck_h_
#define __StQuadBufferCheck_h_

#include <StThreads/StThreads.h>

bool testQuadBufferSupport();
SV_THREAD_FUNCTION testQBThreadFunction(void* outValue);

/**
 * Launch test in other thread to be safe for current thread GL contexts.
 */
inline bool testQuadBufferSupportThreaded() {
    bool result = false;
    StThread testDeviceThreadP(testQBThreadFunction, (void* )&result);
    testDeviceThreadP.wait();
    return result;
}

#endif //__StQuadBufferCheck_h_
