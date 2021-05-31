/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StCocoaLocalPool_h_
#define __StCocoaLocalPool_h_

#include <stTypes.h>

class StCocoaLocalPool {

        public:

    ST_CPPEXPORT StCocoaLocalPool();
    ST_CPPEXPORT ~StCocoaLocalPool();

        private:

    void* myPoolObj;

};

#endif // __StCocoaLocalPool_h_
