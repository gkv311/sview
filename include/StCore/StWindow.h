/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StWindow_h_
#define __StWindow_h_

#include "StWindowInterface.h"

class StLibrary;

/**
 * StWindow - StWindowInterface implementation from StCore library.
 */
class ST_LOCAL StWindow : public StWindowInterface {

        public:

    // allow use type definitions
    friend class StCore;

    // typedef pointer-to-class
    typedef void* StWindow_t;

    // types definitions - needed for each exported function
    typedef StWindow_t (*StWindow_new_t)();
    typedef void (*StWindow_del_t)(StWindow_t );
    typedef void (*StWindow_close_t)(StWindow_t );
    typedef void (*StWindow_setTitle_t)(StWindow_t , const stUtf8_t* );
    typedef void (*StWindow_getAttributes_t)(StWindow_t , StWinAttributes_t* );
    typedef void (*StWindow_setAttributes_t)(StWindow_t , const StWinAttributes_t* );
    typedef stBool_t (*StWindow_isActive_t)(StWindow_t );
    typedef stBool_t (*StWindow_isStereoOutput_t)(StWindow_t );
    typedef void (*StWindow_setStereoOutput_t)(StWindow_t , stBool_t );
    typedef void (*StWindow_show_t)(StWindow_t , const int& , const stBool_t& );
    typedef void (*StWindow_showCursor_t)(StWindow_t , stBool_t );
    typedef stBool_t (*StWindow_isFullScreen_t)(StWindow_t );
    typedef void (*StWindow_setFullScreen_t)(StWindow_t , stBool_t );
    typedef void (*StWindow_getPlacement_t)(StWindow_t , StRectI_t* );
    typedef void (*StWindow_setPlacement_t)(StWindow_t , const StRectI_t* );
    typedef void (*StWindow_getMousePos_t)(StWindow_t , StPointD_t* );
    typedef int (*StWindow_getMouseDown_t)(StWindow_t , StPointD_t* );
    typedef int (*StWindow_getMouseUp_t)(StWindow_t , StPointD_t* );
    typedef int (*StWindow_getDragNDropFile_t)(StWindow_t, const int& , stUtf8_t* , const size_t& );
    typedef stBool_t (*StWindow_stglCreate_t)(StWindow_t , const StWinAttributes_t* , const StNativeWin_t );
    typedef void (*StWindow_stglSwap_t)(StWindow_t , const int& );
    typedef void (*StWindow_stglMakeCurrent_t)(StWindow_t , const int& );
    typedef double (*StWindow_stglGetTargetFps_t)(StWindow_t );
    typedef void (*StWindow_stglSetTargetFps_t)(StWindow_t , const double& );
    typedef void (*StWindow_callback_t)(StWindow_t , StMessage_t* );
    typedef stBool_t (*StWindow_appendMessage_t)(StWindow_t , const StMessage_t& );
    typedef stBool_t (*StWindow_getValue_t)(StWindow_t , const size_t& , size_t* );
    typedef void (*StWindow_setValue_t)(StWindow_t , const size_t& , const size_t& );
    typedef void* (*StWindow_memAlloc_t)(const size_t& );
    typedef void (*StWindow_memFree_t)(void* );

    // exported functions' pointers
    class WindowFunctions {

            public:

        StWindow_new_t StWindow_new;
        StWindow_del_t StWindow_del;
        StWindow_close_t StWindow_close;
        StWindow_setTitle_t StWindow_setTitle;
        StWindow_getAttributes_t StWindow_getAttributes;
        StWindow_setAttributes_t StWindow_setAttributes;
        StWindow_isActive_t StWindow_isActive;
        StWindow_isStereoOutput_t StWindow_isStereoOutput;
        StWindow_setStereoOutput_t StWindow_setStereoOutput;
        StWindow_show_t StWindow_show;
        StWindow_showCursor_t StWindow_showCursor;
        StWindow_isFullScreen_t StWindow_isFullScreen;
        StWindow_setFullScreen_t StWindow_setFullScreen;
        StWindow_getPlacement_t StWindow_getPlacement;
        StWindow_setPlacement_t StWindow_setPlacement;
        StWindow_getMousePos_t StWindow_getMousePos;
        StWindow_getMouseDown_t StWindow_getMouseDown;
        StWindow_getMouseUp_t StWindow_getMouseUp;
        StWindow_getDragNDropFile_t StWindow_getDragNDropFile;
        StWindow_stglCreate_t StWindow_stglCreate;
        StWindow_stglSwap_t StWindow_stglSwap;
        StWindow_stglMakeCurrent_t StWindow_stglMakeCurrent;
        StWindow_stglGetTargetFps_t StWindow_stglGetTargetFps;
        StWindow_stglSetTargetFps_t StWindow_stglSetTargetFps;
        StWindow_callback_t StWindow_callback;
        StWindow_appendMessage_t StWindow_appendMessage;
        StWindow_getValue_t StWindow_getValue;
        StWindow_setValue_t StWindow_setValue;
        StWindow_memAlloc_t StWindow_memAlloc;
        StWindow_memFree_t StWindow_memFree;

            public:

        WindowFunctions();

        void load(StLibrary& theLib);
        bool isNull() const;
        void nullify();

    };

    // core exported functions' pointers
    static WindowFunctions& GetFunctions();

        private:

    StWindowInterface* libInstance;
    bool isPointer;

        public:

    StWindow() {
        isPointer = false;
        libInstance = (StWindowInterface* )GetFunctions().StWindow_new();
    }

    StWindow(StWindowInterface* inst) {
        isPointer = true;
        libInstance = inst;
    }

    StWindowInterface* getLibImpl() {
        return libInstance;
    }

    ~StWindow() {
        if(!isPointer) {
            GetFunctions().StWindow_del(libInstance);
        }
    }

    void close() {
        GetFunctions().StWindow_close(libInstance);
    }

    void setTitle(const StString& theTitle) {
        GetFunctions().StWindow_setTitle(libInstance, theTitle.toCString());
    }

    void getAttributes(StWinAttributes_t* inOutAttributes) {
        GetFunctions().StWindow_getAttributes(libInstance, inOutAttributes);
    }

    void setAttributes(const StWinAttributes_t* inAttributes) {
        GetFunctions().StWindow_setAttributes(libInstance, inAttributes);
    }

    bool isActive() const {
        return GetFunctions().StWindow_isActive(libInstance);
    }

    bool isStereoOutput() {
        return GetFunctions().StWindow_isStereoOutput(libInstance);
    }

    void setStereoOutput(bool stereoState) {
        GetFunctions().StWindow_setStereoOutput(libInstance, stereoState);
    }

    void show(const int& winEnum) {
        GetFunctions().StWindow_show(libInstance, winEnum, ST_TRUE);
    }

    void hide(const int& winEnum) {
        GetFunctions().StWindow_show(libInstance, winEnum, ST_FALSE);
    }

    void showCursor(bool toShow) {
        GetFunctions().StWindow_showCursor(libInstance, toShow);
    }

    bool isFullScreen() {
        return GetFunctions().StWindow_isFullScreen(libInstance);
    }

    void setFullScreen(bool fullscreen) {
        GetFunctions().StWindow_setFullScreen(libInstance, fullscreen);
    }

    StRectI_t getPlacement() {
        StRectI_t rect;
        GetFunctions().StWindow_getPlacement(libInstance, &rect);
        return rect;
    }

    void setPlacement(const StRectI_t& rect) {
        GetFunctions().StWindow_setPlacement(libInstance, &rect);
    }

    StPointD_t getMousePos() {
        StPointD_t point;
        GetFunctions().StWindow_getMousePos(libInstance, &point);
        return point;
    }

    int getMouseDown(StPointD_t* point) {
        return GetFunctions().StWindow_getMouseDown(libInstance, point);
    }

    int getMouseUp(StPointD_t* point) {
        return GetFunctions().StWindow_getMouseUp(libInstance, point);
    }

    int getDragNDropFile(const int& id, stUtf8_t* outFile, const size_t& buffSizeBytes) {
        return GetFunctions().StWindow_getDragNDropFile(libInstance, id, outFile, buffSizeBytes);
    }

    bool stglCreate(const StWinAttributes_t* inAttributes, const StNativeWin_t nativeParentWindow = (StNativeWin_t )NULL) {
        return GetFunctions().StWindow_stglCreate(libInstance, inAttributes, nativeParentWindow);
    }

    void stglSwap(const int& value = ST_WIN_MASTER) {
        GetFunctions().StWindow_stglSwap(libInstance, value);
    }

    void stglMakeCurrent(const int& value) {
        GetFunctions().StWindow_stglMakeCurrent(libInstance, value);
    }

    double stglGetTargetFps() {
        return GetFunctions().StWindow_stglGetTargetFps(libInstance);
    }

    void stglSetTargetFps(const double& fps) {
        GetFunctions().StWindow_stglSetTargetFps(libInstance, fps);
    }

    void callback(StMessage_t* stMessages) {
        GetFunctions().StWindow_callback(libInstance, stMessages);
    }

    stBool_t appendMessage(const StMessage_t& stMessage) {
        return GetFunctions().StWindow_appendMessage(libInstance, stMessage);
    }

    bool getValue(const size_t& key, size_t* value) {
        return GetFunctions().StWindow_getValue(libInstance, key, value);
    }

    void setValue(const size_t& key, const size_t& value) {
        GetFunctions().StWindow_setValue(libInstance, key, value);
    }

    /**
     * Special memory allocator, used to store/share data in StCore library scope.
     */
    static void* memAlloc(const size_t& theSize) {
        return GetFunctions().StWindow_memAlloc(theSize);
    }

    static void memFree(void* thePtr) {
        GetFunctions().StWindow_memFree(thePtr);
    }

    static stUtf8_t* memAllocNCopy(const StString& theString) {
        stUtf8_t* aPtr = (stUtf8_t* )memAlloc(theString.getSize() + sizeof(stUtf8_t));
        stMemCpy(aPtr, theString.toCString(), theString.getSize() + sizeof(stUtf8_t));
        return aPtr;
    }

};

#endif //__StWindow_h_
