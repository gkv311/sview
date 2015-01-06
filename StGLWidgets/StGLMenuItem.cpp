/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StGLTextureButton.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

namespace {
    static const size_t SHARE_PROGRAM_ID = StGLRootWidget::generateShareId();
}

void StGLMenuItem::DeleteWithSubMenus(StGLMenuItem* theMenuItem) {
    if(theMenuItem == NULL) {
        return;
    }
    if(theMenuItem->getSubMenu() != NULL) {
        StGLMenu::DeleteWithSubMenus(theMenuItem->getSubMenu());
        theMenuItem->mySubMenu = NULL;
    }
    delete theMenuItem;
}

StGLMenuItem::StGLMenuItem(StGLMenu* theParent,
                           const int theLeft, const int theTop,
                           StGLMenu* theSubMenu)
: StGLTextArea(theParent,
               theLeft, theTop,
               StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
               theParent->getRoot()->scale(256),
               theParent->getRoot()->scale(32)),
  mySubMenu(theSubMenu),
  myIcon(NULL),
  myProgram(getRoot()->getShare(SHARE_PROGRAM_ID)),
  myIsItemSelected(false),
  myToHilightText(false),
  myToDrawArrow(false) {
    switch(getParentMenu()->getOrient()) {
        case StGLMenu::MENU_VERTICAL: {
            myMarginLeft  = myRoot->scale(32);
            myMarginRight = myRoot->scale(20);
            myToDrawArrow = theSubMenu != NULL;
            break;
        }
        case StGLMenu::MENU_HORIZONTAL: {
            myMarginLeft  = myRoot->scale(2);
            myMarginRight = myRoot->scale(16);
            break;
        }
        default:
        case StGLMenu::MENU_ZERO: {
            myMarginLeft  = 0;
            myMarginRight = 0;
            break;
        }
    }

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

void StGLMenuItem::setIcon(const StString* theImgPaths,
                           const size_t    theCount) {
    const int anIconMargin = myRoot->scale(16 + 8);
    if(myIcon != NULL) {
        delete myIcon;
    } else {
        myMarginLeft += anIconMargin;
    }
    myIcon = new StGLIcon(this, myMarginLeft - anIconMargin, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT), theCount);
    myIcon->setVisibility(true, true);
    myIcon->setTexturePath(theImgPaths, theCount);
}

void StGLMenuItem::setHilightText() {
    myToHilightText = true;
}

void StGLMenuItem::stglResize() {
    StGLContext& aCtx = getContext();

    // back vertices
    StRectI_t aRectPx = getRectPxAbsolute();
    StArray<StGLVec2> aVertices(myToDrawArrow ? 8 : 4);
    myRoot->getRectGl(aRectPx, aVertices, 0);
    if(myToDrawArrow) {
        aRectPx.right()  -= myRoot->scale(8);
        aRectPx.left()    = aRectPx.right() - myRoot->scale(4);
        aRectPx.top()    += myRoot->scale(10);
        aRectPx.bottom() -= myRoot->scale(10);

        StRectD_t aRectGl = myRoot->getRectGl(aRectPx);
        aVertices[4] = StGLVec2(GLfloat(aRectGl.left()),  GLfloat(aRectGl.top()));
        aVertices[5] = StGLVec2(GLfloat(aRectGl.left()),  GLfloat(aRectGl.bottom()));
        aVertices[6] = StGLVec2(GLfloat(aRectGl.right()), GLfloat(aRectGl.bottom() + aRectGl.top()) * 0.5f);
    }
    myBackVertexBuf.init(aCtx, aVertices);

    // update projection matrix
    if(!myProgram.isNull()) {
        myProgram->use(aCtx);
        myProgram->setProjMat(aCtx, getRoot()->getScreenProjection());
        myProgram->unuse(aCtx);
    }

    StGLTextArea::stglResize();
}

