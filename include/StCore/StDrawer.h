/**
 * Copyright © 2007-2010 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StDrawer_h_
#define __StDrawer_h_

#include <StCore/StDrawerInterface.h>
#include <StCore/StDrawerInfo.h>
#include <StStrings/StString.h>
#include <StLibrary.h>

class StDrawer : public StDrawerInterface {

        protected:

    // typedef pointer-to-class
    typedef void* StWindow_t;
    typedef void* StDrawer_t;

    // types definitions - needed for each exported function
    typedef StDrawer_t (*StDrawer_new_t)();
    typedef void (*StDrawer_del_t)(StDrawer_t );
    typedef stBool_t (*StDrawer_init_t)(StDrawer_t , StWindow_t );
    typedef stBool_t (*StDrawer_open_t)(StDrawer_t , const StOpenInfo_t* );
    typedef void (*StDrawer_parseCallback_t)(StDrawer_t , StMessage_t* );
    typedef void (*StDrawer_stglDraw_t)(StDrawer_t , unsigned int );

    typedef const stUtf8_t* (*getMIMEDescription_t)();

        private:

    StLibrary stLib;

    // exported functions' pointers
    StDrawer_new_t StDrawer_new;
    StDrawer_del_t StDrawer_del;
    StDrawer_init_t StDrawer_init;
    StDrawer_open_t StDrawer_open;
    StDrawer_parseCallback_t StDrawer_parseCallback;
    StDrawer_stglDraw_t StDrawer_stglDraw;
    getMIMEDescription_t getMIMEDescription;

    StDrawerInterface* instance;

        public:

    /**
     * Empty constructor. Doesn't create class instance!
     */
    ST_CPPEXPORT StDrawer();

    /**
     * Open the plugin and retrieve function pointers.
     */
    ST_CPPEXPORT bool InitLibrary(const StString& thePluginPath);

    void Instantiate() {
        instance = (StDrawerInterface* )StDrawer_new();
    }

    StDrawerInterface* getLibImpl() {
        return instance;
    }

    bool init(StWindowInterface* stWin) {
        return StDrawer_init(instance, stWin);
    }

    bool open(const StOpenInfo& stOpenInfo = StOpenInfo()) {
        const StOpenInfo_t stOpenInfoStruct = stOpenInfo.getStruct();
        return StDrawer_open(instance, &stOpenInfoStruct);
    }

    void parseCallback(StMessage_t* stMessages) {
        StDrawer_parseCallback(instance, stMessages);
    }

    void stglDraw(unsigned int view) {
        return StDrawer_stglDraw(instance, view);
    }

    ~StDrawer() {
        Destruct();
    }

    ST_CPPEXPORT void Destruct();

    // auxiliary function
    const stUtf8_t* GetMIMEList() const {
        if(getMIMEDescription == NULL) {
            return NULL;
        }
        return getMIMEDescription();
    }

};

#endif //__StDrawer_h_
