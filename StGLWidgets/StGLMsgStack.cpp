/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLMsgStack.h>
#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLRootWidget.h>

StGLMsgStack::StGLMsgStack(StGLWidget*                 theParent,
                           const StHandle<StMsgQueue>& theMsgQueue)
: StGLContainer(theParent, 0, 0),
  myMsgQueue(theMsgQueue) {
    changeRectPx().bottom() = 1;
    changeRectPx().right()  = 1;
}

StGLMsgStack::~StGLMsgStack() {
    //
}

void StGLMsgStack::stglResize() {
    //changeRectPx().bottom() = myRoot->getRectPx().height();
    //changeRectPx().right()  = myRoot->getRectPx().width();
    StGLWidget::stglResize();
}

void StGLMsgStack::stglUpdate(const StPointD_t& thePointZo,
                              bool theIsPreciseInput) {
    StGLWidget::stglUpdate(thePointZo, theIsPreciseInput);

    // check messages stack
    while(myMsgQueue->pop(myMsgTmp)) {
        // always create message boxes in root widget
        // so that StGLRootWidget::setFocus() logic work as expected
        //StGLMessageBox* aMsgBox = new StGLMessageBox(this, "", *myMsgTmp.Text);
        StGLMessageBox* aMsgBox = new StGLMessageBox(myRoot, "", *myMsgTmp.Text);
        aMsgBox->addButton("Close");
        aMsgBox->stglInit();
    }
}
