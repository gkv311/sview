/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutDual library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutDual library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutDistorted.h"

#include <StGL/StGLContext.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGLCore/StGLCore20.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StSettings/StEnumParam.h>
#include <StCore/StSearchMonitors.h>
#include <StVersion.h>

namespace {

    static const char ST_OUT_PLUGIN_NAME[]   = "StOutDistorted";

    static const char ST_SETTING_DEVICE_ID[] = "deviceId";
    static const char ST_SETTING_WINDOWPOS[] = "windowPos";
    static const char ST_SETTING_VSYNC[]     = "vsync";

    // translation resources
    enum {
        STTR_DISTORTED_NAME     = 1000,
        STTR_DISTORTED_DESC     = 1001,

        // parameters
        STTR_PARAMETER_VSYNC    = 1100,
        STTR_PARAMETER_SLAVE_ID = 1102,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

};

/**
 * Distortion GLSL program.
 */
class StProgramBarrel : public StGLProgram {

        private:

    StGLVarLocation atrVVertexLoc;
    StGLVarLocation atrVTexCoordLoc;

        public:

    StProgramBarrel()
    : StGLProgram("StProgramBarrel"),
      atrVVertexLoc(),
      atrVTexCoordLoc() {
        //
    }

    StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    StGLVarLocation getVTexCoordLoc() const {
        return atrVTexCoordLoc;
    }

    virtual bool init(StGLContext& theCtx) {
        const char VERTEX_SHADER[] =
           "attribute vec4 vVertex; \
            attribute vec2 vTexCoord; \
            varying vec2 fTexCoord; \
            void main(void) { \
                fTexCoord = vTexCoord; \
                gl_Position = vVertex; \
            }";

        const char FRAGMENT_SHADER[] =
           "uniform sampler2D texR, texL; \
            varying vec2 fTexCoord; \
            void main(void) { \
                gl_FragColor = texture2D(texR, fTexCoord); \
            }";

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp1(theCtx, aVertexShader);
        aVertexShader.init(theCtx, VERTEX_SHADER);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        aFragmentShader.init(theCtx, FRAGMENT_SHADER);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .link(theCtx)) {
            return false;
        }

        atrVVertexLoc   = StGLProgram::getAttribLocation(theCtx, "vVertex");
        atrVTexCoordLoc = StGLProgram::getAttribLocation(theCtx, "vTexCoord");
        return atrVVertexLoc.isValid() && atrVTexCoordLoc.isValid();
    }

};

StAtomic<int32_t> StOutDistorted::myInstancesNb(0);

StString StOutDistorted::getRendererAbout() const {
    return myAbout;
}

const char* StOutDistorted::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutDistorted::getDeviceId() const {
    return "Distorted";
}

bool StOutDistorted::setDevice(const StString& theDevice) {
    return theDevice == "Distorted";
}

void StOutDistorted::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        theList.add(myDevices[anIter]);
    }
}

void StOutDistorted::getOptions(StParamsList& theList) const {
    theList.add(params.IsVSyncOn);
}

StOutDistorted::StOutDistorted(const StNativeWin_t theParentWindow)
: StWindow(theParentWindow),
  mySettings(new StSettings(ST_OUT_PLUGIN_NAME)),
  myFrBuffer(new StGLFrameBuffer()),
  myProgram(new StProgramBarrel()),
  myToSavePlacement(theParentWindow == (StNativeWin_t )NULL),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    const StSearchMonitors& aMonitors = StWindow::getMonitors();
    StTranslations aLangMap(ST_OUT_PLUGIN_NAME);

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Distorted Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2013 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;

    // detect connected displays
    int aSupportLevel = ST_DEVICE_SUPPORT_NONE;
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = aMonitors[aMonIter];
        if(aMon.getPnPId().isStartsWith("OVR")) {
            // Oculus Rift
            aSupportLevel = ST_DEVICE_SUPPORT_HIGHT;
            break;
        }
    }

    // devices list
    StHandle<StOutDevice> aDevDistorted = new StOutDevice();
    aDevDistorted->PluginId = ST_OUT_PLUGIN_NAME;
    aDevDistorted->DeviceId = "Distorted";
    aDevDistorted->Priority = aSupportLevel;
    aDevDistorted->Name     = aLangMap.changeValueId(STTR_DISTORTED_NAME, "Distorted Output");
    aDevDistorted->Desc     = aLangMap.changeValueId(STTR_DISTORTED_DESC, "Oculus Rift");
    myDevices.add(aDevDistorted);

    // VSync option
    params.IsVSyncOn = new StBoolParamNamed(true, aLangMap.changeValueId(STTR_PARAMETER_VSYNC, "VSync"));
    params.IsVSyncOn->signals.onChanged.connect(this, &StOutDistorted::doVSync);

    // load window position
    StRect<int32_t> aRect(256, 768, 256, 1024);
    mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect);
    StWindow::setPlacement(aRect, true);
    StWindow::setTitle("sView - Distorted Renderer");

    // load VSync option
    mySettings->loadParam(ST_SETTING_VSYNC, params.IsVSyncOn);
}

