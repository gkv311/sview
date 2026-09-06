/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLVarLocation_h_
#define __StGLVarLocation_h_

#include <stTypes.h>

/**
 * Simple class represents GLSL program variable location.
 */
class StGLVarLocation {

        public:

    static constexpr GLint NO_LOC = -1;

        private:

    GLint myLocation;

        public:

    /**
     * Construct a special location which actually means NO location.
     */
    ST_LOCAL StGLVarLocation() : myLocation(NO_LOC) {}

    /**
     * Constructor with initialization.
     */
    ST_LOCAL explicit StGLVarLocation(const GLint theLocation) : myLocation(theLocation) {}

    /**
     * Note you may safely put invalid location in functions like glUniform*
     * - the data passed in will be silently ignored.
     * @return true if location is not equal to -1.
     */
    ST_LOCAL bool isValid() const { return myLocation != NO_LOC; }

    /**
     * Convert operators help silently put object to GL functions like glUniform*.
     */
    ST_LOCAL operator GLint() const { return myLocation; }

};

#endif //__StGLVarLocation_h_
