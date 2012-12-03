/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StGLShader : public StGLResource {

        public:

    static const GLuint NO_SHADER = 0;

        public:

    /**
     * Destructor - should be called after release()!
     */
    virtual ~StGLShader();

    /**
     * Delete shader object and invalidate its id.
     * You should detach this shader from all programs before deleting!
     */
    virtual void release(StGLContext& theCtx);

    /**
     * Implementations should define this method to return:
     * GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_GEOMETRY_SHADER.
     */
    inline GLenum getType() const {
        return myShaderType;
    }

    StString getTypeString() const;

    /**
     * @return true if shader object is valid.
     */
    bool isValid() const {
        return myShaderId != NO_SHADER;
    }

    /**
     * @return user-specified title for this shader.
     */
    virtual const StString& getTitle() const;

    /**
     * Vitual method could be overridden by classes contained
     * shader program text internally.
     */
    virtual bool init();

    /**
     * Initialize the shader program from text buffer.
     * @param theSrcLines0 (const char* ) - shader program source;
     * @return true on success.
     */
    virtual bool init(StGLContext& theCtx,
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
    bool initFile(StGLContext&    theCtx,
                  const StString& theFileName);

        protected:

    /**
     * Empty constructor.
     */
    StGLShader(const StString& theTitle);

    /**
     * Check compilation state.
     * @return true if shader object compiled successfully.
     */
    bool isCompiled(StGLContext& theCtx) const;

    /**
     * Returns compilation information, provided by driver.
     * This string is driver's and vendor's specific.
     * Usually it contains compilation errors and warnings.
     */
    StString getCompileInfo(StGLContext& theCtx) const;

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
class ST_LOCAL StGLVertexShader : public StGLShader {

        public:

    StGLVertexShader(const StString& theTitle);

};

/**
 * Class represents GLSL Fragment Shader.
 */
class ST_LOCAL StGLFragmentShader : public StGLShader {

        public:

    StGLFragmentShader(const StString& theTitle);

};

#endif //__StGLShader_h_
