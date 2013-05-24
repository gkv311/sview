/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

namespace {
    static const StString CLASS_NAME("StGLMenuItem");
    static const size_t SHARE_PROGRAM_ID = StGLRootWidget::generateShareId();
};

void StGLMenuItem::DeleteWithSubMenus(StGLMenuItem* theMenuItem) {
    if(theMenuItem == NULL) {
        return;
    }
    if(theMenuItem->getSubMenu() != NULL) {
        StGLMenu::DeleteWithSubMenus(theMenuItem->getSubMenu());
        theMenuItem->setSubMenu(NULL);
    }
    delete theMenuItem;
}

StGLMenuItem::StGLMenuItem(StGLMenu* theParent,
                           const int theLeft, const int theTop,
                           StGLMenu* theSubMenu)
: StGLTextArea(theParent,
               theLeft, theTop,
               StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT)),
  mySubMenu(theSubMenu),
  myProgram(getRoot()->getShare(SHARE_PROGRAM_ID)),
  myIsItemSelected(false),
  myToHilightText(false) {
    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLMenuItem::doMouseUnclick);

    myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                               StGLTextFormatter::ST_ALIGN_Y_CENTER);

    setBorder(false);
    myBackColor[StGLMenuItem::HIGHLIGHT] = StGLVec4(0.765f, 0.765f, 0.765f, 1.0f);
    myBackColor[StGLMenuItem::CLICKED]   = StGLVec4(0.500f, 0.500f, 0.500f, 1.0f);
}

StGLMenuItem::~StGLMenuItem() {
    myBackVertexBuf.release(getContext());
}

const StString& StGLMenuItem::getClassName() {
    return CLASS_NAME;
}

void StGLMenuItem::setHilightText() {
    myToHilightText = true;
}

const int StGLMenuItem::computeTextWidth() {
    StHandle<StFTFont>& aFont = myFont->getFont();
    if(aFont.isNull() || !aFont->isValid()) {
        return int(10 * (myText.getLength() + 2));
    }

    GLfloat aWidth    = 0.0f;
    GLfloat aWidthMax = 0.0f;
    for(StUtf8Iter anIter = myText.iterator(); *anIter != 0;) {
        const stUtf32_t aCharThis =   *anIter;
        const stUtf32_t aCharNext = *++anIter;

        if(aCharThis == '\x0D') {
            continue; // ignore CR
        } else if(aCharThis == '\x0A') {
            aWidthMax = stMax(aWidthMax, aWidth);
            aWidth = 0.0f;
            continue; // will be processed on second pass
        } else if(aCharThis == ' ') {
            aWidth += aFont->getAdvanceX(aCharThis, aCharNext);
            continue;
        }

        aWidth += aFont->getAdvanceX(aCharThis, aCharNext);
    }
    aWidthMax = stMax(aWidthMax, aWidth);
    return int(aWidthMax);
}

void StGLMenuItem::stglResize() {
    StGLContext& aCtx = getContext();

    // back vertices
    StArray<StGLVec2> aVertices(4);
    getRectGl(aVertices);
    myBackVertexBuf.init(aCtx, aVertices);

    // update projection matrix
    myProgram->use(aCtx);
    myProgram->setProjMat(aCtx, getRoot()->getScreenProjection());
    myProgram->unuse(aCtx);
}

void StGLMenuItem::stglResize(const StRectI_t& theWinRectPx) {
    StGLTextArea::stglResize(theWinRectPx);
    stglResize();
}

void StGLMenuItem::stglUpdate(const StPointD_t& theCursorZo) {
    StGLTextArea::stglUpdate(theCursorZo);
    if(!myIsInitialized || !isVisible()) {
        return;
    }
    if(isClicked(ST_MOUSE_LEFT) || (isSelected() && hasSubMenu())) {
        return;
    }
    if(isPointIn(getRoot()->getCursorZo())) {
        if(getParentMenu()->isActive()) {
            setSelected(true);
        }
    }
}

bool StGLMenuItem::stglInit() {
    myIsInitialized = StGLTextArea::stglInit();
    if(!myIsInitialized) {
        return false;
    }

    // already initialized?
    if(myBackVertexBuf.isValid()) {
        return true;
    }

    // initialize GLSL program
    StGLContext& aCtx = getContext();
    if(myProgram.isNull()) {
        myProgram.create(getRoot()->getContextHandle(), new StGLMenuProgram());
        if(!myProgram->init(aCtx)) {
            myIsInitialized = false;
            return myIsInitialized;
        }
    }

    StArray<StGLVec2> aDummyVert(4);
    myBackVertexBuf.init(aCtx, aDummyVert);

    stglResize();
    return myIsInitialized;
}

