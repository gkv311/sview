/**
 * Copyright © 2012-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifdef _WIN32
    #include <windows.h>
#endif

#include <StGL/StGLContext.h>
#include <StGL/StGLFunctions.h>

#include <StGLCore/StGLCore44.h>
#include <StGL/StGLArbFbo.h>
#include <StGL/StGLTexture.h>

#include <StStrings/StDictionary.h>
#include <StStrings/StLogger.h>

#include <StTemplates/StQuaternion.h>

#include <stAssert.h>

#if defined(__APPLE__)
    #include <dlfcn.h>
    #include <OpenGL/OpenGL.h>
    #include <ApplicationServices/ApplicationServices.h>
#elif defined(ST_HAVE_EGL) || defined(__ANDROID__)
    #include <EGL/egl.h>
#elif !defined(_WIN32)
    #include <GL/glx.h> // glXGetProcAddress()
#endif

StGLContext::StGLContext(const StHandle<StResourceManager>& theResMgr)
: core11(NULL),
  core11fwd(NULL),
  core20(NULL),
  core20fwd(NULL),
  core32(NULL),
  core32back(NULL),
  core41(NULL),
  core41back(NULL),
  core42(NULL),
  core42back(NULL),
  core43(NULL),
  core43back(NULL),
  core44(NULL),
  core44back(NULL),
  arbFbo(NULL),
  arbNPTW(false),
  arbTexRG(false),
  arbTexFloat(false),
  arbTexClear(false),
#if defined(GL_ES_VERSION_2_0)
  hasHighp(false),
  hasTexRGBA8(false),
  extTexBGRA8(false),
  extTexR16(false),
#else
  hasHighp(true),
  hasTexRGBA8(true), // always available on desktop
  extTexBGRA8(true),
  extTexR16(true),
#endif
  extAll(NULL),
  extSwapTear(false),
  khrFlushControl(false),
  myFuncs(new StGLFunctions()),
  myResMgr(theResMgr),
  myGlVendor(GlVendor_UNKNOWN),
  myGpuName(GPU_UNKNOWN),
  myVerMajor(0),
  myVerMinor(0),
  myWasInit(false),
  myFramebufferDraw(0),
  myFramebufferRead(0),
  myIsBound(false) {
    stMemZero(&(*myFuncs),   sizeof(StGLFunctions));
    extAll = &(*myFuncs);
    stMemZero(&myViewport,   sizeof(StGLBoxPx));
    stMemZero(&myWindowBits, sizeof(BufferBits));
    stMemZero(&myFBOBits,    sizeof(BufferBits));
#ifdef __APPLE__
    mySysLib.loadSimple("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL");
#endif

#if defined(GL_ES_VERSION_2_0)
    myDevCaps.hasUnpack = false;
#else
    myDevCaps.hasUnpack = true;
#endif
}

StGLContext::StGLContext(const bool theToInitialize)
: core11(NULL),
  core11fwd(NULL),
  core20(NULL),
  core20fwd(NULL),
  core32(NULL),
  core32back(NULL),
  core41(NULL),
  core41back(NULL),
  core42(NULL),
  core42back(NULL),
  core43(NULL),
  core43back(NULL),
  core44(NULL),
  core44back(NULL),
  arbFbo(NULL),
  arbNPTW(false),
  arbTexRG(false),
  arbTexFloat(false),
  arbTexClear(false),
#if defined(GL_ES_VERSION_2_0)
  hasHighp(false),
  hasTexRGBA8(false),
  extTexBGRA8(false),
  extTexR16(false),
#else
  hasHighp(true),
  hasTexRGBA8(true),
  extTexBGRA8(true),
  extTexR16(true),
#endif
  extAll(NULL),
  extSwapTear(false),
  khrFlushControl(false),
  myFuncs(new StGLFunctions()),
  myGlVendor(GlVendor_UNKNOWN),
  myGpuName(GPU_UNKNOWN),
  myVerMajor(0),
  myVerMinor(0),
  myWasInit(false),
  myFramebufferDraw(0),
  myFramebufferRead(0),
  myIsBound(false) {
    stMemZero(&(*myFuncs),   sizeof(StGLFunctions));
    extAll = &(*myFuncs);
    stMemZero(&myViewport,   sizeof(StGLBoxPx));
    stMemZero(&myWindowBits, sizeof(BufferBits));
    stMemZero(&myFBOBits,    sizeof(BufferBits));
#ifdef __APPLE__
    mySysLib.loadSimple("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL");
#endif

#if defined(GL_ES_VERSION_2_0)
    myDevCaps.hasUnpack = false;
#else
    myDevCaps.hasUnpack = true; // always available on desktop
#endif
    if(theToInitialize) {
        stglInit();
    }
}

StGLContext::~StGLContext() {
    //
}

static void APIENTRY debugCallbackWrap(unsigned int theSource,
                                       unsigned int theType,
                                       unsigned int theId,
                                       unsigned int theSeverity,
                                       int          theLength,
                                       const char*  theMessage,
                                       const void*  theUserParam) {
    ((StGLContext* )theUserParam)->stglDebugCallback(theSource, theType, theId, theSeverity, theLength, theMessage);
}

static const StCString THE_DBGMSG_SOURCE_UNKNOWN = stCString("UNKNOWN");
static const StCString THE_DBGMSG_SOURCES[] = {
    stCString("OpenGL"),          // GL_DEBUG_SOURCE_API
    stCString("Window System"),   // GL_DEBUG_SOURCE_WINDOW_SYSTEM
    stCString("Shader Compiler"), // GL_DEBUG_SOURCE_SHADER_COMPILER
    stCString("Third Party"),     // GL_DEBUG_SOURCE_THIRD_PARTY
    stCString("Application"),     // GL_DEBUG_SOURCE_APPLICATION
    stCString("Other"),           // GL_DEBUG_SOURCE_OTHER
};

static const StCString THE_DBGMSG_TYPE_UNKNOWN = stCString("UNKNOWN");
static const StCString THE_DBGMSG_TYPES[] = {
    stCString("Error"),              // GL_DEBUG_TYPE_ERROR
    stCString("Deprecated"),         // GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
    stCString("Undefined behavior"), // GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
    stCString("Portability"),        // GL_DEBUG_TYPE_PORTABILITY
    stCString("Performance"),        // GL_DEBUG_TYPE_PERFORMANCE
    stCString("Other"),              // GL_DEBUG_TYPE_OTHER
};

static const StCString THE_DBGMSG_SEV_HIGH   = stCString("High");   // GL_DEBUG_SEVERITY_HIGH
static const StCString THE_DBGMSG_SEV_MEDIUM = stCString("Medium"); // GL_DEBUG_SEVERITY_MEDIUM
static const StCString THE_DBGMSG_SEV_LOW    = stCString("Low");    // GL_DEBUG_SEVERITY_LOW

void StGLContext::stglDebugCallback(unsigned int theSource,
                                    unsigned int theType,
                                    unsigned int theId,
                                    unsigned int theSeverity,
                                    int          /*theLength*/,
                                    const char*  theMessage) {
    if(myGlVendor == GlVendor_NVIDIA) {
        // filter too verbose messages
        if(theId == 131185) {
            return;
        }
    }

    const StCString& aSrc = (theSource >= GL_DEBUG_SOURCE_API
                          && theSource <= GL_DEBUG_SOURCE_OTHER)
                          ? THE_DBGMSG_SOURCES[theSource - GL_DEBUG_SOURCE_API]
                          : THE_DBGMSG_SOURCE_UNKNOWN;
    const StCString& aType = (theType >= GL_DEBUG_TYPE_ERROR
                           && theType <= GL_DEBUG_TYPE_OTHER)
                           ? THE_DBGMSG_TYPES[theType - GL_DEBUG_TYPE_ERROR]
                           : THE_DBGMSG_TYPE_UNKNOWN;
    const StCString& aSev = theSeverity == GL_DEBUG_SEVERITY_HIGH
                          ? THE_DBGMSG_SEV_HIGH
                          : (theSeverity == GL_DEBUG_SEVERITY_MEDIUM
                           ? THE_DBGMSG_SEV_MEDIUM
                           : THE_DBGMSG_SEV_LOW);
    StString aMsg = StString("Source:")   + aSrc
               + StString(" | Type:")     + aType
               + StString(" | ID:")       + theId
               + StString(" | Severity:") + aSev
               + StString(" | Message:\n  ") + theMessage + "\n";
    StLogger::GetDefault().write(aMsg,
        theType == GL_DEBUG_TYPE_ERROR ? StLogger::ST_ERROR : StLogger::ST_WARNING);
    //if(!myMsgQueue.isNull()) {
    //    theType == GL_DEBUG_TYPE_ERROR ? myMsgQueue->pushError(aMsg) : myMsgQueue->pushInfo (aMsg);
    //}
}

void StGLContext::setMessagesQueue(const StHandle<StMsgQueue>& theQueue) {
    myMsgQueue = theQueue;
}

void StGLContext::pushError(const StCString& theMessage) {
    if(!myMsgQueue.isNull()) {
        myMsgQueue->pushError(theMessage);
    } else {
        ST_ERROR_LOG(theMessage);
    }
}

void* StGLContext::stglFindProc(const char* theName) const {
#if defined(ST_HAVE_EGL)
    return (void* )eglGetProcAddress(theName);
#elif defined(_WIN32)
    return (void* )wglGetProcAddress(theName);
#elif defined(__APPLE__)
    return mySysLib.isOpened() ? mySysLib.find(theName) : NULL;
#else
    // Notice that some libGL implementations will NEVER return NULL pointer!
    // This is because glXGetProcAddress() can be called without GL context bound
    // (this is explicitly permitted unlike wglGetProcAddress())
    // creating functions table for both known (exported by driver when GL context created)
    // and unknown (requested by user with glXGetProcAddress())
    return (void* )glXGetProcAddress((const GLubyte* )theName);
#endif
}

bool StGLContext::stglCheckExtension(const char* theName) const {
    ST_ASSERT_SLIP(theName != NULL, "stglCheckExtension() called with NULL string", return false);

    // available since OpenGL 3.0
    /*if(isGlGreaterEqual(3, 0)) {
        const int aNameLen = std::strlen(theName);
        GLint anExtNb = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &anExtNb);
        for(GLint anIter = 0; anIter < anExtNb; ++anIter) {
            const char* anExtension = (const char* )core30->glGetStringi(GL_EXTENSIONS, (GLuint )anIter);
            if(anExtension[aNameLen] == '\0'
            && std::strncmp(anExtension, theName, aNameLen) == 0) {
                return true;
            }
        }
        return false;
    }*/

    return stglCheckExtension((const char* )glGetString (GL_EXTENSIONS), theName);
}

