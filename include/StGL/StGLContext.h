/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLContext_h_
#define __StGLContext_h_

#include <StTemplates/StHandle.h>
#include <StTemplates/StRect.h>

#include <StGL/StGLVec.h>

#include <StStrings/StMsgQueue.h>

#ifdef __APPLE__
    #include <StLibrary.h>
#endif

#include <stack>

// forward declarations - you should include appropriate header to use required GL version
struct StGLFunctions;
struct StGLArbFbo;

struct StGLCore11;
struct StGLCore11Fwd;

template<typename theBaseClass_t> struct stglTmplCore12;
typedef stglTmplCore12<StGLCore11>     StGLCore12;
typedef stglTmplCore12<StGLCore11Fwd>  StGLCore12Fwd;

struct StGLCore13;
struct StGLCore13Fwd;

template<typename theBaseClass_t> struct stglTmplCore14;
typedef stglTmplCore14<StGLCore13>     StGLCore14;
typedef stglTmplCore14<StGLCore13Fwd>  StGLCore14Fwd;

template<typename theBaseClass_t> struct stglTmplCore15;
typedef stglTmplCore15<StGLCore14>     StGLCore15;
typedef stglTmplCore15<StGLCore14Fwd>  StGLCore15Fwd;

template<typename theBaseClass_t> struct stglTmplCore20;
typedef stglTmplCore20<StGLCore15>     StGLCore20;
typedef stglTmplCore20<StGLCore15Fwd>  StGLCore20Fwd;

template<typename theBaseClass_t> struct stglTmplCore21;
typedef stglTmplCore21<StGLCore20>     StGLCore21;
typedef stglTmplCore21<StGLCore20Fwd>  StGLCore21Fwd;

template<typename theBaseClass_t> struct stglTmplCore30;
typedef stglTmplCore30<StGLCore21>     StGLCore30;
typedef stglTmplCore30<StGLCore21Fwd>  StGLCore30Fwd;

template<typename theBaseClass_t> struct stglTmplCore31;
typedef stglTmplCore31<StGLCore30>     StGLCore31Back;
typedef stglTmplCore31<StGLCore30Fwd>  StGLCore31;

template<typename theBaseClass_t> struct stglTmplCore32;
typedef stglTmplCore32<StGLCore31Back> StGLCore32Back;
typedef stglTmplCore32<StGLCore31>     StGLCore32;

template<typename theBaseClass_t> struct stglTmplCore33;
typedef stglTmplCore33<StGLCore32Back> StGLCore33Back;
typedef stglTmplCore33<StGLCore32>     StGLCore33;

template<typename theBaseClass_t> struct stglTmplCore40;
typedef stglTmplCore40<StGLCore33Back> StGLCore40Back;
typedef stglTmplCore40<StGLCore33>     StGLCore40;

template<typename theBaseClass_t> struct stglTmplCore41;
typedef stglTmplCore41<StGLCore40Back> StGLCore41Back;
typedef stglTmplCore41<StGLCore40>     StGLCore41;

template<typename theBaseClass_t> struct stglTmplCore42;
typedef stglTmplCore42<StGLCore41Back> StGLCore42Back;
typedef stglTmplCore42<StGLCore41>     StGLCore42;

/**
 * Class provides access to OpenGL functions.
 */
class StGLContext {

        public:

    enum GPU_Name {
        GPU_UNKNOWN,
        GPU_GEFORCE,
        GPU_QUADRO,
        GPU_RADEON,
        GPU_FIREGL,
    };

    enum VSync_Mode {
        VSync_OFF   =  0, //!< turn OFF VSync
        VSync_ON    =  1, //!< turn ON  VSync
        VSync_MIXED =  2, //!< wait for sync only when FPS is higher than monitor refresh rate
    };

        public:    //! @name OpenGL functions - core versions

