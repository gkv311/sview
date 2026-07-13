/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLVertexBuffer.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLContext.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

StGLVertexBuffer::StGLVertexBuffer()
: myBufferId(0),
  myElemSize(4),
  myElemsCount(0),
  myDataType(GL_FLOAT) {
    //
}

StGLVertexBuffer::~StGLVertexBuffer() {
    ST_ASSERT(!isValid(), "~StGLVertexBuffer() with unreleased GL resources");
}

void StGLVertexBuffer::release(StGLContext& theCtx) {
    if(isValid()) {
        theCtx.core20fwd->glDeleteBuffers(1, &myBufferId);
        myBufferId = 0;
        myElemSize = 0;
    }
}

GLenum StGLVertexBuffer::getTarget() const {
    return GL_ARRAY_BUFFER;
}

StGLIndexBuffer::StGLIndexBuffer()
: StGLVertexBuffer() {
    //
}

GLenum StGLIndexBuffer::getTarget() const {
    return GL_ELEMENT_ARRAY_BUFFER;
}

bool StGLVertexBuffer::init(StGLContext&   theCtx,
                            GLsizeiptr     theElemSize,
                            GLsizeiptr     theElemsCount,
                            const GLfloat* theData) {
    if(!init(theCtx)) {
        return false;
    }
    bind(theCtx);
    setData(theCtx, theElemSize, theElemsCount, theData);
    unbind(theCtx);
    return true;
}

bool StGLVertexBuffer::init(StGLContext&   theCtx,
                            GLsizeiptr     theElemSize,
                            GLsizeiptr     theElemsCount,
                            const GLuint*  theData) {
    if(!init(theCtx)) {
        return false;
    }
    bind(theCtx);
    setData(theCtx, theElemSize, theElemsCount, theData);
    unbind(theCtx);
    return true;
}

bool StGLVertexBuffer::init(StGLContext&   theCtx,
                            GLsizeiptr     theElemSize,
                            GLsizeiptr     theElemsCount,
                            const GLubyte* theData) {
    if(!init(theCtx)) {
        return false;
    }
    bind(theCtx);
    setData(theCtx, theElemSize, theElemsCount, theData);
    unbind(theCtx);
    return true;
}

bool StGLVertexBuffer::init(StGLContext& theCtx) {
    if(!isValid() && theCtx.core20fwd != NULL) {
        theCtx.core20fwd->glGenBuffers(1, &myBufferId);
    }
    return isValid();
}

void StGLVertexBuffer::bind(StGLContext& theCtx) const {
    if(isValid()) {
        theCtx.core20fwd->glBindBuffer(getTarget(), myBufferId);
    }
}

void StGLVertexBuffer::unbind(StGLContext& theCtx) const {
    if(isValid()) {
        theCtx.core20fwd->glBindBuffer(getTarget(), 0);
    }
}

void StGLVertexBuffer::setData(StGLContext&   theCtx,
                               GLsizeiptr     theElemSize,
                               GLsizeiptr     theElemsCount,
                               const GLfloat* theData) {
    if(!isValid()) {
        return;
    }

    const GLsizeiptr aSize = theElemsCount * theElemSize * sizeof(GLfloat);
    if(myElemSize   == theElemSize
    && myElemsCount == theElemsCount
    && myDataType   == GL_FLOAT) {
        theCtx.core20fwd->glBufferSubData(getTarget(), 0, aSize, theData);
        return;
    }

    myElemSize   = theElemSize;
    myElemsCount = theElemsCount;
    theCtx.core20fwd->glBufferData(getTarget(), aSize, theData, GL_STATIC_DRAW);
    myDataType = GL_FLOAT;
}

void StGLVertexBuffer::setData(StGLContext&   theCtx,
                               GLsizeiptr     theElemSize,
                               GLsizeiptr     theElemsCount,
                               const GLuint*  theData) {
    if(!isValid()) {
        return;
    }

    const GLsizeiptr aSize = theElemsCount * theElemSize * sizeof(GLuint);
    if(myElemSize   == theElemSize
    && myElemsCount == theElemsCount
    && myDataType   == GL_UNSIGNED_INT) {
        theCtx.core20fwd->glBufferSubData(getTarget(), 0, aSize, theData);
        return;
    }

    myElemSize   = theElemSize;
    myElemsCount = theElemsCount;
    theCtx.core20fwd->glBufferData(getTarget(), aSize, theData, GL_STATIC_DRAW);
    myDataType = GL_UNSIGNED_INT;
}

void StGLVertexBuffer::setData(StGLContext&   theCtx,
                               GLsizeiptr     theElemSize,
                               GLsizeiptr     theElemsCount,
                               const GLubyte* theData) {
    if(!isValid()) {
        return;
    }

    const GLsizeiptr aSize = theElemsCount * theElemSize * sizeof(GLubyte);
    if(myElemSize   == theElemSize
    && myElemsCount == theElemsCount
    && myDataType   == GL_UNSIGNED_BYTE) {
        theCtx.core20fwd->glBufferSubData(getTarget(), 0, aSize, theData);
        return;
    }

    myElemSize   = theElemSize;
    myElemsCount = theElemsCount;
    theCtx.core20fwd->glBufferData(getTarget(), aSize, theData, GL_STATIC_DRAW);
    myDataType = GL_UNSIGNED_BYTE;
}

void StGLVertexBuffer::bindVertexAttrib(StGLContext&    theCtx,
                                        StGLVarLocation theAttribLoc) const {
    if(isValid() && theAttribLoc.isValid()) {
        bind(theCtx);
        theCtx.core20fwd->glEnableVertexAttribArray(theAttribLoc);
        theCtx.core20fwd->glVertexAttribPointer(theAttribLoc, GLint(getElemSize()), getDataType(), GL_FALSE, 0, NULL);
    }
}

void StGLVertexBuffer::unBindVertexAttrib(StGLContext&    theCtx,
                                          StGLVarLocation theAttribLoc) const {
    if(isValid() && theAttribLoc.isValid()) {
        theCtx.core20fwd->glDisableVertexAttribArray(theAttribLoc);
        unbind(theCtx);
    }
}
