/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
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

#include "StTestGlStress.h"

#include <StCore/StWindow.h>

#include <StGL/StGLContext.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLTexture.h>
#include <StGLCore/StGLCore20.h>

#include <StStrings/stConsole.h>
#include <StStrings/StLogger.h>
#include <StThreads/StThread.h>

void StTestGlStress::perform() {
    // create the window
    StHandle<StWindow> aWin = new StWindow();
    aWin->setPlacement(StRectI_t(256, 768, 256, 768));
    aWin->setTitle("sView - Tests");
    aWin->create();

    // perform tests
    aWin->stglMakeCurrent();
    StGLContext aCtx(true);

    const StString aGlInfo = aCtx.stglFullInfo();
#ifndef __ST_DEBUG__
    st::cout << aGlInfo << stostream_text("\n");
#else
    ST_DEBUG_LOG(aGlInfo);
#endif

    StGLTexture aTexture;
    aTexture.initTrash(aCtx, 512, 512);

    StGLVertexShader aShaderVert("GLSL infinite loop Vertex Shader");
    aShaderVert.init(aCtx, "void main(void) { gl_Position = ftransform(); }");

    StGLFragmentShader aShaderFrag("GLSL infinite loop Fragment Shader");
    //aShaderFrag.init(aCtx, "void main(void) { gl_FragColor = vec4(0.0, 0.0, 1.0, 0.0); }");
    aShaderFrag.init(aCtx,
                     "uniform sampler2D uTexture;\n"
                     "void main(void) {\n"
                     "  vec4 aColor = vec4(0.0, 0.0, 1.0, 0.0);\n"
                     "  for(float anIter = 2.0; anIter > -10.0; anIter += 1.0) {\n"
                     "    if(int(mod(texture2D(uTexture, vec2(anIter, -anIter)).r, 2.0)) == 1) {\n"
                     "      aColor.g += texture2D(uTexture, vec2(anIter, -anIter)).g;\n"
                     "      anIter = 4.0;\n"
                     "    } else if(int(mod(texture2D(uTexture, vec2(anIter, -anIter)).r, 2.0)) == 1) {\n"
                     "      aColor.g -= texture2D(uTexture, vec2(-anIter, anIter)).g;\n"
                     "    } else {\n"
                     "      aColor.g -= texture2D(uTexture, vec2(-anIter, -anIter)).g;\n"
                     "      anIter -= 1.0;\n"
                     "    }\n"
                     "  }\n"
                     "  gl_FragColor = aColor;\n"
                     "}\n");

    StGLProgram aProgram("GLSL infinite loop program");
    aProgram.create(aCtx).attachShader(aCtx, aShaderVert).attachShader(aCtx, aShaderFrag).link(aCtx);

    StGLVarLocation uniTexLoc = aProgram.getUniformLocation(aCtx, "uTexture");

    const StGLBoxPx aVPort = aWin->stglViewport(ST_WIN_MASTER);
    aCtx.stglResizeViewport(aVPort);
    for(int anIter = 0; anIter < 10; ++anIter) {
        aCtx.core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        aCtx.core11->glColor3f(1.0f, 0.0f, 0.0f);
        aTexture.bind(aCtx);
        aProgram.use(aCtx);
        aCtx.core20fwd->glUniform1i(uniTexLoc, StGLProgram::TEXTURE_SAMPLE_0);
        aCtx.core11->glBegin(GL_QUADS);
        aCtx.core11->glVertex2f(-1.0f, -1.0f);
        aCtx.core11->glVertex2f(-1.0f,  1.0f);
        aCtx.core11->glVertex2f( 1.0f,  1.0f);
        aCtx.core11->glVertex2f( 1.0f, -1.0f);
        aCtx.core11->glEnd();
        aProgram.unuse(aCtx);
        aTexture.unbind(aCtx);
        aWin->stglSwap();
        aWin->processEvents();
        StThread::sleep(100);
    }
    aShaderVert.release(aCtx);
    aShaderFrag.release(aCtx);
    aProgram.release(aCtx);
    aTexture.release(aCtx);

    // close the window
    aWin.nullify();
}
