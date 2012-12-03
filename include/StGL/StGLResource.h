/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLResource_h_
#define __StGLResource_h_

#include <stTypes.h>

class StGLContext;

/**
 * Interface with special method to release OpenGL resources.
 */
class ST_LOCAL StGLResource {

        public:

    /**
     * Destructor - should be called after release()!
     */
    virtual ~StGLResource();

    /**
     * Release GL resources.
     */
    virtual void release(StGLContext& theCtx) = 0;

};

class ST_LOCAL StGLAutoRelease {

        public:

    /**
     * Main constructor
     */
    StGLAutoRelease(StGLContext&  theCtx,
                    StGLResource& theEntity)
    : myCtx(theCtx),
      myEntity(theEntity) {
        //
    }

    /**
     * Destructor
     */
    ~StGLAutoRelease() {
        myEntity.release(myCtx);
    }

        private:

    StGLContext&  myCtx;
    StGLResource& myEntity;

        private:

    StGLAutoRelease           (const StGLAutoRelease& );
    StGLAutoRelease& operator=(const StGLAutoRelease& );

};

#endif // __StGLResource_h_
