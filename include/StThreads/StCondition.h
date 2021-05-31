/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StCondition_h_
#define __StCondition_h_

#include <stTypes.h>

#ifndef _WIN32
    #include <pthread.h>
#endif

// MacOS headers currently by default define "check" macro for assertions
#ifdef check
    #undef check
#endif

/**
 * This is boolean flag intended for communication between threads.
 * One thread sets this flag to TRUE to indicate some event happened
 * and another thread either waits this event or checks periodically its state
 * to perform job.
 *
 * This class provides interface similar to WinAPI Event objects.
 */
class StCondition {

        public: //! @name public methods

    /**
     * Default constructor, creates event in signaling state.
     */
    ST_CPPEXPORT StCondition();

    /**
     * Default constructor.
     * @param theIsSet Initial flag state
     */
    ST_CPPEXPORT StCondition(const bool theIsSet);

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StCondition();

    /**
     * Set event into signaling state.
     */
    ST_CPPEXPORT void set();

    /**
     * Reset event (unset signaling state)
     */
    ST_CPPEXPORT void reset();

    /**
     * Wait for Event (infinity).
     */
    ST_CPPEXPORT void wait();

    /**
     * Wait for signal requested time.
     * @param theTimeMilliseconds wait limit in milliseconds
     * @return true if get event
     */
    ST_CPPEXPORT bool wait(const size_t theTimeMilliseconds);

    /**
     * Do not wait for signal - just test it state.
     * @return true if get event.
     */
    ST_CPPEXPORT bool check();

    /**
     * Method perform two steps at-once - reset the event object
     * and returns true if it was in signaling state.
     * @return true if event object was in signaling state.
     */
    ST_CPPEXPORT bool checkReset();

#ifdef _WIN32
    /**
     * Access native HANDLE to Event object directly.
     */
    ST_LOCAL void* getHandle() const { return myEvent; }
#endif

        private: //! @name private fields

#ifdef _WIN32
    void*           myEvent;
#else
    pthread_mutex_t myMutex;
    pthread_cond_t  myCond;
    bool            myFlag;
#endif

};

#endif // __StCondition_h_
