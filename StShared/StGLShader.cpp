/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StGLShader.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

#include <StFile/StRawFile.h>
#include <StStrings/StLogger.h>
#include <stAssert.h>

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

bool StGLShader::init(StGLContext& theCtx,
                      const char*  theSrcLines0,
                      const char*  theSrcLines1,
                      const char*  theSrcLines2) {
    if(theSrcLines0 == NULL
    || theCtx.core20fwd == NULL) {
        return false;
    }

    GLsizei aCount = (theSrcLines1 == NULL) ? 1 : ((theSrcLines2 == NULL) ? 2 : 3);
    const char* aSrcArray[3] = {theSrcLines0, theSrcLines1, theSrcLines2};

    myShaderId = theCtx.core20fwd->glCreateShader(getType());
    theCtx.core20fwd->glShaderSource(myShaderId, aCount, aSrcArray, NULL);

    // compile shaders
    theCtx.core20fwd->glCompileShader(myShaderId);

    // check compile success
    if(!isCompiled(theCtx)) {
        theCtx.pushError(StString("Compilation of the ") + getTypeString() + " '" + myTitle
                       + "' failed!\n" + getCompileInfo(theCtx)
                       /*+ "\n=== Source code ===\n"
                       + (theSrcLines0 != NULL ? theSrcLines0 : "")
                       + (theSrcLines1 != NULL ? theSrcLines1 : "")
                       + (theSrcLines2 != NULL ? theSrcLines2 : "")
                       + "==================="*/
        );
        release(theCtx);
        return false;
    }
#ifdef __ST_DEBUG_SHADERS__
    const StString anInfo = getCompileInfo(theCtx);
    ST_DEBUG_LOG(getTypeString() + " '" + myTitle + "' has been compiled"
              + (!anInfo.isEmpty() ? (StString(". Log:\n") + anInfo) : (StString())));
#endif
    return true;
}

bool StGLShader::initFile(StGLContext&    theCtx,
                          const StString& theFileName) {
    StRawFile aTextFile(theFileName);
    if(!aTextFile.readFile()) {
        theCtx.pushError(StString("Shader file '") + theFileName + "' is not found!");
        return false;
    }
    return init(theCtx, (const char* )aTextFile.getBuffer());
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
