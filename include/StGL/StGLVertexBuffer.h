/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLVertexBuffer_h_
#define __StGLVertexBuffer_h_

#include <StStrings/StString.h>
#include <StGL/StGLVarLocation.h>
#include <StGL/StGLVec.h>
#include <StGL/StGLResource.h>

#include <vector>

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
class StGLVertexBuffer : public StGLResource {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLVertexBuffer();

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLVertexBuffer();

    /**
     * Release GL resource.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    /**
     * @return true if this VBO has valid ID (was created but not necessary initialized with valid data!).
     */
    inline bool isValid() const {
        return myBufferId != 0;
    }

    /**
     * Generate VBO name.
     */
    ST_CPPEXPORT bool init(StGLContext& theCtx);

    /**
     * Bind this VBO.
     */
    ST_CPPEXPORT void bind(StGLContext& theCtx) const;

    /**
     * Unbind any VBO.
     */
    ST_CPPEXPORT void unbind(StGLContext& theCtx) const;

    /**
     * VBO should be binded before call.
     * @param theElemSize   specifies the number of components per generic vertex attribute. Must be 1, 2, 3, or 4
     * @param theElemsCount elements count
     * @param theData       data pointer
     */
    ST_CPPEXPORT void setData(StGLContext&   theCtx,
                              GLsizeiptr     theElemSize,
                              GLsizeiptr     theElemsCount,
                              const GLfloat* theData);

    ST_CPPEXPORT void setData(StGLContext&   theCtx,
                              GLsizeiptr     theElemSize,
                              GLsizeiptr     theElemsCount,
                              const GLuint*  theData);

    ST_CPPEXPORT void setData(StGLContext&   theCtx,
                              GLsizeiptr     theElemSize,
                              GLsizeiptr     theElemsCount,
                              const GLubyte* theData);

    inline bool init(StGLContext&             theCtx,
                     const StArray<StGLVec2>& theArray) {
        return init(theCtx, 2, GLsizeiptr(theArray.size()), theArray.getFirst().getData());
    }

    inline bool init(StGLContext&             theCtx,
                     const StArray<StGLVec3>& theArray) {
        return init(theCtx, 3, GLsizeiptr(theArray.size()), theArray.getFirst().getData());
    }

    inline bool init(StGLContext&             theCtx,
                     const StArray<StGLVec4>& theArray) {
        return init(theCtx, 4, GLsizeiptr(theArray.size()), theArray.getFirst().getData());
    }

    ST_LOCAL bool init(StGLContext& theCtx,
                       const std::vector<StGLVec2>& theArray) {
        return init(theCtx, 2, GLsizeiptr(theArray.size()), theArray.front().getData());
    }

    ST_LOCAL bool init(StGLContext& theCtx,
                       const std::vector<StGLVec3>& theArray) {
        return init(theCtx, 3, GLsizeiptr(theArray.size()), theArray.front().getData());
    }

    ST_LOCAL bool init(StGLContext& theCtx,
                       const std::vector<StGLVec4>& theArray) {
        return init(theCtx, 4, GLsizeiptr(theArray.size()), theArray.front().getData());
    }

    ST_CPPEXPORT bool init(StGLContext&   theCtx,
                           GLsizeiptr     theElemSize,
                           GLsizeiptr     theElemsCount,
                           const GLfloat* theData);

    ST_CPPEXPORT bool init(StGLContext&   theCtx,
                           GLsizeiptr     theElemSize,
                           GLsizeiptr     theElemsCount,
                           const GLuint*  theData);

    ST_CPPEXPORT bool init(StGLContext&   theCtx,
                           GLsizeiptr     theElemSize,
                           GLsizeiptr     theElemsCount,
                           const GLubyte* theData);

    /**
     * @return elemSize (GLsizeiptr ) - specifies the number of components per generic vertex attribute. Must be 1, 2, 3, or 4;
     */
    inline GLsizeiptr getElemSize() const {
        return myElemSize;
    }

    inline GLsizeiptr getElemsCount() const {
        return myElemsCount;
    }

    /**
     * @return the data type of each component in the array.
     */
    inline GLenum getDataType() const {
        return myDataType;
    }

    ST_CPPEXPORT virtual GLenum getTarget() const;

    ST_CPPEXPORT void bindVertexAttrib(StGLContext&    theCtx,
                                       StGLVarLocation theAttribLoc) const;

    ST_CPPEXPORT void unBindVertexAttrib(StGLContext&    theCtx,
                                         StGLVarLocation theAttribLoc) const;

    inline StString toString() const {
        return StString("OpenGL buffer #") + myBufferId;
    }

        public:

    inline bool operator==(const StGLVertexBuffer& compare) const {
        return myBufferId == compare.myBufferId &&
               getTarget() == compare.getTarget();
    }

    inline bool operator!=(const StGLVertexBuffer& compare) const {
        return myBufferId != compare.myBufferId ||
               getTarget() != compare.getTarget();
    }

    inline bool operator>(const StGLVertexBuffer& compare) const {
        return myBufferId > compare.myBufferId;
    }

    inline bool operator<(const StGLVertexBuffer& compare) const {
        return myBufferId < compare.myBufferId;
    }

    inline bool operator>=(const StGLVertexBuffer& compare) const {
        return myBufferId >= compare.myBufferId;
    }

    inline bool operator<=(const StGLVertexBuffer& compare) const {
        return myBufferId <= compare.myBufferId;
    }

        private:

    GLuint     myBufferId;
    GLsizeiptr myElemSize;
    GLsizeiptr myElemsCount;
    GLenum     myDataType;

};

template<> inline void StArray<GLuint>::sort() {}

class StGLIndexBuffer : public StGLVertexBuffer {

        public:

    ST_CPPEXPORT StGLIndexBuffer();

    ST_CPPEXPORT virtual GLenum getTarget() const ST_ATTR_OVERRIDE;

    inline bool init(StGLContext&           theCtx,
                     const StArray<GLuint>& theArray) {
        return StGLVertexBuffer::init(theCtx, GLsizeiptr(1), GLsizeiptr(theArray.size()), &theArray.getFirst());
    }

};

#endif //__StGLVertexBuffer_h_
