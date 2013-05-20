/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLMessageBox.h>

StGLMsgStack::StGLMsgStack(StGLWidget* theParent)
: StGLWidget(theParent, 0, 0) {

}

StGLMsgStack::~StGLMsgStack() {
    //
}

void StGLMsgStack::stglResize(const StRectI_t& theWinRectPx) {
    StGLWidget::stglResize(theWinRectPx);
    changeRectPx().bottom() = theWinRectPx.height();
    changeRectPx().right()  = theWinRectPx.width();
}

void StGLMsgStack::stglUpdate(const StPointD_t& thePointZo) {
    StGLWidget::stglUpdate(thePointZo);

    // check messages stack
    if(myMsgMutex.tryLock()) {
        for(size_t aMsgId = 0; aMsgId < myMsgList.size(); ++aMsgId) {
            StGLMessageBox* aMsgBox = new StGLMessageBox(this, myMsgList[aMsgId]);
            aMsgBox->addButton("Close");
            aMsgBox->setVisibility(true, true);
            aMsgBox->stglInit();
        }
        myMsgList.clear();
        myMsgMutex.unlock();
    }
}

void StGLMsgStack::doPushMessage(const StString& theMessageText) {
    myMsgMutex.lock();
    myMsgList.add(theMessageText);
    myMsgMutex.unlock();
}
