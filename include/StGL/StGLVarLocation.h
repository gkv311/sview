/**
 * Copyright © 2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLVarLocation_h_
#define __StGLVarLocation_h_

#include <stTypes.h>

/**
 * Simple class represents GLSL program variable location.
 */
class StGLVarLocation {

        public:

    static const GLint NO_LOC = -1;

        private:

    GLint myLocation;

        public:

    /**
     * Construct a special location which actually means NO location.
     */
    StGLVarLocation() : myLocation(NO_LOC) {}
    StGLVarLocation(const GLint theLocation) : myLocation(theLocation) {}

    /**
     * Note you may safely put invalid location in functions like glUniform*
     * - the data passed in will be silently ignored.
     * @return true if location is not equal to -1.
     */
    bool isValid() const { return myLocation != NO_LOC; }

    /**
     * Convert operators help silently put object to GL functions like glUniform*.
     */
    operator const GLint() const { return myLocation; }
    operator       GLint()       { return myLocation; }

};

#endif //__StGLVarLocation_h_
