/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StCheckUpdates_h_
#define __StCheckUpdates_h_

#include <StSocket/StSocket.h>
#include <StThreads/StMutex.h>
#include <StThreads/StThread.h>
#include <StVersion.h>

/**
 * Helper class to check sView for updates.
 */
class StCheckUpdates {

        private:

    StMutex stMutex;
    StThread* hCheckUpdates;
    bool bInitialized;
    bool bNeedUpdate;

    static SV_THREAD_FUNCTION checkUpdatesThread(void* inCheckUpdates) {
        StCheckUpdates* stCheckUpdates = (StCheckUpdates* )inCheckUpdates;
        if(StCheckUpdates::checkUpdates()) {
            stCheckUpdates->setNeedUpdate(true);
        }
        stCheckUpdates->setInitialized();
        return SV_THREAD_RETURN 0;
    }

        public:

    StCheckUpdates()
    : stMutex(),
      hCheckUpdates(NULL),
      bInitialized(false),
      bNeedUpdate(false) {
        //
    }

    ~StCheckUpdates() {
        if(hCheckUpdates != NULL) {
            hCheckUpdates->wait();
            delete hCheckUpdates;
            hCheckUpdates = NULL;
        }
    }

    bool isNeedUpdate() {
        stMutex.lock();
            bool result = bNeedUpdate;
        stMutex.unlock();
        return result;
    }

    void setNeedUpdate(bool toUpdate) {
        stMutex.lock();
            bNeedUpdate = toUpdate;
        stMutex.unlock();
    }

    bool isInitialized() {
        stMutex.lock();
            bool result = bInitialized;
        stMutex.unlock();
        return result;
    }

    void setInitialized() {
        stMutex.lock();
            bInitialized = true;
        stMutex.unlock();
    }

    void init() {
        /// TODO (Kirill Gavrilov#1) unstability in browsers???
        stMutex.lock();
        if(hCheckUpdates != NULL) {
            stMutex.unlock();
            return;
        }
        hCheckUpdates = new StThread(checkUpdatesThread, (void* )this);
        stMutex.unlock();
    }

    static bool checkUpdates() {
        static const char SVIEW_UPDATES_SERVER[] = "www.sview.ru";
        static const char sViewUpdateRequestTemplate[] = "GET /updates/?appl=sView&year=%d&month=%d&rStatus=%d&rSubVer=%d HTTP/1.1\r\nUser-Agent: sView\r\nHost: www.sview.ru\r\n\r\n";
        static const StVersionInfo stVersion = StVersionInfo::getSDKVersionInfo();
        char sViewUpdateRequest[2048];
        sprintf(sViewUpdateRequest, sViewUpdateRequestTemplate, stVersion.getYear(), stVersion.getMonth(), stVersion.getReleaseStatus(), stVersion.getSubVersion());
        ///sprintf(sViewUpdateRequest, sViewUpdateRequestTemplate, stVersion.getYear(), 8, stVersion.getReleaseStatus(), stVersion.getSubVersion());

        StSocket stSocket;
        if(!stSocket.open()) {
            return false;
        }
        ST_DEBUG_LOG_AT("Opened socket...");

        if(!stSocket.connect(SVIEW_UPDATES_SERVER)) {
            ST_DEBUG_LOG_AT("ERROR connecting...");
            return false;
        }
        ST_DEBUG_LOG_AT("Connection opened...");

        StSocket::Buffer buffer(2048);
        buffer = sViewUpdateRequest;

        // TODO (Kirill Gavrilov#9#) ccreate HTTP-helper class
        if(!stSocket.send(buffer)) {
            ST_DEBUG_LOG_AT("ERROR sending...");
            return false;
        }
        ST_DEBUG_LOG("Sent:\n" + buffer.getData());

        // TODO (Kirill Gavrilov#9#) receive full server answer
        // we assume buffer has inaf size
        if(!stSocket.recv(buffer)) {
            ST_DEBUG_LOG_AT("ERROR receiving...");
            return false;
        }
        ST_DEBUG_LOG("Received:\n" + buffer.getData());

        // TODO (Kirill Gavrilov#9#) process true XML parsing
        static const char searchSubString[] = "<update>yes</update>";
        size_t subStringLength = strlen(searchSubString);

        for(size_t ch = 0; ch < buffer.getSize() - subStringLength; ch++) {
            if(strncmp(&buffer[ch], searchSubString, subStringLength) == 0) {
                ST_DEBUG_LOG("Updated version available!");
                return true;
            }
        }
        return false;
    }

};

#endif //__StCheckUpdates_h_