void StGLMenuItem::stglUpdate(const StPointD_t& theCursorZo) {
    StGLTextArea::stglUpdate(theCursorZo);
    if(!myIsInitialized || !isVisible()) {
        return;
    }
    if(isPointIn(getRoot()->getCursorZo())) {
        if(getParentMenu()->isActive()) {
            setSelected(true);
            if(getRoot()->isMenuPressed()) {
                setClicked(ST_MOUSE_LEFT, getRoot()->isClicked(ST_MOUSE_LEFT));
            }
        }
    } else if(getRoot()->isMenuPressed()) {
        setClicked(ST_MOUSE_LEFT, false);
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

    StArray<StGLVec2> aDummyVert(myToDrawArrow ? 8 : 4);
    myBackVertexBuf.init(aCtx, aDummyVert);

    stglResize();
    return myIsInitialized;
}

void StGLMenuItem::stglDrawArea(const StGLMenuItem::State theState,
                                const bool                theIsOnlyArrow) {
    StGLContext& aCtx = getContext();
    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);

    myProgram->use(aCtx, myBackColor[theState], GLfloat(opacityValue), getRoot()->getScreenDispX());
    if(!theIsOnlyArrow) {
        myBackVertexBuf.bindVertexAttrib(aCtx, myProgram->getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    if(myToDrawArrow) {
        myProgram->setColor(aCtx, myTextColor, GLfloat(opacityValue) * 0.5f);
        myBackVertexBuf.bindVertexAttrib(aCtx, myProgram->getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 4, 3);
    }
    myBackVertexBuf.unBindVertexAttrib(aCtx, myProgram->getVVertexLoc());

    myProgram->unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

void StGLMenuItem::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    StGLMenuItem::State aState = StGLMenuItem::PASSIVE;
    if(isClicked(ST_MOUSE_LEFT)
    || (myIsItemSelected && mySubMenu != NULL)) {
        aState = StGLMenuItem::CLICKED;
    } else if(isPointIn(getRoot()->getCursorZo()) || myHasFocus) {
        aState = StGLMenuItem::HIGHLIGHT;
    }

    if(isResized) {
        stglResize();
        isResized = false;
    }

    if(myToHilightText) {
        if(aState == StGLMenuItem::HIGHLIGHT) {
            setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
        } else {
            setTextColor(StGLVec3(0.8f, 0.8f, 0.8f));
        }
        if(myHasFocus) {
            stglDrawArea(StGLMenuItem::HIGHLIGHT, false);
        }
    } else if(aState != StGLMenuItem::PASSIVE) {
        stglDrawArea(aState, false);
    } else if(myToDrawArrow) {
        stglDrawArea(aState, true);
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
    }
    if(mySubMenu != NULL) {
        mySubMenu->setVisibility(theToSelect, true);
    }
    myIsItemSelected = theToSelect;
}

void StGLMenuItem::setFocus(const bool theValue) {
    myHasFocus = theValue;
}

bool StGLMenuItem::tryClick(const StPointD_t& theCursorZo,
                            const int&        theMouseBtn,
                            bool&             theIsItemClicked) {
    const bool wasClicked = theIsItemClicked;
    if(StGLWidget::tryClick(theCursorZo, theMouseBtn, theIsItemClicked)) {
        theIsItemClicked = true; // always clickable widget
        if(getParentMenu()->isRootMenu()) {
            getParentMenu()->setActive(true); // activate root menu
        }
        if(theMouseBtn == ST_MOUSE_LEFT) {
            getRoot()->setMenuPressed(true);
        }
        return true;
    } else if(!wasClicked && theIsItemClicked) {
        // disable continuous menu item pressing when child item has been clicked
        getRoot()->setMenuPressed(false);
    }
    return false;
}

bool StGLMenuItem::tryUnClick(const StPointD_t& theCursorZo,
                              const int&        theMouseBtn,
                              bool&             theIsItemUnclicked) {
    const bool wasUnclicked = theIsItemUnclicked;
    if(StGLWidget::tryUnClick(theCursorZo, theMouseBtn, theIsItemUnclicked)) {
        theIsItemUnclicked = true; // always clickable widget
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

StGLPassiveMenuItem::StGLPassiveMenuItem(StGLMenu* theParent)
: StGLMenuItem(theParent, 0, 0, NULL) {
    //
}

void StGLPassiveMenuItem::stglUpdate(const StPointD_t& theCursorZo) {
    stglUpdateTextArea(theCursorZo);
    if(!myIsInitialized || !isVisible()) {
        return;
    }
}
