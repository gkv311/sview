/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
    ST_CPPEXPORT virtual void stglResize();
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& thePointZo);

        private:

    StHandle<StMsgQueue> myMsgQueue; //!< messages queue
    StMsg                myMsgTmp;   //!< temporary message object

};

#endif //__StGLMsgStack_h_
