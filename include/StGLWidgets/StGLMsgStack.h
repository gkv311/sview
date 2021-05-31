/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLMsgStack_h_
#define __StGLMsgStack_h_

#include <StGLWidgets/StGLWidget.h>
#include <StStrings/StMsgQueue.h>

/**
 * Widget intended to display text messages.
 */
class ST_LOCAL StGLMsgStack : public StGLContainer {

        public:

    ST_CPPEXPORT StGLMsgStack(StGLWidget*                 theParent,
                              const StHandle<StMsgQueue>& theMsgQueue);
    ST_CPPEXPORT virtual ~StGLMsgStack();
    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& thePointZo,
                                         bool theIsPreciseInput) ST_ATTR_OVERRIDE;

        private:

    StHandle<StMsgQueue> myMsgQueue; //!< messages queue
    StMsg                myMsgTmp;   //!< temporary message object

};

#endif //__StGLMsgStack_h_
