/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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

class  StGLRootWidget;
class  StGLContext;
struct StClickEvent;
struct StKeyEvent;
struct StGestureEvent;
struct StScrollEvent;

/**
 * Auxiliary class to perform linear animation.
 */
class StGLAnimationLerp {

public:

    /**
     * Empty constructor, does not start animation.
     */
    ST_LOCAL StGLAnimationLerp()
    : myValue(0.0),
      myOnMs(2500.0),
      myOffMs(5000.0),
      myOnTimer(false),
      myOffTimer(true) {}

    /**
     * Update animation value in accordance with trend.
     * @param theDirUp   change value in specified direction (positive - till value 1.0; negative - till value 0.0)
     * @param theToForce reset timers to switch value to 0.0/1.0 immediately
     * @return updated value
     */
    ST_CPPEXPORT double perform(bool theDirUp, bool theToForce);

    /**
     * Return value within 0.0 - 1.0 range.
     */
    double getValue() const { return myValue; }

private:

    double  myValue;    //!< value within 0.0 - 1.0 range
    double  myOnMs;     //!< time to increase value
    double  myOffMs;    //!< time to decrease value
    StTimer myOnTimer;  //!< timer
    StTimer myOffTimer; //!< timer

};

/**
 * This is abstract class for active 2D elements (buttons, menues, bars...) representation
 * with pixels (pixels -> GL) coordinates to simplify 2D mouse callbacks.
 * Please do NOT use this for native 3D GUI elements.
 */
class StGLWidget {

        public:

    ST_CPPEXPORT StGLWidget(StGLWidget* theParent,
                            const int   theLeft = 32, const int    theTop = 32,
                            const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                            const int  theWidth = 32, const int theHeight = 32);

    ST_CPPEXPORT virtual ~StGLWidget();

        public:

    /**
     * @return root (absolute parent) widget
     */
    ST_LOCAL StGLRootWidget* getRoot() {
        return myRoot;
    }

    /**
     * @return parent widget (one level up)
     */
    ST_LOCAL StGLWidget* getParent() {
        return myParent;
    }

    ST_LOCAL StGLWidgetList* getChildren() {
        return &myChildren;
    }

    /**
     * @param theWidget      widget to search
     * @param theIsRecursive flag to search through children of children
     * @return true if wpecified widget is child of this object
     */
    ST_CPPEXPORT bool isChild(StGLWidget* theWidget,
                              const bool  theIsRecursive);

    /**
     * @return link to previous item in the list
     */
    ST_LOCAL StGLWidget* getPrev() {
        return myPrev;
    }

    /**
     * Override link to the previous item in list.
     */
    ST_CPPEXPORT void setPrev(StGLWidget* thePrev);

    /**
     * @return link to next item in the list
     */
    ST_LOCAL StGLWidget* getNext() {
        return myNext;
    }

    /**
     * Override link to the next item in list.
     */
    ST_CPPEXPORT void setNext(StGLWidget* theNext);

    /**
     * @return true if this item not first in the list
     */
    ST_LOCAL bool hasPrev() const {
        return myPrev != NULL;
    }

    /**
     * @return true if this item not last in the list
     */
    ST_LOCAL bool hasNext() const {
        return myNext != NULL;
    }

    /**
     * @return position relative to parent widget
     */
    ST_LOCAL const StGLCorner& getCorner() const {
        return myCorner;
    }

    /**
     * Setup widget layout.
     * @param theCorner position relative to parent widget
     */
    ST_LOCAL void setCorner(const StGLCorner& theCorner) {
        myCorner = theCorner;
    }

        public:

    /**
     * Return extra margins before the main content of the widget (text, image, etc.).
     */
    ST_LOCAL const StMarginsI& getMargins() const {
        return myMargins;
    }

    /**
     * Return extra margins before the main content of the widget (text, image, etc.).
     */
    ST_LOCAL StMarginsI& changeMargins() {
        return myMargins;
    }

    /**
     * Function returns <i>current</i> area rectangle (in pixels) relative to root area.
     * @return rectangle
     */
    ST_LOCAL const StRectI_t& getRectPx() const {
        return myRectPx;
    }

