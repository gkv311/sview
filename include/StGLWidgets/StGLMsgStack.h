/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMsgStack_h_
#define __StGLMsgStack_h_

#include <StThreads/StMutex.h>
#include <StStrings/StString.h>
#include <StGLWidgets/StGLWidget.h>

/**
 * Widgets intended to display string messages.
 */
class ST_LOCAL StGLMsgStack : public StGLWidget {

        private:

    StMutex              myMsgMutex; //!< mutex for thread-safe access
    StArrayList<StString> myMsgList; //!< messages list

        public:

    StGLMsgStack(StGLWidget* theParent);
    virtual ~StGLMsgStack();
    virtual void stglResize(const StRectI_t& theWinRectPx);
    virtual void stglUpdate(const StPointD_t& thePointZo);

        public: //!< callback Slots

    void doPushMessage(const StString& theMessageText);

};

#endif //__StGLMsgStack_h_