bool StGLContext::stglCheckExtension(const char* theStringList,
                                     const char* theName) {
    if(theName == NULL || theStringList == NULL) {
        return false;
    }
    const size_t aNameLen = std::strlen(theName);
    const char* aPtrEnd = theStringList + std::strlen(theStringList);
    for(char* aPtrIter = (char* )theStringList; aPtrIter < aPtrEnd;) {
        const size_t aWordLen = std::strcspn(aPtrIter, " ");
        if((aWordLen == aNameLen) && (std::strncmp(aPtrIter, theName, aNameLen) == 0)) {
            return true;
        }
        aPtrIter += (aWordLen + 1);
    }
    return false;
}

StString StGLContext::stglErrorToString(const GLenum theError) {
    switch(theError) {
        case GL_NO_ERROR:          return "GL_NO_ERROR";
        case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    #ifdef GL_STACK_OVERFLOW
        case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
    #endif
        case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
        default: {
            return StString("Unknown GL error #") + int(theError);
        }
    }
}

void StGLContext::stglResetErrors() {
    GLenum aPrevErr = GL_NO_ERROR;
    for(GLenum anErr = glGetError(); anErr != GL_NO_ERROR; anErr = glGetError()) {
        if(anErr == aPrevErr) {
            if(aPrevErr != GL_NO_ERROR) {
                ST_DEBUG_LOG("GL error cannot be released - broken GL context?");
            }
            return;
        }

        ST_DEBUG_LOG("Unhandled GL error (" + stglErrorToString(anErr) + ")");
        aPrevErr = anErr;
    }
}

void StGLContext::stglReadVersion() {
    // reset values
    myVerMajor = 0;
    myVerMinor = 0;

#ifdef GL_MAJOR_VERSION
    // available since OpenGL 3.0 and OpenGL 3.0 ES
    glGetIntegerv(GL_MAJOR_VERSION, &myVerMajor);
    glGetIntegerv(GL_MINOR_VERSION, &myVerMinor);
    if(glGetError() == GL_NO_ERROR
    && myVerMajor >= 3) {
        return;
    }
    stglResetErrors();
#endif

    // Read version string.
    // Notice that only first two numbers split by point '2.1 XXXXX' are significant.
    // Following trash (after space) is vendor-specific.
    // New drivers also returns micro version of GL like '3.3.0' which has no meaning
    // and should be considered as vendor-specific too.
    const char* aVerStr = (const char* )glGetString(GL_VERSION);
    if(aVerStr == NULL || *aVerStr == '\0') {
        // invalid GL context
        return;
    }

//#if defined(GL_ES_VERSION_2_0)
    // skip "OpenGL ES-** " section
    for(; *aVerStr != '\0'; ++aVerStr) {
        if(*aVerStr >= '0' && *aVerStr <= '9') {
            break;
        }
    }
//#endif

    // parse string for major number
    char aMajorStr[32];
    char aMinorStr[32];
    size_t aMajIter = 0;
    while(aVerStr[aMajIter] >= '0' && aVerStr[aMajIter] <= '9') {
        ++aMajIter;
    }
    if(aMajIter == 0 || aMajIter >= sizeof(aMajorStr)) {
        return;
    }
    stMemCpy(aMajorStr, aVerStr, aMajIter);
    aMajorStr[aMajIter] = '\0';

    // parse string for minor number
    aVerStr += aMajIter + 1;
    size_t aMinIter = 0;
    while(aVerStr[aMinIter] >= '0' && aVerStr[aMinIter] <= '9') {
        ++aMinIter;
    }
    if(aMinIter == 0 || aMinIter >= sizeof(aMinorStr)) {
        return;
    }
    stMemCpy(aMinorStr, aVerStr, aMinIter);
    aMinorStr[aMinIter] = '\0';

    // read numbers
    myVerMajor = std::atoi(aMajorStr);
    myVerMinor = std::atoi(aMinorStr);

    if(myVerMajor <= 0) {
        myVerMajor = 0;
        myVerMinor = 0;
    }
}

StString StGLContext::stglInfo() {
    StString anInfo = StString("OpenGL info:\n")
        + "  GLvendor    = '" + (const char* )glGetString(GL_VENDOR)   + "'\n"
        + "  GLdevice    = '" + (const char* )glGetString(GL_RENDERER) + "'\n"
        + "  GLversion   = '" + (const char* )glGetString(GL_VERSION)  + "'\n"
        + "  GLSLversion = '" + (const char* )glGetString(GL_SHADING_LANGUAGE_VERSION) + "'\n";
    return anInfo;
}

void StGLContext::stglFillBitsFBO(const GLuint theBuffId,
                                  const GLint  theSizeX,
                                  const GLint  theSizeY) {
    stMemZero(&myFBOBits, sizeof(BufferBits));
    myFBOBits.SizeX = theSizeX;
    myFBOBits.SizeY = theSizeY;
    if(theBuffId == 0) {
        return;
    }

    GLint aBitsRed   = 0;
    GLint aBitsGreen = 0;
    GLint aBitsBlue  = 0;
#if !defined(GL_ES_VERSION_2_0) || defined(GL_ES_VERSION_3_0)
    arbFbo->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, theBuffId);
    arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,     &aBitsRed);
    arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE,   &aBitsGreen);
    arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,    &aBitsBlue);
    arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,   &myFBOBits.Alpha);

    GLint aType = GL_NONE;
    arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,   GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &aType);
    if(aType != GL_NONE) {
        arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,   GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,   &myFBOBits.Depth);
    }
    aType = GL_NONE;
    arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &aType);
    if(aType != GL_NONE) {
        arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &myFBOBits.Stencil);
    }
    arbFbo->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, myFramebufferDraw);
#endif
    myFBOBits.RGB = aBitsRed + aBitsGreen + aBitsBlue;
}

StString StGLContext::stglFullInfo() const {
    StDictionary aMap;
    stglFullInfo(aMap);
    StString aText;
    bool isFirst = true;
    for(size_t aKeyIter = 0; aKeyIter < aMap.size(); ++aKeyIter) {
        if(isFirst) {
            isFirst = false;
        } else {
            aText += "\n";
        }
        const StDictEntry& aPair = aMap.getFromIndex(aKeyIter);
        aText += StString("  ") + aPair.getKey() + ": " + aPair.getValue();
    }
    return aText;
}

void StGLContext::stglFullInfo(StDictionary& theMap) const {
    StString aFBOBits;
    if(myFBOBits.RGB > 0) {
        aFBOBits = StString() + myFBOBits.SizeX + "x" + myFBOBits.SizeY
                              + " RGB" + myFBOBits.RGB + " A" + myFBOBits.Alpha
                              + " D" + myFBOBits.Depth + " S" + myFBOBits.Stencil;
    }

    theMap.add(StDictEntry("GLvendor",    (const char* )glGetString(GL_VENDOR)));
    theMap.add(StDictEntry("GLdevice",    (const char* )glGetString(GL_RENDERER)));
    theMap.add(StDictEntry("GLversion",   (const char* )glGetString(GL_VERSION)));
    theMap.add(StDictEntry("GLSLversion", (const char* )glGetString(GL_SHADING_LANGUAGE_VERSION)));
    theMap.add(StDictEntry("Max texture size", myDevCaps.maxTexDim));
    theMap.add(StDictEntry("Window Info", StString()
            + myViewport.width() + "x" + myViewport.height()
            + " RGB" + myWindowBits.RGB + " A" + myWindowBits.Alpha
            + " D" + myWindowBits.Depth + " S" + myWindowBits.Stencil));
    if(!aFBOBits.isEmpty()) {
        theMap.add(StDictEntry("FBO    Info", aFBOBits));
    }

#ifdef __APPLE__
    GLint aGlRendId = 0;
    CGLGetParameter(CGLGetCurrentContext(), kCGLCPCurrentRendererID, &aGlRendId);

    CGLRendererInfoObj  aRendObj  = NULL;
    CGOpenGLDisplayMask aDispMask = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
    GLint aRendNb = 0;
    CGLQueryRendererInfo(aDispMask, &aRendObj, &aRendNb);
    for(GLint aRendIter = 0; aRendIter < aRendNb; ++aRendIter) {
        GLint aRendId = 0;
        if(CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPRendererID, &aRendId) != kCGLNoError
        || aRendId != aGlRendId) {
            continue;
        }

        //kCGLRPVideoMemoryMegabytes   = 131;
        //kCGLRPTextureMemoryMegabytes = 132;
        GLint aVMem = 0;
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
        if(CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPVideoMemoryMegabytes, &aVMem) == kCGLNoError) {
            theMap.add(StDictEntry("GPU memory",         StString() + aVMem + " MiB"));
        }
        if(CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPTextureMemoryMegabytes, &aVMem) == kCGLNoError) {
            theMap.add(StDictEntry("GPU Texture memory", StString() + aVMem + " MiB"));
        }
    #else
        if(CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPVideoMemory, &aVMem) == kCGLNoError) {
            theMap.add(StDictEntry("GPU memory",         StString() + (aVMem / (1024 * 1024))  + " MiB"));
        }
        if(CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPTextureMemory, &aVMem) == kCGLNoError) {
            theMap.add(StDictEntry("GPU Texture memory", StString() + (aVMem / (1024 * 1024))  + " MiB"));
        }
    #endif
    }
#endif

#if !defined(GL_ES_VERSION_2_0)
    if(stglCheckExtension("GL_ATI_meminfo")) {
        GLint aMemInfo[4]; stMemSet(aMemInfo, -1, sizeof(aMemInfo));
        core11fwd->glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, aMemInfo);
        theMap.add(StDictEntry("Free GPU memory", StString() + (aMemInfo[0] / 1024)  + " MiB"));
    } else if(stglCheckExtension("GL_NVX_gpu_memory_info")) {
        GLint aDedicated     = -1;
        GLint aDedicatedFree = -1;
        glGetIntegerv(0x9047, &aDedicated);     // GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX
        glGetIntegerv(0x9049, &aDedicatedFree); // GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
        theMap.add(StDictEntry("Free GPU memory",
                   StString() + (aDedicatedFree / 1024)  + " MiB (from " + (aDedicated / 1024) + " MiB)"));
    }
#if defined(_WIN32)
    else if(myFuncs->wglGetGPUInfoAMD != NULL
         && myFuncs->wglGetContextGPUIDAMD != NULL) {
        GLuint aVMemMiB = 0;
        HGLRC aGlRc = wglGetCurrentContext();
        if(aGlRc != NULL) {
            UINT anAmdId = myFuncs->wglGetContextGPUIDAMD(aGlRc);
            if(anAmdId != 0
            && myFuncs->wglGetGPUInfoAMD(anAmdId, WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(aVMemMiB), &aVMemMiB) > 0) {
                theMap.add(StDictEntry("GPU memory", StString() + aVMemMiB  + " MiB"));
            }
        }
    }
#endif
#endif

