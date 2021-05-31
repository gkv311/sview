/**
 * Copyright © 2014-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLProgramMatrix_h_
#define __StGLProgramMatrix_h_

#include <StGL/StGLProgram.h>

/**
 * This class re-presents GLSL program which has alternative code paths,
 * dynamically switched depending on context.
 * Aka "Uber-shader".
 *
 * For performance reasons each combination is cached as independent program object.
 *
 * For compatibility with OpenGL ES, code paths related to single Shader stage
 * are concatenated as strings, not as complete Shader objects dynamically linked together.
 *
 * @param theNbVShaderSections section number in Vertex   Shader
 * @param theNbFShaderSections section number in Fragment Shader
 */
template<int theNbVShaderSections, int theNbFShaderSections, class theProgramClass_t = StGLProgram>
class StGLProgramMatrix : public StGLResource {

        public:

    /**
     * Empty constructor.
     */
    ST_LOCAL StGLProgramMatrix()
    : myIsFirstInit(true) {
        stMemZero(myVShader, sizeof(myVShader));
        stMemZero(myFShader, sizeof(myFShader));
    }

    /**
     * Destructor.
     */
    ST_LOCAL virtual ~StGLProgramMatrix() {
        //
    }

    /**
     * Release OpenGL resources.
     */
    ST_LOCAL virtual void release(StGLContext& theCtx) {
        releaseShaders<StGLVertexShader,   theNbVShaderSections>(theCtx, myVShaderParts);
        releaseShaders<StGLFragmentShader, theNbFShaderSections>(theCtx, myFShaderParts);
        if(!myActiveProgram.isNull()) {
            myActiveProgram->release(theCtx);
        }
    }

    /**
     * Register new code part for specified section in Vertex Shader.
     * Should be done before initialization.
     */
    ST_LOCAL void registerVertexShaderPart(const int       theSection,
                                           const int       thePartIndex,
                                           const StString& thePartText) {
        if(theSection < 0
        || theSection >= theNbVShaderSections) {
            return;
        }

        StArrayList<StString>& aParts = myVShaderSrc[theSection];
        while(aParts.size() <= (size_t )thePartIndex) {
            aParts.add("");
        }

        aParts.changeValue(thePartIndex) = thePartText;
    }

    /**
     * Register new code part for specified section in Fragment Shader.
     * Should be done before initialization.
     */
    ST_LOCAL void registerFragmentShaderPart(const int       theSection,
                                             const int       thePartIndex,
                                             const StString& thePartText) {
        if(theSection < 0
        || theSection >= theNbFShaderSections) {
            return;
        }

        StArrayList<StString>& aParts = myFShaderSrc[theSection];
        while(aParts.size() <= (size_t )thePartIndex) {
            aParts.add("");
        }

        aParts.changeValue(thePartIndex) = thePartText;
    }

    /**
     * Select code part for specified section in Vertex Shader.
     * @return true if program has been changed
     */
    ST_LOCAL bool setVertexShaderPart(StGLContext& theCtx,
                                      const int    theSection,
                                      const int    thePartIndex) {
        if(theSection   < 0
        || theSection   >= theNbVShaderSections
        || thePartIndex >= (int )myVShaderSrc[theSection].size()) {
            return false;
        } else if(myVShader[theSection] == thePartIndex) {
            return false;
        }

        myVShader[theSection] = thePartIndex;
        if(!myActiveProgram.isNull()) {
            myActiveProgram->release(theCtx);
            myActiveProgram.nullify();
        }
        return true;
    }

    /**
     * Select code part for specified section in Fragment Shader.
     * @return true if program has been changed
     */
    ST_LOCAL bool setFragmentShaderPart(StGLContext& theCtx,
                                        const int    theSection,
                                        const int    thePartIndex) {
        if(theSection   < 0
        || theSection   >= theNbFShaderSections
        || thePartIndex >= (int )myFShaderSrc[theSection].size()) {
            return false;
        } else if(myFShader[theSection] == thePartIndex) {
            return false;
        }

        myFShader[theSection] = thePartIndex;
        if(!myActiveProgram.isNull()) {
            myActiveProgram->release(theCtx);
            myActiveProgram.nullify();
        }
        return true;
    }

    /**
     * Return index of active part within specified section in Vertex Shader.
     */
    ST_LOCAL int getVertexShaderPart(const int theSection) const {
        if(theSection < 0
        || theSection >= theNbVShaderSections) {
            return 0;
        }
        return myVShader[theSection];
    }

    /**
     * Return index of active part within specified section in Fragment Shader.
     */
    ST_LOCAL int getFragmentShaderPart(const int theSection) const {
        if(theSection < 0
        || theSection >= theNbFShaderSections) {
            return 0;
        }
        return myFShader[theSection];
    }

    /**
     * Return program object for active configuration.
     */
    StHandle<theProgramClass_t>& getActiveProgram() {
        return myActiveProgram;
    }

    /**
     * Return true if active program is valid.
     */
    bool isValid() const {
        return !myActiveProgram.isNull()
            &&  myActiveProgram->isValid()
            &&  myIsActiveValid;
    }

