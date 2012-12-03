/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLVertexBuffer_h_
#define __StGLVertexBuffer_h_

#include <StStrings/StString.h>
#include <StGL/StGLVarLocation.h>
#include <StGL/StGLVec.h>
#include <StGL/StGLResource.h>

/**
 * Enumeration indicates the data stored in VBO.
 */
typedef enum tagStGLVBOType {
    ST_VBO_VERTEX, //!< vertices
    ST_VBO_NORMAL, //!< normals
    ST_VBO_TCOORD, //!< texture coordinates
    ST_VBO_COLORS, //!< colors
    ST_VBO_INDEX,  //!< indices
} StGLVBOType;

/**
 * Vertex Buffer Object.
 */
class ST_LOCAL StGLVertexBuffer : public StGLResource {

        public:

    /**
     * Empty constructor.
     */
    StGLVertexBuffer();

    /**
     * Destructor - should be called after release()!
     */
    virtual ~StGLVertexBuffer();

    /**
     * Release GL resource.
     */
    virtual void release(StGLContext& theCtx);

    /**
     * @return true if this VBO has valid ID (was created but not necessary initialized with valid data!).
     */
    bool isValid() const {
        return myBufferId != 0;
    }

    /**
     * Generate VBO name.
     */
    bool init(StGLContext& theCtx);

    /**
     * Bind this VBO.
     */
    void bind(StGLContext& theCtx) const;

    /**
     * Unbind any VBO.
     */
    void unbind(StGLContext& theCtx) const;

    /**
     * VBO should be binded before call.
     * @param theElemSize   - specifies the number of components per generic vertex attribute. Must be 1, 2, 3, or 4;
     * @param theElemsCount - elements count;
     * @param theData       - data pointer.
     */
    void setData(StGLContext&   theCtx,
                 GLsizeiptr     theElemSize,
                 GLsizeiptr     theElemsCount,
                 const GLfloat* theData);

    void setData(StGLContext&   theCtx,
                 GLsizeiptr     theElemSize,
                 GLsizeiptr     theElemsCount,
                 const GLuint*  theData);

    void setData(StGLContext&   theCtx,
                 GLsizeiptr     theElemSize,
                 GLsizeiptr     theElemsCount,
                 const GLubyte* theData);

    bool init(StGLContext&             theCtx,
              const StArray<StGLVec2>& theArray) {
        return init(theCtx, 2, GLsizeiptr(theArray.size()), theArray.getFirst().getData());
    }

    bool init(StGLContext&             theCtx,
              const StArray<StGLVec3>& theArray) {
        return init(theCtx, 3, GLsizeiptr(theArray.size()), theArray.getFirst().getData());
    }

    bool init(StGLContext&             theCtx,
              const StArray<StGLVec4>& theArray) {
        return init(theCtx, 4, GLsizeiptr(theArray.size()), theArray.getFirst().getData());
    }

    bool init(StGLContext&   theCtx,
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

    bool init(StGLContext&   theCtx,
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

    bool init(StGLContext&   theCtx,
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

    /**
     * @return elemSize (GLsizeiptr ) - specifies the number of components per generic vertex attribute. Must be 1, 2, 3, or 4;
     */
    GLsizeiptr getElemSize() const {
        return myElemSize;
    }

    GLsizeiptr getElemsCount() const {
        return myElemsCount;
    }

    /**
     * @return the data type of each component in the array.
     */
    GLenum getDataType() const {
        return myDataType;
    }

    virtual GLenum getTarget() const;

    void bindVertexAttrib(StGLContext&    theCtx,
                          StGLVarLocation theAttribLoc) const;

    void unBindVertexAttrib(StGLContext&    theCtx,
                            StGLVarLocation theAttribLoc) const;

    StString toString() const {
        return StString("OpenGL buffer #") + myBufferId;
    }

        public:

    bool operator==(const StGLVertexBuffer& compare) const {
        return myBufferId == compare.myBufferId &&
               getTarget() == compare.getTarget();
    }

    bool operator!=(const StGLVertexBuffer& compare) const {
        return myBufferId != compare.myBufferId ||
               getTarget() != compare.getTarget();
    }

    bool operator>(const StGLVertexBuffer& compare) const {
        return myBufferId > compare.myBufferId;
    }

    bool operator<(const StGLVertexBuffer& compare) const {
        return myBufferId < compare.myBufferId;
    }

    bool operator>=(const StGLVertexBuffer& compare) const {
        return myBufferId >= compare.myBufferId;
    }

    bool operator<=(const StGLVertexBuffer& compare) const {
        return myBufferId <= compare.myBufferId;
    }

        private:

    GLuint     myBufferId;
    GLsizeiptr myElemSize;
    GLsizeiptr myElemsCount;
    GLenum     myDataType;

};

template<> inline void StArray<GLuint>::sort() {}

class ST_LOCAL StGLIndexBuffer : public StGLVertexBuffer {

        public:

    StGLIndexBuffer()
    : StGLVertexBuffer() {}

    virtual GLenum getTarget() const;

    bool init(StGLContext&           theCtx,
              const StArray<GLuint>& theArray) {
        return StGLVertexBuffer::init(theCtx, GLsizeiptr(1), GLsizeiptr(theArray.size()), &theArray.getFirst());
    }

};

#endif //__StGLVertexBuffer_h_
