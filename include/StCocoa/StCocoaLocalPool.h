/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StCocoaLocalPool_h_
#define __StCocoaLocalPool_h_

#include <stTypes.h>

class ST_LOCAL StCocoaLocalPool {

        private:

    void* myPoolObj;

        public:

    StCocoaLocalPool();
    ~StCocoaLocalPool();

};

#endif // __StCocoaLocalPool_h_
