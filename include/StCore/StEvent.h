/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StEvent_h_
#define __StEvent_h_

#include <StTemplates/StVec2.h>
#include <StThreads/StMutex.h>
#include <StThreads/StTimer.h>
#include "StVirtualKeys.h"

#define ST_MAX_TOUCHES 10
#define ST_TOUCH_INVALID_ID (size_t(-1))

/**
 * Touch state.
 */
struct StTouch {

    size_t Id;       //!< unique touch id
    size_t DeviceId; //!< device id
    float  PointX;   //!< touch position defined relative to window from top-left (0,0) to the bottom-right (1,1)
    float  PointY;   //!< touchpad coordinates are mapped to the window as well, but has no direct meaning
    bool   OnScreen; //!< flag indicating touchscreen (as alternative to touchpad)

    /**
     * Return true for defined touch.
     */
    bool isDefined() const {
        return Id != ST_TOUCH_INVALID_ID;
    }

    /**
     * Return definition of empty touch.
     */
    static StTouch Empty() {
        StTouch aTouch;
        aTouch.Id       = ST_TOUCH_INVALID_ID;
        aTouch.DeviceId = ST_TOUCH_INVALID_ID;
        aTouch.PointX   = 0.0f;
        aTouch.PointY   = 0.0f;
        aTouch.OnScreen = false;
        return aTouch;
    }

};

/**
 * Event type.
 */
enum StEventType {
    stEvent_None,       //!< StAnyEvent,    undefined event
    stEvent_Close,      //!< StCloseEvent,  window close requested
    stEvent_Pause,      //!< StPauseEvent,  window can be closed at any moment
    stEvent_Size,       //!< StSizeEvent,   window resized
    stEvent_NewMonitor, //!< StSizeEvent,   window moved to another monitor
    stEvent_KeyDown,    //!< StKeyEvent,    keyboard key pressed
    stEvent_KeyUp,      //!< StKeyEvent,    keyboard key released
    stEvent_KeyHold,    //!< StKeyEvent,    keyboard key holded
    stEvent_MouseDown,  //!< StClickEvent,  mouse button pressed
    stEvent_MouseUp,    //!< StClickEvent,  mouse button released
    stEvent_MouseCancel,//!< StClickEvent,  cancel simulated mouse button press event
    stEvent_TouchDown,  //!< StTouchEvent,  touch pressed
    stEvent_TouchUp,    //!< StTouchEvent,  touch released
    stEvent_TouchMove,  //!< StTouchEvent,  touch moved
    stEvent_TouchCancel,//!< StTouchEvent,  touch cancelled
    stEvent_GestureCancel,     //!< StGestureEvent, abort the gestures
    stEvent_Gesture1Tap,       //!< StGestureEvent, 1 finger  single tap
    stEvent_Gesture1DoubleTap, //!< StGestureEvent, 1 finger  double tap
    stEvent_Gesture2Move,      //!< StGestureEvent, 2 fingers moving in sync
    stEvent_Gesture2Rotate,    //!< StGestureEvent, 2 fingers rotating
    stEvent_Gesture2Pinch,     //!< StGestureEvent, 2 fingers pinch
    stEvent_Scroll,     //!< StScrollEvent, scrolling
    stEvent_FileDrop,   //!< StDNDropEvent, file Drag & Drop
    stEvent_Navigate,   //!< StNavigEvent,  navigation event
    stEvent_Action,     //!< StActionEvent, queued application event
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
 * Pause window request.
 */
struct StPauseEvent {

    StEventType   Type;   //!< event type
    double        Time;   //!< time in seconds when event was registered

};

/**
 * Window resize event.
 */
struct StSizeEvent {

    StEventType   Type;    //!< event type
    double        Time;    //!< time in seconds when event was registered
    unsigned int  SizeX;   //!< new window width  in pixels
    unsigned int  SizeY;   //!< new window height in pixels
    double        Aspect;  //!< new window aspect ratio (width / height)

    /**
     * Initialize size event.
     */
    void init(double theTime,
              unsigned int theSizeX,
              unsigned int theSizeY,
              double theForcedAspect) {
        Type  = stEvent_Size;
        Time  = theTime;
        SizeX = theSizeX;
        SizeY = theSizeY;
        if(theForcedAspect > 0.0) {
            Aspect = theForcedAspect;
        } else {
            Aspect = double(theSizeX != 0 ? theSizeX : 1) / double(theSizeY != 0 ? theSizeY : 1);
        }
    }

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
 * Scroll event.
 */
struct StScrollEvent {

    StEventType   Type;             //!< event type
    double        Time;             //!< time in seconds when event was registered
    double        PointX;           //!< mouse cursor point defined relative to window from top-left (0,0) to the bottom-right (1,1)
    double        PointY;
    int           StepsX;           //!< discrete steps for horizontal scroll
    int           StepsY;           //!< discrete steps for vertical   scroll
    float         DeltaX;           //!< precise  delta for horizontal scroll
    float         DeltaY;           //!< precise  delta for vertical   scroll
    bool          IsFromMultiTouch; //!< when true, scrolling is simulated from multi-touch gesture by system (OS X) and touches will come in parallel

    /**
     * Reset event.
     */
    void reset() {
        Type  = stEvent_Scroll;
        Time  = 0.0;
        PointX = 0.0;
        PointY = 0.0;
        StepsX = 0;
        StepsY = 0;
        DeltaX = 0.0f;
        DeltaY = 0.0f;
        IsFromMultiTouch = false;
    }