#if !defined(GL_ES_VERSION_2_0) && !defined(__APPLE__) && !defined(_WIN32)
    // GLX_RENDERER_VENDOR_ID_MESA
    if(myFuncs->glXQueryCurrentRendererIntegerMESA != NULL) {
        unsigned int aVMemMiB = 0;
        if(myFuncs->glXQueryCurrentRendererIntegerMESA(GLX_RENDERER_VIDEO_MEMORY_MESA, &aVMemMiB) != False) {
            theMap.add(StDictEntry("GPU memory", StString() + aVMemMiB  + " MiB"));
        }
    }
#endif
}

void StGLContext::stglSyncState() {
    while(!myScissorStack.empty()) {
        myScissorStack.pop();
    }

    if(core11fwd->glIsEnabled(GL_SCISSOR_TEST)) {
        StGLBoxPx aRect;
        core11fwd->glGetIntegerv(GL_SCISSOR_BOX, aRect.v);
        myScissorStack.push(aRect);
    }
}

void StGLContext::stglSetScissorRect(const StGLBoxPx& theRect,
                                     const bool       thePushStack) {
    if(myScissorStack.empty()) {
        core11fwd->glEnable(GL_SCISSOR_TEST);
    }
    if(thePushStack || myScissorStack.empty()) {
        StGLBoxPx aDummyRect; // will be initialized right after
        myScissorStack.push(aDummyRect);
    }

    StGLBoxPx& aRect = myScissorStack.top();
    aRect = theRect;
    core11fwd->glScissor(aRect.x(), aRect.y(), aRect.width(), aRect.height());
}

void StGLContext::stglResetScissorRect() {
    if(!myScissorStack.empty()) {
        myScissorStack.pop();
    }
    if(myScissorStack.empty()) {
        core11fwd->glDisable(GL_SCISSOR_TEST);
        return;
    }

    // setup previous value in stack
    const StGLBoxPx& aRect = myScissorStack.top();
    core11fwd->glScissor(aRect.x(), aRect.y(), aRect.width(), aRect.height());
}

void StGLContext::stglResizeViewport(const StGLBoxPx& theRect) {
    const GLsizei aHeight = (theRect.height() == 0) ? 1 : theRect.height();
    core11fwd->glViewport(theRect.x(), theRect.y(), theRect.width(), aHeight);
    myViewport = theRect;
}

void StGLContext::stglBindFramebufferDraw(const GLuint theFramebuffer) {
    myFramebufferDraw = theFramebuffer;
#if defined(GL_ES_VERSION_2_0)
    arbFbo->glBindFramebuffer(GL_FRAMEBUFFER,      theFramebuffer);
#else
    arbFbo->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, theFramebuffer);
#endif
}

void StGLContext::stglBindFramebufferRead(const GLuint theFramebuffer) {
    myFramebufferRead = theFramebuffer;
#if defined(GL_ES_VERSION_2_0)
    arbFbo->glBindFramebuffer(GL_FRAMEBUFFER,      theFramebuffer);
#else
    arbFbo->glBindFramebuffer(GL_READ_FRAMEBUFFER, theFramebuffer);
#endif
}

void StGLContext::stglBindFramebuffer(const GLuint theFramebuffer) {
    myFramebufferDraw = theFramebuffer;
    myFramebufferRead = theFramebuffer;
    arbFbo->glBindFramebuffer(GL_FRAMEBUFFER, theFramebuffer);
}

bool StGLContext::stglSetVSync(const VSync_Mode theVSyncMode) {
    GLint aSyncInt = 0;
    switch(theVSyncMode) {
        case VSync_MIXED:
            if(extSwapTear) {
                aSyncInt = -1;
                break;
            }
        case VSync_ON:
            aSyncInt = 1;
            break;
        case VSync_OFF:
        default:
            aSyncInt = 0;
            break;
    }

#ifdef ST_HAVE_EGL
    if(eglSwapInterval(eglGetCurrentDisplay(), aSyncInt) == EGL_TRUE) {
        return true;
    }
#elif defined(_WIN32)
    if(myFuncs->wglSwapIntervalEXT != NULL) {
        myFuncs->wglSwapIntervalEXT(aSyncInt);
        return true;
    }
#elif defined(__APPLE__)
    return CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &aSyncInt) == kCGLNoError;
#else
    if(aSyncInt == -1 && myFuncs->glXSwapIntervalEXT != NULL) {
        typedef int (*glXSwapIntervalEXT_t_x)(Display* theDisplay, GLXDrawable theDrawable, int theInterval);
        glXSwapIntervalEXT_t_x aFuncPtr = (glXSwapIntervalEXT_t_x )myFuncs->glXSwapIntervalEXT;
        aFuncPtr(glXGetCurrentDisplay(), glXGetCurrentDrawable(), aSyncInt);
        return true;
    } else if(myFuncs->glXSwapIntervalSGI != NULL) {
        myFuncs->glXSwapIntervalSGI(aSyncInt);
        return true;
    }
#endif
    return false;
}

bool StGLContext::stglInit() {
    if(myWasInit) {
        return true;
    }

    // read version
    stglReadVersion();

    core11    = (StGLCore11*    )(&(*myFuncs));
    core11fwd = (StGLCore11Fwd* )(&(*myFuncs));

    myDevCaps.maxTexDim = 2048;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &myDevCaps.maxTexDim);

#if !defined(GL_ES_VERSION_2_0)
    bool has12 = false;
    bool has13 = false;
    bool has14 = false;
    bool has15 = false;
    bool has20 = false;
    bool has21 = false;
    bool has30 = false;
    bool has31 = false;
    bool has32 = false;
    bool has33 = false;
    bool has40 = false;
    bool has41 = false;
    bool has42 = false;
    bool has43 = false;
    bool has44 = false;
#endif

    #define STGL_READ_FUNC(theFunc) stglFindProc(#theFunc, myFuncs->theFunc)

    // retrieve platform-dependent extensions
#if defined(ST_HAVE_EGL)
    const char* anEglExts = eglQueryString(eglGetCurrentDisplay(), EGL_EXTENSIONS);
    khrFlushControl = stglCheckExtension(anEglExts, "EGL_KHR_context_flush_control");
#elif defined(_WIN32)
    if(STGL_READ_FUNC(wglGetExtensionsStringARB)) {
        const char* aWglExts = myFuncs->wglGetExtensionsStringARB(wglGetCurrentDC());
        if(stglCheckExtension(aWglExts, "WGL_EXT_swap_control")) {
            STGL_READ_FUNC(wglSwapIntervalEXT);
        }
        if(stglCheckExtension(aWglExts, "WGL_ARB_pixel_format")) {
            //STGL_READ_FUNC(wglGetPixelFormatAttribivARB);
            //STGL_READ_FUNC(wglGetPixelFormatAttribfvARB);
            STGL_READ_FUNC(wglChoosePixelFormatARB);
        }
        if(stglCheckExtension(aWglExts, "WGL_ARB_create_context_profile")) {
            STGL_READ_FUNC(wglCreateContextAttribsARB);
            khrFlushControl = stglCheckExtension(aWglExts, "WGL_ARB_context_flush_control");
        }
        extSwapTear = stglCheckExtension(aWglExts, "WGL_EXT_swap_control_tear");
        if(stglCheckExtension(aWglExts, "WGL_NV_DX_interop")) {
            STGL_READ_FUNC(wglDXSetResourceShareHandleNV);
            STGL_READ_FUNC(wglDXOpenDeviceNV);
            STGL_READ_FUNC(wglDXCloseDeviceNV);
            STGL_READ_FUNC(wglDXRegisterObjectNV);
            STGL_READ_FUNC(wglDXUnregisterObjectNV);
            STGL_READ_FUNC(wglDXObjectAccessNV);
            STGL_READ_FUNC(wglDXLockObjectsNV);
            STGL_READ_FUNC(wglDXUnlockObjectsNV);
        }
        if(stglCheckExtension(aWglExts, "WGL_AMD_gpu_association")) {
            STGL_READ_FUNC(wglGetGPUIDsAMD);
            STGL_READ_FUNC(wglGetGPUInfoAMD);
            STGL_READ_FUNC(wglGetContextGPUIDAMD);
        }
    }
#elif defined(__APPLE__)
    //
#else
    Display* aDisp = glXGetCurrentDisplay();
    const char* aGlxExts = glXQueryExtensionsString(aDisp, DefaultScreen(aDisp));
    if(stglCheckExtension(aGlxExts, "GLX_EXT_swap_control")) {
        STGL_READ_FUNC(glXSwapIntervalEXT);
    }
    if(stglCheckExtension(aGlxExts, "GLX_SGI_swap_control")) {
        STGL_READ_FUNC(glXSwapIntervalSGI);
    }
    if(stglCheckExtension(aGlxExts, "GLX_MESA_query_renderer")) {
        STGL_READ_FUNC(glXQueryRendererIntegerMESA);
        STGL_READ_FUNC(glXQueryCurrentRendererIntegerMESA);
        STGL_READ_FUNC(glXQueryRendererStringMESA);
        STGL_READ_FUNC(glXQueryCurrentRendererStringMESA);
    }
    extSwapTear = stglCheckExtension(aGlxExts, "GLX_EXT_swap_control_tear");
    khrFlushControl = stglCheckExtension(aGlxExts, "GLX_ARB_context_flush_control");
#endif

#if defined(GL_ES_VERSION_2_0)
    // non-power-of-two textures are valid within OpenGL ES 2.0, but have limitations
    //arbNPTW     = isGlGreaterEqual(3, 0)
    //           || stglCheckExtension("GL_OES_texture_npot");
    arbNPTW = true;
    hasTexRGBA8 = isGlGreaterEqual(3, 0)
               || stglCheckExtension("GL_OES_rgb8_rgba8");
    extTexBGRA8 = stglCheckExtension("GL_EXT_texture_format_BGRA8888");
    extTexR16   = stglCheckExtension("GL_EXT_texture_norm16");
    arbTexRG    = isGlGreaterEqual(3, 0)
               || stglCheckExtension("GL_EXT_texture_rg");
    arbTexFloat = isGlGreaterEqual(3, 0);
    const bool hasFBO = isGlGreaterEqual(2, 0)
                     || stglCheckExtension("GL_OES_framebuffer_object");
    myDevCaps.hasUnpack = isGlGreaterEqual(3, 0);

    if(isGlGreaterEqual(2, 0)) {
        // enable compatible functions
        core20    = (StGLCore20*    )(&(*myFuncs));
        core20fwd = (StGLCore20Fwd* )(&(*myFuncs));
    }

    hasHighp = stglCheckExtension("GL_OES_fragment_precision_high");
    GLint aRange[2] = {0, 0};
    GLint aPrec     = 0;
    ::glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, aRange, &aPrec);
    if(aPrec != 0) {
        hasHighp = true;
    }

