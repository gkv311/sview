/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#if (defined(__APPLE__))

#ifndef __StCocoaCoords_h_
#define __StCocoaCoords_h_

#include <StTemplates/StRect.h>
#include <ApplicationServices/ApplicationServices.h>

/**
 * Auxiliary class to perform conversions between different coordinate-systems used in Cocoa.
 */
class ST_LOCAL StCocoaCoords {

        public:

    /**
     * Default constructor.
     */
    StCocoaCoords();

    /**
     * Reinitialize the class.
     */
    bool init();

    /**
     * Perform conversion.
     * @param theRectSt (const StRectI_t& ) - rectangle in screen coordinates (from upper-left corner);
     * @return rectangle in cartesian coordinates system (Cocoa, from bottom-left corner).
     */
    CGRect normalToCocoa(const StRectI_t& theRectSt) const;

    /**
     * Perform conversion.
     * @param rectangle in cartesian coordinates system (Cocoa, from bottom-left corner);
     * @return theRectSt (const StRectI_t& ) - rectangle in screen coordinates (from upper-left corner).
     */
    StRectI_t cocoaToNormal(const CGRect& theRectNs) const;

        private:

    CGFloat myScreenBottom;
    CGFloat myScale;
    CGFloat myUnScale;

};

#endif // __StCocoaCoords_h_
#endif // __APPLE__
