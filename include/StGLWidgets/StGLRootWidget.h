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
class ST_LOCAL StGLRootWidget : public StGLWidget {

        public:

    /**
     * Main constructor.
     */
    StGLRootWidget();

    /**
     * Destructor.
     */
    virtual ~StGLRootWidget();

    virtual const StString& getClassName();

    /**
     * Process initialization.
     * @return true on success.
     */
    virtual bool stglInit();

    /**
     * Draw all children.
     * Root widget caches OpenGL state (like viewport).
     */
    virtual void stglDraw(unsigned int theView);

    GLdouble getRootScaleX() const {
        return myScaleGlX;
    }

    GLdouble getRootScaleY() const {
        return myScaleGlY;
    }

    StRectD_t getRootRectGl() const {
        return myRectGl;
    }

    /**
     * Convert pixel coordinates (absolute) into GL coordinates.
     */
    StRectD_t getRectGl(const StRectI_t& theRectPx) const;

    /**
     * Convert pixel coordinates (absolute) into GL coordinates.
     * Array should be 4 items length or you will got an exception!
     */
    void getRectGl(const StRectI_t& theRectPx,
                   StArray<StGLVec2>& theVertices,
                   const size_t theFromId = 0) const;

    /**
     * Access the shared resource by unique id.
     */
    StGLSharePointer* getShare(const size_t theResId);

    /**
     * Generate next unique id for shared resource.
     */
    static size_t generateShareId();

    StGLProjCamera* getCamera() {
        return &myProjCamera;
    }

    /**
     * @return OpenGL context.
     */
    StGLContext& getContext();

    /**
     * @return OpenGL context.
     */
    const StHandle<StGLContext>& getContextHandle();

    /**
     * Setup OpenGL context.
     */
    void setContext(const StHandle<StGLContext>& theCtx);

    /**
     * Returns camera projection matrix within to-screen displacement
     * thus it can be used for vertices given in only 2D-coordinates.
     */
    const StGLMatrix& getScreenProjection() const {
        return myScrProjMat;
    }

    /**
     * Returns mouse cursor position in GL coordinates.
     */
    StPointD_t getCursorZo() {
        return cursorZo;
    }

    virtual void stglUpdate(const StPointD_t& cursorZo);
    virtual void stglResize(const StRectI_t& winRectPx);

    /**
     * @return viewport dimensions from bound GL context (4-indices array).
     */
    const GLint* getViewport() const {
        return myViewport;
    }

    /**
     * Computes scissor rectangle in OpenGL viewport.
     * @param theRect        Rectangle in window coordinates
     * @param theScissorRect Scissor rectangle for glScissor() call
     */
    void stglScissorRect(const StRectI_t& theRect,
                         GLint*           theScissorRect) const;

        private:

    /// TODO (Kirill Gavrilov#9) - replace with list to StHandle from some base class
    StGLSharePointer**    myShareArray; //!< resources shared within GL context (commonly used)
    size_t                myShareSize;
    StGLProjCamera        myProjCamera; //!< projection camera
    StGLMatrix            myScrProjMat; //!< projection matrix within translation to the screen
    StHandle<StGLContext> myGlCtx;      //!< OpenGL context

    StRectD_t             myRectGl;     //!< rectangle in GL coordinates
    GLdouble              myScaleGlX;   //!< scale factor to optimize convertion from Pixels -> GL coordinates
    GLdouble              myScaleGlY;   //!< scale factor to optimize convertion from Pixels -> GL coordinates
    StPointD_t            cursorZo;     //!< mouse cursor position
    GLint                 myViewport[4];//!< cached GL viewport

};

#endif //__StGLRootWidget_h_
