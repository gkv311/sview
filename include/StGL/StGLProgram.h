/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLProgram_h_
#define __StGLProgram_h_

#include <StGL/StGLShader.h>
#include <StGL/StGLVarLocation.h>

/**
 * Class represents GLSL program.
 */
class StGLProgram : public StGLResource {

        public:

    static const GLuint NO_PROGRAM = 0;

    enum {
        TEXTURE_SAMPLE_0 = 0, // GL_TEXTURE0
        TEXTURE_SAMPLE_1 = 1, // GL_TEXTURE1
        TEXTURE_SAMPLE_2 = 2, // GL_TEXTURE2
        TEXTURE_SAMPLE_3 = 3, // GL_TEXTURE3
    };

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLProgram(const StString& theTitle);

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLProgram();

    /**
     * Delete program object and invalidate its id.
     * All attached shaders will be automatically detached
     * thus you should just delete shaders to clean up its memory.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    /**
     * @return true if program object is valid.
     */
    inline bool isValid() const {
        return myProgramId != NO_PROGRAM;
    }

    /**
     * @return user-specified title for this program.
     */
    ST_CPPEXPORT const StString& getTitle() const;

    /**
     * Method to override.
     */
    ST_CPPEXPORT virtual bool init(StGLContext& theCtx);

    /**
     * Just create an empty program.
     */
    ST_CPPEXPORT StGLProgram& create(StGLContext& theCtx);

    /**
     * Attach more compiled shader objects to this program.
     */
    ST_CPPEXPORT StGLProgram& attachShader(StGLContext&      theCtx,
                                           const StGLShader& theShader);

    /**
     * Detach shader objects from this program.
     */
    ST_CPPEXPORT StGLProgram& detachShader(StGLContext&      theCtx,
                                           const StGLShader& theShader);

    /**
     * Switch one shader object to another.
     */
    inline StGLProgram& swapShader(StGLContext&      theCtx,
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
    ST_CPPEXPORT virtual bool link(StGLContext& theCtx);

    /**
     * @return uniform variable location in the whole shader
     */
    ST_CPPEXPORT StGLVarLocation getUniformLocation(StGLContext& theCtx,
                                                    const char*  theVarName) const;

    /**
     * @return per-vertex attribute location (in the vertex shader, like vertex, normal, color and so on)
     */
    ST_CPPEXPORT StGLVarLocation getAttribLocation(StGLContext& theCtx,
                                                   const char*  theVarName) const;

    /**
     * Associates a generic vertex attribute index with a named attribute variable.
     * Should be called before linkage!
     */
    ST_CPPEXPORT StGLProgram& bindAttribLocation(StGLContext&    theCtx,
                                                 const char*     theVarName,
                                                 StGLVarLocation theLocation);

    /**
     * Use this program.
     */
    ST_CPPEXPORT virtual void use(StGLContext& theCtx) const;

    /**
     * Unuse this program.
     */
    ST_CPPEXPORT virtual void unuse(StGLContext& theCtx) const;

    ST_CPPEXPORT static void unuseGlobal(StGLContext& theCtx);

        protected:

    /**
     * Check linkage state.
     * @return true if program object linked successfully.
     */
    ST_CPPEXPORT bool isLinked(StGLContext& theCtx) const;

    /**
     * Returns linkage information, provided by driver.
     * This string is driver's and vendor's specific.
     * Usually it contains linkage errors and warnings.
     */
    ST_CPPEXPORT StString getLinkageInfo(StGLContext& theCtx) const;

        protected:

    StString myTitle;     //!< just program title
    GLuint   myProgramId; //!< OpenGL shader ID

};

#endif //__StGLProgram_h_