    /**
     * Initialize program for active configuration.
     */
    bool initProgram(StGLContext& theCtx) {
        myIsActiveValid = false;
        StString aVertSrc, aFragSrc, aCfg;
        for(int aSectionIter = 0; aSectionIter < theNbVShaderSections; ++aSectionIter) {
            const StArrayList<StString>& aSources = myVShaderSrc[aSectionIter];
            if(!aSources.isEmpty()) {
                const int anActiveSrc = myVShader[aSectionIter];
                aVertSrc += aSources.getValue(anActiveSrc);
                aCfg     += anActiveSrc;
            }
        #if defined(ST_DEBUG_SHADERS) && !defined(ST_HAVE_GLES2) && !defined(__ANDROID__)
            if(!myIsFirstInit) {
                continue;
            }

            StArrayList< StHandle<StGLVertexShader> >& aShaders = myVShaderParts[aSectionIter];
            for(size_t aShaderIter = 0; aShaderIter < aSources.size(); ++aShaderIter) {
                const StString& aSource = aSources[aShaderIter];
                StHandle<StGLVertexShader> aShader;
                if(aSource.isEmpty()) {
                    aShaders.add(aShader);
                    continue;
                }
                aShader = new StGLVertexShader(myTitle + "::VS" + aSectionIter + "::" + aShaderIter);
                aShader->init(theCtx, aSource.toCString());
                aShaders.add(aShader);
            }
        #endif
        }

        for(int aSectionIter = 0; aSectionIter < theNbFShaderSections; ++aSectionIter) {
            const StArrayList<StString>& aSources = myFShaderSrc[aSectionIter];
            if(!aSources.isEmpty()) {
                const int anActiveSrc = myFShader[aSectionIter];
                aFragSrc += aSources.getValue(anActiveSrc);
                aCfg     += anActiveSrc;
            }
        #if defined(ST_DEBUG_SHADERS) && !defined(ST_HAVE_GLES2) && !defined(__ANDROID__)
            if(!myIsFirstInit) {
                continue;
            }

            StArrayList< StHandle<StGLFragmentShader> >& aShaders = myFShaderParts[aSectionIter];
            for(size_t aShaderIter = 0; aShaderIter < aSources.size(); ++aShaderIter) {
                const StString& aSource = aSources[aShaderIter];
                StHandle<StGLFragmentShader> aShader;
                if(aSource.isEmpty()) {
                    aShaders.add(aShader);
                    continue;
                }
                aShader = new StGLFragmentShader(myTitle + "::FS" + aSectionIter + "::" + aShaderIter);
                aShader->init(theCtx, aSource.toCString());
                aShaders.add(aShader);
            }
        #endif
        }
        myIsFirstInit = false;

        if(!myActiveProgram.isNull()) {
            myActiveProgram->release(theCtx);
            myActiveProgram.nullify();
        }

        StGLVertexShader   aVertShader(myTitle + "::" + aCfg + "::VS");
        StGLFragmentShader aFragShader(myTitle + "::" + aCfg + "::FS");
        StGLAutoRelease    aTmp1(theCtx, aVertShader);
        StGLAutoRelease    aTmp2(theCtx, aFragShader);
        myActiveProgram = new theProgramClass_t(myTitle + "::" + aCfg);

        bool isOk = true;
        isOk = aVertShader.init(theCtx, aVertSrc.toCString()) && isOk;
        isOk = aFragShader.init(theCtx, aFragSrc.toCString()) && isOk;
        if(isOk) {
            myIsActiveValid = myActiveProgram->create(theCtx)
                                              .attachShader(theCtx, aVertShader)
                                              .attachShader(theCtx, aFragShader)
                                              .link(theCtx);
        }
        aVertShader.release(theCtx);
        aFragShader.release(theCtx);
        return myIsActiveValid;
    }

        private:

    /**
     * Release array of shaders.
     */
    template<class theShader_t, int theNbSections>
    ST_LOCAL void releaseShaders(StGLContext&                          theCtx,
                                 StArrayList< StHandle<theShader_t> >* theArray) {
        for(int aSectionIter = 0; aSectionIter < theNbSections; ++aSectionIter) {
            StArrayList< StHandle<theShader_t> >& aShaders = theArray[aSectionIter];
            for(size_t aVShaderIter = 0; aVShaderIter < aShaders.size(); ++aVShaderIter) {
                StHandle<theShader_t>& aShader = aShaders.changeValue(aVShaderIter);
                if(!aShader.isNull()) {
                    aShader->release(theCtx);
                    aShader.nullify();
                }
            }
        }
    }

        protected:

    StArrayList<StString>                       myVShaderSrc[theNbVShaderSections];   //!< per-section code parts combinations for Vertex   Shader
    StArrayList<StString>                       myFShaderSrc[theNbFShaderSections];   //!< per-section code parts combinations for Fragment Shader

    StArrayList< StHandle<StGLVertexShader> >   myVShaderParts[theNbVShaderSections];
    StArrayList< StHandle<StGLFragmentShader> > myFShaderParts[theNbFShaderSections];

    int                                         myVShader[theNbVShaderSections];      //!< currently activated code parts in each section of Vertex   Shader
    int                                         myFShader[theNbFShaderSections];      //!< currently activated code parts in each section of Fragment Shader

    //std::map< int, StHandle<theProgramClass_t> > myPrograms;      //!< map of initialized GLSL programs
    StHandle<theProgramClass_t>                 myActiveProgram;                      //!< currently active program
    bool                                        myIsFirstInit;
    bool                                        myIsActiveValid;

    StString                                    myTitle;                              //!< shader program title prefix

};

#endif // __StGLProgramMatrix_h_
