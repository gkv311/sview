/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StApplication_h_
#define __StApplication_h_

#include "StApplicationInterface.h"

class StLibrary;

class StApplication : public StApplicationInterface {

        public:

    // allow use type definitions
    friend class StCore;

    // typedef pointer-to-class
    typedef void* StApplication_t;

    // types definitions - needed for each exported function
    typedef StApplication_t (*StApplication_new_t)();
    typedef void (*StApplication_del_t)(StApplication_t );
    typedef stBool_t (*StApplication_isOpened_t)(StApplication_t );
    typedef stBool_t (*StApplication_isFullscreen_t)(StApplication_t );
    typedef stBool_t (*StApplication_create_t)(StApplication_t , const StNativeWin_t );
    typedef stBool_t (*StApplication_open_t)(StApplication_t , const StOpenInfo_t* );
    typedef void (*StApplication_callback_t)(StApplication_t , StMessage_t* );

    class AppFunctions {

            public:

        StApplication_new_t StApplication_new;
        StApplication_del_t StApplication_del;
        StApplication_isOpened_t     StApplication_isOpened;
        StApplication_isFullscreen_t StApplication_isFullscreen;
        StApplication_create_t   StApplication_create;
        StApplication_open_t     StApplication_open;
        StApplication_callback_t StApplication_callback;

            public:

        ST_CPPEXPORT AppFunctions();
        ST_CPPEXPORT ~AppFunctions();

        ST_CPPEXPORT void load(StLibrary& theLib);
        ST_CPPEXPORT bool isNull() const;
        ST_CPPEXPORT void nullify();

    };

    // core exported functions' pointers
    ST_CPPEXPORT static AppFunctions& GetFunctions();

        private:

    StApplicationInterface* libInstance;
    bool isPointer;

        public:

    StApplication() {
        isPointer = false;
        libInstance = (StApplicationInterface* )GetFunctions().StApplication_new();
    }

    StApplication(StApplicationInterface* inst) {
        isPointer = true;
        libInstance = inst;
    }

    StApplicationInterface* getLibImpl() {
        return libInstance;
    }

    ~StApplication() {
        if(!isPointer) {
            GetFunctions().StApplication_del(libInstance);
        }
    }

    bool isOpened() {
        return GetFunctions().StApplication_isOpened(libInstance);
    }

    bool isFullscreen() {
        return GetFunctions().StApplication_isFullscreen(libInstance);
    }

    bool create(const StNativeWin_t nativeParent = (StNativeWin_t )NULL) {
        return GetFunctions().StApplication_create(libInstance, nativeParent);
    }

    bool open(const StOpenInfo& stOpenInfo = StOpenInfo()) {
        const StOpenInfo_t stOpenInfoStruct = stOpenInfo.getStruct();
        return GetFunctions().StApplication_open(libInstance, &stOpenInfoStruct);
    }

    void callback(StMessage_t* stMessages = NULL) {
        return GetFunctions().StApplication_callback(libInstance, stMessages);
    }

};

#endif //__StApplication_h_
