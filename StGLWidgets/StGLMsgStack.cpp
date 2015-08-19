/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLRootWidget.h>

StGLMsgStack::StGLMsgStack(StGLWidget*                 theParent,
                           const StHandle<StMsgQueue>& theMsgQueue)
: StGLContainer(theParent, 0, 0),
  myMsgQueue(theMsgQueue) {
    //
}

StGLMsgStack::~StGLMsgStack() {
    //
}

void StGLMsgStack::stglResize() {
    changeRectPx().bottom() = myRoot->getRectPx().height();
    changeRectPx().right()  = myRoot->getRectPx().width();
    StGLWidget::stglResize();
}

void StGLMsgStack::stglUpdate(const StPointD_t& thePointZo) {
    StGLWidget::stglUpdate(thePointZo);

    // check messages stack
    while(myMsgQueue->pop(myMsgTmp)) {
        StGLMessageBox* aMsgBox = new StGLMessageBox(this, "", *myMsgTmp.Text);
        aMsgBox->addButton("Close");
        aMsgBox->stglInit();
    }
}
