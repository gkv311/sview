/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include "StTestMutex.h"

#include <StStrings/stConsole.h>
#include <StThreads/StMutex.h>
#include <StThreads/StMutexSlim.h>

namespace {

    static const size_t LOCK_ITERATIONS = 9000000;
    static StMutexSlim  TheSlimLock; // slim mutex
    static StMutex      TheLock;     // usual mutex

};

/**
 * Lock usual mutex in loop.
 */
SV_THREAD_FUNCTION StTestMutex::lockLoop(void* ) {
    for(size_t anIter = 0; anIter < LOCK_ITERATIONS; ++anIter) {
        TheLock.lock();
        // dummy
        TheLock.unlock();
    }
    return SV_THREAD_RETURN 0;
}

/**
 * Lock slim mutex in loop.
 */
SV_THREAD_FUNCTION StTestMutex::slimLockLoop(void* ) {
    for(size_t anIter = 0; anIter < LOCK_ITERATIONS; ++anIter) {
        TheSlimLock.lock();
        // dummy
        TheSlimLock.unlock();
    }
    return SV_THREAD_RETURN 0;
}

void StTestMutex::perform() {
    st::cout << stostream_text("Mutex speed tests (") << LOCK_ITERATIONS << stostream_text(" iterations).\n");
    st::cout << stostream_text("Normal mutex:\n");

    myTimer.restart();
    lockLoop(NULL);
    double aTimeAllMSec  = myTimer.getElapsedTimeInMilliSec();
    double aTimeMicroSec = 1000.0 * aTimeAllMSec / double(LOCK_ITERATIONS);
    st::cout << stostream_text("  1  thread:\t") << aTimeAllMSec  << stostream_text(" msec")
             << stostream_text(" (one lock:\t")  << aTimeMicroSec << stostream_text(" microsec)\n");

    myTimer.restart();
    StThread aLockThread(lockLoop, NULL);
    lockLoop(NULL);
    aLockThread.wait();
    aTimeAllMSec  = myTimer.getElapsedTimeInMilliSec();
    aTimeMicroSec = 1000.0 * aTimeAllMSec / double(LOCK_ITERATIONS);
    st::cout << stostream_text("  2 threads:\t") << aTimeAllMSec  << stostream_text(" msec")
             << stostream_text(" (one lock:\t")  << aTimeMicroSec << stostream_text(" microsec)\n");

    st::cout << stostream_text("Slim mutex:\n");

    myTimer.restart();
    slimLockLoop(NULL);
    aTimeAllMSec  = myTimer.getElapsedTimeInMilliSec();
    aTimeMicroSec = 1000.0 * aTimeAllMSec / double(LOCK_ITERATIONS);
    st::cout << stostream_text("  1  thread:\t") << aTimeAllMSec  << stostream_text(" msec")
             << stostream_text(" (one lock:\t")  << aTimeMicroSec << stostream_text(" microsec)\n");

    myTimer.restart();
    StThread aSlimLockThread(slimLockLoop, NULL);
    slimLockLoop(NULL);
    aSlimLockThread.wait();
    aTimeAllMSec  = myTimer.getElapsedTimeInMilliSec();
    aTimeMicroSec = 1000.0 * aTimeAllMSec / double(LOCK_ITERATIONS);
    st::cout << stostream_text("  2 threads:\t") << aTimeAllMSec  << stostream_text(" msec")
             << stostream_text(" (one lock:\t")  << aTimeMicroSec << stostream_text(" microsec)\n");
}
