/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#ifndef __StCocoaCoords_h_
#define __StCocoaCoords_h_

#include <StTemplates/StRect.h>
#include <ApplicationServices/ApplicationServices.h>

/**
 * Auxiliary class to perform conversions between different coordinate-systems used in Cocoa.
 */
class StCocoaCoords {

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StCocoaCoords();

    /**
     * Reinitialize the class.
     */
    ST_CPPEXPORT bool init();

    /**
     * Perform conversion.
     * @param theRectSt (const StRectI_t& ) - rectangle in screen coordinates (from upper-left corner);
     * @return rectangle in cartesian coordinates system (Cocoa, from bottom-left corner).
     */
    ST_CPPEXPORT CGRect normalToCocoa(const StRectI_t& theRectSt) const;

    /**
     * Perform conversion.
     * @param rectangle in cartesian coordinates system (Cocoa, from bottom-left corner);
     * @return theRectSt (const StRectI_t& ) - rectangle in screen coordinates (from upper-left corner).
     */
    ST_CPPEXPORT StRectI_t cocoaToNormal(const CGRect& theRectNs) const;

        private:

    CGFloat myScreenBottom;
    CGFloat myScale;
    CGFloat myUnScale;

};

#endif // __StCocoaCoords_h_
#endif // __APPLE__
