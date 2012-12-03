/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
    }
}

GLenum StGLVertexBuffer::getTarget() const {
    return GL_ARRAY_BUFFER;
}

GLenum StGLIndexBuffer::getTarget() const {
    return GL_ELEMENT_ARRAY_BUFFER;
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

    myElemSize   = theElemSize;
    myElemsCount = theElemsCount;
    theCtx.core20fwd->glBufferData(getTarget(), myElemsCount * myElemSize * sizeof(GLfloat), theData, GL_STATIC_DRAW);
    myDataType = GL_FLOAT;
}

void StGLVertexBuffer::setData(StGLContext&   theCtx,
                               GLsizeiptr     theElemSize,
                               GLsizeiptr     theElemsCount,
                               const GLuint*  theData) {
    if(!isValid()) {
        return;
    }

    myElemSize   = theElemSize;
    myElemsCount = theElemsCount;
    theCtx.core20fwd->glBufferData(getTarget(), myElemsCount * myElemSize * sizeof(GLuint), theData, GL_STATIC_DRAW);
    myDataType = GL_UNSIGNED_INT;
}

void StGLVertexBuffer::setData(StGLContext&   theCtx,
                               GLsizeiptr     theElemSize,
                               GLsizeiptr     theElemsCount,
                               const GLubyte* theData) {
    if(!isValid()) {
        return;
    }

    myElemSize   = theElemSize;
    myElemsCount = theElemsCount;
    theCtx.core20fwd->glBufferData(getTarget(), myElemsCount * myElemSize * sizeof(GLubyte), theData, GL_STATIC_DRAW);
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