void StOutDistorted::releaseResources() {
    if(!myContext.isNull()) {
        myProgram->release(*myContext);
        myVertFlatBuf.release(*myContext);
        myTexCoordBuf.release(*myContext);
        myFrBuffer->release(*myContext);
    }
    myContext.nullify();

    // read windowed placement
    StWindow::hide();
    if(myToSavePlacement) {
        StWindow::setFullScreen(false);
        mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, StWindow::getPlacement());
    }
    mySettings->saveParam(ST_SETTING_VSYNC, params.IsVSyncOn);
}

StOutDistorted::~StOutDistorted() {
    myInstancesNb.decrement();
    releaseResources();
}

void StOutDistorted::close() {
    releaseResources();
    StWindow::close();
}

bool StOutDistorted::create() {
    StWindow::show();
    if(!StWindow::create()) {
        return false;
    }

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL2.0+ not available!");
        return false;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync(params.IsVSyncOn->getValue() ? StGLContext::VSync_ON : StGLContext::VSync_OFF);

    if(!myProgram->init(*myContext)) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Shader");
        return false;
    }
    // create vertices buffers to draw simple textured quad
    const GLfloat QUAD_VERTICES[4 * 4] = {
         1.0f, -1.0f, 0.0f, 1.0f, // top-right
         1.0f,  1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f, -1.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f, 0.0f, 1.0f  // bottom-left
    };

    const GLfloat QUAD_TEXCOORD[2 * 4] = {
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };

    myVertFlatBuf.init(*myContext, 4, 4, QUAD_VERTICES);
    myTexCoordBuf.init(*myContext, 2, 4, QUAD_TEXCOORD);

    return true;
}

void StOutDistorted::processEvents(StMessage_t* theMessages) {
    StWindow::processEvents(theMessages);
    for(size_t anIter = 0; theMessages[anIter].uin != StMessageList::MSG_NULL; ++anIter) {
        if(theMessages[anIter].uin != StMessageList::MSG_KEYS) {
            continue;
        }

        //bool* aKeys = ((bool* )theMessages[anIter].data);
    }
}

void StOutDistorted::stglDraw() {
    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    const StGLBoxPx aViewPort = StWindow::stglViewport(ST_WIN_MASTER);
    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    if(!StWindow::isStereoOutput() || myIsBroken) {
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myContext->stglResizeViewport(aViewPort);
        myContext->stglSetScissorRect(aViewPort, false);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        myContext->stglResetScissorRect();

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        StWindow::stglSwap(ST_WIN_ALL);
        ++myFPSControl;
        return;
    }

    StGLBoxPx aViewPortL = aViewPort;
    aViewPortL.width() /= 2;
    StGLBoxPx aViewPortR = aViewPortL;
    aViewPortR.x() += aViewPortL.width();

    // resize FBO
    if(!myFrBuffer->initLazy(*myContext, aViewPortL.width(), aViewPortL.height(), StWindow::hasDepthBuffer())) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Frame Buffer");
        myIsBroken = true;
        return;
    }

    // reduce viewport to avoid additional aliasing of narrow lines
    GLfloat aDX = GLfloat(myFrBuffer->getVPSizeX()) / GLfloat(myFrBuffer->getSizeX());
    GLfloat aDY = GLfloat(myFrBuffer->getVPSizeY()) / GLfloat(myFrBuffer->getSizeY());
    StArray<StGLVec2> aTCoords(4);
    aTCoords[0] = StGLVec2(aDX,  0.0f);
    aTCoords[1] = StGLVec2(aDX,  aDY);
    aTCoords[2] = StGLVec2(0.0f, 0.0f);
    aTCoords[3] = StGLVec2(0.0f, aDY);
    myTexCoordBuf.init(*myContext, aTCoords);

    // draw Left View into virtual frame buffer
    myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
    myFrBuffer->unbindBuffer(*myContext);

    // now draw to real screen buffer
    // clear the screen and the depth buffer
    myContext->stglResizeViewport(aViewPortL);
    myContext->stglSetScissorRect(aViewPortL, false);
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myFrBuffer->bindTexture(*myContext);
    myProgram->use(*myContext);
        myVertFlatBuf.bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        myTexCoordBuf.bindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());

        myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myTexCoordBuf.unBindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());
        myVertFlatBuf.unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());
    myProgram->unuse(*myContext);
    myFrBuffer->unbindTexture(*myContext);
    myContext->stglResetScissorRect();

    // draw Right View into virtual frame buffer
    myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
    myFrBuffer->unbindBuffer(*myContext);

    // clear the screen and the depth buffer
    myContext->stglResizeViewport(aViewPortR);
    myContext->stglSetScissorRect(aViewPortR, false);
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myFrBuffer->bindTexture(*myContext);
    myProgram->use(*myContext);
    myVertFlatBuf.bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
    myTexCoordBuf.bindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());

    myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myTexCoordBuf.unBindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());
    myVertFlatBuf.unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());

    myProgram->unuse(*myContext);
    myFrBuffer->unbindTexture(*myContext);
    myContext->stglResetScissorRect();

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    StWindow::stglSwap(ST_WIN_ALL);
    ++myFPSControl;
}

void StOutDistorted::doVSync(const bool theValue) {
    if(myContext.isNull()) {
        return;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync(theValue ? StGLContext::VSync_ON : StGLContext::VSync_OFF);
}