#if defined(ST_DEBUG_GL)
    // this extension is buggy on OpenGL ES - enable it only for debug builds
    if(stglCheckExtension("GL_KHR_debug")) {
        // According to GL_KHR_debug spec, all functions should have KHR suffix.
        // However, some implementations can export these functions without suffix.
        stglFindProc("glDebugMessageControlKHR",  myFuncs->glDebugMessageControl);
        stglFindProc("glDebugMessageInsertKHR",   myFuncs->glDebugMessageInsert);
        stglFindProc("glDebugMessageCallbackKHR", myFuncs->glDebugMessageCallback);
        stglFindProc("glGetDebugMessageLogKHR",   myFuncs->glGetDebugMessageLog);
        if(myFuncs->glDebugMessageCallback != NULL) {
            // setup default callback
            myFuncs->glDebugMessageCallback(debugCallbackWrap, this);
            core11fwd->glEnable(GL_DEBUG_OUTPUT);
            // note that some broken implementations (e.g. simulators) might generate error message on this call
            core11fwd->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        } else {
            pushError(stCString("GL_KHR_debug is broken!"));
        }
    }
#else
    (void )debugCallbackWrap;
#endif

    //ST_DEBUG_LOG("glGetString(GL_EXTENSIONS)=\n" + (const char* )glGetString(GL_EXTENSIONS));
