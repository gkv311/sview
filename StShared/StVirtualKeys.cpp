/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StVirtualKeys.h>

namespace {

    static const char* THE_VKEYS_NAMES[] = {
        "",      // ST_VK_NULL
        "",      // ST_VK_x01
        "",      // ST_VK_x02
        "",      // ST_VK_x03
        "",      // ST_VK_x04
        "",      // ST_VK_x05
        "",      // ST_VK_x06
        "",      // ST_VK_x07
        "Back",  // ST_VK_BACK
        "Tab",   // ST_VK_TAB
        "",      // ST_VK_x0A
        "",      // ST_VK_x0B
        "Clear", // ST_VK_CLEAR
        "Enter", // ST_VK_RETURN
        "",      // ST_VK_x0E
        "",      // ST_VK_x0F
        "Shift", // ST_VK_SHIFT
        "Ctrl",  // ST_VK_CONTROL
        "Menu",  // ST_VK_MENU
        "Pause", // ST_VK_PAUSE
        "Caps",  // ST_VK_CAPITAL
        "",      // ST_VK_x15
        "",      // ST_VK_x16
        "",      // ST_VK_x17
        "",      // ST_VK_x18
        "",      // ST_VK_x19
        "",      // ST_VK_x1A
        "Esc",   // ST_VK_ESCAPE
        "",      // ST_VK_CONVERT
        "",      // ST_VK_NONCONVERT
        "",      // ST_VK_ACCEPT
        "",      // ST_VK_MODECHANGE
        "Space", // ST_VK_SPACE
        "PgUp",  // ST_VK_PRIOR
        "PgDw",  // ST_VK_NEXT
        "End",   // ST_VK_END
        "Home",  // ST_VK_HOME
        "Left",  // ST_VK_LEFT
        "Up",    // ST_VK_UP
        "Right", // ST_VK_RIGHT
        "Down",  // ST_VK_DOWN
        "",      // ST_VK_SELECT
        "Print", // ST_VK_PRINT
        "Exec",  // ST_VK_EXECUTEĶ
        "Snapshot",// ST_VK_SNAPSHOT
        "Ins",   // ST_VK_INSERT
        "Del",   // ST_VK_DELETE
        "Help",  // ST_VK_HELP
        "0",     // ST_VK_0
        "1",     // ST_VK_1
        "2",     // ST_VK_2
        "3",     // ST_VK_3
        "4",     // ST_VK_4
        "5",     // ST_VK_5
        "6",     // ST_VK_6
        "7",     // ST_VK_7
        "8",     // ST_VK_8
        "9",     // ST_VK_9
        "",      // ST_VK_x3A
        "",      // ST_VK_x3B
        "",      // ST_VK_x3C
        "",      // ST_VK_x3D
        "",      // ST_VK_x3E
        "",      // ST_VK_x3F
        "",      // ST_VK_x40
        "A",     // ST_VK_A
        "B",     // ST_VK_B
        "C",     // ST_VK_C
        "D",     // ST_VK_D
        "E",     // ST_VK_E
        "F",     // ST_VK_F
        "G",     // ST_VK_G
        "H",     // ST_VK_H
        "I",     // ST_VK_I
        "J",     // ST_VK_J
        "K",     // ST_VK_K
        "L",     // ST_VK_L
        "M",     // ST_VK_M
        "N",     // ST_VK_N
        "O",     // ST_VK_O
        "P",     // ST_VK_P
        "Q",     // ST_VK_Q
        "R",     // ST_VK_R
        "S",     // ST_VK_S
        "T",     // ST_VK_T
        "U",     // ST_VK_U
        "V",     // ST_VK_V
        "W",     // ST_VK_W
        "X",     // ST_VK_X
        "Y",     // ST_VK_Y
        "Z",     // ST_VK_Z
        "LWin",  // ST_VK_LWIN
        "RWin",  // ST_VK_RWIN
        "Apps",  // ST_VK_APPS
        "",      // ST_VK_x5E
        "Sleep", // ST_VK_SLEEP
        "Num0",  // ST_VK_NUMPAD0
        "Num1",  // ST_VK_NUMPAD1
        "Num2",  // ST_VK_NUMPAD2
        "Num3",  // ST_VK_NUMPAD3
        "Num4",  // ST_VK_NUMPAD4
        "Num5",  // ST_VK_NUMPAD5
        "Num6",  // ST_VK_NUMPAD6
        "Num7",  // ST_VK_NUMPAD7
        "Num8",  // ST_VK_NUMPAD8
        "Num9",  // ST_VK_NUMPAD9
        "Num*",  // ST_VK_MULTIPLY
        "Num+",  // ST_VK_ADD
        "",      // ST_VK_SEPARATOR
        "Num-",  // ST_VK_SUBTRACT
        "",      // ST_VK_DECIMAL
        "Num/",  // ST_VK_DIVIDE
        "F1",    // ST_VK_F1
        "F2",    // ST_VK_F2
        "F3",    // ST_VK_F3
        "F4",    // ST_VK_F4
        "F5",    // ST_VK_F5
        "F6",    // ST_VK_F6
        "F7",    // ST_VK_F7
        "F8",    // ST_VK_F8
        "F9",    // ST_VK_F9
        "F10",   // ST_VK_F10
        "F11",   // ST_VK_F11
        "F12",   // ST_VK_F12
        "F13",   // ST_VK_F13
        "F14",   // ST_VK_F14
        "F15",   // ST_VK_F15
        "F16",   // ST_VK_F16
        "F17",   // ST_VK_F17
        "F18",   // ST_VK_F18
        "F19",   // ST_VK_F19
        "F20",   // ST_VK_F20
        "F21",   // ST_VK_F21
        "F22",   // ST_VK_F22
        "F23",   // ST_VK_F23
        "F24",   // ST_VK_F24
        "",      // ST_VK_x88
        "",      // ST_VK_x89
        "",      // ST_VK_x8A
        "",      // ST_VK_x8B
        "",      // ST_VK_x8C
        "",      // ST_VK_x8D
        "",      // ST_VK_x8E
        "",      // ST_VK_x8F
        "NumLock", // ST_VK_NUMLOCK
        "Scroll",// ST_VK_SCROLL
        "",      // ST_VK_x92
        "",      // ST_VK_x93
        "",      // ST_VK_x94
        "",      // ST_VK_x95
        "",      // ST_VK_x96
        "",      // ST_VK_x97
        "",      // ST_VK_x98
        "",      // ST_VK_x99
        "",      // ST_VK_x9A
        "",      // ST_VK_x9B
        "",      // ST_VK_x9C
        "",      // ST_VK_x9D
        "",      // ST_VK_x9E
        "",      // ST_VK_x9F
        "LShift",// ST_VK_LSHIFT
        "RShift",// ST_VK_RSHIFT
        "LCrtl", // ST_VK_LCONTROL
        "RCtrl", // ST_VK_RCONTROL
        "LMenu", // ST_VK_LMENU
        "RMenu", // ST_VK_RMENU
        "BrowserBack",     // ST_VK_BROWSER_BACK
        "BrowserForw",     // ST_VK_BROWSER_FORWARD
        "BrowserRefresh",  // ST_VK_BROWSER_REFRESH
        "BrowserStop",     // ST_VK_BROWSER_STOP
        "BrowserSearch",   // ST_VK_BROWSER_SEARCH
        "BrowserFavorite", // ST_VK_BROWSER_FAVORITES
        "BrowserHome",     // ST_VK_BROWSER_HOME
        "VolMute",         // ST_VK_VOLUME_MUTE
        "VolDown",         // ST_VK_VOLUME_DOWN
        "VolUp",           // ST_VK_VOLUME_UP
        "MediaNext",       // ST_VK_MEDIA_NEXT_TRACK
        "MediaPrev",       // ST_VK_MEDIA_PREV_TRACK
        "MediaStop",       // ST_VK_MEDIA_STOP
        "MediaPlay",       // ST_VK_MEDIA_PLAY_PAUSE
        "Mail",            // ST_VK_LAUNCH_MAIL
        "",      // ST_VK_LAUNCH_MEDIA_SELECT
        "",      // ST_VK_LAUNCH_APP1
        "",      // ST_VK_LAUNCH_APP2
        "",      // ST_VK_xB8
        "",      // ST_VK_xB9
        ";",     // ST_VK_SEMICOLON
        "+",     // ST_VK_OEM_PLUS
        ",",     // ST_VK_COMMA
        "-",     // ST_VK_OEM_MINUS
        ".",     // ST_VK_PERIOD
        "",      // ST_VK_OEM_2
        "",      // ST_VK_OEM_3
        "",      // 193
        "",      // 194
        "",      // 195
        "",      // 196
        "",      // 197
        "",      // 198
        "",      // 199
        "",      // 200
        "",      // 201
        "",      // 202
        "",      // 203
        "",      // 204
        "",      // 205
        "",      // 206
        "",      // 207
        "",      // 208
        "",      // 209
        "",      // 210
        "",      // 211
        "",      // 212
        "",      // 213
        "",      // 214
        "",      // 215
        "",      // 216
        "",      // 217
        "",      // 218
        "[",     // ST_VK_BRACKETLEFT
        "\\",    // ST_VK_BACKSLASH
        "]",     // ST_VK_BRACKETRIGHT
        "'",     // ST_VK_APOSTROPHE
    };

};

StString encodeHotKey(const unsigned int theKey) {
    StString aStr;
    unsigned int aKey = theKey;
    if(theKey & ST_VF_SHIFT) {
        aKey &= ~(ST_VF_SHIFT);
        if(aKey == ST_VK_SHIFT) {
            return THE_VKEYS_NAMES[aKey];
        }
        aStr += "Shift+";
    }
    if(theKey & ST_VF_CONTROL) {
        aKey &= ~(ST_VF_CONTROL);
        if(aKey == ST_VK_CONTROL) {
            return THE_VKEYS_NAMES[aKey];
        }
        aStr += "Ctrl+";
    }
    if(aKey == 0
    || aKey > 255) {
        return "";
    }
    if(aKey == ST_VK_SHIFT
    || aKey == ST_VK_CONTROL) {
        return aStr;
    }
    return aStr + THE_VKEYS_NAMES[aKey];
}

const char* encodeVirtKey(const StVirtKey theKey) {
    if(theKey < 0 || theKey > 255) {
        return "";
    }
    return THE_VKEYS_NAMES[theKey];
}
