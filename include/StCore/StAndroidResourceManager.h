/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if defined(__ANDROID__)

#ifndef __StAndroidResourceManager_h_
#define __StAndroidResourceManager_h_

#include <StThreads/StResourceManager.h>

class StAndroidGlue;

/**
 * Resource manager.
 */
class StAndroidResourceManager : public StResourceManager {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StAndroidResourceManager(StAndroidGlue*  theGlueApp,
                                          const StString& theAppName = "com.sview");

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StAndroidResourceManager();

    /**
     * Access resource with specified name.
     */
    ST_CPPEXPORT virtual int openFileDescriptor(const StString& thePath) const ST_ATTR_OVERRIDE;

        private:

    StAndroidGlue* myGlueApp; //!< pointer to application glue

};

#endif // __StAndroidResourceManager_h_
#endif // __ANDROID__
