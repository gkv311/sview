/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLWidget_h_
#define __StGLWidget_h_

#include <StGL/StGLEnums.h>
#include <StGLStereo/StGLProjCamera.h>
#include <StGLWidgets/StGLCorner.h>
#include <StGLWidgets/StGLWidgetList.h>

#include <StThreads/StTimer.h>
#include <StSlots/StSignal.h>

#include <StCore/StVirtualKeys.h> // mouse keys

class StGLRootWidget;
class StGLContext;

/**
 * This is abstract class for active 2D elements (buttons, menues, bars...) representation
 * with pixels (pixels -> GL) coordinates to simplify 2D mouse callbacks.
 * Please do NOT use this for native 3D GUI elements.
 */
class ST_LOCAL StGLWidget {

    friend class StGLRootWidget;

        private:   //!< private fields

    StGLRootWidget*       myRoot; //!< root widget - GL context
    StGLWidget*         myParent; //!< all elements must have parent widget, NULL only for root

    StGLWidgetList    myChildren; //!< children widgets
    StGLWidget*           myPrev; //!< previous item in array
    StGLWidget*           myNext; //!< next item in array

    size_t              userData; // user-defined data
    StRectI_t             rectPx; // area coordinates in pixels
    bool mouseClicked[ST_MOUSE_MAX_ID + 1]; // mouse clicking state

        protected: //!< fields available to inheritors

    StGLCorner          myCorner; //!< corner (left / top / right / bottom) - relative to the parent widget
    GLdouble        opacityValue; // 1.0 means 100% visible
    double           opacityOnMs; // time to increase opacity
    double          opacityOffMs; // time to decrease opacity
    StTimer       opacityOnTimer; // timer
    StTimer      opacityOffTimer; // timer
    bool               isResized;

        protected: //!< methods available to inheritors

    /**
     * Destroy all child widgets. Use carefully.
     * Automatically called on widget destruction.
     */
    void destroyChildren();

    /**
     * @return scale (const GLdouble& )
     */
    GLdouble getScaleX() const;
    GLdouble getScaleY() const;

    /**
     * @return rectGl (StRectD_t ) - widget 2D rectangle with GL coordinates.
     */
    StRectD_t getRectGl() const;

    void getRectGl(StArray<StGLVec2>& theVertices) const;

    /**
     * Returns link to the projection camera from root widget.
     */
    StGLProjCamera* getCamera();

    /**
     * @return OpenGL context from root widget.
     */
    StGLContext& getContext();

        public:

    StGLWidget(StGLWidget* theParent,
               const int   theLeft = 32, const int    theTop = 32,
               const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
               const int  theWidth = 32, const int theHeight = 32);

    virtual ~StGLWidget();

    /**
     * This is class identification string (currently intended for debug purposes).
     */
    virtual const StString& getClassName();

        public:

    /**
     * @param root (StGLRootWidget* ) - returns root (absolute parent) widget.
     */
    StGLRootWidget* getRoot() {
        return myRoot;
    }

    /**
     * @param parent (StGLWidget* ) - returns parent widget (one level up).
     */
    StGLWidget* getParent() {
        return myParent;
    }

    StGLWidgetList* getChildren() {
        return &myChildren;
    }

    /**
     * Returns link to previous item in the list.
     */
    StGLWidget* getPrev() {
        return myPrev;
    }

    /**
     * Override link to the previous item in list.
     */
    void setPrev(StGLWidget* thePrev);

    /**
     * Returns link to next item in the list.
     */
    StGLWidget* getNext() {
        return myNext;
    }

    /**
     * Override link to the next item in list.
     */
    void setNext(StGLWidget* theNext);

    /**
     * Returns true if this item not first in the list.
     */
    bool hasPrev() const {
        return myPrev != NULL;
    }

    /**
     * Returns true if this item not last in the list.
     */
    bool hasNext() const {
        return myNext != NULL;
    }

        public:

    /**
     * Function returns <i>current</i> area rectangle (in pixels) relative to root area.
     * @return rectangle (const StRectI_t& ).
     */
    const StRectI_t& getRectPx() const {
        return rectPx;
    }

    void setRectPx(const StRectI_t& rectPx) {
        isResized = true;
        this->rectPx = rectPx;
    }

