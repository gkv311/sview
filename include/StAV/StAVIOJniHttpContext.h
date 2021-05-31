/**
 * Copyright © 2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAVIOJniHttpContext_h_
#define __StAVIOJniHttpContext_h_

#include <StJNI/StJNIEnv.h>
#include <StAV/StAVIOContext.h>

/**
 * Custom AVIO context for reading http/https file using JNI
 * (java.net.URLConnection and java.nio.channels.ReadableByteChannel classes).
 * The main purpose of this class is providing a fallback https protocol support
 * in case when FFmpeg has been built without this support.
 */
class StAVIOJniHttpContext : public StAVIOContext {

        public:

    /**
     * Empty constructor.
     * WARNING! av_jni_get_java_vm() will be used to attach current thread to JavaVM and initialize pointers.
     * Therefore, JavaVM should be set to FFmpeg beforehand, and this context should be used only within the same working thread.
     */
    ST_CPPEXPORT StAVIOJniHttpContext();

    /**
     * Destructor. Closes open stream and detaches current thread from JavaVM.
     */
    ST_CPPEXPORT virtual ~StAVIOJniHttpContext();

    /**
     * Close the file.
     */
    ST_CPPEXPORT void close();

    /**
     * Open specified URL.
     */
    ST_CPPEXPORT bool open(const StString& theUrl);

    /**
     * Read from the file.
     */
    ST_CPPEXPORT virtual int read(uint8_t* theBuf,
                                  int      theBufSize) ST_ATTR_OVERRIDE;

    /**
     * Writing is not supported.
     */
    ST_LOCAL virtual int write(uint8_t* , int ) ST_ATTR_OVERRIDE { return -1; }

    /**
     * Seek within the file.
     */
    ST_CPPEXPORT virtual int64_t seek(int64_t theOffset,
                                      int     theWhence) ST_ATTR_OVERRIDE;

        private:

    /**
     * (Re)open HTTPS stream at specific offset.
     */
    bool reopenReadChannel(int64_t theOffset);
    struct FunctionTable;

        protected:

    FunctionTable* myFunctions; //!< JNI function table
    StJNIEnv myJEnv;        //!< JavaVM environment
    StString myUrl;         //!< opened URL
    void*    myReadChannel; //!< pointer to java.nio.channels.ReadableByteChannel
    int64_t  myContentLen;  //!< file length
    int64_t  myPosition;    //!< current position within the stream

};

ST_DEFINE_HANDLE(StAVIOJniHttpContext, StAVIOContext);

#endif // __StAVIOJniHttpContext_h_
