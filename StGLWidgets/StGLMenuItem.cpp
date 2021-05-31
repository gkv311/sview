/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMenuProgram.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StGLWidgets/StGLTextureButton.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StCore/StEvent.h>

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
               theParent->getItemHeight()),
  mySubMenu(theSubMenu),
  myIcon(NULL),
  myArrowIcon(Arrow_None),
  myIsItemSelected(false),
  myToHilightText(false) {
    switch(getParentMenu()->getOrient()) {
        case StGLMenu::MENU_VERTICAL: {
            myMargins.left  = myRoot->scale(32);
            myMargins.right = myRoot->scale(20);
            myArrowIcon     = theSubMenu != NULL ? Arrow_Right : Arrow_None;
            break;
        }
        case StGLMenu::MENU_VERTICAL_COMPACT: {
            myMargins.left  = myRoot->scale(8);
            myMargins.right = myRoot->scale(16);
            myArrowIcon     = theSubMenu != NULL ? Arrow_Right : Arrow_None;
            break;
        }
        case StGLMenu::MENU_HORIZONTAL: {
            myMargins.left  = myRoot->scale(2);
            myMargins.right = myRoot->scale(16);
            break;
        }
        default:
        case StGLMenu::MENU_ZERO: {
            myMargins.left  = 0;
            myMargins.right = 0;
            break;
        }
    }

    StGLWidget::signals.onMouseUnclick = stSlot(this, &StGLMenuItem::doMouseUnclick);

    myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                               StGLTextFormatter::ST_ALIGN_Y_CENTER);

    setBorder(false);

    myBackColor[StGLMenuItem::HIGHLIGHT] = getRoot()->getColorForElement(StGLRootWidget::Color_MenuHighlighted);
    myBackColor[StGLMenuItem::CLICKED]   = getRoot()->getColorForElement(StGLRootWidget::Color_MenuClicked);
    setTextColor(getRoot()->getColorForElement(StGLRootWidget::Color_MenuText));
}

StGLMenuItem::~StGLMenuItem() {
    myBackVertexBuf.release(getContext());
}

StGLMenuItem* StGLMenuItem::setIcon(const StString* theImgPaths,
                                    const size_t    theCount,
                                    const bool      theToAddMargin) {
    const int anIconMargin = myRoot->scale(16 + 8);
    if(myIcon != NULL) {
        delete myIcon;
    } else {
        myMargins.left += theToAddMargin ? anIconMargin : 0;
    }
    myIcon = new StGLIcon(this, myMargins.left - anIconMargin, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT), theCount);
    myIcon->setColor(myRoot->getColorForElement(StGLRootWidget::Color_MenuIcon));
    myIcon->setTexturePath(theImgPaths, theCount);
    return this;
}

void StGLMenuItem::setIcon(StGLIcon* theIcon) {
    if(myIcon != NULL) {
        delete myIcon;
    }
    myIcon = theIcon;
}

void StGLMenuItem::setHilightText() {
    myToHilightText = true;
}

void StGLMenuItem::stglResize() {
    StGLContext& aCtx = getContext();

    // back vertices
    StRectI_t aRectPx = getRectPxAbsolute();
    StArray<StGLVec2> aVertices(myArrowIcon != Arrow_None ? 8 : 4);
    myRoot->getRectGl(aRectPx, aVertices, 0);
    switch(myArrowIcon) {
        case Arrow_None: {
            break;
        }
        case Arrow_Right: {
            const int anIconHeight = myRoot->scale(12);
            const int aHeight      = aRectPx.height();
            aRectPx.right()  -= myRoot->scale(8);
            aRectPx.left()    = aRectPx.right() - myRoot->scale(4);
            aRectPx.top()     = aRectPx.top() + aHeight / 2 - anIconHeight / 2;
            aRectPx.bottom()  = aRectPx.top() + anIconHeight;

            StRectD_t aRectGl = myRoot->getRectGl(aRectPx);
            aVertices[4] = StGLVec2(GLfloat(aRectGl.left()),  GLfloat(aRectGl.top()));
            aVertices[5] = StGLVec2(GLfloat(aRectGl.left()),  GLfloat(aRectGl.bottom()));
            aVertices[6] = StGLVec2(GLfloat(aRectGl.right()), GLfloat(aRectGl.bottom() + aRectGl.top()) * 0.5f);
            break;
        }
        case Arrow_Bottom: {
            const int anIconHeight = myRoot->scale(6);
            const int aHeight      = aRectPx.height();
            aRectPx.right()  -= myRoot->scale(8);
            aRectPx.left()    = aRectPx.right() - myRoot->scale(8);
            aRectPx.top()     = aRectPx.top() + aHeight / 2;
            aRectPx.bottom()  = aRectPx.top() + anIconHeight;

            StRectD_t aRectGl = myRoot->getRectGl(aRectPx);
            aVertices[4] = StGLVec2(GLfloat(aRectGl.left()),                          GLfloat(aRectGl.top()));
            aVertices[5] = StGLVec2(GLfloat(aRectGl.right()),                         GLfloat(aRectGl.top()));
            aVertices[6] = StGLVec2(GLfloat(aRectGl.left() + aRectGl.right()) * 0.5f, GLfloat(aRectGl.bottom()));
            break;
        }
    }
    myBackVertexBuf.init(aCtx, aVertices);

    StGLTextArea::stglResize();
}

