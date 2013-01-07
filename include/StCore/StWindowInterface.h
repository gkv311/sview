/**
 * Header define StWindow interface,
 * wich help to create one/double GL context window/double-window
 *
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

#ifndef __StWindowInterface_h_
#define __StWindowInterface_h_

#include <stTypes.h>
#include <StStrings/StString.h>
#include <StTemplates/StRect.h>

#include "StWinErrorCodes.h" // Header with error codes
#include "StMessageList.h"
#include "StNativeWin_t.h"

// StWindow enumeration
enum {
    ST_WIN_MASTER = 0,
    ST_WIN_SLAVE  = 1,
};

/**
 * StWindow attributes structure.
 * Notice that some options couln't be changed after window was created!
 */
typedef struct tagStWinAttributes {
    size_t nSize;
    // general attributes
    stBool_t isNoDecor;          //!< to decorate master window or not (will be ignored in case of embedded and fullscreen)
    stBool_t isStereoOutput;     //!< indicate stereoscopic output on / off (used for interconnection between modules)
    stBool_t isGlStereo;         //!< request OpenGL hardware accelerated QuadBuffer
    stBool_t isFullScreen;       //!< to show in fullscreen mode
    stBool_t isHide;             //!< to hide the master window
    stBool_t isHideCursor;       //!< to hide cursor
    stBool_t toBlockSleepSystem; //!< prevent system  going to sleep (display could be turned off)
    stBool_t toBlockSleepDisplay;//!< prevent display going to sleep
    stBool_t areGlobalMediaKeys; //!< register system hot-key to capture multimedia even without window focus
    // slave configuration
    stBool_t isSlave;            //!< create StWindow with slave window
    stBool_t isSlaveXMirrow;     //!< flip slave window position along X axis (horizontally)
    stBool_t isSlaveYMirrow;     //!< flip slave window position along Y axis (vertically)
    stBool_t isSlaveHLineTop;    //!< slave window - is a horizontal line at top of the display where master window shown
    stBool_t isSlaveHTop2Px;     //!< slave window - is a horizontal line 2 pixels long at top of the display where master window shown
    stBool_t isSlaveHLineBottom; //!< slave window - is a horizontal line at bottom of the display where master window shown
    stBool_t isSlaveHide;        //!< do not show slave window
    int8_t   slaveMonId;         //!< on which monitor show slave window (1 by default)
} StWinAttributes_t;

/**
 * Initialize the default StWindow attributes.
 * Use this function to avoid unused / forgotten parameters to be setted with invalid values.
 */
inline StWinAttributes_t stDefaultWinAttributes() {
    StWinAttributes_t stWinAttribs;
    stMemSet(&stWinAttribs, 0, sizeof(stWinAttribs));
    stWinAttribs.nSize = sizeof(stWinAttribs);
    stWinAttribs.slaveMonId = 1;
    return stWinAttribs;
}

/**
 * Compare two StWindow attributes structures.
 */
inline stBool_t areSame(const StWinAttributes_t* theAttrib1,
                        const StWinAttributes_t* theAttrib2) {
    if(theAttrib1->nSize != sizeof(StWinAttributes_t) ||
       theAttrib2->nSize != sizeof(StWinAttributes_t)) {
        // should be compared only structs
        return ST_FALSE;
    }
    // compare all known fields
    return (theAttrib1->isNoDecor          == theAttrib2->isNoDecor &&
            theAttrib1->isStereoOutput     == theAttrib2->isStereoOutput &&
            theAttrib1->isGlStereo         == theAttrib2->isGlStereo &&
            theAttrib1->isFullScreen       == theAttrib2->isFullScreen &&
            theAttrib1->isHide             == theAttrib2->isHide &&
            theAttrib1->isHideCursor       == theAttrib2->isHideCursor &&
            theAttrib1->toBlockSleepSystem == theAttrib2->toBlockSleepSystem &&
            theAttrib1->toBlockSleepDisplay== theAttrib2->toBlockSleepDisplay &&
            theAttrib1->areGlobalMediaKeys == theAttrib2->areGlobalMediaKeys &&
            theAttrib1->isSlave            == theAttrib2->isSlave &&
            theAttrib1->isSlaveXMirrow     == theAttrib2->isSlaveXMirrow &&
            theAttrib1->isSlaveYMirrow     == theAttrib2->isSlaveYMirrow &&
            theAttrib1->isSlaveHLineTop    == theAttrib2->isSlaveHLineTop &&
            theAttrib1->isSlaveHTop2Px     == theAttrib2->isSlaveHTop2Px &&
            theAttrib1->isSlaveHLineBottom == theAttrib2->isSlaveHLineBottom &&
            theAttrib1->isSlaveHide        == theAttrib2->isSlaveHide &&
            theAttrib1->slaveMonId         == theAttrib2->slaveMonId);
}

enum {
    ST_WIN_DATAKEYS_RENDERER = 0,
};

typedef struct tagStSlaveWindowCfg {
    int idMaster;
    int idSlave;      // slave window always should be assigned to monitor
    int xAdd;         // coordinates computed with this algorithm:
    int xSub;         // xAdd*(monLeft + left) + xSub*(monRight - right)
    int yAdd;         // yAdd*(monTop + top) + ySub*(monBottom - bottom)
    int ySub;
} StSlaveWindowCfg_t;

