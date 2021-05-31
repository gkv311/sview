/**
 * Copyright © 2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StAV/StAVIOJniHttpContext.h>

#include <StStrings/StLogger.h>

extern "C" {
    #include <libavutil/error.h>
};

#if defined(__ANDROID__)
    extern "C" {
        #include <libavcodec/jni.h>
    };
    #include <android/window.h>
    #include <jni.h>

/**
 * java.net.URLConnection wrapper.
 */
struct StAVIOJniHttpContext::FunctionTable {

    /**
     * Create java.nio.channels.ReadableByteChannel by opening java.net.URLConnection to specified URL.
     * @param theJEnv   [in] attached JavaVM environment
     * @param theUrl    [in] URL to open
     * @param theOffset [in] file offset
     * @param theContentLen [out] file length
     * @return local reference to java.nio.channels.ReadableByteChannel
     */
    jobject createNewChannel(StJNIEnv& theJEnv,
                             const StString& theUrl,
                             int64_t theOffset,
                             int64_t& theContentLen) {
        theJEnv->ExceptionClear();
        jobject aJInputStream = createInputStream(theJEnv, theUrl, theOffset, theContentLen);
        if(aJInputStream == NULL) {
            return NULL;
        }
        jobject aJReadChannel = theJEnv->CallStaticObjectMethod(myNioChannels_Class, myNioChannels_newChannel, aJInputStream);
        theJEnv->DeleteLocalRef(aJInputStream);
        return aJReadChannel;
    }

    /**
     * Read portion of data from opened channel.
     * @sa int java.nio.channels.ReadableByteChannel::read(java.nio.ByteBuffer theBuffer).
     * @param theJEnv    [in] attached JavaVM environment
     * @param theChannel [in] opened channel java.nio.channels.ReadableByteChannel
     * @param theBuf    [out] buffer to fill
     * @param theBufSize [in] buffer length
     * @return amount of bytes read from channel or -1 on error/EOF; could be also 0 bytes.
     */
    int readChannelIntoBuffer(StJNIEnv& theJEnv,
                              jobject theChannel,
                              uint8_t* theBuf,
                              int theBufSize) {
        theJEnv->ExceptionClear();
        jobject aJBuffer = theJEnv->NewDirectByteBuffer(theBuf, theBufSize);
        if(aJBuffer == NULL) {
            return -1;
        }

        int aNbReadBytes = theJEnv->CallIntMethod(theChannel, myNioReadableByteChannel_read, aJBuffer);
        theJEnv->DeleteLocalRef(aJBuffer);
        if(theJEnv->ExceptionCheck()) {
            theJEnv->ExceptionClear();
            return -1;
        }
        return aNbReadBytes;
    }

