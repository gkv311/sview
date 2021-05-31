/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLShader_h_
#define __StGLShader_h_

#include <StStrings/StString.h>
#include <StTemplates/StArrayList.h>
#include <StGL/StGLResource.h>

/**
 * Virtual class represents GLSL shader.
 */
class StGLShader : public StGLResource {

        public:

    static const GLuint NO_SHADER = 0;

        public:

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLShader();

    /**
     * Delete shader object and invalidate its id.
     * You should detach this shader from all programs before deleting!
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    /**
     * Implementations should define this method to return:
     * GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_GEOMETRY_SHADER.
     */
    inline GLenum getType() const {
        return myShaderType;
    }

    ST_CPPEXPORT StString getTypeString() const;

    /**
     * @return true if shader object is valid.
     */
    inline bool isValid() const {
        return myShaderId != NO_SHADER;
    }

    /**
     * @return user-specified title for this shader.
     */
    ST_CPPEXPORT virtual const StString& getTitle() const;

    /**
     * Virtual method could be overridden by classes contained
     * shader program text internally.
     */
    ST_CPPEXPORT virtual bool init();

    /**
     * Initialize the shader program from text buffer.
     * @param theNbParts  number of shader source parts
     * @param theSrcParts source code
     * @param theSrcLens  lengths of each source part
     * @return true on success
     */
    ST_CPPEXPORT bool init(StGLContext&       theCtx,
                           const GLsizei      theNbParts,
                           const char* const* theSrcParts,
                           const GLint*       theSrcLens = NULL);

    /**
     * Initialize the shader from text resource.
     * @param theCtx  bound OpenGL context
     * @param theName text resource to read
     * @return true on success
     */
    ST_CPPEXPORT bool initFile(StGLContext&    theCtx,
                               const StString& theName);

    /**
     * Initialize the shader from single string.
     */
    ST_LOCAL bool init(StGLContext& theCtx,
                       const char*  theSrc) {
        return init(theCtx, 1, &theSrc, NULL);
    }

    /**
     * Initialize the shader from two strings.
     */
    ST_LOCAL bool init(StGLContext& theCtx,
                       const char*  theSrc1,
                       const char*  theSrc2) {
        const char* const aSrc[] = { theSrc1, theSrc2 };
        return init(theCtx, theSrc2 != NULL ? 2 : 1, aSrc, NULL);
    }

    /**
     * Initialize the shader from three strings.
     */
    ST_LOCAL bool init(StGLContext& theCtx,
                       const char*  theSrc1,
                       const char*  theSrc2,
                       const char*  theSrc3) {
        const char* const aSrc[] = { theSrc1, theSrc2, theSrc3 };
        return init(theCtx, theSrc3 != NULL ? 3 : (theSrc2 != NULL ? 2 : 1), aSrc, NULL);
    }

        protected:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLShader(const StString& theTitle);

    /**
     * Check compilation state.
     * @return true if shader object compiled successfully.
     */
    ST_CPPEXPORT bool isCompiled(StGLContext& theCtx) const;

    /**
     * Returns compilation information, provided by driver.
     * This string is driver's and vendor's specific.
     * Usually it contains compilation errors and warnings.
     */
    ST_CPPEXPORT StString getCompileInfo(StGLContext& theCtx) const;

        protected:

    StString myTitle;      //!< just shader title
    GLenum   myShaderType; //!< shder type
    GLuint   myShaderId;   //!< OpenGL shader ID

        private:

    friend class StGLProgram;

};

/**
 * Class represents GLSL Vertex Shader.
 */
class StGLVertexShader : public StGLShader {

        public:

    ST_CPPEXPORT StGLVertexShader(const StString& theTitle);

};

/**
 * Class represents GLSL Fragment Shader.
 */
class StGLFragmentShader : public StGLShader {

        public:

    ST_CPPEXPORT StGLFragmentShader(const StString& theTitle);

};

template<> inline void StArray< StHandle<StGLVertexShader>   >::sort() {}
template<> inline void StArray< StHandle<StGLFragmentShader> >::sort() {}

#endif // __StGLShader_h_