void StGLMenuItem::stglDrawArea(const StGLMenuItem::State theState) {
    StGLContext& aCtx = getContext();
    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    myProgram->use(aCtx);

    myProgram->setColor(aCtx, myBackColor[theState], GLfloat(opacityValue));
    myBackVertexBuf.bindVertexAttrib(aCtx, myProgram->getVVertexLoc());
    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    myBackVertexBuf.unBindVertexAttrib(aCtx, myProgram->getVVertexLoc());

    myProgram->unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

void StGLMenuItem::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    switch(getParentMenu()->getOrient()) {
        case StGLMenu::MENU_VERTICAL: {
            myMarginLeft = 32;
            break;
        }
        case StGLMenu::MENU_HORIZONTAL: {
            myMarginLeft = 2;
            break;
        }
        default:
        case StGLMenu::MENU_ZERO: {
            myMarginLeft = 0;
            break;
        }
    }

    StGLMenuItem::State aState = StGLMenuItem::PASSIVE;
    if(isClicked(ST_MOUSE_LEFT) || (isSelected() && hasSubMenu())) {
        aState = StGLMenuItem::CLICKED;
    } else if(isPointIn(getRoot()->getCursorZo()) || myHasFocus) {
        aState = StGLMenuItem::HIGHLIGHT;
    }

    if(isResized) {
        stglResize();
        isResized = false;
    }

    if(myToHilightText) {
        if(myHasFocus) {
            stglDrawArea(StGLMenuItem::HIGHLIGHT);
        }
        if(aState == StGLMenuItem::HIGHLIGHT) {
            setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
        } else {
            setTextColor(StGLVec3(0.8f, 0.8f, 0.8f));
        }
    } else if(aState != StGLMenuItem::PASSIVE) {
        stglDrawArea(aState);
    }

    StGLTextArea::stglDraw(theView);
}

void StGLMenuItem::setSelected(bool theToSelect) {
    if(theToSelect) {
        for(StGLWidget* aChild = getParent()->getChildren()->getStart(); aChild != NULL; aChild = aChild->getNext()) {
            if(aChild != this) {
                ((StGLMenuItem* )aChild)->setSelected(false);
            }
        }
        if(hasSubMenu()) {
            mySubMenu->setVisibility(true, true);
        }
    } else {
        if(hasSubMenu()) {
            mySubMenu->setVisibility(false, true);
        }
    }
    myIsItemSelected = theToSelect;
}

void StGLMenuItem::setFocus(const bool theValue) {
    myHasFocus = theValue;
}

bool StGLMenuItem::tryClick(const StPointD_t& theCursorZo,
                            const int&        theMouseBtn,
                            bool&             theIsItemClicked) {
    if(StGLWidget::tryClick(theCursorZo, theMouseBtn, theIsItemClicked)) {
        theIsItemClicked = true; // always clickable widget
        return true;
    }
    return false;
}

bool StGLMenuItem::tryUnClick(const StPointD_t& theCursorZo,
                              const int&        theMouseBtn,
                              bool&             theIsItemUnclicked) {
    const bool wasUnclicked = theIsItemUnclicked;
    if(StGLWidget::tryUnClick(theCursorZo, theMouseBtn, theIsItemUnclicked)) {
        theIsItemUnclicked = true; // always clickable widget


        if(getParentMenu()->isRootMenu()) {
            getParentMenu()->setActive(true); // activate root menu
        }
        return true;
    }

    if(!wasUnclicked && theIsItemUnclicked) {
        // keep menu active if some control has been clicked inside this menu item
        getParentMenu()->setKeepActive();
    }

    return false;
}

void StGLMenuItem::doMouseUnclick(const int theBtnId) {
    if(theBtnId == ST_MOUSE_LEFT) {
        signals.onItemClick(getUserData());
    }
}

void StGLMenuItem::setHilightColor(const StGLVec4& theValue) {
    myBackColor[StGLMenuItem::HIGHLIGHT] = theValue;
}

void StGLMenuItem::resetHilightColor() {
    myBackColor[StGLMenuItem::HIGHLIGHT] = StGLVec4(0.765f, 0.765f, 0.765f, 1.0f);
}
