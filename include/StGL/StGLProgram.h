/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLProgram_h_
#define __StGLProgram_h_

#include <StGL/StGLShader.h>
#include <StGL/StGLVarLocation.h>

/**
 * Class represents GLSL program.
 */
class ST_LOCAL StGLProgram : public StGLResource {

        public:

    static const GLuint NO_PROGRAM = 0;

    enum {
        TEXTURE_SAMPLE_0 = 0, // GL_TEXTURE0
        TEXTURE_SAMPLE_1 = 1, // GL_TEXTURE1
        TEXTURE_SAMPLE_2 = 2, // GL_TEXTURE2
    };

        public:

    /**
     * Empty constructor.
     */
    StGLProgram(const StString& theTitle);

    /**
     * Destructor - should be called after release()!
     */
    virtual ~StGLProgram();

    /**
     * Delete program object and invalidate its id.
     * All attached shaders will be automatically detached
     * thus you should just delete shaders to clean up its memory.
     */
    virtual void release(StGLContext& theCtx);

    /**
     * @return true if program object is valid.
     */
    bool isValid() const {
        return myProgramId != NO_PROGRAM;
    }

    /**
     * @return user-specified title for this program.
     */
    virtual const StString& getTitle() const {
        return myTitle;
    }

    /**
     * Method to override.
     */
    virtual bool init(StGLContext& theCtx);

    /**
     * Just create an empty program.
     */
    StGLProgram& create(StGLContext& theCtx);

    /**
     * Attach more compiled shader objects to this program.
     */
    StGLProgram& attachShader(StGLContext&      theCtx,
                              const StGLShader& theShader);

    /**
     * Detach shader objects from this program.
     */
    StGLProgram& detachShader(StGLContext&      theCtx,
                              const StGLShader& theShader);

    /**
     * Switch one shader object to another.
     */
    StGLProgram& swapShader(StGLContext&      theCtx,
                            const StGLShader& theShaderFrom,
                            const StGLShader& theShaderTo) {
        if(&theShaderFrom != &theShaderTo) {
            detachShader(theCtx, theShaderFrom).attachShader(theCtx, theShaderTo).link(theCtx);
        }
        return *this;
    }

    /**
     * A vertex shader and fragment shader (and geometry shader) must be put together
     * to a program unit before it is possible to link.
     * Notice that default 0 value for any variable will be set after each (re)link!
     * This is good idea to perform searching for variables locations and setting your default
     * values here (using inheritance).
     * @return true if linkage success.
     */
    virtual bool link(StGLContext& theCtx);

    /**
     * @return uniform variable location in the whole shader (like projection/model matrix).
     */
    StGLVarLocation getUniformLocation(StGLContext& theCtx,
                                       const char*  theVarName) const;

    /**
     * @return per-vertex attribute location (in the vertex shader, like vertex, normal, color and so on).
     */
    StGLVarLocation getAttribLocation(StGLContext& theCtx,
                                      const char*  theVarName) const;

    /**
     * Associates a generic vertex attribute index with a named attribute variable.
     * Should be called before linkage!
     */
    void bindAttribLocation(StGLContext&    theCtx,
                            const char*     theVarName,
                            StGLVarLocation theLocation);

    /**
     * Use this program.
     */
    virtual void use(StGLContext& theCtx) const;

    /**
     * Unuse this program.
     */
    virtual void unuse(StGLContext& theCtx) const;

    static void unuseGlobal(StGLContext& theCtx);

        protected:

    /**
     * Check linkage state.
     * @return true if program object linked successfully.
     */
    bool isLinked(StGLContext& theCtx) const;

    /**
     * Returns linkage information, provided by driver.
     * This string is driver's and vendor's specific.
     * Usually it contains linkage errors and warnings.
     */
    StString getLinkageInfo(StGLContext& theCtx) const;

        protected:

    StString myTitle;     //!< just program title
    GLuint   myProgramId; //!< OpenGL shader ID

};

#endif //__StGLProgram_h_
