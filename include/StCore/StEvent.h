/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StEvent_h_
#define __StEvent_h_

#include <StTemplates/StVec2.h>
#include <StThreads/StMutex.h>
#include <StThreads/StTimer.h>
#include "StVirtualKeys.h"

enum StEventType {
    stEvent_None,       //!< StAnyEvent,    undefined event
    stEvent_Close,      //!< StCloseEvent,  window close requested
    stEvent_Size,       //!< StSizeEvent,   window resized
    stEvent_NewMonitor, //!< StSizeEvent,   window moved to another monitor
    stEvent_KeyDown,    //!< StKeyEvent,    keyboard key pressed
    stEvent_KeyUp,      //!< StKeyEvent,    keyboard key released
    stEvent_KeyHold,    //!< StKeyEvent,    keyboard key holded
    stEvent_MouseDown,  //!< StClickEvent,  mouse button pressed
    stEvent_MouseUp,    //!< StClickEvent,  mouse button released
    stEvent_FileDrop,   //!< StDNDropEvent, file Drag & Drop
    ///MSG_MOUSE_MOVE,
    ///MSG_GO_TOP,
    ///MSG_GO_BOTTOM,
    ///MSG_GO_BACKWARD,
    ///MSG_GO_FORWARD,
};

/**
 * Fields shared by all events.
 */
struct StAnyEvent {

    StEventType   Type;   //!< event type
    double        Time;   //!< time in seconds when event was registered

};

/**
 * Close window request.
 */
struct StCloseEvent {

    StEventType   Type;   //!< event type
    double        Time;   //!< time in seconds when event was registered

};

/**
 * Window resize event.
 */
struct StSizeEvent {

    StEventType   Type;    //!< event type
    double        Time;    //!< time in seconds when event was registered
    unsigned int  SizeX;   //!< new window rectangle in pixels
    unsigned int  SizeY;

};

/**
 * Keyboard key down/up event.
 */
struct StKeyEvent {

    StEventType   Type;     //!< event type
    double        Time;     //!< time in seconds when event was registered
    double        Duration; //!< time in seconds, how long key is/was holded
    double        Progress; //!< time in seconds, how long key is/was holded since last callback (stEvent_KeyHold)
    StVirtKey     VKey;     //!< virtual key code (language independent and case insensitive)
    StVirtFlags   Flags;    //!< modifier keys pressed in the moment of event
    stUtf32_t     Char;     //!< associated UTF-32 character code

};

/**
 * Mouse button down/up event.
 */
struct StClickEvent {

    StEventType   Type;    //!< event type
    double        Time;    //!< time in seconds when event was registered
    double        PointX;  //!< click point defined relative to window from top-left (0,0) to the bottom-right (1,1)
    double        PointY;
    StVirtButton  Button;  //!< mouse button identifier (reason for event)
    unsigned int  Buttons; //!< other mouse buttons pressed in the moment of event

};

/**
 * File Drag & Drop event.
 */
struct StDNDropEvent {

    StEventType   Type;   //!< event type
    double        Time;   //!< time in seconds when event was registered
    const char*   File;   //!< file path

};

/**
 * Window event structure.
 * Defined as union to avoid memory fragmentation.
 */
union StEvent {

    StEventType   Type;    //!< event type
    StAnyEvent    Base;    //!< fields shared between all event
    StCloseEvent  Close;   //!< window close  event
    StSizeEvent   Size;    //!< window resize event
    StKeyEvent    Key;     //!< keyboard key down/up event
    StClickEvent  Button;  //!< mouse button down/up event
    StDNDropEvent DNDrop;  //!< file Drag & Drop event

};

#endif // __StEvent_h_
