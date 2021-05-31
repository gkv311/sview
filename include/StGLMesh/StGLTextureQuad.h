/**
 * Copyright © 2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLTextureQuad_h_
#define __StGLTextureQuad_h_

#include <StGL/StGLTexture.h>

#include "StGLMesh.h"

#include <StGLCore/StGLCore20.h>

class StGLTextureQuad : public StGLTexture {

        public:

    StGLTextureQuad() : StGLTexture(GL_RGBA8) {
        //
    }

    void stglDraw(StGLContext& theCtx) {
        if(!isValid()) {
            return;
        }

    #if !defined(GL_ES_VERSION_2_0)
        theCtx.core20fwd->glDisable(GL_DEPTH_TEST);
        theCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        theCtx.core20fwd->glEnable(GL_BLEND);
        theCtx.core11->glEnable(GL_TEXTURE_2D);

        StGLTexture::bind(theCtx);

        const StGLBoxPx aVPort = theCtx.stglViewport();
        const int aWinSizeX = aVPort.width();
        const int aWinSizeY = aVPort.height();
        const GLfloat aWidth  = (aWinSizeX > 0) ?        GLfloat(getSizeX()) / GLfloat(aWinSizeX) : 1.0f;
        const GLfloat aBottom = (aWinSizeY > 0) ? 100.0f / GLfloat(aWinSizeY) : 0.0f;
        const GLfloat aHeight = (aWinSizeY > 0) ? 2.0f * GLfloat(getSizeY()) / GLfloat(aWinSizeY) : 1.0f;

        const GLfloat aVerts[] = {
             aWidth, -1.0f + aBottom + aHeight,
             aWidth, -1.0f + aBottom,
            -aWidth, -1.0f + aBottom + aHeight,
            -aWidth, -1.0f + aBottom,
        };

        const GLfloat aTCrds[] = {
            1.0f, 0.0f, // top-right
            1.0f, 1.0f, // bottom-right
            0.0f, 0.0f, // top-left
            0.0f, 1.0f  // bottom-left
        };

        theCtx.core11->glLoadIdentity();

        theCtx.core11->glEnableClientState(GL_VERTEX_ARRAY);
        theCtx.core11->glVertexPointer(2, GL_FLOAT, 0, aVerts);
        theCtx.core11->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        theCtx.core11->glTexCoordPointer(2, GL_FLOAT, 0, aTCrds);

        theCtx.core11fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        theCtx.core11->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        theCtx.core11->glDisableClientState(GL_VERTEX_ARRAY);

        StGLTexture::unbind(theCtx);
        theCtx.core11->glDisable(GL_TEXTURE_2D);
        theCtx.core20fwd->glDisable(GL_BLEND);
    #endif
    }

};

#endif // __StGLTextureQuad_h_