    /**
     * Initialize event.
     */
    void init(double theTime,
              double thePointX,
              double thePointY,
              float  theDeltaX,
              float  theDeltaY,
              bool   theIsFromMultiTouch) {
        Type  = stEvent_Scroll;
        Time  = theTime;
        PointX = thePointX;
        PointY = thePointY;
        StepsX = 0;
        StepsY = 0;
        DeltaX = theDeltaX;
        DeltaY = theDeltaY;
        IsFromMultiTouch = theIsFromMultiTouch;
    }

    /**
     * Compute accumulated integer steps from X scroll event.
     */
    int accumulateStepsX(int theInc, int theStepSize) { return accumulateSteps(StepsX, theInc, theStepSize); }

    /**
     * Compute accumulated integer steps from Y scroll event.
     */
    int accumulateStepsY(int theInc, int theStepSize) { return accumulateSteps(StepsY, theInc, theStepSize); }

    /**
     * Compute accumulated integer steps from scroll event.
     */
    static int accumulateSteps(int& theAcc, int theInc, int theStepSize) {
        theAcc += theInc;
        int aNbSteps = 0;
        for(; theAcc <= -theStepSize; theAcc += theStepSize) {
            --aNbSteps;
        }
        for(; theAcc >= theStepSize; theAcc -= theStepSize) {
            ++aNbSteps;
        }
        return aNbSteps;
    }
};

/**
 * Touch event.
 */
struct StTouchEvent {

    StEventType   Type;      //!< event type
    double        Time;      //!< time in seconds when event was registered
    int           NbTouches; //!< active number of touches (including currently pressing/releasing)
    StTouch       Touches[ST_MAX_TOUCHES];

    /**
     * Find the touch with specified id.
     */
    StTouch findTouchById(size_t theId) const {
        if(theId == ST_TOUCH_INVALID_ID) {
            return StTouch::Empty();
        }

        for(int aTouchIter = 0; aTouchIter < NbTouches; ++aTouchIter) {
            if(Touches[aTouchIter].Id == theId) {
                return Touches[aTouchIter];
            }
        }
        return StTouch::Empty();
    }

    /**
     * Add new touch only if not already exists.
     */
    bool addTouch(const StTouch& theTouch) {
        if(theTouch.Id == ST_TOUCH_INVALID_ID) {
            return false;
        }

        for(int aTouchIter = 0; aTouchIter < NbTouches; ++aTouchIter) {
            if(Touches[aTouchIter].Id == theTouch.Id) {
                return false;
            }
        }
        if(NbTouches >= ST_MAX_TOUCHES) {
            return false;
        }
        Touches[NbTouches++] = theTouch;
        return true;
    }

    /**
     * Reset the touch list.
     */
    void clearTouches() {
        NbTouches = 0;
        for(int aTouchIter = 0; aTouchIter < ST_MAX_TOUCHES; ++aTouchIter) {
            Touches[aTouchIter] = StTouch::Empty();
        }
    }

};

/**
 * Gesture event.
 */
struct StGestureEvent {

    StEventType   Type;      //!< event type
    double        Time;      //!< time in seconds when event was registered
    float         Point1X;   //!< anchor point 1
    float         Point1Y;
    float         Point2X;   //!< anchor point 2
    float         Point2Y;
    float         Value;     //!< gesture value
    bool          OnScreen;  //!< gesture comes from touchscreen

    /**
     * Reset the gesture.
     */
    void clearGesture() {
        Point1X  = 0.0f;
        Point1Y  = 0.0f;
        Point2X  = 0.0f;
        Point2Y  = 0.0f;
        Value    = 0.0f;
        OnScreen = false;
    }

};

/**
 * File Drag & Drop event.
 */
struct StDNDropEvent {

    StEventType   Type;   //!< event type
    double        Time;   //!< time in seconds when event was registered
    const char**  Files;  //!< file paths
    uint32_t      NbFiles;//!< number of files

};

enum StNavigValues {
    stNavigate_Top,
    stNavigate_Bottom,
    stNavigate_Backward,
    stNavigate_Forward,
};

/**
 * Navigation event.
 */
struct StNavigEvent {

    StEventType   Type;   //!< event type
    double        Time;   //!< time in seconds when event was registered
    StNavigValues Target; //!< navigation target

};

/**
 * Queued application action event.
 */
struct StActionEvent {

    StEventType   Type;     //!< event type
    double        Time;     //!< time in seconds when event was registered
    int           ActionId; //!< action unique identifier
    double        Progress; //!< time in seconds, optional

};

/**
 * Window event structure.
 * Defined as union to avoid memory fragmentation.
 */
union StEvent {

    StEventType    Type;     //!< event type
    StAnyEvent     Base;     //!< fields shared between all event
    StCloseEvent   Close;    //!< window close  event
    StPauseEvent   Pause;    //!< window pause  event
    StSizeEvent    Size;     //!< window resize event
    StKeyEvent     Key;      //!< keyboard key down/up event
    StClickEvent   Button;   //!< mouse button down/up event
    StTouchEvent   Touch;    //!< multi-touch event
    StGestureEvent Gesture;  //!< gesture event
    StScrollEvent  Scroll;   //!< scrolling event
    StDNDropEvent  DNDrop;   //!< file Drag & Drop event
    StNavigEvent   Navigate; //!< navigation event
    StActionEvent  Action;   //!< queued application action event

};

#endif // __StEvent_h_
