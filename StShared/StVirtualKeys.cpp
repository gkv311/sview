/**
 * Copyright © 2013-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StCore/StVirtualKeys.h>

namespace {

    static const StCStringUtf8 THE_VKEYS_NAMES[] = {
        stCString(""),      // ST_VK_NULL
        stCString(""),      // ST_VK_x01
        stCString(""),      // ST_VK_x02
        stCString(""),      // ST_VK_x03
        stCString(""),      // ST_VK_x04
        stCString(""),      // ST_VK_x05
        stCString(""),      // ST_VK_x06
        stCString(""),      // ST_VK_x07
        stCString("Back"),  // ST_VK_BACK
        stCString("Tab"),   // ST_VK_TAB
        stCString(""),      // ST_VK_x0A
        stCString(""),      // ST_VK_x0B
        stCString("Clear"), // ST_VK_CLEAR
        stCString("Enter"), // ST_VK_RETURN
        stCString(""),      // ST_VK_x0E
        stCString(""),      // ST_VK_x0F
        stCString("Shift"), // ST_VK_SHIFT
        stCString("Ctrl"),  // ST_VK_CONTROL
        stCString("Alt"),   // ST_VK_MENU
        stCString("Pause"), // ST_VK_PAUSE
        stCString("Caps"),  // ST_VK_CAPITAL
        stCString(""),      // ST_VK_x15
        stCString(""),      // ST_VK_x16
        stCString(""),      // ST_VK_x17
        stCString(""),      // ST_VK_x18
        stCString(""),      // ST_VK_x19
        stCString(""),      // ST_VK_x1A
        stCString("Esc"),   // ST_VK_ESCAPE
        stCString(""),      // ST_VK_CONVERT
        stCString(""),      // ST_VK_NONCONVERT
        stCString(""),      // ST_VK_ACCEPT
        stCString(""),      // ST_VK_MODECHANGE
        stCString("Space"), // ST_VK_SPACE
        stCString("PgUp"),  // ST_VK_PRIOR
        stCString("PgDw"),  // ST_VK_NEXT
        stCString("End"),   // ST_VK_END
        stCString("Home"),  // ST_VK_HOME
        stCString("Left"),  // ST_VK_LEFT
        stCString("Up"),    // ST_VK_UP
        stCString("Right"), // ST_VK_RIGHT
        stCString("Down"),  // ST_VK_DOWN
        stCString(""),      // ST_VK_SELECT
        stCString("Print"), // ST_VK_PRINT
        stCString("Exec"),  // ST_VK_EXECUTEĶ
        stCString("Snapshot"),// ST_VK_SNAPSHOT
        stCString("Ins"),   // ST_VK_INSERT
        stCString("Del"),   // ST_VK_DELETE
        stCString("Help"),  // ST_VK_HELP
        stCString("0"),     // ST_VK_0
        stCString("1"),     // ST_VK_1
        stCString("2"),     // ST_VK_2
        stCString("3"),     // ST_VK_3
        stCString("4"),     // ST_VK_4
        stCString("5"),     // ST_VK_5
        stCString("6"),     // ST_VK_6
        stCString("7"),     // ST_VK_7
        stCString("8"),     // ST_VK_8
        stCString("9"),     // ST_VK_9
        stCString(""),      // ST_VK_x3A
        stCString(""),      // ST_VK_x3B
        stCString(""),      // ST_VK_x3C
        stCString(""),      // ST_VK_x3D
        stCString(""),      // ST_VK_x3E
        stCString(""),      // ST_VK_x3F
        stCString(""),      // ST_VK_x40
        stCString("A"),     // ST_VK_A
        stCString("B"),     // ST_VK_B
        stCString("C"),     // ST_VK_C
        stCString("D"),     // ST_VK_D
        stCString("E"),     // ST_VK_E
        stCString("F"),     // ST_VK_F
        stCString("G"),     // ST_VK_G
        stCString("H"),     // ST_VK_H
        stCString("I"),     // ST_VK_I
        stCString("J"),     // ST_VK_J
        stCString("K"),     // ST_VK_K
        stCString("L"),     // ST_VK_L
        stCString("M"),     // ST_VK_M
        stCString("N"),     // ST_VK_N
        stCString("O"),     // ST_VK_O
        stCString("P"),     // ST_VK_P
        stCString("Q"),     // ST_VK_Q
        stCString("R"),     // ST_VK_R
        stCString("S"),     // ST_VK_S
        stCString("T"),     // ST_VK_T
        stCString("U"),     // ST_VK_U
        stCString("V"),     // ST_VK_V
        stCString("W"),     // ST_VK_W
        stCString("X"),     // ST_VK_X
        stCString("Y"),     // ST_VK_Y
        stCString("Z"),     // ST_VK_Z
        stCString("LWin"),  // ST_VK_LWIN
        stCString("RWin"),  // ST_VK_RWIN
        stCString("Apps"),  // ST_VK_APPS
        stCString(""),      // ST_VK_x5E
        stCString("Sleep"), // ST_VK_SLEEP
        stCString("Num0"),  // ST_VK_NUMPAD0
        stCString("Num1"),  // ST_VK_NUMPAD1
        stCString("Num2"),  // ST_VK_NUMPAD2
        stCString("Num3"),  // ST_VK_NUMPAD3
        stCString("Num4"),  // ST_VK_NUMPAD4
        stCString("Num5"),  // ST_VK_NUMPAD5
        stCString("Num6"),  // ST_VK_NUMPAD6
        stCString("Num7"),  // ST_VK_NUMPAD7
        stCString("Num8"),  // ST_VK_NUMPAD8
        stCString("Num9"),  // ST_VK_NUMPAD9
        stCString("Num*"),  // ST_VK_MULTIPLY
        stCString("NumAdd"),// ST_VK_ADD
        stCString(""),      // ST_VK_SEPARATOR
        stCString("Num-"),  // ST_VK_SUBTRACT
        stCString(""),      // ST_VK_DECIMAL
        stCString("Num/"),  // ST_VK_DIVIDE
        stCString("F1"),    // ST_VK_F1
        stCString("F2"),    // ST_VK_F2
        stCString("F3"),    // ST_VK_F3
        stCString("F4"),    // ST_VK_F4
        stCString("F5"),    // ST_VK_F5
        stCString("F6"),    // ST_VK_F6
        stCString("F7"),    // ST_VK_F7
        stCString("F8"),    // ST_VK_F8
        stCString("F9"),    // ST_VK_F9
        stCString("F10"),   // ST_VK_F10
        stCString("F11"),   // ST_VK_F11
        stCString("F12"),   // ST_VK_F12
        stCString("F13"),   // ST_VK_F13
        stCString("F14"),   // ST_VK_F14
        stCString("F15"),   // ST_VK_F15
        stCString("F16"),   // ST_VK_F16
        stCString("F17"),   // ST_VK_F17
        stCString("F18"),   // ST_VK_F18
        stCString("F19"),   // ST_VK_F19
        stCString("F20"),   // ST_VK_F20
        stCString("F21"),   // ST_VK_F21
        stCString("F22"),   // ST_VK_F22
        stCString("F23"),   // ST_VK_F23
        stCString("F24"),   // ST_VK_F24
        stCString(""),      // ST_VK_x88
        stCString(""),      // ST_VK_x89
        stCString(""),      // ST_VK_x8A
        stCString(""),      // ST_VK_x8B
        stCString(""),      // ST_VK_x8C
        stCString(""),      // ST_VK_x8D
        stCString(""),      // ST_VK_x8E
        stCString(""),      // ST_VK_x8F
        stCString("NumLock"), // ST_VK_NUMLOCK
        stCString("Scroll"),// ST_VK_SCROLL
        stCString(""),      // ST_VK_x92
        stCString(""),      // ST_VK_x93
        stCString(""),      // ST_VK_x94
        stCString(""),      // ST_VK_x95
        stCString(""),      // ST_VK_x96
        stCString(""),      // ST_VK_x97
        stCString(""),      // ST_VK_x98
        stCString(""),      // ST_VK_x99
        stCString(""),      // ST_VK_x9A
        stCString(""),      // ST_VK_x9B
        stCString(""),      // ST_VK_x9C
        stCString(""),      // ST_VK_x9D
        stCString(""),      // ST_VK_x9E
        stCString(""),      // ST_VK_x9F
        stCString("LShift"),// ST_VK_LSHIFT
        stCString("RShift"),// ST_VK_RSHIFT
        stCString("LCrtl"), // ST_VK_LCONTROL
        stCString("RCtrl"), // ST_VK_RCONTROL
        stCString("LMenu"), // ST_VK_LMENU
        stCString("RMenu"), // ST_VK_RMENU
        stCString("BrowserBack"),     // ST_VK_BROWSER_BACK
        stCString("BrowserForw"),     // ST_VK_BROWSER_FORWARD
        stCString("BrowserRefresh"),  // ST_VK_BROWSER_REFRESH
        stCString("BrowserStop"),     // ST_VK_BROWSER_STOP
        stCString("BrowserSearch"),   // ST_VK_BROWSER_SEARCH
        stCString("BrowserFavorite"), // ST_VK_BROWSER_FAVORITES
        stCString("BrowserHome"),     // ST_VK_BROWSER_HOME
        stCString("VolMute"),         // ST_VK_VOLUME_MUTE
        stCString("VolDown"),         // ST_VK_VOLUME_DOWN
        stCString("VolUp"),           // ST_VK_VOLUME_UP
        stCString("MediaNext"),       // ST_VK_MEDIA_NEXT_TRACK
        stCString("MediaPrev"),       // ST_VK_MEDIA_PREV_TRACK
        stCString("MediaStop"),       // ST_VK_MEDIA_STOP
        stCString("MediaPlay"),       // ST_VK_MEDIA_PLAY_PAUSE
        stCString("Mail"),            // ST_VK_LAUNCH_MAIL
        stCString(""),      // ST_VK_LAUNCH_MEDIA_SELECT
        stCString(""),      // ST_VK_LAUNCH_APP1
        stCString(""),      // ST_VK_LAUNCH_APP2
        stCString(""),      // ST_VK_xB8
        stCString(""),      // ST_VK_xB9
        stCString(";"),     // ST_VK_SEMICOLON
        stCString("+"),     // ST_VK_OEM_PLUS
        stCString(","),     // ST_VK_COMMA
        stCString("-"),     // ST_VK_OEM_MINUS
        stCString("."),     // ST_VK_PERIOD
        stCString("/"),     // ST_VK_SLASH
        stCString("~"),     // ST_VK_TILDE
        stCString(""),      // 193
        stCString(""),      // 194
        stCString(""),      // 195
        stCString(""),      // 196
        stCString(""),      // 197
        stCString(""),      // 198
        stCString(""),      // 199
        stCString(""),      // 200
        stCString(""),      // 201
        stCString(""),      // 202
        stCString(""),      // 203
        stCString(""),      // 204
        stCString(""),      // 205
        stCString(""),      // 206
        stCString(""),      // 207
        stCString(""),      // 208
        stCString(""),      // 209
        stCString(""),      // 210
        stCString(""),      // 211
        stCString(""),      // 212
        stCString(""),      // 213
        stCString(""),      // 214
        stCString(""),      // 215
        stCString("Cmd"),   // 216, ST_VK_COMMAND
        stCString("Fn"),    // 217, ST_VK_FUNCTION
        stCString(""),      // 218
        stCString("["),     // ST_VK_BRACKETLEFT
        stCString("\\"),    // ST_VK_BACKSLASH
        stCString("]"),     // ST_VK_BRACKETRIGHT
        stCString("'"),     // ST_VK_APOSTROPHE
        stCString(""),      // 223
        stCString(""),      // 224
        stCString(""),      // 225
        stCString(""),      // 226
        stCString(""),      // 227
        stCString(""),      // 228
        stCString(""),      // 229
        stCString(""),      // 230
        stCString(""),      // 231
        stCString(""),      // 232
        stCString(""),      // 233
        stCString(""),      // 234
        stCString(""),      // 235
        stCString(""),      // 236
        stCString(""),      // 237
        stCString(""),      // 238
        stCString(""),      // 239
        stCString(""),      // 240
        stCString(""),      // 241
        stCString(""),      // 242
        stCString(""),      // 243
        stCString(""),      // 244
        stCString(""),      // 245
        stCString(""),      // 246
        stCString(""),      // 247
        stCString(""),      // 248
        stCString(""),      // 249
        stCString(""),      // 250
        stCString(""),      // 251
        stCString(""),      // 252
        stCString(""),      // 253
        stCString(""),      // 254
        stCString(""),      // 255
    };

};

const char* encodeVirtKey(const StVirtKey theKey) {
    if(theKey < 0 || theKey > 255) {
        return "";
    }
    return THE_VKEYS_NAMES[theKey].String;
}

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
    if(theKey & ST_VF_MENU) {
        aKey &= ~(ST_VF_MENU);
        if(aKey == ST_VK_MENU) {
            return THE_VKEYS_NAMES[aKey];
        }
        aStr += "Alt+";
    }
    if(theKey & ST_VF_COMMAND) {
        aKey &= ~(ST_VF_COMMAND);
        if(aKey == ST_VK_COMMAND) {
            return THE_VKEYS_NAMES[aKey];
        }
        aStr += "Cmd+";
    }
    if(theKey & ST_VF_FUNCTION) {
        aKey &= ~(ST_VF_FUNCTION);
        if(aKey == ST_VK_FUNCTION) {
            return THE_VKEYS_NAMES[aKey];
        }
        aStr += "Fn+";
    }
    if(aKey == 0
    || aKey >= ST_VK_NB) {
        return "";
    }
    if(aKey == ST_VK_SHIFT
    || aKey == ST_VK_CONTROL
    || aKey == ST_VK_MENU
    || aKey == ST_VK_COMMAND
    || aKey == ST_VK_FUNCTION) {
        return aStr;
    }
    return aStr + THE_VKEYS_NAMES[aKey];
}

unsigned int decodeHotKey(const StString& theString) {
    unsigned int aKey = 0;
    if(theString.isEmpty()) {
        return aKey;
    }

    // decode flags (split by + separator)
    StUtf8Iter aFrom  = theString.iterator();
    StUtf8Iter anIter = theString.iterator();
    for(; *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t('+')) {
            if(anIter.getIndex() == 0) {
                return ST_VK_OEM_PLUS; // single '+'
            }

            const StCStringUtf8 aSubStr = {
                aFrom.getBufferHere(),
                size_t(anIter.getBufferHere() - aFrom.getBufferHere()),
                       anIter.getIndex()      - aFrom.getIndex()
            };

            if(aSubStr.isEquals(THE_VKEYS_NAMES[ST_VK_SHIFT])) {
                aKey |= ST_VF_SHIFT;
            } else if(aSubStr.isEquals(THE_VKEYS_NAMES[ST_VK_CONTROL])) {
                aKey |= ST_VF_CONTROL;
            } else if(aSubStr.isEquals(THE_VKEYS_NAMES[ST_VK_MENU])) {
                aKey |= ST_VF_MENU;
            } else if(aSubStr.isEquals(THE_VKEYS_NAMES[ST_VK_COMMAND])) {
                aKey |= ST_VF_COMMAND;
            } else if(aSubStr.isEquals(THE_VKEYS_NAMES[ST_VK_FUNCTION])) {
                aKey |= ST_VF_FUNCTION;
            }

            aFrom = anIter;
            ++aFrom;
        }
    }

    // decode VKey itself
    const StCStringUtf8 aSubStr = {
        aFrom.getBufferHere(),
        size_t(anIter.getBufferHere() - aFrom.getBufferHere()),
               anIter.getIndex()      - aFrom.getIndex()
    };

    if(aSubStr.Size == 1) {
        // optimized code for letters
        if(*aSubStr.String >= 'A'
        && *aSubStr.String <= 'Z') {
            aKey |= (unsigned int )*aSubStr.String;
            return aKey;
        } else if(*aSubStr.String >= '0'
               && *aSubStr.String <= '9') {
            aKey |= (unsigned int )*aSubStr.String;
            return aKey;
        }
    }
    for(unsigned int aKeyIter = 0; aKeyIter <= 223; ++aKeyIter) {
        if(aSubStr.isEquals(THE_VKEYS_NAMES[aKeyIter])) {
            aKey |= aKeyIter;
            break;
        }
    }

    return aKey;
}