    StRectI_t& changeRectPx() {
        isResized = true;
        return rectPx;
    }

    /**
     * Function returns <i>global</i> area rectangle (in pixels).
     * @return rectangle (StRectI ).
     */
    StRectI_t getRectPxAbsolute() const;

    /**
     * Convert coordinates relative to the parent widget
     * into absolute pixel coordinates.
     */
    StRectI_t getAbsolute(const StRectI_t& theRectPx) const;

    /**
     * @param theScissorRect rectangle for OpenGL scissor test
     */
    void stglScissorRect(GLint* theScissorRect) const;

    /**
     * @param thePointZo (const StPointD_t& ) - point in Zero2One coordinates to convert;
     * @return pointGl (StRectD_t ) - converted point in GL coordinates.
     */
    StPointD_t getPointGl(const StPointD_t& thePointZo) const;

    /**
     * @param thePointZo (const StPointD_t& ) - point in Zero2One coordinates;
     * @return pointZoInArea (StPointD_t ) - point in ActiveArea in Zero2One coordinates.
     */
    StPointD_t getPointIn(const StPointD_t& thePointZo) const;

    /**
     * @return true if opacity > 0.0.
     */
    bool isVisible() const {
        return opacityValue > 0.0;
    }

    /**
     * Modify opacity due to visibility flag and opacity timers.
     * @param isVisible (bool ) - opacity UP/DOWN;
     * @param isForce (bool ) - change opacity to 0 or 1 instantly.
     */
    virtual void setVisibility(bool isVisible, bool isForce = false);

    /**
     * Returns clicking state.
     * @param mouseBtn (const int& ) - mouse button id.
     * @return isClicked (bool ).
     */
    bool isClicked(const int& mouseBtn) const;

    /**
     * Change clicking state.
     * @param mouseBtn (const int& ) - mouse button id.
     * @param isClicked (bool ).
     */
    void setClicked(const int& mouseBtn, bool isClicked);

    /**
     * Function iterate children and self to change clicking state.
     * @param cursorZo (const StPointD_t& ) - point in Zero2One coordinates;
     * @param mouseBtn (const int& ) - mouse button id;
     */
    bool tryClick(const StPointD_t& cursorZo, const int& mouseBtn) {
        bool isItemClicked = false;
        return tryClick(cursorZo, mouseBtn, isItemClicked);
    }
    virtual bool tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked);

    /**
     * Function iterate children and self for unclicking state.
     * @param cursorZo (const StPointD_t& ) - point in Zero2One coordinates;
     * @param mouseBtn (const int& ) - mouse button id;
     */
    bool tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn) {
        bool isItemUnclicked = false;
        return tryUnClick(cursorZo, mouseBtn, isItemUnclicked);
    }
    virtual bool tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked);

    /**
     * @param pointZo (const StPointD_t& ) - point in Zero2One coordinates;
     * @return true if input pointer in area rectangle.
     */
    virtual bool isPointIn(const StPointD_t& pointZo) const;

    /**
     * Update parameters.
     * @param cursorZo (const StPointD_t& ) - mouse cursor.
     */
    virtual void stglUpdate(const StPointD_t& cursorZo);

    /**
     * Update widget and sub-widgets according to new window's dimensions.
     * @param theWinRectPx (StRectI_t& ) - new window rectangle.
     */
    virtual void stglResize(const StRectI_t& theWinRectPx);

    /**
     * Process initialization.
     * @return true on success.
     */
    virtual bool stglInit();

    /**
     * Draw area.
     */
    virtual void stglDraw(unsigned int view);

    /**
     * @return userData (size_t ) - user-defined data.
     */
    size_t getUserData() const {
        return userData;
    }

    /**
     * @param userData (size_t ) - user-defined data.
     */
    void setUserData(const size_t userData) {
        this->userData = userData;
    }

        protected: //!< Signals

    struct {
        /**
         * Emit callback Slot on mouse click over this widget.
         * @param theMouseBtnId (const int ) - clicked mouse button.
         */
        StSignal<void (const int )> onMouseClick;

        /**
         * Emit callback Slot on mouse unclick over this widget.
         * @param theMouseBtnId (const int ) - unclicked mouse button.
         */
        StSignal<void (const int )> onMouseUnclick;
    } signals;

};

#endif //__StGLWidget_h_