    StGLCore11*     core11;     //!< OpenGL 1.1 core functionality
    StGLCore11Fwd*  core11fwd;  //!< OpenGL 1.1 without deprecated entry points
    StGLCore20*     core20;     //!< OpenGL 2.0 core functionality (includes 1.5)
    StGLCore20Fwd*  core20fwd;  //!< OpenGL 2.0 without deprecated entry points
    StGLCore32*     core32;     //!< OpenGL 3.2 core profile
    StGLCore32Back* core32back; //!< OpenGL 3.2 backward compatibility profile
    StGLCore41*     core41;     //!< OpenGL 4.1 core profile
    StGLCore41Back* core41back; //!< OpenGL 4.1 backward compatibility profile
    StGLCore42*     core42;     //!< OpenGL 4.2 core profile
    StGLCore42Back* core42back; //!< OpenGL 4.2 backward compatibility profile

        public:    //! @name OpenGL functions - extensions

    StGLArbFbo*     arbFbo;     //!< GL_ARB_framebuffer_object
    bool            arbNPTW;    //!< GL_ARB_texture_non_power_of_two
    bool            arbTexRG;   //!< GL_ARB_texture_rg
    StGLFunctions*  extAll;     //!< access to ALL extensions for advanced users
    bool            extSwapTear;//!< WGL_EXT_swap_control_tear/GLX_EXT_swap_control_tear

        public:    //! @name class interface

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLContext();

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StGLContext(const bool theToInitialize);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLContext();

    /**
     * Setup messages queue.
     */
    ST_CPPEXPORT void setMessagesQueue(const StHandle<StMsgQueue>& theQueue);

    /**
     * Push error message to the messages queue.
     */
    ST_CPPEXPORT void pushError(const StCString& theMessage);

    /**
     * Initialize class with currently bound context.
     */
    ST_CPPEXPORT bool stglInit();

    /**
     * Check string in GL extensions.
     */
    ST_CPPEXPORT bool stglCheckExtension(const char* theExtName) const;

    /**
     * Auxiliary template to retrieve GL function pointer + cast it to specified prototype.
     * Notice that not NULL pointer doesn't means success.
     * You should check extension availability before this!
     */
    template <typename Function_t>
    inline bool stglFindProc(const char* theName,
                             Function_t& theFunction) const {
        theFunction = (Function_t )stglFindProc(theName);
        return (theFunction != NULL);
    }

    /**
     * @return true if detected GL version is greater or equal to requested one.
     */
    inline bool isGlGreaterEqual(const GLint theMajor,
                                 const GLint theMinor) {
        return (myVerMajor >  theMajor)
            || (myVerMajor == theMajor && myVerMinor >= theMinor);
    }

    inline GLint getVersionMajor() const {
        return myVerMajor;
    }

    inline GLint getVersionMinor() const {
        return myVerMinor;
    }

    inline GPU_Name getGPUName() const {
        return myGpuName;
    }

    /**
     * @return value for GL_MAX_TEXTURE_SIZE.
     */
    inline GLint getMaxTextureSize() const {
        return myMaxTexDim;
    }

    /**
     * Retrieve info from OpenGL context and create info string.
     */
    ST_CPPEXPORT static StString stglInfo();

    /**
     * Retrieve info from OpenGL context and create info string.
     */
    ST_CPPEXPORT StString stglFullInfo() const;

    /**
     * This method intended to synchronize current OpenGL state and local cache.
     */
    ST_CPPEXPORT void stglSyncState();

    /**
     * Enable scissor test for this context (glScissor).
     * @param thePushStack If true than current rectangle will be pushed into stack
     */
    ST_CPPEXPORT void stglSetScissorRect(const StGLBoxPx& theRect,
                                         const bool       thePushStack);

    /**
     * Disable scissor test for this context (glDisable(GL_SCISSOR_TEST)).
     * If stack of scissor rectangles is not empty than previous value will be restored instead.
     */
    ST_CPPEXPORT void stglResetScissorRect();

    /**
     * @return true if scissor test was activated
     */
    ST_LOCAL inline bool stglHasScissorRect() const {
        return !myScissorStack.empty();
    }

