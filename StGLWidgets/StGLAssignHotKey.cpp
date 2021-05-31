/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLAssignHotKey.h>

#include <StCore/StEvent.h>
#include <StGLWidgets/StGLButton.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StGLScrollArea.h>

StGLAssignHotKey::StGLAssignHotKey(StGLRootWidget*           theParent,
                                   const StHandle<StAction>& theAction,
                                   const int                 theHKeyIndex)
: StGLMessageBox(theParent, "Assign new Hot Key\n\n", "", theParent->scale(400), theParent->scale(250)),
  myTitleFrmt("Assign new Hot Key for action\n<i>{0}</i>"),
  myConflictFrmt("Conflicts with: <i>{0}</i>"),
  myAssignLab("Assign"),
  myDefaultLab("Default"),
  myCancelLab("Cancel"),
  myAction(theAction),
  myHKeyLabel(NULL),
  myConflictLabel(NULL),
  myHKeyIndex(theHKeyIndex),
  myKeyFlags(0) {
    //
}

void StGLAssignHotKey::create() {
    if(myHKeyLabel != NULL) {
        return;
    }

    setTitle(myTitleFrmt.format(myAction->getName()));
    StGLButton* anAssignBtn = addButton(myAssignLab);
    StGLButton* aDefaultBtn = addButton(myDefaultLab);
    addButton(myCancelLab);
    setOpacity(1.0f, true);

    StGLWidget* aContent = new StGLContainer(getContent(), 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                             getContent()->getRectPx().width(), getContent()->getRectPx().height());
    aContent->setOpacity(1.0f, true);

    myHKeyLabel = new StGLTextArea(aContent, 0, 0, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                   aContent->getRectPx().width(), myRoot->scale(10));
    myHKeyLabel->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                StGLTextFormatter::ST_ALIGN_Y_TOP);
    myHKeyLabel->setText("...\n");
    myHKeyLabel->setTextColor(getRoot()->getColorForElement(StGLRootWidget::Color_MessageText));
    myHKeyLabel->stglInitAutoHeight();

    const StGLVec3 aRed(1.0f, 0.0f, 0.0f);
    myConflictLabel = new StGLTextArea(aContent, 0, myHKeyLabel->getRectPx().bottom(), StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                       aContent->getRectPx().width(), myRoot->scale(10));
    myConflictLabel->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                    StGLTextFormatter::ST_ALIGN_Y_TOP);
    myConflictLabel->setText("");
    myConflictLabel->setTextColor(aRed);
    myConflictLabel->stglInitAutoHeight();

    anAssignBtn->signals.onBtnClick = stSlot(this, &StGLAssignHotKey::doSave);
    aDefaultBtn->signals.onBtnClick = stSlot(this, &StGLAssignHotKey::doReset);
}

StGLAssignHotKey::~StGLAssignHotKey() {
    //
}

void StGLAssignHotKey::unsetHotKey(StHandle<StAction>& theAction) {
    if(theAction.isNull()
    || myKeyFlags == 0) {
        return;
    } else if(theAction->getHotKey1() == myKeyFlags) {
        theAction->setHotKey1(0);
    } else if(theAction->getHotKey2() == myKeyFlags) {
        theAction->setHotKey2(0);
    }
}

void StGLAssignHotKey::doSave(const size_t ) {
    unsetHotKey(myAction);
    unsetHotKey(myConflictAction);
    if(myHKeyIndex == 2) {
        myAction->setHotKey2(myKeyFlags);
    } else {
        myAction->setHotKey1(myKeyFlags);
    }
    destroyWithDelay(this);
}

void StGLAssignHotKey::doReset(const size_t ) {
    if(myHKeyIndex == 2) {
        if(myKeyFlags != myAction->getDefaultHotKey2()) {
            myKeyFlags = myAction->getDefaultHotKey2();
            updateText();
        }
    } else {
        if(myKeyFlags != myAction->getDefaultHotKey1()) {
            myKeyFlags = myAction->getDefaultHotKey1();
            updateText();
        }
    }
}

void StGLAssignHotKey::updateText() {
    myHKeyLabel->setText(myKeyFlags != 0 ? encodeHotKey(myKeyFlags) : StString("..."));
    StHandle<StAction> anOtherAction = getActionForKey(myKeyFlags);
    if(!anOtherAction.isNull()
     && anOtherAction != myAction) {
        myConflictAction = anOtherAction;
        myConflictLabel->setText(myConflictFrmt.format(myConflictAction->getName()));
    } else {
        myConflictAction.nullify();
        myConflictLabel->setText("");
    }
    stglInit();
}

bool StGLAssignHotKey::doKeyDown(const StKeyEvent& theEvent) {
    switch(theEvent.VKey) {
        case ST_VK_SHIFT:
        case ST_VK_CONTROL:
        case ST_VK_MENU:
        case ST_VK_COMMAND:
        case ST_VK_FUNCTION: {
            myKeyFlags = theEvent.VKey | theEvent.Flags;
            StString aText = encodeHotKey(myKeyFlags);
            if(!aText.isEndsWith('+')) {
                aText += "+";
            }
            myHKeyLabel->setText(aText + "...");
            return true;
        }
        case ST_VK_RETURN: {
            if( myKeyFlags != 0
            && !myHKeyLabel->getText().isEndsWith(stCString("..."))) {
                return StGLMessageBox::doKeyDown(theEvent);
            }
        }
        default: {
            if(theEvent.VKey == ST_VK_ESCAPE) {
                if(myKeyFlags == 0) {
                    return StGLMessageBox::doKeyDown(theEvent);
                }
                myKeyFlags = 0;
            } else {
                myKeyFlags = theEvent.VKey | theEvent.Flags;
            }
            updateText();
            return true;
        }
    }
}