    /**
     * Load functions.
     */
    bool load(StJNIEnv& theJEnv) {
        if(!myjava_net_URL.load(theJEnv)) {
            return false;
        }

        jclass aClass = theJEnv->FindClass("java/net/URLConnection");
        if(aClass == NULL) {
            ST_ERROR_LOG("Unable to find java.net.URLConnection class");
            return false;
        }

        myconnect = theJEnv->GetMethodID(aClass, "connect", "()V");
        mysetRequestProperty = theJEnv->GetMethodID(aClass, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
        mygetInputStream = theJEnv->GetMethodID(aClass, "getInputStream", "()Ljava/io/InputStream;");
        mygetContentLength = theJEnv->GetMethodID(aClass, "getContentLength", "()I");
        mygetContentLengthLong = theJEnv->GetMethodID(aClass, "getContentLengthLong", "()J");
        if(myconnect == NULL || mysetRequestProperty == NULL || mygetInputStream == NULL || mygetContentLength == NULL) {
            ST_ERROR_LOG("Unable to find java.net.URLConnection class methods");
            return false;
        }

        myNioChannels_Class      = theJEnv->FindClass("java/nio/channels/Channels");
        myNioChannels_newChannel = theJEnv->GetStaticMethodID(myNioChannels_Class, "newChannel",
                                                              "(Ljava/io/InputStream;)Ljava/nio/channels/ReadableByteChannel;");
        if(myNioChannels_Class == NULL
        || myNioChannels_newChannel == NULL) {
            ST_ERROR_LOG("Unable to find java.nio.channels.Channels class");
            return false;
        }
        myNioChannels_Class = (jclass )theJEnv->NewGlobalRef(myNioChannels_Class);

        jclass aNioReadableByteChannel_Class = theJEnv->FindClass("java/nio/channels/ReadableByteChannel");
        myNioReadableByteChannel_read = aNioReadableByteChannel_Class != NULL
                                      ? theJEnv->GetMethodID(aNioReadableByteChannel_Class, "read", "(Ljava/nio/ByteBuffer;)I")
                                      : NULL;
        if(myNioReadableByteChannel_read == NULL) {
            ST_ERROR_LOG("Unable to find java.nio.channels.ReadableByteChannel class");
            return false;
        }
        return true;
    }

    /**
     * Release global references (to Java classes).
     */
    void release(StJNIEnv& theJEnv) {
        myjava_net_URL.release(theJEnv);
        if(myNioChannels_Class != NULL) {
            theJEnv->DeleteGlobalRef(myNioChannels_Class);
            myNioChannels_Class = NULL;
        }
    }

        private:

    /**
     * Create java.io.InputStream by opening java.net.URLConnection to specified URL.
     * @param theJEnv   [in] attached JavaVM environment
     * @param theUrl    [in] URL to open
     * @param theOffset [in] file offset
     * @param theContentLen [out] file length
     * @return local reference to java.io.InputStream
     */
    jobject createInputStream(StJNIEnv& theJEnv,
                              const StString& theUrl,
                              int64_t theOffset,
                              int64_t& theContentLen) {
        jobject aJConnection = myjava_net_URL.openConnection(theJEnv, theUrl);
        if(aJConnection == NULL) {
            return NULL;
        }

        if(theOffset != 0) {
            StString anOffset = StString("bytes=") + theOffset + "-";
            jstring aJStr_Range  = theJEnv->NewStringUTF("Range");
            jstring aJStr_Offset = theJEnv->NewStringUTF(anOffset.toCString());
            theJEnv->CallVoidMethod(aJConnection, mysetRequestProperty, aJStr_Range, aJStr_Offset);
            theJEnv->DeleteLocalRef(aJStr_Range);
            theJEnv->DeleteLocalRef(aJStr_Offset);
        }
        theJEnv->CallVoidMethod(aJConnection, myconnect);
        if(theJEnv->ExceptionCheck()) {
            ST_ERROR_LOG("Unable to open connection java.net.URLConnection '" + theUrl + "'");
            theJEnv->ExceptionClear();
            theJEnv->DeleteLocalRef(aJConnection);
            return NULL;
        }

        if(mygetContentLengthLong != NULL) {
            theContentLen = theJEnv->CallLongMethod(aJConnection, mygetContentLengthLong);
        } else {
            theContentLen = theJEnv->CallIntMethod(aJConnection, mygetContentLength);
        }

        jobject aJInputStream = theJEnv->CallObjectMethod(aJConnection, mygetInputStream);
        theJEnv->DeleteLocalRef(aJConnection);
        if(theJEnv->ExceptionCheck()) {
            theJEnv->ExceptionClear();
            if(aJInputStream != NULL) {
                theJEnv->DeleteLocalRef(aJInputStream);
                aJInputStream = NULL;
            }
        }
        if(aJInputStream == NULL) {
            ST_ERROR_LOG("Unable to create java.io.InputStream");
        }
        return aJInputStream;
    }

    // java.net.URL wrapper.
    struct st_java_net_URL {

        /**
         * Load java.net.URL class and its methods.
         */
        bool load(StJNIEnv& theJEnv) {
            myClass = theJEnv->FindClass("java/net/URL");
            if(myClass == NULL) {
                ST_ERROR_LOG("Unable to find java.net.URL class");
                return false;
            }

            myClass = (jclass )theJEnv->NewGlobalRef(myClass);
            myInit = theJEnv->GetMethodID(myClass, "<init>", "(Ljava/lang/String;)V");
            myopenConnection = theJEnv->GetMethodID(myClass, "openConnection", "()Ljava/net/URLConnection;");
            if(myInit == NULL || myopenConnection == NULL) {
                ST_ERROR_LOG("Unable to find java.net.URL class methods");
                release(theJEnv);
                return false;
            }
            return true;
        }

        /**
         * Release global references (to Java class).
         */
        void release(StJNIEnv& theJEnv) {
            if(myClass != NULL) {
                theJEnv->DeleteGlobalRef(myClass);
                myClass = NULL;
            }
        }

        /**
         * Create java.net.URLConnection opening specified URL.
         * Return local reference or NULL on failure.
         */
        jobject openConnection(StJNIEnv& theJEnv, const StString& theUrl) {
            jobject aJNetUrl = create(theJEnv, theUrl);
            if(aJNetUrl == NULL) {
                ST_ERROR_LOG("Unable to create java.net.URL");
                return NULL;
            }

            jobject aJNetUrlCon = theJEnv->CallObjectMethod(aJNetUrl, myopenConnection);
            theJEnv->DeleteLocalRef(aJNetUrl);
            if(theJEnv->ExceptionCheck()) {
                theJEnv->ExceptionClear();
                if(aJNetUrlCon != NULL) {
                    theJEnv->DeleteLocalRef(aJNetUrlCon);
                    aJNetUrlCon = NULL;
                }
            }
            if(aJNetUrlCon == NULL) {
                ST_ERROR_LOG("Unable to create java.net.URLConnection");
                return NULL;
            }
            return aJNetUrlCon;
        }

            private:

        /**
         * Create java.net.URL class instance with specified URL; returns local reference.
         */
        jobject create(StJNIEnv& theJEnv, const StString& theUrl) {
            jstring aJStr_Url = theJEnv->NewStringUTF(theUrl.toCString());
            jobject aNetURL = theJEnv->NewObject(myClass, myInit, aJStr_Url);
            theJEnv->DeleteLocalRef(aJStr_Url);
            return aNetURL;
        }

            private:

        jclass    myClass = NULL;
        jmethodID myInit  = NULL;
        jmethodID myopenConnection = NULL;

    } myjava_net_URL;

    // java.net.URLConnection
    jmethodID myconnect = NULL;
    jmethodID mysetRequestProperty = NULL;
    jmethodID mygetInputStream = NULL;
    jmethodID mygetContentLength = NULL;
    jmethodID mygetContentLengthLong = NULL;

    // java.nio.channels.Channels
    jclass    myNioChannels_Class = NULL;
    jmethodID myNioChannels_newChannel = NULL;

    //java.nio.channels.ReadableByteChannel
    //myNioReadableByteChannel_Class = NULL
    jmethodID myNioReadableByteChannel_read = NULL;

};
#endif

StAVIOJniHttpContext::StAVIOJniHttpContext()
: myFunctions(NULL),
#if defined(__ANDROID__)
  myJEnv((JavaVM* )av_jni_get_java_vm(NULL)),
#else
  myJEnv(NULL),
#endif
  myReadChannel(NULL),
  myContentLen(0),
  myPosition(0) {
#if defined(__ANDROID__)
    if(!myJEnv.isNull()) {
        myFunctions = new FunctionTable();
        if(!myFunctions->load(myJEnv)) {
            myFunctions->release(myJEnv);
            myJEnv.detach();
        }
    }
#endif
}

StAVIOJniHttpContext::~StAVIOJniHttpContext() {
    close();
    if(myFunctions != NULL) {
    #if defined(__ANDROID__)
        myFunctions->release(myJEnv);
        delete myFunctions;
    #endif
    }
}

void StAVIOJniHttpContext::close() {
    if(myReadChannel != NULL) {
    #if defined(__ANDROID__)
        myJEnv->DeleteGlobalRef((jobject )myReadChannel);
    #endif
        myReadChannel = NULL;
    }
}

bool StAVIOJniHttpContext::reopenReadChannel(int64_t theOffset) {
    if(myJEnv.isNull()) {
        return false;
    }

#if defined(__ANDROID__)
    if(myReadChannel != NULL) {
        myJEnv->DeleteGlobalRef((jobject )myReadChannel);
        myReadChannel = NULL;
    }

    /**
     * java.net.URL anUrl = new java.net.URL(aPath);
     * java.net.URLConnection anUrlConnection = anUrl.openConnection();
     * anUrlConnection.setRequestProperty ("Range", "bytes="+offset+"-");
     * anUrlConnection.connect();
     * java.io.InputStream anInputStream = anUrlConnection.getInputStream();
     * java.nio.channels.ReadableByteChannel aReadableByteChannel = java.nio.channels.Channels.newChannel(anInputStream);
     */
    jobject aJReadChannel = myFunctions->createNewChannel(myJEnv, myUrl, theOffset, myContentLen);
    if(aJReadChannel == NULL) {
        myReadChannel = NULL;
        return false;
    }
    myReadChannel = myJEnv->NewGlobalRef(aJReadChannel);
    myPosition = theOffset;
    return true;
#else
    (void )theOffset;
    return false;
#endif
}

bool StAVIOJniHttpContext::open(const StString& theUrl) {
    close();
    myUrl = theUrl;
    return reopenReadChannel(0);
}

int StAVIOJniHttpContext::read(uint8_t* theBuf,
                               int      theBufSize) {
    if(myReadChannel == NULL
    || theBuf == NULL
    || theBufSize <= 0) {
        return -1;
    }

#if defined(__ANDROID__)
    int aNbRead = myFunctions->readChannelIntoBuffer(myJEnv, (jobject )myReadChannel, theBuf, theBufSize);
    if(aNbRead == -1) {
        return AVERROR_EOF;
    }
    return aNbRead;
#else
    return -1;
#endif
}

int64_t StAVIOJniHttpContext::seek(int64_t theOffset,
                                   int     theWhence) {
    if(myReadChannel == NULL) {
        return -1;
    }

    switch(theWhence) {
        case AVSEEK_SIZE: {
            return myContentLen != 0 ? myContentLen : -1;
        }
        case SEEK_SET: {
            reopenReadChannel(theOffset);
            return myPosition;
        }
        case SEEK_CUR: {
            reopenReadChannel(myPosition + theOffset);
            return myPosition;
        }
        case SEEK_END: {
            if(myContentLen <= 0) {
                return -1;
            }
            reopenReadChannel(myContentLen + theOffset);
            return myPosition;
        }
    }
    return -1;
}
