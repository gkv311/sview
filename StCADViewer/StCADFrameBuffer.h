/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StCADFrameBuffer_h_
#define __StCADFrameBuffer_h_

#include <OpenGl_FrameBuffer.hxx>
#include <StGL/StGLContext.h>

/**
 * Bridge FBO for rendering into sView.
 */
class StCADFrameBuffer : public OpenGl_FrameBuffer {

        public:

    /**
     * Empty constructor.
     */
    ST_LOCAL StCADFrameBuffer();

    /**
     * Destructor, should be called after Release().
     */
    ST_LOCAL ~StCADFrameBuffer();

    /**
     * Bind for rendering into this FBO.
     */
    ST_LOCAL virtual void BindBuffer(const Handle(OpenGl_Context)& theGlCtx) Standard_OVERRIDE;

    /**
     * Bind frame buffer for drawing GL_DRAW_FRAMEBUFFER (to render into the texture).
     */
    ST_LOCAL virtual void BindDrawBuffer(const Handle(OpenGl_Context)& theGlCtx);

    /**
     * Bind frame buffer for reading GL_READ_FRAMEBUFFER
     */
    ST_LOCAL virtual void BindReadBuffer(const Handle(OpenGl_Context)& theGlCtx);

    /**
     * Setup viewport to render into FBO
     */
    //ST_LOCAL void SetupViewport(const Handle(OpenGl_Context)& theGlCtx);

    /**
     * Main initializer.
     */
    ST_LOCAL void wrapFbo(const StGLContext& theStCtx);

        protected:

    int  myVPortX;
    int  myVPortY;
    bool myIsFakeFboId;

        public:

    DEFINE_STANDARD_RTTI_INLINE(StCADFrameBuffer, OpenGl_FrameBuffer)

};

DEFINE_STANDARD_HANDLE(StCADFrameBuffer, OpenGl_FrameBuffer)

#endif // __StCADFrameBuffer_h_
