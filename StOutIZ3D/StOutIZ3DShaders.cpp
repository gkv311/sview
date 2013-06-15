/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutIZ3D library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutIZ3D library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutIZ3DShaders.h"
#include <StThreads/StProcess.h>

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>

#include <stAssert.h>

StOutIZ3DShaders::StOutIZ3DShaders()
: myBack(NULL),
  myFront(NULL),
  myBackClassic ("iZ3D Classic Back"),
  myFrontClassic("iZ3D Classic Front"),
  myBackTable   ("iZ3D Table Back"),
  myFrontTable  ("iZ3D Table Front"),
  myMode(IZ3D_TABLE_OLD) {
    myBack  = &myBackClassic;
    myFront = &myFrontClassic;
}

StOutIZ3DShaders::~StOutIZ3DShaders() {
    ST_ASSERT(!myBackClassic.isValid()
           && !myFrontClassic.isValid()
           && !myBackTable.isValid()
           && !myFrontTable.isValid(),
              "~StOutIZ3DShaders() with unreleased GL resources");
}

void StOutIZ3DShaders::setMode(const int theMode) {
    myMode = theMode;
    switch(theMode) {
        case IZ3D_TABLE_OLD:
        case IZ3D_TABLE_NEW: {
            myBack  = &myBackTable;
            myFront = &myFrontTable;
            break;
        }
        default: {
            myBack  = &myBackClassic;
            myFront = &myFrontClassic;
        }
    }
}

bool StOutIZ3DShaders::init(StGLContext& theCtx) {
    // shaders data
    const StString VSHADER                    = "vIZ3D.shv";
    const StString FSHADER_IZ3D_BACK_CLASSIC  = "fIZ3DBackClassic.shf";
    const StString FSHADER_IZ3D_FRONT_CLASSIC = "fIZ3DFrontClassic.shf";
    const StString FSHADER_IZ3D_BACK_TABLE    = "fIZ3DBackTable.shf";
    const StString FSHADER_IZ3D_FRONT_TABLE   = "fIZ3DFrontTable.shf";

    StString aShadersError("StOutIZ3D Plugin, Failed to init Shaders");
    const StString aShadersRoot = StProcess::getStShareFolder() + "shaders" + SYS_FS_SPLITTER
                                   + "StOutIZ3D" + SYS_FS_SPLITTER;

    // initialize shaders
    StGLVertexShader aVertexShader("iZ3D"); // common vertex shader
    StGLAutoRelease aTmp1(theCtx, aVertexShader);
    if(!aVertexShader.initFile(theCtx, aShadersRoot + VSHADER)) {
        stError(aShadersError);
        return false;
    }

    StGLFragmentShader stFBackClassic(myBackClassic.getTitle());
    StGLAutoRelease aTmp2(theCtx, stFBackClassic);
    if(!stFBackClassic.initFile(theCtx, aShadersRoot + FSHADER_IZ3D_BACK_CLASSIC)) {
        stError(aShadersError);
        return false;
    }
    myBackClassic.create(theCtx)
                 .attachShader(theCtx, aVertexShader)
                 .attachShader(theCtx, stFBackClassic)
                 .link(theCtx);

    StGLFragmentShader stFFrontClassic(myFrontClassic.getTitle());
    StGLAutoRelease aTmp3(theCtx, stFFrontClassic);
    if(!stFFrontClassic.initFile(theCtx, aShadersRoot + FSHADER_IZ3D_FRONT_CLASSIC)) {
        stError(aShadersError);
        return false;
    }
    myFrontClassic.create(theCtx)
                  .attachShader(theCtx, aVertexShader)
                  .attachShader(theCtx, stFFrontClassic)
                  .link(theCtx);

    StGLFragmentShader stFBackTable(myBackTable.getTitle());
    StGLAutoRelease aTmp4(theCtx, stFBackTable);
    if(!stFBackTable.initFile(theCtx, aShadersRoot + FSHADER_IZ3D_BACK_TABLE)) {
        stError(aShadersError);
        return false;
    }
    myBackTable.create(theCtx)
               .attachShader(theCtx, aVertexShader)
               .attachShader(theCtx, stFBackTable)
               .link(theCtx);

    StGLFragmentShader stFFrontTable(myFrontTable.getTitle());
    StGLAutoRelease aTmp5(theCtx, stFFrontTable);
    if(!stFFrontTable.initFile(theCtx, aShadersRoot + FSHADER_IZ3D_FRONT_TABLE)) {
        stError(aShadersError);
        return false;
    }
    myFrontTable.create(theCtx)
                .attachShader(theCtx, aVertexShader)
                .attachShader(theCtx, stFFrontTable)
                .link(theCtx);

    // get shaders' variables' locations
    StGLVarLocation uniTTLocBack  =  myBackTable.getUniformLocation(theCtx, "textT");
    StGLVarLocation uniTTLocFront = myFrontTable.getUniformLocation(theCtx, "textT");

    if(myBackTable.isValid() && uniTTLocBack.isValid()) {
        myBackTable.use(theCtx);
        theCtx.core20fwd->glUniform1i(uniTTLocBack, 2); // GL_TEXTURE2
        myBackTable.unuse(theCtx);
    }

    if(myFrontTable.isValid() && uniTTLocFront.isValid()) {
        myFrontTable.use(theCtx);
        theCtx.core20fwd->glUniform1i(uniTTLocFront, 2); // GL_TEXTURE2
        myFrontTable.unuse(theCtx);
    }

    return (myBackClassic.isValid() && myFrontClassic.isValid())
        || (  myBackTable.isValid() && myFrontTable.isValid() && uniTTLocBack.isValid() && uniTTLocFront.isValid());
}

void StOutIZ3DShaders::release(StGLContext& theCtx) {
    myBackClassic.release(theCtx);
    myFrontClassic.release(theCtx);
    myBackTable.release(theCtx);
    myFrontTable.release(theCtx);
}
