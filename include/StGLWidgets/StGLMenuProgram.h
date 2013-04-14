/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

class StGLMenuProgram : public StGLProgram {

        public:

    ST_CPPEXPORT StGLMenuProgram();

    inline StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    ST_CPPEXPORT void setProjMat(StGLContext&      theCtx,
                                 const StGLMatrix& theProjMat);

    ST_CPPEXPORT void setColor(StGLContext&    theCtx,
                               const StGLVec4& theColor,
                               const GLfloat   theOpacityValue);

    ST_CPPEXPORT virtual bool init(StGLContext& theCtx);

        private:

    StGLVarLocation uniProjMatLoc;
    StGLVarLocation uniColorLoc;
    StGLVarLocation atrVVertexLoc;

};

#endif //__StGLMenuProgram_h_
