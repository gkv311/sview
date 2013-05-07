/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StTests program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StTests program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StTestGlBand.h"

#include <StCore/StWindow.h>

#include <StGL/StGLTexture.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <StStrings/stConsole.h>

#include <StImage/StImagePlane.h>
#include <StTemplates/StHandle.h>

namespace {

    static const size_t TEST_ITERATIONS   = 100;
    static const double TEST_ITERATIONS_F = 100.0;

};

void StTestGlBand::testTextureFill(StGLContext&  theCtx,
                                   const GLsizei theFrameSizeX,
                                   const GLsizei theFrameSizeY) {
    StImagePlane anImgPlane;
    if(!anImgPlane.initZero(StImagePlane::ImgRGB, theFrameSizeX, theFrameSizeY)) {
    //if(!anImgPlane.initZero(StImagePlane::ImgRGB, theFrameSizeX, theFrameSizeY, 3 * theFrameSizeX + 512)) {
        st::cout << stostream_text("Fail to initialize RGB image plane...\n");
    }

    StGLTexture aTexture(GL_RGB8);
    if(!aTexture.initTrash(theCtx, theFrameSizeX, theFrameSizeY)) {
        st::cout << stostream_text("Fail to create texture ") << theFrameSizeX << stostream_text(" x ") << theFrameSizeY << stostream_text("\n");
        return;
    }
    st::cout << stostream_text("Fill RGB Texture from frame ") << theFrameSizeX << stostream_text(" x ") << theFrameSizeY << stostream_text("\n");
    myTimer.restart();
    for(size_t anIter = 0; anIter < TEST_ITERATIONS; ++anIter) {
        if(!aTexture.fill(theCtx, anImgPlane)) {
            st::cout << stostream_text("Fail to fill texture...\n");
            aTexture.release(theCtx);
            return;
        }
    }
    double aTimeAllSec = myTimer.getElapsedTimeInSec();
    double aTimeSec = aTimeAllSec / TEST_ITERATIONS_F;
    double aSpeed = (TEST_ITERATIONS_F * anImgPlane.getSizeBytes() / aTimeAllSec) / (1024.0 * 1024.0);
    st::cout << stostream_text("  fill speed:\t") << aSpeed << stostream_text(" MiB/sec\n");
    st::cout << stostream_text("  fill time: \t") << (1000.0 * aTimeSec)  << stostream_text(" msec\n");
    st::cout << stostream_text("  fill FPS:  \t") << (TEST_ITERATIONS_F / aTimeAllSec)  << stostream_text("\n");

    aTexture.release(theCtx);
}

void StTestGlBand::testTextureRead(StGLContext&  theCtx,
                                   const GLsizei theFrameSizeX,
                                   const GLsizei theFrameSizeY) {
    StGLTexture aTexture(GL_RGB8);
    if(!aTexture.initBlack(theCtx, theFrameSizeX, theFrameSizeY)) {
        st::cout << stostream_text("Fail to create texture ") << theFrameSizeX << stostream_text(" x ") << theFrameSizeY << stostream_text("\n");
        return;
    }

    StImagePlane anImgPlane;
    if(!anImgPlane.initTrash(StImagePlane::ImgBGR, theFrameSizeX, theFrameSizeY)) {
        st::cout << stostream_text("Fail to initialize RGB image plane...\n");
    }

    aTexture.bind(theCtx);
    st::cout << stostream_text("Read RGB Texture to frame ") << theFrameSizeX << stostream_text(" x ") << theFrameSizeY << stostream_text("\n");
    myTimer.restart();
    for(size_t anIter = 0; anIter < TEST_ITERATIONS; ++anIter) {
        theCtx.core11fwd->glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, anImgPlane.changeData());
    }
    aTexture.unbind(theCtx);
    double aTimeAllSec = myTimer.getElapsedTimeInSec();
    double aTimeSec = aTimeAllSec / TEST_ITERATIONS_F;
    double aSpeed = (TEST_ITERATIONS_F * anImgPlane.getSizeBytes() / aTimeAllSec) / (1024.0 * 1024.0);
    st::cout << stostream_text("  read speed:\t") << aSpeed << stostream_text(" MiB/sec\n");
    st::cout << stostream_text("  read time: \t") << (1000.0 * aTimeSec)  << stostream_text(" msec\n");
    st::cout << stostream_text("  read FPS:  \t") << (TEST_ITERATIONS_F / aTimeAllSec)  << stostream_text("\n");

    aTexture.release(theCtx);
}

void StTestGlBand::testFrameCopyRAM(const GLsizei theFrameSizeX,
                                    const GLsizei theFrameSizeY) {
    StImagePlane anImgPlaneSrc, anImgPlaneDst;
    if(!anImgPlaneSrc.initTrash(StImagePlane::ImgRGB, theFrameSizeX, theFrameSizeY)
    || !anImgPlaneDst.initTrash(StImagePlane::ImgRGB, theFrameSizeX, theFrameSizeY)) {
        st::cout << stostream_text("Fail to initialize RGB image plane...\n");
        return;
    }

    st::cout << stostream_text("RGB Frame to Frame ") << theFrameSizeX << stostream_text(" x ") << theFrameSizeY << stostream_text(" (RAM)\n");
    myTimer.restart();
    for(size_t anIter = 0; anIter < TEST_ITERATIONS; ++anIter) {
        if(!anImgPlaneDst.fill(anImgPlaneSrc)) {
            st::cout << stostream_text("Fail to fill the image plane...\n");
            return;
        }
    }
    double aTimeAllSec = myTimer.getElapsedTimeInSec();
    double aTimeSec = aTimeAllSec / TEST_ITERATIONS_F;
    double aSpeed = (TEST_ITERATIONS_F * anImgPlaneSrc.getSizeBytes() / aTimeAllSec) / (1024.0 * 1024.0);
    st::cout << stostream_text("  fill speed:\t") << aSpeed << stostream_text(" MiB/sec\n");
    st::cout << stostream_text("  fill time: \t") << (1000.0 * aTimeSec)  << stostream_text(" msec\n");
    st::cout << stostream_text("  fill FPS:  \t") << (TEST_ITERATIONS_F / aTimeAllSec)  << stostream_text("\n");
}

void StTestGlBand::perform() {
    // create the window
    StHandle<StWindow> aWin = new StWindow();
    aWin->setPlacement(StRectI_t(256, 768, 256, 768));
    aWin->setTitle("sView - Tests");
    aWin->create();

    // perform tests
    aWin->stglMakeCurrent();
    StGLContext aCtx(true);

    const StGLBoxPx aVPort = aWin->stglViewport(ST_WIN_MASTER);
    aCtx.stglResizeViewport(aVPort);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    aWin->stglSwap();

    // 2x 1080p
    GLsizei aFrameSizeX = 1920;
    GLsizei aFrameSizeY = 1080 * 2;
    testTextureFill(aCtx, aFrameSizeX, aFrameSizeY);
    testTextureRead(aCtx, aFrameSizeX, aFrameSizeY);
    testFrameCopyRAM(aFrameSizeX, aFrameSizeY);

    // close the window
    aWin.nullify();
}