/**
 * This is Stereo Window interface.
 * Those windows:
 *   - contains OpenGL rendering context;
 *   - may have 'slaves', means another place-synchronized windows with own OpenGL rendering contexts;
 *   - master window could be embedded into native window;
 *   - implements own callback function;
 *   - hide system calls.
 */
class StWindowInterface {

        public:

    virtual StWindowInterface* getLibImpl() = 0;
    virtual ~StWindowInterface() {} // never forget it!

    virtual void close() = 0;

    /**
     * @param title (const StString& ) - new title.
     */
    virtual void setTitle(const StString& ) = 0;

    virtual void getAttributes(StWinAttributes_t* inOutAttributes) = 0;
    virtual void setAttributes(const StWinAttributes_t* inAttributes) = 0;

    /**
     * @return true if window content is visible.
     */
    virtual bool isActive() const = 0;

    /**
     * @return true if stereo output enabled.
     */
    virtual bool isStereoOutput() = 0;

    /**
     * @param stereoState(bool ) - enable/disable stereooutput.
     */
    virtual void setStereoOutput(bool stereoState) = 0;

    /**
     * ShowUp window.
     * @param winEnum (const int& ) - subwindow.
     */
    virtual void show(const int& ) = 0;

    /**
     * Hide window.
     * @param winEnum (const int& ) - subwindow.
     */
    virtual void hide(const int& ) = 0;

    /**
     * Show/Hide mouse cursor.
     * @param toShow (bool ) - true to show cursor.
     */
    virtual void showCursor(bool ) = 0;

    /**
     * @return true if in fullscreen state.
     */
    virtual bool isFullScreen() = 0;

    /**
     * @param fullscreen (bool ).
     */
    virtual void setFullScreen(bool ) = 0;

    /**
     * Get GUI GL window placement
     * @return rect (StRectI_t );
     */
    virtual StRectI_t getPlacement() = 0;

    /**
     * Change GUI GL window placement
     * @param rect (const StRectI_t& );
     */
    virtual void setPlacement(const StRectI_t& ) = 0;

    /**
     * @return point (StPointD_t ) - relative to window mouse position.
     * (0,0) - is top left of the window and (1,1) right buttom.
     */
    // TODO (Kirill Gavrilov#9#) add subwindow parameter
    virtual StPointD_t getMousePos() = 0;

    /**
     * Returns mouse click position from stack.
     * @param point (StPointD_t* ) - click point in relative coordinates;
     * @return (int ) mouse button id if any click in stack.
     */
    virtual int getMouseDown(StPointD_t* point) = 0;
    virtual int getMouseUp(StPointD_t* point) = 0;

    /**
     * Function to get Drag&Drop file list.
     * @param id (const int& ) - file if in list;
     * @param outFile (stUtf8_t* ) - buffer for file path;
     * @param buffSizeBytes (const size_t& ) - buffer size in bytes;
     * @return (int ) - number of files in list if id < 0 or outFile is NULL,
     * -1 on error.
     */
    virtual int getDragNDropFile(const int& id, stUtf8_t* outFile, const size_t& buffSizeBytes) = 0;

    /**
     * Function create a GL window.
     * @param attr (const StWinAttributes_t* ) - attributes;
     * @param nativeParent - handle with native window information (to create embedded StWindow);
     * @return true on success.
     */
    virtual bool stglCreate(const StWinAttributes_t* inAttributes,
                            const StNativeWin_t      nativeParent = (StNativeWin_t) NULL) = 0;

    /**
     * Swap dualbuffered GL context for specified window;
     * @param winEnum (const int& ) - subwindow to swap.
     */
    virtual void stglSwap(const int& ) = 0;

    /**
     * Make GL context for specified window active in current thread;
     * @param winEnum (const int& ) - subwindow to activate GL context.
     */
    virtual void stglMakeCurrent(const int& ) = 0;

    virtual double stglGetTargetFps() = 0;
    virtual void stglSetTargetFps(const double& ) = 0;

    /**
     * CallBack function.
     * @param stMessages (StMessage_t* ) - buffer to get new messages.
     */
    virtual void callback(StMessage_t* ) = 0;

    /**
     * Append message into callback list.
     * Message will be read on next callback() call.
     * @param stMessage (const StMessage_t& ) - message to append.
     */
    virtual stBool_t appendMessage(const StMessage_t& ) = 0;

    /**
     * Function to read size_t value in common StWindow instance map.
     * @param key (const size_t& ) - key in the map;
     * @param value (size_t* ) - value to read;
     * @return true if key was mapped.
     */
    virtual bool getValue(const size_t& key, size_t* value) = 0;

    /**
     * Function to store size_t value in common StWindow instance map.
     * Unicum key should be used.
     * @param key (const size_t& ) - key in the map;
     * @param value (const size_t& ) - value to store.
     */
    virtual void setValue(const size_t& key, const size_t& value) = 0;

};

#endif //__StWindowInterface_h_