    /**
     * Setup viewport.
     */
    ST_CPPEXPORT void stglResizeViewport(const StGLBoxPx& theRect);

    /**
     * Setup viewport.
     */
    inline void stglResizeViewport(const GLsizei theSizeX,
                                   const GLsizei theSizeY) {
        const StGLBoxPx aRect = {{ 0, 0, theSizeX, theSizeY }};
        stglResizeViewport(aRect);
    }

    /**
     * @return current viewport rectangle
     */
    inline const StGLBoxPx& stglViewport() const {
        return myViewport;
    }

    /**
     * Setup viewport.
     */
    inline void stglResize(const StRect<GLint>& theWinRect) {
        const StGLBoxPx aRect = {{ 0, 0, theWinRect.width(), theWinRect.height() }};
        stglResizeViewport(aRect);
    }

    /**
     * @return currently bound FBO for drawing operations
     */
    inline GLuint stglFramebufferDraw() const {
        return myFramebufferDraw;
    }

    /**
     * @return currently bound FBO for reading operations
     */
    inline GLuint stglFramebufferRead() const {
        return myFramebufferDraw;
    }

    /**
     * Setup new FBO for drawing operations
     */
    ST_CPPEXPORT void stglBindFramebufferDraw(const GLuint theFramebuffer);

    /**
     * Setup new FBO for reading operations
     */
    ST_CPPEXPORT void stglBindFramebufferRead(const GLuint theFramebuffer);

    /**
     * Setup new FBO for both - drawing and reading operations
     */
    ST_CPPEXPORT void stglBindFramebuffer(const GLuint theFramebuffer);

    /**
     * Control VSync.
     */
    ST_CPPEXPORT bool stglSetVSync(const VSync_Mode theVSyncMode);

    /**
     * @return string representation for known GL error code.
     */
    ST_CPPEXPORT static StString stglErrorToString(const GLenum theError);

    /**
     * Clean up errors stack for this GL context (glGetError() in loop).
     */
    ST_CPPEXPORT void stglResetErrors();

    /**
     * Proceed OpenGL debug message.
     */
    ST_CPPEXPORT virtual void stglDebugCallback(unsigned int theSource,
                                                unsigned int theType,
                                                unsigned int theId,
                                                unsigned int theSeverity,
                                                int          theLength,
                                                const char*  theMessage);

        public: //! @name auxiliary methods

    /**
     * Calls system function to retrieve GL function pointer by name.
     */
    ST_CPPEXPORT void* stglFindProc(const char* theName) const;

    /**
     * Check string in specified string
     * (old way with huge string for all extensions).
     */
    ST_CPPEXPORT static bool stglCheckExtension(const char* theStringList,
                                                const char* theName);

    /**
     * Read OpenGL version information from active context.
     */
    ST_CPPEXPORT void stglReadVersion();

        private:   //! @name copying is forbidden

    StGLContext           (const StGLContext& theCopy);
    StGLContext& operator=(const StGLContext& theCopy);

        protected: //! @name class fields

#ifdef __APPLE__
    StLibrary               mySysLib;             //!< optional handle to system GL library (MacOS X specific)
#endif
    StHandle<StGLFunctions> myFuncs;              //!< mega structure for all GL functions
    StHandle<StMsgQueue>    myMsgQueue;           //!< messages queue
    GPU_Name                myGpuName;            //!< GPU name
    GLint                   myVerMajor;           //!< cached GL version major number
    GLint                   myVerMinor;           //!< cached GL version minor number
    GLint                   myMaxTexDim;          //!< maximum texture dimension
    bool                    myWasInit;            //!< initialization state

        protected: //! @name current state

    std::stack<StGLBoxPx>   myScissorStack;       //!< cached stack of scissor rectangles
    StGLBoxPx               myViewport;           //!< cached viewport rectangle
    GLuint                  myFramebufferDraw;    //!< bound draw buffer
    GLuint                  myFramebufferRead;    //!< bound read buffer

};

#endif // __StGLContext_h_
