/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLShader.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

#include <StFile/StRawFile.h>
#include <StStrings/StLogger.h>
#include <stAssert.h>

namespace {
#ifdef GL_ES_VERSION_2_0
    static const char THE_FRAG_PREC_HIGH[] = "precision highp float;\n";
    static const char THE_FRAG_PREC_LOW[]  = "precision mediump float;\n";
#endif
}

StString StGLShader::getTypeString() const {
    switch(getType()) {
        case GL_VERTEX_SHADER:   return StString("Vertex Shader");
        case GL_FRAGMENT_SHADER: return StString("Fragment Shader");
        ///case GL_GEOMETRY_SHADER: return StString("Geometry Shader");
        default:                 return StString("Unknown Shader");
    }
}

StGLShader::StGLShader(const StString& theTitle)
: myTitle(theTitle),
  myShaderType(0),
  myShaderId(NO_SHADER) {
    //
}

StGLShader::~StGLShader() {
    ST_ASSERT(!isValid(), "~StGLShader() with unreleased GL resources");
}

void StGLShader::release(StGLContext& theCtx) {
    if(isValid()) {
        theCtx.core20fwd->glDeleteShader(myShaderId);
        myShaderId = NO_SHADER;
    }
}

bool StGLShader::init() {
    return false;
}

const StString& StGLShader::getTitle() const {
    return myTitle;
}

bool StGLShader::init(StGLContext&       theCtx,
                      const GLsizei      theNbParts,
                      const char* const* theSrcParts,
                      const GLint*       theSrcLens) {
    if(theNbParts < 1
    || theSrcParts == NULL
    || theSrcParts[0] == NULL
    || theCtx.core20fwd == NULL) {
        return false;
    }

    if(myShaderId == NO_SHADER) {
        myShaderId = theCtx.core20fwd->glCreateShader(getType());
    }

#if defined(GL_ES_VERSION_2_0)
    if(myShaderType == GL_FRAGMENT_SHADER) {
        const char** aSrcParts = (const char** )alloca(sizeof(char*) * (theNbParts + 1));
        GLint*       aSrcLens  = (GLint*       )alloca(sizeof(GLint) * (theNbParts + 1));
        aSrcParts[0] = theCtx.hasHighp ? THE_FRAG_PREC_HIGH : THE_FRAG_PREC_LOW;
        aSrcLens [0] = -1;
        for(GLsizei aPartIter = 0; aPartIter < theNbParts; ++aPartIter) {
            aSrcParts[aPartIter + 1] = theSrcParts[aPartIter];
            aSrcLens [aPartIter + 1] = theSrcLens != NULL ? theSrcLens[aPartIter] : -1;
        }
        theCtx.core20fwd->glShaderSource(myShaderId, theNbParts + 1, aSrcParts, aSrcLens);
    } else {
        theCtx.core20fwd->glShaderSource(myShaderId, theNbParts, (const GLchar** )theSrcParts, theSrcLens);
    }
#else
    theCtx.core20fwd->glShaderSource(myShaderId, theNbParts, theSrcParts, theSrcLens);
#endif

    // compile shaders
    theCtx.core20fwd->glCompileShader(myShaderId);

    // check compile success
    if(!isCompiled(theCtx)) {
        StString aSrc;
        StString aSrcNumbered;
        GLsizei aPartFrom = 0;
        char aNumBuff[128];

    #if defined(GL_ES_VERSION_2_0)
        if(myShaderType == GL_FRAGMENT_SHADER) {
            aPartFrom = 1;
            StString aLine = theCtx.hasHighp ? THE_FRAG_PREC_HIGH : THE_FRAG_PREC_LOW;
            aSrc += aLine;
            stsprintf(aNumBuff, 127, "%d:%03d ", 0, 1);
            aSrcNumbered += StString(aNumBuff) + aLine.subString(0, aLine.getLength() - 1);
        }
    #endif
        for(GLsizei aPartIter = 0; aPartIter < theNbParts; ++aPartIter) {
            StString aPartSrc = theSrcParts[aPartIter];
            aSrc += aPartSrc;

            StHandle <StArrayList<StString> > anArray = aPartSrc.split('\n');
            for(size_t aLineIter = 0; aLineIter < anArray->size(); ++aLineIter) {
                stsprintf(aNumBuff, 127, "%d:%03d ", aPartFrom + aPartIter, int(aLineIter + 1));
                StString aLine = StString(aNumBuff) + anArray->getValue(aLineIter);
                if(!aSrcNumbered.isEmpty()) {
                    aSrcNumbered += "\n";
                }
                aSrcNumbered += aLine;
            }
        }

        theCtx.pushError(StString("Compilation of the ") + getTypeString() + " '" + myTitle
                       + "' failed!\n" + getCompileInfo(theCtx)
                       + "\n=== Source code ===\n"
                       + aSrcNumbered
                       + "\n==================="
        );
        release(theCtx);
        return false;
    }
#ifdef ST_DEBUG_SHADERS
    const StString anInfo = getCompileInfo(theCtx);
    ST_DEBUG_LOG(getTypeString() + " '" + myTitle + "' has been compiled"
              + (!anInfo.isEmpty() ? (StString(". Log:\n") + anInfo) : (StString())));
#endif
    return true;
}

bool StGLShader::initFile(StGLContext&    theCtx,
                          const StString& theName) {
    StHandle<StResource> aRes = theCtx.getResourceManager()->getResource(theName);
    if( aRes.isNull()
    || !aRes->read()) {
        theCtx.pushError(StString("Shader file '") + theName + "' is not found!");
        return false;
    }
    const char* aSrc = (const char* )aRes->getData();
    GLint       aLen = aRes->getSize();
    return init(theCtx, 1, &aSrc, &aLen);
}

bool StGLShader::isCompiled(StGLContext& theCtx) const {
    GLint isSuccess = GL_FALSE;
    theCtx.core20fwd->glGetShaderiv(myShaderId, GL_COMPILE_STATUS, &isSuccess);
    return isSuccess == GL_TRUE;
}

StString StGLShader::getCompileInfo(StGLContext& theCtx) const {
    GLint anInfoLen = 0;
    theCtx.core20fwd->glGetShaderiv(myShaderId, GL_INFO_LOG_LENGTH, &anInfoLen);
    if(anInfoLen <= 0) {
        return "";
    }

    GLchar* anInfoStr = new GLchar[anInfoLen];
    GLsizei aCharsWritten = 0;
    theCtx.core20fwd->glGetShaderInfoLog(myShaderId, anInfoLen, &aCharsWritten, anInfoStr);
    StString aCompileInfo(anInfoStr);
    delete[] anInfoStr;
    return aCompileInfo;
}

StGLVertexShader::StGLVertexShader(const StString& theTitle)
: StGLShader(theTitle) {
    myShaderType = GL_VERTEX_SHADER;
}

StGLFragmentShader::StGLFragmentShader(const StString& theTitle)
: StGLShader(theTitle) {
    myShaderType = GL_FRAGMENT_SHADER;
}