    /**
     * Assign new area rectangle for modification and set IsResized flag on.
     */
    ST_LOCAL void setRectPx(const StRectI_t& theRectPx) {
        myIsResized = true;
        myRectPx = theRectPx;
    }

    /**
     * This function returns area rectangle for modification
     * and sets IsResized flag on.
     */
    ST_LOCAL StRectI_t& changeRectPx() {
        myIsResized = true;
        return myRectPx;
    }

    /**
     * Return true if widget has been marked resized, but not yet updated.
     */
    ST_LOCAL bool wasResized() const { return myIsResized; }

    /**
     * Function returns <i>global</i> area rectangle (in pixels).
     * @return rectangle
     */
    ST_CPPEXPORT StRectI_t getRectPxAbsolute() const;

    /**
     * Convert coordinates relative to the parent widget
     * into absolute pixel coordinates.
     */
    ST_CPPEXPORT StRectI_t getAbsolute(const StRectI_t& theRectPx) const;

    /**
     * @param theScissorRect rectangle for OpenGL scissor test
     */
    ST_CPPEXPORT void stglScissorRect2d(StGLBoxPx& theScissorRect) const;

    /**
    * @param theScissorRect rectangle for OpenGL scissor test
    */
    ST_CPPEXPORT void stglScissorRect3d(StGLBoxPx& theScissorRect) const;

    /**
     * @param thePointZo point in Zero2One coordinates to convert
     * @return converted point in GL coordinates
     */
    ST_CPPEXPORT StPointD_t getPointGl(const StPointD_t& thePointZo) const;

    /**
     * @param thePointZo point in Zero2One coordinates
     * @return point in ActiveArea in Zero2One coordinates
     */
    ST_LOCAL StPointD_t getPointIn(const StPointD_t& thePointZo) const {
        const StRectD_t  aRectGl  = getRectGl();
        const StPointD_t aPointGl = getPointGl(thePointZo);
        return StPointD_t((aPointGl.x() - aRectGl.left()) / (aRectGl.right() - aRectGl.left()),
                          (aRectGl.top() - aPointGl.y())  / (aRectGl.top() - aRectGl.bottom()));
    }

    /**
     * @return true if opacity > 0.0
     */
    ST_LOCAL bool isVisible() const {
        return myOpacity > 0.0f;
    }

    /**
     * Check visibility of this widget taking into account visibility of parents.
     */
    ST_LOCAL bool isVisibleWithParents() const {
        return myOpacity > 0.0f
            && (myParent == NULL || myParent->isVisibleWithParents());
    }

    /**
     * Return opacity value.
     */
    ST_LOCAL float getOpacity() const {
        return myOpacity;
    }

    /**
     * Setup opacity value.
     */
    ST_CPPEXPORT virtual void setOpacity(const float theOpacity, bool theToSetChildren);

    /**
     * @return true if widget can process input events
     */
    ST_LOCAL bool isTopWidget() const {
        return myIsTopWidget;
    }

    /**
     * Returns clicking state.
     * @param theMouseBtn mouse button id
     * @return isClicked
     */
    ST_CPPEXPORT bool isClicked(int theMouseBtn) const;

    /**
     * Change clicking state.
     * @param theMouseBtn mouse button id
     * @param isClicked.
     */
    ST_CPPEXPORT void setClicked(int theMouseBtn, bool isClicked);

    /**
     * Function iterate children and self to change clicking state.
     */
    ST_CPPEXPORT virtual bool tryClick(const StClickEvent& theEvent,
                                       bool&               theIsItemClicked);