#else
    hasTexRGBA8 = true;
    extTexBGRA8 = true;
    extTexR16   = true;
    arbNPTW     = stglCheckExtension("GL_ARB_texture_non_power_of_two");
    arbTexRG    = stglCheckExtension("GL_ARB_texture_rg");
    arbTexFloat = isGlGreaterEqual(3, 0)
               || stglCheckExtension("GL_ARB_texture_float");

    // load OpenGL 1.2 new functions
    has12 = isGlGreaterEqual(1, 2)
         && STGL_READ_FUNC(glBlendColor)
         && STGL_READ_FUNC(glBlendEquation)
         && STGL_READ_FUNC(glDrawRangeElements)
         && STGL_READ_FUNC(glTexImage3D)
         && STGL_READ_FUNC(glTexSubImage3D)
         && STGL_READ_FUNC(glCopyTexSubImage3D);

    // load OpenGL 1.3 new functions
    has13 = isGlGreaterEqual(1, 3)
         && STGL_READ_FUNC(glActiveTexture)
         && STGL_READ_FUNC(glSampleCoverage)
         && STGL_READ_FUNC(glCompressedTexImage3D)
         && STGL_READ_FUNC(glCompressedTexImage2D)
         && STGL_READ_FUNC(glCompressedTexImage1D)
         && STGL_READ_FUNC(glCompressedTexSubImage3D)
         && STGL_READ_FUNC(glCompressedTexSubImage2D)
         && STGL_READ_FUNC(glCompressedTexSubImage1D)
         && STGL_READ_FUNC(glGetCompressedTexImage)
         && STGL_READ_FUNC(glClientActiveTexture)
         && STGL_READ_FUNC(glMultiTexCoord1d)
         && STGL_READ_FUNC(glMultiTexCoord1dv)
         && STGL_READ_FUNC(glMultiTexCoord1f)
         && STGL_READ_FUNC(glMultiTexCoord1fv)
         && STGL_READ_FUNC(glMultiTexCoord1i)
         && STGL_READ_FUNC(glMultiTexCoord1iv)
         && STGL_READ_FUNC(glMultiTexCoord1s)
         && STGL_READ_FUNC(glMultiTexCoord1sv)
         && STGL_READ_FUNC(glMultiTexCoord2d)
         && STGL_READ_FUNC(glMultiTexCoord2dv)
         && STGL_READ_FUNC(glMultiTexCoord2f)
         && STGL_READ_FUNC(glMultiTexCoord2fv)
         && STGL_READ_FUNC(glMultiTexCoord2i)
         && STGL_READ_FUNC(glMultiTexCoord2iv)
         && STGL_READ_FUNC(glMultiTexCoord2s)
         && STGL_READ_FUNC(glMultiTexCoord2sv)
         && STGL_READ_FUNC(glMultiTexCoord3d)
         && STGL_READ_FUNC(glMultiTexCoord3dv)
         && STGL_READ_FUNC(glMultiTexCoord3f)
         && STGL_READ_FUNC(glMultiTexCoord3fv)
         && STGL_READ_FUNC(glMultiTexCoord3i)
         && STGL_READ_FUNC(glMultiTexCoord3iv)
         && STGL_READ_FUNC(glMultiTexCoord3s)
         && STGL_READ_FUNC(glMultiTexCoord3sv)
         && STGL_READ_FUNC(glMultiTexCoord4d)
         && STGL_READ_FUNC(glMultiTexCoord4dv)
         && STGL_READ_FUNC(glMultiTexCoord4f)
         && STGL_READ_FUNC(glMultiTexCoord4fv)
         && STGL_READ_FUNC(glMultiTexCoord4i)
         && STGL_READ_FUNC(glMultiTexCoord4iv)
         && STGL_READ_FUNC(glMultiTexCoord4s)
         && STGL_READ_FUNC(glMultiTexCoord4sv)
         && STGL_READ_FUNC(glLoadTransposeMatrixf)
         && STGL_READ_FUNC(glLoadTransposeMatrixd)
         && STGL_READ_FUNC(glMultTransposeMatrixf)
         && STGL_READ_FUNC(glMultTransposeMatrixd);

    // load OpenGL 1.4 new functions
    has14 = isGlGreaterEqual(1, 4)
         && STGL_READ_FUNC(glBlendFuncSeparate)
         && STGL_READ_FUNC(glMultiDrawArrays)
         && STGL_READ_FUNC(glMultiDrawElements)
         && STGL_READ_FUNC(glPointParameterf)
         && STGL_READ_FUNC(glPointParameterfv)
         && STGL_READ_FUNC(glPointParameteri)
         && STGL_READ_FUNC(glPointParameteriv);

    // load OpenGL 1.5 new functions
    has15 = isGlGreaterEqual(1, 5)
         && STGL_READ_FUNC(glGenQueries)
         && STGL_READ_FUNC(glDeleteQueries)
         && STGL_READ_FUNC(glIsQuery)
         && STGL_READ_FUNC(glBeginQuery)
         && STGL_READ_FUNC(glEndQuery)
         && STGL_READ_FUNC(glGetQueryiv)
         && STGL_READ_FUNC(glGetQueryObjectiv)
         && STGL_READ_FUNC(glGetQueryObjectuiv)
         && STGL_READ_FUNC(glBindBuffer)
         && STGL_READ_FUNC(glDeleteBuffers)
         && STGL_READ_FUNC(glGenBuffers)
         && STGL_READ_FUNC(glIsBuffer)
         && STGL_READ_FUNC(glBufferData)
         && STGL_READ_FUNC(glBufferSubData)
         && STGL_READ_FUNC(glGetBufferSubData)
         && STGL_READ_FUNC(glMapBuffer)
         && STGL_READ_FUNC(glUnmapBuffer)
         && STGL_READ_FUNC(glGetBufferParameteriv)
         && STGL_READ_FUNC(glGetBufferPointerv);

    // load OpenGL 2.0 new functions
    has20 = isGlGreaterEqual(2, 0)
         && STGL_READ_FUNC(glBlendEquationSeparate)
         && STGL_READ_FUNC(glDrawBuffers)
         && STGL_READ_FUNC(glStencilOpSeparate)
         && STGL_READ_FUNC(glStencilFuncSeparate)
         && STGL_READ_FUNC(glStencilMaskSeparate)
         && STGL_READ_FUNC(glAttachShader)
         && STGL_READ_FUNC(glBindAttribLocation)
         && STGL_READ_FUNC(glCompileShader)
         && STGL_READ_FUNC(glCreateProgram)
         && STGL_READ_FUNC(glCreateShader)
         && STGL_READ_FUNC(glDeleteProgram)
         && STGL_READ_FUNC(glDeleteShader)
         && STGL_READ_FUNC(glDetachShader)
         && STGL_READ_FUNC(glDisableVertexAttribArray)
         && STGL_READ_FUNC(glEnableVertexAttribArray)
         && STGL_READ_FUNC(glGetActiveAttrib)
         && STGL_READ_FUNC(glGetActiveUniform)
         && STGL_READ_FUNC(glGetAttachedShaders)
         && STGL_READ_FUNC(glGetAttribLocation)
         && STGL_READ_FUNC(glGetProgramiv)
         && STGL_READ_FUNC(glGetProgramInfoLog)
         && STGL_READ_FUNC(glGetShaderiv)
         && STGL_READ_FUNC(glGetShaderInfoLog)
         && STGL_READ_FUNC(glGetShaderSource)
         && STGL_READ_FUNC(glGetUniformLocation)
         && STGL_READ_FUNC(glGetUniformfv)
         && STGL_READ_FUNC(glGetUniformiv)
         && STGL_READ_FUNC(glGetVertexAttribdv)
         && STGL_READ_FUNC(glGetVertexAttribfv)
         && STGL_READ_FUNC(glGetVertexAttribiv)
         && STGL_READ_FUNC(glGetVertexAttribPointerv)
         && STGL_READ_FUNC(glIsProgram)
         && STGL_READ_FUNC(glIsShader)
         && STGL_READ_FUNC(glLinkProgram)
         && STGL_READ_FUNC(glShaderSource)
         && STGL_READ_FUNC(glUseProgram)
         && STGL_READ_FUNC(glUniform1f)
         && STGL_READ_FUNC(glUniform2f)
         && STGL_READ_FUNC(glUniform3f)
         && STGL_READ_FUNC(glUniform4f)
         && STGL_READ_FUNC(glUniform1i)
         && STGL_READ_FUNC(glUniform2i)
         && STGL_READ_FUNC(glUniform3i)
         && STGL_READ_FUNC(glUniform4i)
         && STGL_READ_FUNC(glUniform1fv)
         && STGL_READ_FUNC(glUniform2fv)
         && STGL_READ_FUNC(glUniform3fv)
         && STGL_READ_FUNC(glUniform4fv)
         && STGL_READ_FUNC(glUniform1iv)
         && STGL_READ_FUNC(glUniform2iv)
         && STGL_READ_FUNC(glUniform3iv)
         && STGL_READ_FUNC(glUniform4iv)
         && STGL_READ_FUNC(glUniformMatrix2fv)
         && STGL_READ_FUNC(glUniformMatrix3fv)
         && STGL_READ_FUNC(glUniformMatrix4fv)
         && STGL_READ_FUNC(glValidateProgram)
         && STGL_READ_FUNC(glVertexAttrib1d)
         && STGL_READ_FUNC(glVertexAttrib1dv)
         && STGL_READ_FUNC(glVertexAttrib1f)
         && STGL_READ_FUNC(glVertexAttrib1fv)
         && STGL_READ_FUNC(glVertexAttrib1s)
         && STGL_READ_FUNC(glVertexAttrib1sv)
         && STGL_READ_FUNC(glVertexAttrib2d)
         && STGL_READ_FUNC(glVertexAttrib2dv)
         && STGL_READ_FUNC(glVertexAttrib2f)
         && STGL_READ_FUNC(glVertexAttrib2fv)
         && STGL_READ_FUNC(glVertexAttrib2s)
         && STGL_READ_FUNC(glVertexAttrib2sv)
         && STGL_READ_FUNC(glVertexAttrib3d)
         && STGL_READ_FUNC(glVertexAttrib3dv)
         && STGL_READ_FUNC(glVertexAttrib3f)
         && STGL_READ_FUNC(glVertexAttrib3fv)
         && STGL_READ_FUNC(glVertexAttrib3s)
         && STGL_READ_FUNC(glVertexAttrib3sv)
         && STGL_READ_FUNC(glVertexAttrib4Nbv)
         && STGL_READ_FUNC(glVertexAttrib4Niv)
         && STGL_READ_FUNC(glVertexAttrib4Nsv)
         && STGL_READ_FUNC(glVertexAttrib4Nub)
         && STGL_READ_FUNC(glVertexAttrib4Nubv)
         && STGL_READ_FUNC(glVertexAttrib4Nuiv)
         && STGL_READ_FUNC(glVertexAttrib4Nusv)
         && STGL_READ_FUNC(glVertexAttrib4bv)
         && STGL_READ_FUNC(glVertexAttrib4d)
         && STGL_READ_FUNC(glVertexAttrib4dv)
         && STGL_READ_FUNC(glVertexAttrib4f)
         && STGL_READ_FUNC(glVertexAttrib4fv)
         && STGL_READ_FUNC(glVertexAttrib4iv)
         && STGL_READ_FUNC(glVertexAttrib4s)
         && STGL_READ_FUNC(glVertexAttrib4sv)
         && STGL_READ_FUNC(glVertexAttrib4ubv)
         && STGL_READ_FUNC(glVertexAttrib4uiv)
         && STGL_READ_FUNC(glVertexAttrib4usv)
         && STGL_READ_FUNC(glVertexAttribPointer);

    // load OpenGL 2.1 new functions
    has21 = isGlGreaterEqual(2, 1)
         && STGL_READ_FUNC(glUniformMatrix2x3fv)
         && STGL_READ_FUNC(glUniformMatrix3x2fv)
         && STGL_READ_FUNC(glUniformMatrix2x4fv)
         && STGL_READ_FUNC(glUniformMatrix4x2fv)
         && STGL_READ_FUNC(glUniformMatrix3x4fv)
         && STGL_READ_FUNC(glUniformMatrix4x3fv);

    // load GL_ARB_framebuffer_object (added to OpenGL 3.0 core)
    const bool hasFBO = (isGlGreaterEqual(3, 0) || stglCheckExtension("GL_ARB_framebuffer_object"))
         && STGL_READ_FUNC(glIsRenderbuffer)
         && STGL_READ_FUNC(glBindRenderbuffer)
         && STGL_READ_FUNC(glDeleteRenderbuffers)
         && STGL_READ_FUNC(glGenRenderbuffers)
         && STGL_READ_FUNC(glRenderbufferStorage)
         && STGL_READ_FUNC(glGetRenderbufferParameteriv)
         && STGL_READ_FUNC(glIsFramebuffer)
         && STGL_READ_FUNC(glBindFramebuffer)
         && STGL_READ_FUNC(glDeleteFramebuffers)
         && STGL_READ_FUNC(glGenFramebuffers)
         && STGL_READ_FUNC(glCheckFramebufferStatus)
         && STGL_READ_FUNC(glFramebufferTexture1D)
         && STGL_READ_FUNC(glFramebufferTexture2D)
         && STGL_READ_FUNC(glFramebufferTexture3D)
         && STGL_READ_FUNC(glFramebufferRenderbuffer)
         && STGL_READ_FUNC(glGetFramebufferAttachmentParameteriv)
         && STGL_READ_FUNC(glGenerateMipmap)
         && STGL_READ_FUNC(glBlitFramebuffer)
         && STGL_READ_FUNC(glRenderbufferStorageMultisample)
         && STGL_READ_FUNC(glFramebufferTextureLayer);

    // load GL_ARB_vertex_array_object (added to OpenGL 3.0 core)
    const bool hasVAO = (isGlGreaterEqual(3, 0) || stglCheckExtension("GL_ARB_vertex_array_object"))
         && STGL_READ_FUNC(glBindVertexArray)
         && STGL_READ_FUNC(glDeleteVertexArrays)
         && STGL_READ_FUNC(glGenVertexArrays)
         && STGL_READ_FUNC(glIsVertexArray);

    // load GL_ARB_map_buffer_range (added to OpenGL 3.0 core)
    const bool hasMapBufferRange = (isGlGreaterEqual(3, 0) || stglCheckExtension("GL_ARB_map_buffer_range"))
         && STGL_READ_FUNC(glMapBufferRange)
         && STGL_READ_FUNC(glFlushMappedBufferRange);

    // load OpenGL 3.0 new functions
    has30 = isGlGreaterEqual(3, 0)
         && hasFBO
         && hasVAO
         && hasMapBufferRange
         && STGL_READ_FUNC(glColorMaski)
         && STGL_READ_FUNC(glGetBooleani_v)
         && STGL_READ_FUNC(glGetIntegeri_v)
         && STGL_READ_FUNC(glEnablei)
         && STGL_READ_FUNC(glDisablei)
         && STGL_READ_FUNC(glIsEnabledi)
         && STGL_READ_FUNC(glBeginTransformFeedback)
         && STGL_READ_FUNC(glEndTransformFeedback)
         && STGL_READ_FUNC(glBindBufferRange)
         && STGL_READ_FUNC(glBindBufferBase)
         && STGL_READ_FUNC(glTransformFeedbackVaryings)
         && STGL_READ_FUNC(glGetTransformFeedbackVarying)
         && STGL_READ_FUNC(glClampColor)
         && STGL_READ_FUNC(glBeginConditionalRender)
         && STGL_READ_FUNC(glEndConditionalRender)
         && STGL_READ_FUNC(glVertexAttribIPointer)
         && STGL_READ_FUNC(glGetVertexAttribIiv)
         && STGL_READ_FUNC(glGetVertexAttribIuiv)
         && STGL_READ_FUNC(glVertexAttribI1i)
         && STGL_READ_FUNC(glVertexAttribI2i)
         && STGL_READ_FUNC(glVertexAttribI3i)
         && STGL_READ_FUNC(glVertexAttribI4i)
         && STGL_READ_FUNC(glVertexAttribI1ui)
         && STGL_READ_FUNC(glVertexAttribI2ui)
         && STGL_READ_FUNC(glVertexAttribI3ui)
         && STGL_READ_FUNC(glVertexAttribI4ui)
         && STGL_READ_FUNC(glVertexAttribI1iv)
         && STGL_READ_FUNC(glVertexAttribI2iv)
         && STGL_READ_FUNC(glVertexAttribI3iv)
         && STGL_READ_FUNC(glVertexAttribI4iv)
         && STGL_READ_FUNC(glVertexAttribI1uiv)
         && STGL_READ_FUNC(glVertexAttribI2uiv)
         && STGL_READ_FUNC(glVertexAttribI3uiv)
         && STGL_READ_FUNC(glVertexAttribI4uiv)
         && STGL_READ_FUNC(glVertexAttribI4bv)
         && STGL_READ_FUNC(glVertexAttribI4sv)
         && STGL_READ_FUNC(glVertexAttribI4ubv)
         && STGL_READ_FUNC(glVertexAttribI4usv)
         && STGL_READ_FUNC(glGetUniformuiv)
         && STGL_READ_FUNC(glBindFragDataLocation)
         && STGL_READ_FUNC(glGetFragDataLocation)
         && STGL_READ_FUNC(glUniform1ui)
         && STGL_READ_FUNC(glUniform2ui)
         && STGL_READ_FUNC(glUniform3ui)
         && STGL_READ_FUNC(glUniform4ui)
         && STGL_READ_FUNC(glUniform1uiv)
         && STGL_READ_FUNC(glUniform2uiv)
         && STGL_READ_FUNC(glUniform3uiv)
         && STGL_READ_FUNC(glUniform4uiv)
         && STGL_READ_FUNC(glTexParameterIiv)
         && STGL_READ_FUNC(glTexParameterIuiv)
         && STGL_READ_FUNC(glGetTexParameterIiv)
         && STGL_READ_FUNC(glGetTexParameterIuiv)
         && STGL_READ_FUNC(glClearBufferiv)
         && STGL_READ_FUNC(glClearBufferuiv)
         && STGL_READ_FUNC(glClearBufferfv)
         && STGL_READ_FUNC(glClearBufferfi)
         && STGL_READ_FUNC(glGetStringi);

    // load GL_ARB_uniform_buffer_object (added to OpenGL 3.1 core)
    const bool hasUBO = (isGlGreaterEqual(3, 1) || stglCheckExtension("GL_ARB_uniform_buffer_object"))
         && STGL_READ_FUNC(glGetUniformIndices)
         && STGL_READ_FUNC(glGetActiveUniformsiv)
         && STGL_READ_FUNC(glGetActiveUniformName)
         && STGL_READ_FUNC(glGetUniformBlockIndex)
         && STGL_READ_FUNC(glGetActiveUniformBlockiv)
         && STGL_READ_FUNC(glGetActiveUniformBlockName)
         && STGL_READ_FUNC(glUniformBlockBinding);

    // load GL_ARB_copy_buffer (added to OpenGL 3.1 core)
    const bool hasCopyBufSubData = (isGlGreaterEqual(3, 1) || stglCheckExtension("GL_ARB_copy_buffer"))
         && STGL_READ_FUNC(glCopyBufferSubData);

    if(has30) {
        // NPOT textures are required by OpenGL 2.0 specifications
        // but doesn't hardware accellerated by some ancient OpenGL 2.1 hardware (GeForce FX, RadeOn 9700 etc.)
        arbNPTW  = true;
        arbTexRG = true;
    }

    // load OpenGL 3.1 new functions
    has31 = isGlGreaterEqual(3, 1)
         && hasUBO
         && hasCopyBufSubData
         && STGL_READ_FUNC(glDrawArraysInstanced)
         && STGL_READ_FUNC(glDrawElementsInstanced)
         && STGL_READ_FUNC(glTexBuffer)
         && STGL_READ_FUNC(glPrimitiveRestartIndex);

    // load GL_ARB_draw_elements_base_vertex (added to OpenGL 3.2 core)
    const bool hasDrawElemsBaseVert = (isGlGreaterEqual(3, 2) || stglCheckExtension("GL_ARB_draw_elements_base_vertex"))
         && STGL_READ_FUNC(glDrawElementsBaseVertex)
         && STGL_READ_FUNC(glDrawRangeElementsBaseVertex)
         && STGL_READ_FUNC(glDrawElementsInstancedBaseVertex)
         && STGL_READ_FUNC(glMultiDrawElementsBaseVertex);

    // load GL_ARB_provoking_vertex (added to OpenGL 3.2 core)
    const bool hasProvokingVert = (isGlGreaterEqual(3, 2) || stglCheckExtension("GL_ARB_provoking_vertex"))
         && STGL_READ_FUNC(glProvokingVertex);

    // load GL_ARB_sync (added to OpenGL 3.2 core)
    const bool hasSync = (isGlGreaterEqual(3, 2) || stglCheckExtension("GL_ARB_sync"))
         && STGL_READ_FUNC(glFenceSync)
         && STGL_READ_FUNC(glIsSync)
         && STGL_READ_FUNC(glDeleteSync)
         && STGL_READ_FUNC(glClientWaitSync)
         && STGL_READ_FUNC(glWaitSync)
         && STGL_READ_FUNC(glGetInteger64v)
         && STGL_READ_FUNC(glGetSynciv);

    // load GL_ARB_texture_multisample (added to OpenGL 3.2 core)
    const bool hasTextureMultisample = (isGlGreaterEqual(3, 2) || stglCheckExtension("GL_ARB_texture_multisample"))
         && STGL_READ_FUNC(glTexImage2DMultisample)
         && STGL_READ_FUNC(glTexImage3DMultisample)
         && STGL_READ_FUNC(glGetMultisamplefv)
         && STGL_READ_FUNC(glSampleMaski);

    // load OpenGL 3.2 new functions
    has32 = isGlGreaterEqual(3, 2)
         && hasDrawElemsBaseVert
         && hasProvokingVert
         && hasSync
         && hasTextureMultisample
         && STGL_READ_FUNC(glGetInteger64i_v)
         && STGL_READ_FUNC(glGetBufferParameteri64v)
         && STGL_READ_FUNC(glFramebufferTexture);

    // load GL_ARB_blend_func_extended (added to OpenGL 3.3 core)
    const bool hasBlendFuncExtended = (isGlGreaterEqual(3, 3) || stglCheckExtension("GL_ARB_blend_func_extended"))
         && STGL_READ_FUNC(glBindFragDataLocationIndexed)
         && STGL_READ_FUNC(glGetFragDataIndex);

    // load GL_ARB_sampler_objects (added to OpenGL 3.3 core)
    const bool hasSamplerObjects = (isGlGreaterEqual(3, 3) || stglCheckExtension("GL_ARB_sampler_objects"))
         && STGL_READ_FUNC(glGenSamplers)
         && STGL_READ_FUNC(glDeleteSamplers)
         && STGL_READ_FUNC(glIsSampler)
         && STGL_READ_FUNC(glBindSampler)
         && STGL_READ_FUNC(glSamplerParameteri)
         && STGL_READ_FUNC(glSamplerParameteriv)
         && STGL_READ_FUNC(glSamplerParameterf)
         && STGL_READ_FUNC(glSamplerParameterfv)
         && STGL_READ_FUNC(glSamplerParameterIiv)
         && STGL_READ_FUNC(glSamplerParameterIuiv)
         && STGL_READ_FUNC(glGetSamplerParameteriv)
         && STGL_READ_FUNC(glGetSamplerParameterIiv)
         && STGL_READ_FUNC(glGetSamplerParameterfv)
         && STGL_READ_FUNC(glGetSamplerParameterIuiv);

    // load GL_ARB_timer_query (added to OpenGL 3.3 core)
    const bool hasTimerQuery = (isGlGreaterEqual(3, 3) || stglCheckExtension("GL_ARB_timer_query"))
         && STGL_READ_FUNC(glQueryCounter)
         && STGL_READ_FUNC(glGetQueryObjecti64v)
         && STGL_READ_FUNC(glGetQueryObjectui64v);

    // load GL_ARB_vertex_type_2_10_10_10_rev (added to OpenGL 3.3 core)
    const bool hasVertType21010101rev = (isGlGreaterEqual(3, 3) || stglCheckExtension("GL_ARB_vertex_type_2_10_10_10_rev"))
         && STGL_READ_FUNC(glVertexP2ui)
         && STGL_READ_FUNC(glVertexP2uiv)
         && STGL_READ_FUNC(glVertexP3ui)
         && STGL_READ_FUNC(glVertexP3uiv)
         && STGL_READ_FUNC(glVertexP4ui)
         && STGL_READ_FUNC(glVertexP4uiv)
         && STGL_READ_FUNC(glTexCoordP1ui)
         && STGL_READ_FUNC(glTexCoordP1uiv)
         && STGL_READ_FUNC(glTexCoordP2ui)
         && STGL_READ_FUNC(glTexCoordP2uiv)
         && STGL_READ_FUNC(glTexCoordP3ui)
         && STGL_READ_FUNC(glTexCoordP3uiv)
         && STGL_READ_FUNC(glTexCoordP4ui)
         && STGL_READ_FUNC(glTexCoordP4uiv)
         && STGL_READ_FUNC(glMultiTexCoordP1ui)
         && STGL_READ_FUNC(glMultiTexCoordP1uiv)
         && STGL_READ_FUNC(glMultiTexCoordP2ui)
         && STGL_READ_FUNC(glMultiTexCoordP2uiv)
         && STGL_READ_FUNC(glMultiTexCoordP3ui)
         && STGL_READ_FUNC(glMultiTexCoordP3uiv)
         && STGL_READ_FUNC(glMultiTexCoordP4ui)
         && STGL_READ_FUNC(glMultiTexCoordP4uiv)
         && STGL_READ_FUNC(glNormalP3ui)
         && STGL_READ_FUNC(glNormalP3uiv)
         && STGL_READ_FUNC(glColorP3ui)
         && STGL_READ_FUNC(glColorP3uiv)
         && STGL_READ_FUNC(glColorP4ui)
         && STGL_READ_FUNC(glColorP4uiv)
         && STGL_READ_FUNC(glSecondaryColorP3ui)
         && STGL_READ_FUNC(glSecondaryColorP3uiv)
         && STGL_READ_FUNC(glVertexAttribP1ui)
         && STGL_READ_FUNC(glVertexAttribP1uiv)
         && STGL_READ_FUNC(glVertexAttribP2ui)
         && STGL_READ_FUNC(glVertexAttribP2uiv)
         && STGL_READ_FUNC(glVertexAttribP3ui)
         && STGL_READ_FUNC(glVertexAttribP3uiv)
         && STGL_READ_FUNC(glVertexAttribP4ui)
         && STGL_READ_FUNC(glVertexAttribP4uiv);

    // load OpenGL 3.3 extra functions
    has33 = isGlGreaterEqual(3, 3)
         && hasBlendFuncExtended
         && hasSamplerObjects
         && hasTimerQuery
         && hasVertType21010101rev
         && STGL_READ_FUNC(glVertexAttribDivisor);

    // load GL_ARB_draw_indirect (added to OpenGL 4.0 core)
    const bool hasDrawIndirect = (isGlGreaterEqual(4, 0) || stglCheckExtension("GL_ARB_draw_indirect"))
         && STGL_READ_FUNC(glDrawArraysIndirect)
         && STGL_READ_FUNC(glDrawElementsIndirect);

    // load GL_ARB_gpu_shader_fp64 (added to OpenGL 4.0 core)
    const bool hasShaderFP64 = (isGlGreaterEqual(4, 0) || stglCheckExtension("GL_ARB_gpu_shader_fp64"))
         && STGL_READ_FUNC(glUniform1d)
         && STGL_READ_FUNC(glUniform2d)
         && STGL_READ_FUNC(glUniform3d)
         && STGL_READ_FUNC(glUniform4d)
         && STGL_READ_FUNC(glUniform1dv)
         && STGL_READ_FUNC(glUniform2dv)
         && STGL_READ_FUNC(glUniform3dv)
         && STGL_READ_FUNC(glUniform4dv)
         && STGL_READ_FUNC(glUniformMatrix2dv)
         && STGL_READ_FUNC(glUniformMatrix3dv)
         && STGL_READ_FUNC(glUniformMatrix4dv)
         && STGL_READ_FUNC(glUniformMatrix2x3dv)
         && STGL_READ_FUNC(glUniformMatrix2x4dv)
         && STGL_READ_FUNC(glUniformMatrix3x2dv)
         && STGL_READ_FUNC(glUniformMatrix3x4dv)
         && STGL_READ_FUNC(glUniformMatrix4x2dv)
         && STGL_READ_FUNC(glUniformMatrix4x3dv)
         && STGL_READ_FUNC(glGetUniformdv);

    // load GL_ARB_shader_subroutine (added to OpenGL 4.0 core)
    const bool hasShaderSubroutine = (isGlGreaterEqual(4, 0) || stglCheckExtension("GL_ARB_shader_subroutine"))
         && STGL_READ_FUNC(glGetSubroutineUniformLocation)
         && STGL_READ_FUNC(glGetSubroutineIndex)
         && STGL_READ_FUNC(glGetActiveSubroutineUniformiv)
         && STGL_READ_FUNC(glGetActiveSubroutineUniformName)
         && STGL_READ_FUNC(glGetActiveSubroutineName)
         && STGL_READ_FUNC(glUniformSubroutinesuiv)
         && STGL_READ_FUNC(glGetUniformSubroutineuiv)
         && STGL_READ_FUNC(glGetProgramStageiv);

    // load GL_ARB_tessellation_shader (added to OpenGL 4.0 core)
    const bool hasTessellationShader = (isGlGreaterEqual(4, 0) || stglCheckExtension("GL_ARB_tessellation_shader"))
         && STGL_READ_FUNC(glPatchParameteri)
         && STGL_READ_FUNC(glPatchParameterfv);

    // load GL_ARB_transform_feedback2 (added to OpenGL 4.0 core)
    const bool hasTrsfFeedback2 = (isGlGreaterEqual(4, 0) || stglCheckExtension("GL_ARB_transform_feedback2"))
         && STGL_READ_FUNC(glBindTransformFeedback)
         && STGL_READ_FUNC(glDeleteTransformFeedbacks)
         && STGL_READ_FUNC(glGenTransformFeedbacks)
         && STGL_READ_FUNC(glIsTransformFeedback)
         && STGL_READ_FUNC(glPauseTransformFeedback)
         && STGL_READ_FUNC(glResumeTransformFeedback)
         && STGL_READ_FUNC(glDrawTransformFeedback);

    // load GL_ARB_transform_feedback3 (added to OpenGL 4.0 core)
    const bool hasTrsfFeedback3 = (isGlGreaterEqual(4, 0) || stglCheckExtension("GL_ARB_transform_feedback3"))
         && STGL_READ_FUNC(glDrawTransformFeedbackStream)
         && STGL_READ_FUNC(glBeginQueryIndexed)
         && STGL_READ_FUNC(glEndQueryIndexed)
         && STGL_READ_FUNC(glGetQueryIndexediv);

    // load OpenGL 4.0 new functions
    has40 = isGlGreaterEqual(4, 0)
        && hasDrawIndirect
        && hasShaderFP64
        && hasShaderSubroutine
        && hasTessellationShader
        && hasTrsfFeedback2
        && hasTrsfFeedback3
        && STGL_READ_FUNC(glMinSampleShading)
        && STGL_READ_FUNC(glBlendEquationi)
        && STGL_READ_FUNC(glBlendEquationSeparatei)
        && STGL_READ_FUNC(glBlendFunci)
        && STGL_READ_FUNC(glBlendFuncSeparatei);

    // load GL_ARB_ES2_compatibility (added to OpenGL 4.1 core)
    const bool hasES2Compatibility = (isGlGreaterEqual(4, 1) || stglCheckExtension("GL_ARB_ES2_compatibility"))
         && STGL_READ_FUNC(glReleaseShaderCompiler)
         && STGL_READ_FUNC(glShaderBinary)
         && STGL_READ_FUNC(glGetShaderPrecisionFormat)
         && STGL_READ_FUNC(glDepthRangef)
         && STGL_READ_FUNC(glClearDepthf);

    // load GL_ARB_get_program_binary (added to OpenGL 4.1 core)
    const bool hasGetProgramBinary = (isGlGreaterEqual(4, 1) || stglCheckExtension("GL_ARB_get_program_binary"))
         && STGL_READ_FUNC(glGetProgramBinary)
         && STGL_READ_FUNC(glProgramBinary)
         && STGL_READ_FUNC(glProgramParameteri);


    // load GL_ARB_separate_shader_objects (added to OpenGL 4.1 core)
    const bool hasSeparateShaderObjects = (isGlGreaterEqual(4, 1) || stglCheckExtension("GL_ARB_separate_shader_objects"))
         && STGL_READ_FUNC(glUseProgramStages)
         && STGL_READ_FUNC(glActiveShaderProgram)
         && STGL_READ_FUNC(glCreateShaderProgramv)
         && STGL_READ_FUNC(glBindProgramPipeline)
         && STGL_READ_FUNC(glDeleteProgramPipelines)
         && STGL_READ_FUNC(glGenProgramPipelines)
         && STGL_READ_FUNC(glIsProgramPipeline)
         && STGL_READ_FUNC(glGetProgramPipelineiv)
         && STGL_READ_FUNC(glProgramUniform1i)
         && STGL_READ_FUNC(glProgramUniform1iv)
         && STGL_READ_FUNC(glProgramUniform1f)
         && STGL_READ_FUNC(glProgramUniform1fv)
         && STGL_READ_FUNC(glProgramUniform1d)
         && STGL_READ_FUNC(glProgramUniform1dv)
         && STGL_READ_FUNC(glProgramUniform1ui)
         && STGL_READ_FUNC(glProgramUniform1uiv)
         && STGL_READ_FUNC(glProgramUniform2i)
         && STGL_READ_FUNC(glProgramUniform2iv)
         && STGL_READ_FUNC(glProgramUniform2f)
         && STGL_READ_FUNC(glProgramUniform2fv)
         && STGL_READ_FUNC(glProgramUniform2d)
         && STGL_READ_FUNC(glProgramUniform2dv)
         && STGL_READ_FUNC(glProgramUniform2ui)
         && STGL_READ_FUNC(glProgramUniform2uiv)
         && STGL_READ_FUNC(glProgramUniform3i)
         && STGL_READ_FUNC(glProgramUniform3iv)
         && STGL_READ_FUNC(glProgramUniform3f)
         && STGL_READ_FUNC(glProgramUniform3fv)
         && STGL_READ_FUNC(glProgramUniform3d)
         && STGL_READ_FUNC(glProgramUniform3dv)
         && STGL_READ_FUNC(glProgramUniform3ui)
         && STGL_READ_FUNC(glProgramUniform3uiv)
         && STGL_READ_FUNC(glProgramUniform4i)
         && STGL_READ_FUNC(glProgramUniform4iv)
         && STGL_READ_FUNC(glProgramUniform4f)
         && STGL_READ_FUNC(glProgramUniform4fv)
         && STGL_READ_FUNC(glProgramUniform4d)
         && STGL_READ_FUNC(glProgramUniform4dv)
         && STGL_READ_FUNC(glProgramUniform4ui)
         && STGL_READ_FUNC(glProgramUniform4uiv)
         && STGL_READ_FUNC(glProgramUniformMatrix2fv)
         && STGL_READ_FUNC(glProgramUniformMatrix3fv)
         && STGL_READ_FUNC(glProgramUniformMatrix4fv)
         && STGL_READ_FUNC(glProgramUniformMatrix2dv)
         && STGL_READ_FUNC(glProgramUniformMatrix3dv)
         && STGL_READ_FUNC(glProgramUniformMatrix4dv)
         && STGL_READ_FUNC(glProgramUniformMatrix2x3fv)
         && STGL_READ_FUNC(glProgramUniformMatrix3x2fv)
         && STGL_READ_FUNC(glProgramUniformMatrix2x4fv)
         && STGL_READ_FUNC(glProgramUniformMatrix4x2fv)
         && STGL_READ_FUNC(glProgramUniformMatrix3x4fv)
         && STGL_READ_FUNC(glProgramUniformMatrix4x3fv)
         && STGL_READ_FUNC(glProgramUniformMatrix2x3dv)
         && STGL_READ_FUNC(glProgramUniformMatrix3x2dv)
         && STGL_READ_FUNC(glProgramUniformMatrix2x4dv)
         && STGL_READ_FUNC(glProgramUniformMatrix4x2dv)
         && STGL_READ_FUNC(glProgramUniformMatrix3x4dv)
         && STGL_READ_FUNC(glProgramUniformMatrix4x3dv)
         && STGL_READ_FUNC(glValidateProgramPipeline)
         && STGL_READ_FUNC(glGetProgramPipelineInfoLog);

    // load GL_ARB_vertex_attrib_64bit (added to OpenGL 4.1 core)
    const bool hasVertAttrib64bit = (isGlGreaterEqual(4, 1) || stglCheckExtension("GL_ARB_vertex_attrib_64bit"))
         && STGL_READ_FUNC(glVertexAttribL1d)
         && STGL_READ_FUNC(glVertexAttribL2d)
         && STGL_READ_FUNC(glVertexAttribL3d)
         && STGL_READ_FUNC(glVertexAttribL4d)
         && STGL_READ_FUNC(glVertexAttribL1dv)
         && STGL_READ_FUNC(glVertexAttribL2dv)
         && STGL_READ_FUNC(glVertexAttribL3dv)
         && STGL_READ_FUNC(glVertexAttribL4dv)
         && STGL_READ_FUNC(glVertexAttribLPointer)
         && STGL_READ_FUNC(glGetVertexAttribLdv);

    // load GL_ARB_viewport_array (added to OpenGL 4.1 core)
    const bool hasViewportArray = (isGlGreaterEqual(4, 1) || stglCheckExtension("GL_ARB_viewport_array"))
         && STGL_READ_FUNC(glViewportArrayv)
         && STGL_READ_FUNC(glViewportIndexedf)
         && STGL_READ_FUNC(glViewportIndexedfv)
         && STGL_READ_FUNC(glScissorArrayv)
         && STGL_READ_FUNC(glScissorIndexed)
         && STGL_READ_FUNC(glScissorIndexedv)
         && STGL_READ_FUNC(glDepthRangeArrayv)
         && STGL_READ_FUNC(glDepthRangeIndexed)
         && STGL_READ_FUNC(glGetFloati_v)
         && STGL_READ_FUNC(glGetDoublei_v);

    has41 = isGlGreaterEqual(4, 1)
         && hasES2Compatibility
         && hasGetProgramBinary
         && hasSeparateShaderObjects
         && hasVertAttrib64bit
         && hasViewportArray;

    // load GL_ARB_base_instance (added to OpenGL 4.2 core)
    const bool hasBaseInstance = (isGlGreaterEqual(4, 2) || stglCheckExtension("GL_ARB_base_instance"))
         && STGL_READ_FUNC(glDrawArraysInstancedBaseInstance)
         && STGL_READ_FUNC(glDrawElementsInstancedBaseInstance)
         && STGL_READ_FUNC(glDrawElementsInstancedBaseVertexBaseInstance);

    // load GL_ARB_transform_feedback_instanced (added to OpenGL 4.2 core)
    const bool hasTrsfFeedbackInstanced = (isGlGreaterEqual(4, 2) || stglCheckExtension("GL_ARB_transform_feedback_instanced"))
         && STGL_READ_FUNC(glDrawTransformFeedbackInstanced)
         && STGL_READ_FUNC(glDrawTransformFeedbackStreamInstanced);

    // load GL_ARB_internalformat_query (added to OpenGL 4.2 core)
    const bool hasInternalFormatQuery = (isGlGreaterEqual(4, 2) || stglCheckExtension("GL_ARB_internalformat_query"))
         && STGL_READ_FUNC(glGetInternalformativ);

    // load GL_ARB_shader_atomic_counters (added to OpenGL 4.2 core)
    const bool hasShaderAtomicCounters = (isGlGreaterEqual(4, 2) || stglCheckExtension("GL_ARB_shader_atomic_counters"))
         && STGL_READ_FUNC(glGetActiveAtomicCounterBufferiv);

    // load GL_ARB_shader_image_load_store (added to OpenGL 4.2 core)
    const bool hasShaderImgLoadStore = (isGlGreaterEqual(4, 2) || stglCheckExtension("GL_ARB_shader_image_load_store"))
         && STGL_READ_FUNC(glBindImageTexture)
         && STGL_READ_FUNC(glMemoryBarrier);

    // load GL_ARB_texture_storage (added to OpenGL 4.2 core)
    const bool hasTextureStorage = (isGlGreaterEqual(4, 2) || stglCheckExtension("GL_ARB_texture_storage"))
         && STGL_READ_FUNC(glTexStorage1D)
         && STGL_READ_FUNC(glTexStorage2D)
         && STGL_READ_FUNC(glTexStorage3D)
         && STGL_READ_FUNC(glTextureStorage1DEXT)
         && STGL_READ_FUNC(glTextureStorage2DEXT)
         && STGL_READ_FUNC(glTextureStorage3DEXT);

    has42 = isGlGreaterEqual(4, 2)
         && hasBaseInstance
         && hasTrsfFeedbackInstanced
         && hasInternalFormatQuery
         && hasShaderAtomicCounters
         && hasShaderImgLoadStore
         && hasTextureStorage;

    has43 = isGlGreaterEqual(4, 3)
         && STGL_READ_FUNC(glClearBufferData)
         && STGL_READ_FUNC(glClearBufferSubData)
         && STGL_READ_FUNC(glDispatchCompute)
         && STGL_READ_FUNC(glDispatchComputeIndirect)
         && STGL_READ_FUNC(glCopyImageSubData)
         && STGL_READ_FUNC(glFramebufferParameteri)
         && STGL_READ_FUNC(glGetFramebufferParameteriv)
         && STGL_READ_FUNC(glGetInternalformati64v)
         && STGL_READ_FUNC(glInvalidateTexSubImage)
         && STGL_READ_FUNC(glInvalidateTexImage)
         && STGL_READ_FUNC(glInvalidateBufferSubData)
         && STGL_READ_FUNC(glInvalidateBufferData)
         && STGL_READ_FUNC(glInvalidateFramebuffer)
         && STGL_READ_FUNC(glInvalidateSubFramebuffer)
         && STGL_READ_FUNC(glMultiDrawArraysIndirect)
         && STGL_READ_FUNC(glMultiDrawElementsIndirect)
         && STGL_READ_FUNC(glGetProgramInterfaceiv)
         && STGL_READ_FUNC(glGetProgramResourceIndex)
         && STGL_READ_FUNC(glGetProgramResourceName)
         && STGL_READ_FUNC(glGetProgramResourceiv)
         && STGL_READ_FUNC(glGetProgramResourceLocation)
         && STGL_READ_FUNC(glGetProgramResourceLocationIndex)
         && STGL_READ_FUNC(glShaderStorageBlockBinding)
         && STGL_READ_FUNC(glTexBufferRange)
         && STGL_READ_FUNC(glTexStorage2DMultisample)
         && STGL_READ_FUNC(glTexStorage3DMultisample)
         && STGL_READ_FUNC(glTextureView)
         && STGL_READ_FUNC(glBindVertexBuffer)
         && STGL_READ_FUNC(glVertexAttribFormat)
         && STGL_READ_FUNC(glVertexAttribIFormat)
         && STGL_READ_FUNC(glVertexAttribLFormat)
         && STGL_READ_FUNC(glVertexAttribBinding)
         && STGL_READ_FUNC(glVertexBindingDivisor)
         && STGL_READ_FUNC(glDebugMessageControl)
         && STGL_READ_FUNC(glDebugMessageInsert)
         && STGL_READ_FUNC(glDebugMessageCallback)
         && STGL_READ_FUNC(glGetDebugMessageLog)
         && STGL_READ_FUNC(glPushDebugGroup)
         && STGL_READ_FUNC(glPopDebugGroup)
         && STGL_READ_FUNC(glObjectLabel)
         && STGL_READ_FUNC(glGetObjectLabel)
         && STGL_READ_FUNC(glObjectPtrLabel)
         && STGL_READ_FUNC(glGetObjectPtrLabel);

    // load GL_ARB_clear_texture (added to OpenGL 4.4 core)
    arbTexClear = (isGlGreaterEqual(4, 4) || stglCheckExtension("GL_ARB_clear_texture"))
         && STGL_READ_FUNC(glClearTexImage)
         && STGL_READ_FUNC(glClearTexSubImage);

    has44 = isGlGreaterEqual(4, 4)
         && arbTexClear
         && STGL_READ_FUNC(glBufferStorage)
         && STGL_READ_FUNC(glBindBuffersBase)
         && STGL_READ_FUNC(glBindBuffersRange)
         && STGL_READ_FUNC(glBindTextures)
         && STGL_READ_FUNC(glBindSamplers)
         && STGL_READ_FUNC(glBindImageTextures)
         && STGL_READ_FUNC(glBindVertexBuffers);

    if(stglCheckExtension("GL_ARB_debug_output")) {
        if(!has43) {
            stglFindProc("glDebugMessageControlARB",  myFuncs->glDebugMessageControl);
            stglFindProc("glDebugMessageInsertARB",   myFuncs->glDebugMessageInsert);
            stglFindProc("glDebugMessageCallbackARB", myFuncs->glDebugMessageCallback);
            stglFindProc("glGetDebugMessageLogARB",   myFuncs->glGetDebugMessageLog);
        }
        if(myFuncs->glDebugMessageCallback != NULL) {
            // setup default callback
            myFuncs->glDebugMessageCallback(debugCallbackWrap, this);
        #if defined(ST_DEBUG) || defined(ST_DEBUG_GL)
            if(has43) {
                core11fwd->glEnable(GL_DEBUG_OUTPUT);
            }
            core11fwd->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        #endif
        }
    }

