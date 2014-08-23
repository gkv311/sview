/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLShader_h_
#define __StGLShader_h_

#include <StStrings/StString.h>
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
    ST_CPPEXPORT virtual void release(StGLContext& theCtx);

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
     * Vitual method could be overridden by classes contained
     * shader program text internally.
     */
    ST_CPPEXPORT virtual bool init();

    /**
     * Initialize the shader program from text buffer.
     * @param theSrcLines0 (const char* ) - shader program source;
     * @return true on success.
     */
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx,
                                   const char*  theSrcLines0,
                                   const char*  theSrcLines1 = NULL,
                                   const char*  theSrcLines2 = NULL);

    /**virtual bool init(const StString& sourceUtfString) {
        std::string sourceString = sourceUtfString.ansiText();
        return init(sourceString.c_str());
    }*/

    /**
     * Initialize the shader from text file.
     * @param theFileName (const StString& ) - file to read;
     * @return true on success.
     */
    ST_CPPEXPORT bool initFile(StGLContext&    theCtx,
                               const StString& theFileName);

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

#endif //__StGLShader_h_
