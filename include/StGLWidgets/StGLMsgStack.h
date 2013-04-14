/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
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
class StGLMsgStack : public StGLWidget {

        public:

    ST_CPPEXPORT StGLMsgStack(StGLWidget* theParent);
    ST_CPPEXPORT virtual ~StGLMsgStack();
    ST_CPPEXPORT virtual void stglResize(const StRectI_t& theWinRectPx);
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& thePointZo);

        public: //!< callback Slots

    ST_CPPEXPORT void doPushMessage(const StString& theMessageText);

        private:

    StMutex               myMsgMutex; //!< mutex for thread-safe access
    StArrayList<StString> myMsgList;  //!< messages list

};

#endif //__StGLMsgStack_h_