#endif // OpenGL desktop or ES

    const StString aGlVendor((const char* )core11fwd->glGetString(GL_VENDOR));
    if(aGlVendor.isContains(stCString("NVIDIA"))) {
        myGlVendor = GlVendor_NVIDIA;
    } else if(aGlVendor.isContains(stCString("ATI Technologies"))) {
        myGlVendor = GlVendor_AMD;
    } else if(aGlVendor.isContains(stCString("Intel"))) {
        myGlVendor = GlVendor_Intel;
    } else if(aGlVendor.isContains(stCString("Qualcomm"))) {
        myGlVendor = GlVendor_Qualcomm;
    } else if(aGlVendor.isContains(stCString("Imagination Technologies"))) {
        myGlVendor = GlVendor_ImaginationTechnologies;
    }

    const StString aGlRenderer((const char* )core11fwd->glGetString(GL_RENDERER));
    if(aGlRenderer.isContains(stCString("GeForce"))) {
        myGpuName = GPU_GEFORCE;
    } else if(aGlRenderer.isContains(stCString("Quadro"))) {
        myGpuName = GPU_QUADRO;
    } else if(aGlRenderer.isContains(stCString("Radeon"))
           || aGlRenderer.isContains(stCString("RADEON"))) {
        myGpuName = GPU_RADEON;
    } else if(aGlRenderer.isContains(stCString("FireGL"))) {
        myGpuName = GPU_FIREGL;
    } else if(aGlRenderer.isContains(stCString("Adreno"))) {
        myGpuName = GPU_Adreno;
    } else if(aGlRenderer.isContains(stCString("PowerVR"))) {
        myGpuName = GPU_PowerVR;
    } else {
        myGpuName = GPU_UNKNOWN;
    }

    myWasInit = true;

    // deprecated in core!
    GLint aBitsRed = 0, aBitsGreen = 0, aBitsBlue = 0;
    core11fwd->glGetIntegerv(GL_RED_BITS,     &aBitsRed);
    core11fwd->glGetIntegerv(GL_GREEN_BITS,   &aBitsGreen);
    core11fwd->glGetIntegerv(GL_BLUE_BITS,    &aBitsBlue);
    core11fwd->glGetIntegerv(GL_ALPHA_BITS,   &myWindowBits.Alpha);
    core11fwd->glGetIntegerv(GL_DEPTH_BITS,   &myWindowBits.Depth);
    core11fwd->glGetIntegerv(GL_STENCIL_BITS, &myWindowBits.Stencil);
    myWindowBits.RGB = aBitsRed + aBitsGreen + aBitsBlue;

    if(hasFBO) {
        arbFbo = (StGLArbFbo* )(&(*myFuncs));

        // invalid attachment (GL_INVALID_ENUM) on AMD, Windows
        /*GLint aParam = GL_NONE;
        arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,     &aParam);
        if(aParam != GL_NONE) {
            arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,     &aBitsRed);
            arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE,   &aBitsGreen);
            arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,    &aBitsBlue);
            arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,   &myWindowBits.Alpha);
            arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH,     GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,   &myWindowBits.Depth);
            arbFbo->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL,   GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &myWindowBits.Stencil);
            myWindowBits.RGB = aBitsRed + aBitsGreen + aBitsBlue;
        }*/
    }

    for(int aFormatIter = 0; aFormatIter < StImagePlane::ImgNB; ++aFormatIter) {
        const StImagePlane::ImgFormat aFormat = (StImagePlane::ImgFormat )aFormatIter;
        GLint aDummy = 0;
        myDevCaps.setSupportedFormat(aFormat, StGLTexture::getInternalFormat(*this, aFormat, aDummy));
    }

    // log OpenGL info
    ST_DEBUG_LOG("Created new GL context:\n" + stglFullInfo());