    /**
     * Function iterate children and self for unclicking state.
     */
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent,
                                         bool&               theIsItemUnclicked);

    /**
     * Process key down event. Default implementation do nothing.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyDown(const StKeyEvent& theEvent);

    /**
     * Process key hold event. Default implementation do nothing.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyHold(const StKeyEvent& theEvent);

    /**
     * Process key up event. Default implementation do nothing.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyUp  (const StKeyEvent& theEvent);

    /**
     * Process scroll event. Default implementation do nothing.
     * @param theEvent scroll event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doScroll(const StScrollEvent& theEvent);

    /**
     * @param pointZo point in Zero2One coordinates
     * @return true if input pointer in area rectangle
     */
    ST_LOCAL bool isPointIn(const StPointD_t& thePointZo) const {
        const StRectD_t aRectGl = getRectGl();
        StPointD_t aPointGl = getPointGl(thePointZo);
        return aPointGl.x() > aRectGl.left()
            && aPointGl.x() < aRectGl.right()
            && aPointGl.y() > aRectGl.bottom()
            && aPointGl.y() < aRectGl.top();
    }

    /**
     * Return true if it is visible and specified point is within this element.
     */
    ST_LOCAL bool isVisibleAndPointIn(const StPointD_t& thePointZo) const {
        return isVisible()
            && isPointIn(thePointZo);
    }

    /**
     * Update parameters.
     * @param theCursorZo mouse cursor
     */
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo,
                                         bool theIsPreciseInput);

    /**
     * Update widget and sub-widgets according to new backing store dimensions.
     */
    ST_CPPEXPORT virtual void stglResize();

    /**
     * Process initialization.
     * @return true on success
     */
    ST_CPPEXPORT virtual bool stglInit();

    /**
     * Draw area.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * @return user-defined data
     */
    ST_LOCAL size_t getUserData() const {
        return myUserData;
    }

    /**
     * @param userData user-defined data
     */
    ST_LOCAL void setUserData(const size_t theUserData) {
        myUserData = theUserData;
    }

    /**
     * Append widget to destroy list.
     * This method should be used to destroy widget within callback processing
     * to prevent corruption during widgets iteration.
     * @param theWidget the widget to destroy
     */
    ST_CPPEXPORT virtual void destroyWithDelay(StGLWidget* theWidget);

    /**
     * Destroy all child widgets. Use carefully.
     * Automatically called on widget destruction.
     */
    ST_CPPEXPORT void destroyChildren();

        public: //! @name signals

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

        protected: //! @name methods available to inheritors

    friend class StGLRootWidget;

    /**
     * @return scale (const GLdouble& )
     */
    ST_CPPEXPORT GLdouble getScaleX() const;
    ST_CPPEXPORT GLdouble getScaleY() const;

    /**
     * @return rectGl (StRectD_t ) - widget 2D rectangle with GL coordinates.
     */
    ST_CPPEXPORT StRectD_t getRectGl() const;

    ST_CPPEXPORT void getRectGl(StArray<StGLVec2>& theVertices) const;

    /**
     * Returns link to the projection camera from root widget.
     */
    ST_CPPEXPORT const StGLProjCamera* getCamera() const;

    /**
     * Returns link to the projection camera from root widget.
     */
    ST_CPPEXPORT StGLProjCamera* changeCamera();

    /**
     * @return OpenGL context from root widget.
     */
    ST_CPPEXPORT StGLContext& getContext();

        protected: //! @name protected fields

    StGLRootWidget* myRoot;          //!< root widget - GL context
    StGLWidget*     myParent;        //!< all elements must have parent widget, NULL only for root

    StGLWidgetList  myChildren;      //!< children widgets
    StGLWidget*     myPrev;          //!< previous item in array
    StGLWidget*     myNext;          //!< next item in array

    size_t          myUserData;      //!< user-defined data
    StRectI_t       myRectPx;        //!< area coordinates in pixels
    StMarginsI      myMargins;       //!< extra margins before main content of the widget (text, image, etc.)
    bool myMouseClicked[ST_MOUSE_MAX_ID + 1]; //!< mouse clicking state

        protected: //! @name fields available to inheritors

    StGLCorner      myCorner;        //!< corner (left / top / right / bottom) - relative to the parent widget
    float           myOpacity;       //!< 1.0f means 100% visible (1.0f is default)
    bool            myIsResized;
    bool            myHasFocus;
    bool            myIsTopWidget;

};

/**
 * Simple container widget.
 */
class ST_LOCAL StGLContainer : public StGLWidget {

        public: //! @name public methods

    ST_CPPEXPORT StGLContainer(StGLWidget* theParent,
                               int theLeft = 32,  int theTop = 32,
                               StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                               int theWidth = 32, int theHeight = 32);

    ST_CPPEXPORT virtual ~StGLContainer();

    ST_CPPEXPORT virtual bool tryClick  (const StClickEvent& theEvent, bool& theIsItemClicked)   ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent, bool& theIsItemUnclicked) ST_ATTR_OVERRIDE;

};

#endif //__StGLWidget_h_
