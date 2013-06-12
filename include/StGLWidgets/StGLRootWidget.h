/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLRootWidget_h_
#define __StGLRootWidget_h_

#include <StGLWidgets/StGLShare.h>
#include <StGLWidgets/StGLWidget.h>

#include <StTemplates/StHandle.h>

/**
 * Full OpenGL-window widget, must be ROOT for other widgets.
 */
class StGLRootWidget : public StGLWidget {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLRootWidget();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLRootWidget();

    ST_CPPEXPORT virtual const StString& getClassName();

    /**
     * Process initialization.
     * @return true on success
     */
    ST_CPPEXPORT virtual bool stglInit();

    /**
     * Draw all children.
     * Root widget caches OpenGL state (like viewport).
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * Function iterate children and self for unclicking state.
     * @param theCursorZo point in Zero2One coordinates
     * @param theMouseBtn mouse button id
     */
    inline bool tryUnClick(const StPointD_t& theCursorZo,
                           const int&        theMouseBtn) {
        bool isItemUnclicked = false;
        return tryUnClick(theCursorZo, theMouseBtn, isItemUnclicked);
    }

    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& theCursorZo,
                                         const int&        theMouseBtn,
                                         bool&             theIsItemUnclicked);

    /**
     * Process key down event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyDown(const StKeyEvent& theEvent);

    /**
     * Process key hold event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyHold(const StKeyEvent& theEvent);

    /**
     * Process key up event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyUp  (const StKeyEvent& theEvent);

    inline GLdouble getRootScaleX() const {
        return myScaleGlX;
    }

    inline GLdouble getRootScaleY() const {
        return myScaleGlY;
    }

    inline StRectD_t getRootRectGl() const {
        return myRectGl;
    }

    inline const StRectI_t& getRootMarginsPx() const {
        return myMarginsPx;
    }

    inline void setRootMarginsPx(const StRectI_t& theMargins) {
        myMarginsPx = theMargins;
    }

    /**
     * Convert pixel coordinates (absolute) into GL coordinates.
     */
    ST_CPPEXPORT StRectD_t getRectGl(const StRectI_t& theRectPx) const;

    /**
     * Convert pixel coordinates (absolute) into GL coordinates.
     * Array should be 4 items length or you will got an exception!
     */
    ST_CPPEXPORT void getRectGl(const StRectI_t&   theRectPx,
                                StArray<StGLVec2>& theVertices,
                                const size_t       theFromId = 0) const;

    /**
     * Access the shared resource by unique id.
     */
    ST_CPPEXPORT StGLSharePointer* getShare(const size_t theResId);

    /**
     * Generate next unique id for shared resource.
     */
    ST_CPPEXPORT static size_t generateShareId();

    inline StGLProjCamera* getCamera() {
        return &myProjCamera;
    }

    /**
     * @return widget in focus
     */
    inline StGLWidget* getFocus() {
        return myFocusWidget;
    }

    /**
     * @param theWidget new widget to set focus
     * @return previous widget focus
     */
    ST_CPPEXPORT StGLWidget* setFocus(StGLWidget* theWidget);

    /**
     * @return OpenGL context.
     */
    ST_CPPEXPORT StGLContext& getContext();

    /**
     * @return OpenGL context.
     */
    ST_CPPEXPORT const StHandle<StGLContext>& getContextHandle();

    /**
     * Setup OpenGL context.
     */
    ST_CPPEXPORT void setContext(const StHandle<StGLContext>& theCtx);

    /**
     * Returns camera projection matrix within to-screen displacement
     * thus it can be used for vertices given in only 2D-coordinates.
     */
    inline const StGLMatrix& getScreenProjection() const {
        return myScrProjMat;
    }

    inline GLfloat getScreenDispX() const {
        return myScrDispX;
    }

    inline GLfloat getLensDist() const {
        return myLensDist;
    }

    inline void setLensDist(const GLfloat theLensDist) {
        myLensDist = theLensDist;
    }

    /**
     * Returns mouse cursor position in GL coordinates.
     */
    inline StPointD_t getCursorZo() {
        return cursorZo;
    }

    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& cursorZo);
    ST_CPPEXPORT virtual void stglResize(const StRectI_t& winRectPx);

    /**
     * @return viewport dimensions from bound GL context (4-indices array)
     */
    inline const GLint* getViewport() const {
        return myViewport;
    }

    /**
     * Computes scissor rectangle in OpenGL viewport.
     * @param theRect        Rectangle in window coordinates
     * @param theScissorRect Scissor rectangle for glScissor() call
     */
    ST_CPPEXPORT void stglScissorRect(const StRectI_t& theRect,
                                      StGLBoxPx&       theScissorRect) const;

    /**
     * Append widget to destroy list.
     * This method should be used to destroy widget within callback processing
     * to prevent corruption during widgets iteration.
     * @param theWidget the widget to destroy
     */
    ST_CPPEXPORT virtual void destroyWithDelay(StGLWidget* theWidget);

        private:

    /**
     * Perform pending destroy requests.
     */
    ST_CPPEXPORT void clearDestroyList();

        private:

    StGLSharePointer**       myShareArray;  //!< resources shared within GL context (commonly used)
    size_t                   myShareSize;
    StGLProjCamera           myProjCamera;  //!< projection camera
    StGLMatrix               myScrProjMat;  //!< projection matrix within translation to the screen
    StHandle<StGLContext>    myGlCtx;       //!< OpenGL context
    GLfloat                  myScrDispX;
    GLfloat                  myLensDist;

    StRectI_t                myMarginsPx;   //!< active area margins in pixels
    StRectD_t                myRectGl;      //!< rectangle in GL coordinates
    GLdouble                 myScaleGlX;    //!< scale factor to optimize convertion from Pixels -> GL coordinates
    GLdouble                 myScaleGlY;    //!< scale factor to optimize convertion from Pixels -> GL coordinates
    StPointD_t               cursorZo;      //!< mouse cursor position
    GLint                    myViewport[4]; //!< cached GL viewport

    StArrayList<StGLWidget*> myDestroyList; //!< list of widgets to be destroyed
    StGLWidget*              myFocusWidget; //!< widget currently in focus

};

#endif //__StGLRootWidget_h_