#if !defined(GL_ES_VERSION_2_0)
    if(!has12) {
        myVerMajor = 1;
        myVerMinor = 1;
        return true;
    }

    if(!has13) {
        myVerMajor = 1;
        myVerMinor = 2;
        return true;
    }

    if(!has14) {
        myVerMajor = 1;
        myVerMinor = 3;
        return true;
    }

    if(!has15) {
        myVerMajor = 1;
        myVerMinor = 4;
        return true;
    }

    if(has20) {
        core20    = (StGLCore20*    )(&(*myFuncs));
        core20fwd = (StGLCore20Fwd* )(&(*myFuncs));
    } else {
        myVerMajor = 1;
        myVerMinor = 5;
        return true;
    }

    if(!has21) {
        myVerMajor = 2;
        myVerMinor = 0;
        return true;
    }

    if(!has30) {
        myVerMajor = 2;
        myVerMinor = 1;
        return true;
    }

    if(!has31) {
        myVerMajor = 3;
        myVerMinor = 0;
        return true;
    }

    if(has32) {
        core32     = (StGLCore32*     )(&(*myFuncs));
        core32back = (StGLCore32Back* )(&(*myFuncs));
    } else {
        myVerMajor = 3;
        myVerMinor = 1;
        return true;
    }

    if(!has33) {
        myVerMajor = 3;
        myVerMinor = 2;
        return true;
    }

    if(!has40) {
        myVerMajor = 3;
        myVerMinor = 3;
        return true;
    }

    if(has41) {
        core41     = (StGLCore41*     )(&(*myFuncs));
        core41back = (StGLCore41Back* )(&(*myFuncs));
    } else {
        myVerMajor = 4;
        myVerMinor = 0;
        return true;
    }

    if(has42) {
        core42     = (StGLCore42*     )(&(*myFuncs));
        core42back = (StGLCore42Back* )(&(*myFuncs));
    } else {
        myVerMajor = 4;
        myVerMinor = 1;
        return true;
    }

    if(has43) {
        core43     = (StGLCore43*     )(&(*myFuncs));
        core43back = (StGLCore43Back* )(&(*myFuncs));
    } else {
        myVerMajor = 4;
        myVerMinor = 2;
        return true;
    }

    if(has44) {
        core44     = (StGLCore44*     )(&(*myFuncs));
        core44back = (StGLCore44Back* )(&(*myFuncs));
    } else {
        myVerMajor = 4;
        myVerMinor = 3;
        return true;
    }
#endif

    return true;
}

template class StVec2<float>;
template class StVec2<double>;
template class StVec3<float>;
template class StVec3<double>;
template class StVec4<float>;
template class StVec4<double>;
template class StQuaternion<float>;
template class StQuaternion<double>;
