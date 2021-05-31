/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLDescription_h_
#define __StGLDescription_h_

#include <StGLWidgets/StGLTextArea.h>

class StGLDescription : public StGLTextArea {

        public:

    /**
     * Creates description widget with default size 256x96.
     */
    ST_CPPEXPORT StGLDescription(StGLRootWidget* theParent);

    ST_CPPEXPORT StGLDescription(StGLRootWidget* theParent,
                                 const int theWidth);

    ST_CPPEXPORT virtual ~StGLDescription();

    /**
     * Make sure you put point in RootWidget!
     * Mouse cursor putted here in most cases, and RootWidget must be window rectangle in this case.
     * @param pointZo (const StPointD_t& ) - point in RootWidget;
     */
    ST_CPPEXPORT void setPoint(const StPointD_t& pointZo);

};

#endif // __StGLDescription_h_
