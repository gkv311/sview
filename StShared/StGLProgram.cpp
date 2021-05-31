/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLProgram.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

StGLProgram::StGLProgram(const StString& theTitle)
: myTitle(theTitle),
  myProgramId(NO_PROGRAM) {
    //
}

StGLProgram::~StGLProgram() {
    ST_ASSERT(!isValid(), "~StGLProgram with unreleased GL resources");
}

const StString& StGLProgram::getTitle() const {
    return myTitle;
}

void StGLProgram::release(StGLContext& theCtx) {
    if(isValid()) {
        theCtx.core20fwd->glDeleteProgram(myProgramId);
        myProgramId = NO_PROGRAM;
    }
}

bool StGLProgram::init(StGLContext& ) {
    return false;
}

StGLProgram& StGLProgram::create(StGLContext& theCtx) {
    if(isValid()) {
        release(theCtx);
    }

    if(theCtx.core20fwd != NULL) {
        myProgramId = theCtx.core20fwd->glCreateProgram();
    }
    return *this;
}

StGLProgram& StGLProgram::attachShader(StGLContext&      theCtx,
                                       const StGLShader& theShader) {
    if(isValid() && theShader.isValid()) {
        theCtx.core20fwd->glAttachShader(myProgramId, theShader.myShaderId);
    }
    return *this;
}

StGLProgram& StGLProgram::detachShader(StGLContext&      theCtx,
                                       const StGLShader& theShader) {
    if(isValid() && theShader.isValid()) {
        theCtx.core20fwd->glDetachShader(myProgramId, theShader.myShaderId);
    }
    return *this;
}

bool StGLProgram::link(StGLContext& theCtx) {
    if(!isValid()) {
        return false;
    }
    theCtx.core20fwd->glLinkProgram(myProgramId);

    // if linkage failed - automatically remove the program!
    if(!isLinked(theCtx)) {
        theCtx.pushError(StString("Linking of the program '") + myTitle + "' failed!\n" + getLinkageInfo(theCtx));
        release(theCtx);
        return false;
    }
#ifdef ST_DEBUG_SHADERS
    const StString anInfo = getLinkageInfo(theCtx);
    ST_DEBUG_LOG("Program '" + myTitle + "' has been linked"
              + (!anInfo.isEmpty() ? (StString(". Log:\n") + anInfo) : (StString())));
#endif
    return true;
}

StGLVarLocation StGLProgram::getUniformLocation(StGLContext& theCtx,
                                                const char*  theVarName) const {
    if(!isValid()) {
        return StGLVarLocation();
    }
    const StGLVarLocation aLocation(theCtx.core20fwd->glGetUniformLocation(myProgramId, theVarName));
#ifdef ST_DEBUG
    if(!aLocation.isValid()) {
        ST_DEBUG_LOG("Warning! Uniform variable '" + theVarName + "' doesn't found in the whole shader!");
    }
#endif
    return aLocation;
}

StGLVarLocation StGLProgram::getAttribLocation(StGLContext& theCtx,
                                               const char*  theVarName) const {
    if(!isValid()) {
        return StGLVarLocation();
    }
    const StGLVarLocation aLocation(theCtx.core20fwd->glGetAttribLocation(myProgramId, theVarName));
#ifdef ST_DEBUG
    if(!aLocation.isValid()) {
        ST_DEBUG_LOG("Warning! Attribute variable '" + theVarName + "' doesn't found in the vertex shader!");
    }
#endif
    return aLocation;
}

StGLProgram& StGLProgram::bindAttribLocation(StGLContext&    theCtx,
                                             const char*     theVarName,
                                             StGLVarLocation theLocation) {
    if(!isValid()) {
        return *this;
    }
    theCtx.core20fwd->glBindAttribLocation(myProgramId, theLocation, theVarName);
    return *this;
}

void StGLProgram::use(StGLContext& theCtx) const {
    if(isValid()) {
        theCtx.core20fwd->glUseProgram(myProgramId); // use our shader
    }
}

void StGLProgram::unuse(StGLContext& theCtx) const {
    unuseGlobal(theCtx);
}

void StGLProgram::unuseGlobal(StGLContext& theCtx) {
    if(theCtx.core20fwd != NULL) {
        theCtx.core20fwd->glUseProgram(NO_PROGRAM); // use fixed instructions
    }
}

bool StGLProgram::isLinked(StGLContext& theCtx) const {
    GLint isSuccess = GL_FALSE;
    theCtx.core20fwd->glGetProgramiv(myProgramId, GL_LINK_STATUS, &isSuccess);
    return isSuccess == GL_TRUE;
}

StString StGLProgram::getLinkageInfo(StGLContext& theCtx) const {
    GLint anInfoLen = 0;
    theCtx.core20fwd->glGetProgramiv(myProgramId, GL_INFO_LOG_LENGTH, &anInfoLen);
    if(anInfoLen <= 0) {
        return "";
    }

    GLchar* anInfoStr = new GLchar[anInfoLen];
    GLsizei aCharsWritten = 0;
    theCtx.core20fwd->glGetProgramInfoLog(myProgramId, anInfoLen, &aCharsWritten, anInfoStr);
    StString aCompileInfo(anInfoStr);
    delete[] anInfoStr;
    return aCompileInfo;
}
