/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMenuProgram_h_
#define __StGLMenuProgram_h_

#include <StGL/StGLProgram.h>
#include <StGL/StGLVec.h>

class StGLMatrix;

class ST_LOCAL StGLMenuProgram : public StGLProgram {

        public:

    StGLMenuProgram();

    StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    void setProjMat(StGLContext&      theCtx,
                    const StGLMatrix& theProjMat);

    void setColor(StGLContext&    theCtx,
                  const StGLVec4& theColor,
                  const GLfloat   theOpacityValue);

    virtual bool init(StGLContext& theCtx);

        private:

    StGLVarLocation uniProjMatLoc;
    StGLVarLocation uniColorLoc;
    StGLVarLocation atrVVertexLoc;

};

#endif //__StGLMenuProgram_h_