void StGLMenuItem::stglUpdate(const StPointD_t& theCursorZo,
                              bool theIsPreciseInput) {
    StGLTextArea::stglUpdate(theCursorZo, theIsPreciseInput);
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
    StArray<StGLVec2> aDummyVert(myArrowIcon != Arrow_None ? 8 : 4);
    if(!myBackVertexBuf.init(aCtx, aDummyVert)) {
        myIsInitialized = false;
        return false;
    }

    stglResize();
    return myIsInitialized;
}

void StGLMenuItem::stglDrawArea(const StGLMenuItem::State theState,
                                const bool                theIsOnlyArrow) {
    StGLContext& aCtx = getContext();
    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);

    StGLMenuProgram& aProgram = myRoot->getMenuProgram();
    aProgram.use(aCtx, myBackColor[theState], myOpacity, getRoot()->getScreenDispX());
    if(!theIsOnlyArrow) {
        myBackVertexBuf.bindVertexAttrib(aCtx, aProgram.getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    if(myArrowIcon != Arrow_None) {
        aProgram.setColor(aCtx, myTextColor, myOpacity * 0.5f);
        myBackVertexBuf.bindVertexAttrib(aCtx, aProgram.getVVertexLoc());
        aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 4, 3);
    }
    myBackVertexBuf.unBindVertexAttrib(aCtx, aProgram.getVVertexLoc());

    aProgram.unuse(aCtx);
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

    if(myIsResized) {
        stglResize();
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
    } else if(myArrowIcon != Arrow_None) {
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
        mySubMenu->setOpacity(theToSelect ? 1.0f : 0.0f, true);
    }
    myIsItemSelected = theToSelect;
}

void StGLMenuItem::setFocus(const bool theValue) {
    myHasFocus = theValue;
}

bool StGLMenuItem::tryClick(const StClickEvent& theEvent,
                            bool&               theIsItemClicked) {
    const bool wasClicked = theIsItemClicked;
    if(StGLWidget::tryClick(theEvent, theIsItemClicked)) {
        theIsItemClicked = true; // always clickable widget
        if(getParentMenu()->isRootMenu()) {
            getParentMenu()->setActive(true); // activate root menu
        }
        if(theEvent.Button == ST_MOUSE_LEFT) {
            getRoot()->setMenuPressed(true);
        }
        return true;
    } else if(!wasClicked && theIsItemClicked) {
        // disable continuous menu item pressing when child item has been clicked
        getRoot()->setMenuPressed(false);
    }
    return false;
}

bool StGLMenuItem::tryUnClick(const StClickEvent& theEvent,
                              bool&               theIsItemUnclicked) {
    const bool wasUnclicked = theIsItemUnclicked;
    if(StGLWidget::tryUnClick(theEvent, theIsItemUnclicked)) {
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
    myBackColor[StGLMenuItem::HIGHLIGHT] = getRoot()->getColorForElement(StGLRootWidget::Color_MenuHighlighted);
}

StGLPassiveMenuItem::StGLPassiveMenuItem(StGLMenu* theParent)
: StGLMenuItem(theParent, 0, 0, NULL) {
    //
}

void StGLPassiveMenuItem::stglUpdate(const StPointD_t& theCursorZo,
                                     bool theIsPreciseInput) {
    stglUpdateTextArea(theCursorZo, theIsPreciseInput);
    if(!myIsInitialized || !isVisible()) {
        return;
    }
}
