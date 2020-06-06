/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StVirtualKey_h_
#define __StVirtualKey_h_

#include <StStrings/StString.h>

/**
 * Mouse button.
 */
enum StVirtButton {
    ST_NOMOUSE = 0,
    ST_MOUSE_LEFT = 1,
    ST_MOUSE_RIGHT = 2,
    ST_MOUSE_MIDDLE = 3,
    ST_MOUSE_X1, // win
    ST_MOUSE_X2,
    ST_MOUSE_MAX_ID = ST_MOUSE_X2,
};

/**
 * Key modifier.
 */
enum StVirtFlags {
    ST_VF_NONE     = 0,
    // we reserve first 8 bits to combine value with StVirtKey
    ST_VF_SHIFT    = 1 <<  8, // ST_VK_SHIFT
    ST_VF_CONTROL  = 1 <<  9, // ST_VK_CONTROL
    ST_VF_MENU     = 1 << 10, // ST_VK_MENU
    ST_VF_COMMAND  = 1 << 11, // ST_VK_COMMAND
    ST_VF_FUNCTION = 1 << 12, // ST_VK_FUNCTION
};

/**
 * This is VIRTUAL keys' codes definitions in 8-255 range.
 * This map useful for 'game-style' or short-cuts keyboard events lookup:
 * language independent, modifiers independent (SHIFT, CAPS, etc).
 * Do NOT use this for TEXT input!
 */
enum StVirtKey {

    ST_VK_NULL       = 0x00, // 000 = VK unassigned  | ASCII 'NUL'

    // unassigned 0x01-0x07
    ST_VK_x01        = 0x01, // 001 = VK_LBUTTON     | ASCII 'SOH'(start of heading)
    ST_VK_x02        = 0x02, // 002 = VK_RBUTTON     | ASCII 'STX'(start of text)
    ST_VK_x03        = 0x03, // 003 = VK_CANCEL      | ASCII 'ETX'(end of text )
    ST_VK_x04        = 0x04, // 004 = VK_MBUTTON     | ASCII 'EOT'(end of transmission)
    ST_VK_x05        = 0x05, // 005 = VK_XBUTTON1    | ASCII 'ENQ'(enquiry)
    ST_VK_x06        = 0x06, // 006 = VK_XBUTTON2    | ASCII 'ACK'(acknowledge)
    ST_VK_x07        = 0x07, // 007 = VK unassigned  | ASCII 'BEL'(bell)

    ST_VK_BACK       = 0x08, // 008 = VK_BACK        | ASCII 'BS'(backspace)
    ST_VK_TAB        = 0x09, // 009 = VK_TAB         | ASCII 'HT'(horizontal tab)

    // unassigned 0x0A-0x0B
    ST_VK_x0A        = 0x0A, // 010 = VK unassigned  | ASCII 'LF'(line feed or 'NL' = new line
    ST_VK_x0B        = 0x0B, // 011 = VK unassigned  | ASCII 'VT'(vertical tab)

    ST_VK_CLEAR      = 0x0C, // 012 = VK_CLEAR       | ???
    ST_VK_RETURN     = 0x0D, // 013 = VK_RETURN      | ASCII 'CR'(carriage return) | MAIN (not NUM ?) enter key

    // unassigned 0x0E-0x0F
    ST_VK_x0E        = 0x0E, // 014 = VK unassigned  | ASCII 'SO'(shift out)
    ST_VK_x0F        = 0x0F, // 015 = VK unassigned  | ASCII 'SI'(shift in)

    ST_VK_SHIFT      = 0x10, // 016 = VK_SHIFT       | 'Shift' modifier key
    ST_VK_CONTROL    = 0x11, // 017 = VK_CONTROL     | 'Ctrl' modifier key
    ST_VK_MENU       = 0x12, // 018 = VK_MENU        | 'Alt' modifier (Menu) key
    ST_VK_PAUSE      = 0x13, // 019 = VK_PAUSE       | Pause/break key
    ST_VK_CAPITAL    = 0x14, // 020 = VK_CAPITAL     | 'Caps Lock' modifier key

    // obsolete 0x15-0x1A
    ST_VK_x15        = 0x15, // 021 = VK_KANA
    ST_VK_x16        = 0x16, // 022 = VK_HANGEUL
    ST_VK_x17        = 0x17, // 023 = VK_HANGUL
    ST_VK_x18        = 0x18, // 024 = VK_JUNJA
    ST_VK_x19        = 0x19, // 025 = VK_HANJA
    ST_VK_x1A        = 0x1A, // 026 = VK_KANJI

    ST_VK_ESCAPE     = 0x1B, // 027 = VK_ESCAPE      | ASCII 'ESC'(escape)

    // ??? 0x1C-0x1F
    ST_VK_CONVERT    = 0x1C, // 028 = VK_CONVERT     | ???
    ST_VK_NONCONVERT = 0x1D, // 029 = VK_NONCONVERT  | ???
    ST_VK_ACCEPT     = 0x1E, // 030 = VK_ACCEPT      | ???
    ST_VK_MODECHANGE = 0x1F, // 031 = VK_MODECHANGE  | ???

    ST_VK_SPACE      = 0x20, // 032 = VK_SPACE       | ASCII 'SP'(space)

    ST_VK_PRIOR      = 0x21, // 033 = VK_PRIOR       | Page Up
    ST_VK_PAGE_UP    = 0x21, // synonym
    ST_VK_PAGE_PRIOR = 0x21, // synonym
    ST_VK_NEXT       = 0x22, // 034 = VK_NEXT        | Page Down
    ST_VK_PAGE_DW    = 0x22, // synonym
    ST_VK_PAGE_DOWN  = 0x22, // synonym
    ST_VK_PAGE_NEXT  = 0x22, // synonym
    ST_VK_END        = 0x23, // 035 = VK_END         | End (last page)
    ST_VK_PAGE_LAST  = 0x23, // synonym
    ST_VK_HOME       = 0x24, // 036 = VK_HOME        | Home (first page)
    ST_VK_PAGE_FIRST = 0x24, // synonym
    ST_VK_LEFT       = 0x25, // 037 = VK_LEFT        | left
    ST_VK_UP         = 0x26, // 038 = VK_UP          | up
    ST_VK_RIGHT      = 0x27, // 039 = VK_RIGHT       | right
    ST_VK_DOWN       = 0x28, // 040 = VK_DOWN        | down

    // ???
    ST_VK_SELECT     = 0x29, // 041 = VK_SELECT      | ???
    ST_VK_PRINT      = 0x2A, // 042 = VK_PRINT       | ???
    ST_VK_EXECUTE    = 0x2B, // 043 = VK_EXECUTE     | ???
    ST_VK_SNAPSHOT   = 0x2C, // 044 = VK_SNAPSHOT    | ???

    ST_VK_INSERT     = 0x2D, // 045 = VK_INSERT      | insert key
    ST_VK_DELETE     = 0x2E, // 046 = VK_DELETE      | delete key

    // obsolete ???
    ST_VK_HELP       = 0x2F, // 047 = VK_HELP        | help key

    // numeric keys
    ST_VK_0          = 0x30, // 048 = VK_0           | ASCII '0'
    ST_VK_1          = 0x31, // 049 = VK_1           | ASCII '1'
    ST_VK_2          = 0x32, // 050 = VK_2           | ASCII '2'
    ST_VK_3          = 0x33, // 051 = VK_3           | ASCII '3'
    ST_VK_4          = 0x34, // 052 = VK_4           | ASCII '4'
    ST_VK_5          = 0x35, // 053 = VK_5           | ASCII '5'
    ST_VK_6          = 0x36, // 054 = VK_6           | ASCII '6'
    ST_VK_7          = 0x37, // 055 = VK_7           | ASCII '7'
    ST_VK_8          = 0x38, // 056 = VK_8           | ASCII '8'
    ST_VK_9          = 0x39, // 057 = VK_9           | ASCII '9'

    // ???
    ST_VK_x3A        = 0x3A, // 058 = VK unassigned  | ASCII ':'
    ST_VK_x3B        = 0x3B, // 059 = VK unassigned  | ASCII ';'
    ST_VK_x3C        = 0x3C, // 060 = VK unassigned  | ASCII '<'
    ST_VK_x3D        = 0x3D, // 061 = VK unassigned  | ASCII '='
    ST_VK_x3E        = 0x3E, // 062 = VK unassigned  | ASCII '>'
    ST_VK_x3F        = 0x3F, // 063 = VK unassigned  | ASCII '?'
    ST_VK_x40        = 0x40, // 064 = VK unassigned  | ASCII '@'

    // main latin alphabet keys
    ST_VK_A          = 0x41, // 065 = VK_A           | ASCII 'A'
    ST_VK_B          = 0x42, // 066 = VK_B           | ASCII 'B'
    ST_VK_C          = 0x43, // 067 = VK_C           | ASCII 'C'
    ST_VK_D          = 0x44, // 068 = VK_D           | ASCII 'D'
    ST_VK_E          = 0x45, // 069 = VK_E           | ASCII 'E'
    ST_VK_F          = 0x46, // 070 = VK_F           | ASCII 'F'
    ST_VK_G          = 0x47, // 071 = VK_G           | ASCII 'G'
    ST_VK_H          = 0x48, // 072 = VK_H           | ASCII 'H'
    ST_VK_I          = 0x49, // 073 = VK_I           | ASCII 'I'
    ST_VK_J          = 0x4A, // 074 = VK_J           | ASCII 'J'
    ST_VK_K          = 0x4B, // 075 = VK_K           | ASCII 'K'
    ST_VK_L          = 0x4C, // 076 = VK_L           | ASCII 'L'
    ST_VK_M          = 0x4D, // 077 = VK_M           | ASCII 'M'
    ST_VK_N          = 0x4E, // 078 = VK_N           | ASCII 'N'
    ST_VK_O          = 0x4F, // 079 = VK_O           | ASCII 'O'
    ST_VK_P          = 0x50, // 080 = VK_P           | ASCII 'P'
    ST_VK_Q          = 0x51, // 081 = VK_Q           | ASCII 'Q'
    ST_VK_R          = 0x52, // 082 = VK_R           | ASCII 'R'
    ST_VK_S          = 0x53, // 083 = VK_S           | ASCII 'S'
    ST_VK_T          = 0x54, // 084 = VK_T           | ASCII 'T'
    ST_VK_U          = 0x55, // 085 = VK_U           | ASCII 'U'
    ST_VK_V          = 0x56, // 086 = VK_V           | ASCII 'V'
    ST_VK_W          = 0x57, // 087 = VK_W           | ASCII 'W'
    ST_VK_X          = 0x58, // 088 = VK_X           | ASCII 'X'
    ST_VK_Y          = 0x59, // 089 = VK_Y           | ASCII 'Y'
    ST_VK_Z          = 0x5A, // 090 = VK_Z           | ASCII 'Z'

    // ???
    ST_VK_LWIN       = 0x5B, // 091 = VK_LWIN        | left Windows key
    ST_VK_RWIN       = 0x5C, // 092 = VK_RWIN        | right Windows key
    ST_VK_APPS       = 0x5D, // 093 = VK_APPS        | ???
    ST_VK_x5E        = 0x5E, // 094 = VK unassigned  |
    ST_VK_SLEEP      = 0x5F, // 095 = VK_SLEEP       | ???

    // numpad keys
    ST_VK_NUMPAD0    = 0x60, // 096 = VK_NUMPAD0     | numpad '0'
    ST_VK_NUMPAD1    = 0x61, // 097 = VK_NUMPAD1     | numpad '1'
    ST_VK_NUMPAD2    = 0x62, // 098 = VK_NUMPAD2     | numpad '2'
    ST_VK_NUMPAD3    = 0x63, // 099 = VK_NUMPAD3     | numpad '3'
    ST_VK_NUMPAD4    = 0x64, // 100 = VK_NUMPAD4     | numpad '4'
    ST_VK_NUMPAD5    = 0x65, // 101 = VK_NUMPAD5     | numpad '5'
    ST_VK_NUMPAD6    = 0x66, // 102 = VK_NUMPAD6     | numpad '6'
    ST_VK_NUMPAD7    = 0x67, // 103 = VK_NUMPAD7     | numpad '7'
    ST_VK_NUMPAD8    = 0x68, // 104 = VK_NUMPAD8     | numpad '8'
    ST_VK_NUMPAD9    = 0x69, // 105 = VK_NUMPAD9     | numpad '9'
    ST_VK_MULTIPLY   = 0x6A, // 106 = VK_MULTIPLY    | numpad '*'
    ST_VK_ADD        = 0x6B, // 107 = VK_ADD         | numpad '+'
    ST_VK_SEPARATOR  = 0x6C, // 108 = VK_SEPARATOR   | ???
    ST_VK_SUBTRACT   = 0x6D, // 109 = VK_SUBTRACT    | numpad '-'
    ST_VK_DECIMAL    = 0x6E, // 110 = VK_DECIMAL     | ???
    ST_VK_DIVIDE     = 0x6F, // 111 = VK_DIVIDE      | numpad '/'

    // special keys
    ST_VK_F1         = 0x70, // 112 = VK_F1          | 'F1'
    ST_VK_F2         = 0x71, // 113 = VK_F2          | 'F2'
    ST_VK_F3         = 0x72, // 114 = VK_F3          | 'F3'
    ST_VK_F4         = 0x73, // 115 = VK_F4          | 'F4'
    ST_VK_F5         = 0x74, // 116 = VK_F5          | 'F5'
    ST_VK_F6         = 0x75, // 117 = VK_F6          | 'F6'
    ST_VK_F7         = 0x76, // 118 = VK_F7          | 'F7'
    ST_VK_F8         = 0x77, // 119 = VK_F8          | 'F8'
    ST_VK_F9         = 0x78, // 120 = VK_F9          | 'F9'
    ST_VK_F10        = 0x79, // 121 = VK_F10         | 'F10'
    ST_VK_F11        = 0x7A, // 122 = VK_F11         | 'F11'
    ST_VK_F12        = 0x7B, // 123 = VK_F12         | 'F12'

    // MORE special keys
    ST_VK_F13        = 0x7C, // 124 = VK_F13         | 'F13'
    ST_VK_F14        = 0x7D, // 125 = VK_F14         | 'F14'
    ST_VK_F15        = 0x7E, // 126 = VK_F15         | 'F15'
    ST_VK_F16        = 0x7F, // 127 = VK_F16         | 'F16'
    ST_VK_F17        = 0x80, // 128 = VK_F17         | 'F17'
    ST_VK_F18        = 0x81, // 129 = VK_F18         | 'F18'
    ST_VK_F19        = 0x82, // 130 = VK_F19         | 'F19'
    ST_VK_F20        = 0x83, // 131 = VK_F20         | 'F20'
    ST_VK_F21        = 0x84, // 132 = VK_F21         | 'F21'
    ST_VK_F22        = 0x85, // 133 = VK_F22         | 'F22'
    ST_VK_F23        = 0x86, // 134 = VK_F23         | 'F23'
    ST_VK_F24        = 0x87, // 135 = VK_F24         | 'F24'

    // unassigned 0x88-0x8F
    ST_VK_x88        = 0x88, // 136 = VK unassigned  |
    ST_VK_x89        = 0x89, // 137 = VK unassigned  |
    ST_VK_x8A        = 0x8A, // 138 = VK unassigned  |
    ST_VK_x8B        = 0x8B, // 139 = VK unassigned  |
    ST_VK_x8C        = 0x8C, // 140 = VK unassigned  |
    ST_VK_x8D        = 0x8D, // 141 = VK unassigned  |
    ST_VK_x8E        = 0x8E, // 142 = VK unassigned  |
    ST_VK_x8F        = 0x8F, // 143 = VK unassigned  |

    ST_VK_NUMLOCK    = 0x90, // 144 = VK_NUMLOCK     | Num Lock key
    ST_VK_SCROLL     = 0x91, // 145 = VK_SCROLL      | Scroll Lock key

    // obsolete definitions 0x92-0x96
    ST_VK_x92        = 0x92, // 146 = VK_OEM_NEC_EQUAL (numpad '=') or VK_OEM_FJ_JISHO ('Dictionary' key)
    ST_VK_x93        = 0x93, // 147 = VK_OEM_FJ_MASSHOU ('Unregister word' key)
    ST_VK_x94        = 0x94, // 148 = VK_OEM_FJ_TOUROKU ('Register word' key)
    ST_VK_x95        = 0x95, // 149 = VK_OEM_FJ_LOYA ('Left OYAYUBI' key)
    ST_VK_x96        = 0x96, // 150 = VK_OEM_FJ_ROYA ('Right OYAYUBI' key)

    // unassigned 0x97-0x9F
    ST_VK_x97        = 0x97, // 151 = VK unassigned  |
    ST_VK_x98        = 0x98, // 152 = VK unassigned  |
    ST_VK_x99        = 0x99, // 153 = VK unassigned  |
    ST_VK_x9A        = 0x9A, // 154 = VK unassigned  |
    ST_VK_x9B        = 0x9B, // 155 = VK unassigned  |
    ST_VK_x9C        = 0x9C, // 156 = VK unassigned  |
    ST_VK_x9D        = 0x9D, // 157 = VK unassigned  |
    ST_VK_x9E        = 0x9E, // 158 = VK unassigned  |
    ST_VK_x9F        = 0x9F, // 159 = VK unassigned  |

    // VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
    // Used only as parameters to GetAsyncKeyState() and GetKeyState().
    // No other API or message will distinguish left and right keys in this way.
    ST_VK_LSHIFT     = 0xA0, // 160 = VK_LSHIFT      | left 'Shift'
    ST_VK_RSHIFT     = 0xA1, // 161 = VK_RSHIFT      | right 'Shift'
    ST_VK_LCONTROL   = 0xA2, // 162 = VK_LCONTROL    | left 'Ctrl'
    ST_VK_RCONTROL   = 0xA3, // 163 = VK_RCONTROL    | right 'Ctrl'
    ST_VK_LMENU      = 0xA4, // 164 = VK_LMENU       | left 'menu'
    ST_VK_RMENU      = 0xA5, // 165 = VK_RMENU       | right 'menu'

    // Multimedia keys
    ST_VK_BROWSER_BACK        = 0xA6, // VK_BROWSER_BACK
    ST_VK_BROWSER_FORWARD     = 0xA7,
    ST_VK_BROWSER_REFRESH     = 0xA8,
    ST_VK_BROWSER_STOP        = 0xA9,
    ST_VK_BROWSER_SEARCH      = 0xAA,
    ST_VK_BROWSER_FAVORITES   = 0xAB,
    ST_VK_BROWSER_HOME        = 0xAC,

    ST_VK_VOLUME_MUTE         = 0xAD,
    ST_VK_VOLUME_DOWN         = 0xAE,
    ST_VK_VOLUME_UP           = 0xAF,
    ST_VK_MEDIA_NEXT_TRACK    = 0xB0,
    ST_VK_MEDIA_PREV_TRACK    = 0xB1,
    ST_VK_MEDIA_STOP          = 0xB2,
    ST_VK_MEDIA_PLAY_PAUSE    = 0xB3,
    ST_VK_LAUNCH_MAIL         = 0xB4,
    ST_VK_LAUNCH_MEDIA_SELECT = 0xB5,
    ST_VK_LAUNCH_APP1         = 0xB6,
    ST_VK_LAUNCH_APP2         = 0xB7,

    // reserved 0xB8-0xB9
    ST_VK_xB8        = 0xB8,   // 184 = VK unassigned  |
    ST_VK_xB9        = 0xB9,   // 185 = VK unassigned  |

    ST_VK_SEMICOLON  = 0xBA,   // 186 = VK_OEM_1       | ';:' for US

    ST_VK_OEM_PLUS   = 0xBB,   // 187 = VK_OEM_PLUS    | '+' any country
    // TODO (Kirill Gavrilov#9#)
    ST_VK_COMMA      = 0xBC,   // 188 = VK_OEM_COMMA   | ',' any country
    ST_VK_OEM_MINUS  = 0xBD,   // 189 = VK_OEM_MINUS   | '-' any country
    ST_VK_PERIOD     = 0xBE,   // 190 = VK_OEM_PERIOD  | '.' any country
    ST_VK_SLASH      = 0xBF,   // 191 = VK_OEM_2       | '/?' for US
    ST_VK_TILDE      = 0xC0,   // 192 = VK_OEM_3       | '`~' for US*

    ST_VK_BRACKETLEFT  = 0xDB, // 219 = VK_OEM_4       | '[{' for US
    ST_VK_BACKSLASH    = 0xDC, // 220 = VK_OEM_5       | '\|' for US
    ST_VK_BRACKETRIGHT = 0xDD, // 221 = VK_OEM_6       | ']}' for US
    ST_VK_APOSTROPHE   = 0xDE, // 222 = VK_OEM_7       | ''"' for US

        // extensions

    ST_VK_COMMAND      = 0xD8, // 216 | OS X command Key (NSCommandKeyMask)
    ST_VK_FUNCTION     = 0xD9, // 217 | OS X fn key      (NSFunctionKeyMask)

};
enum {
    ST_VK_NB = 256 //!< maximum number of virtual keys
};

/**
 * Remove modifiers from hot key combination and return the base virtual key.
 */
ST_LOCAL inline StVirtKey getBaseKeyFromHotKey(unsigned int theHotKey) {
    unsigned int aKey = theHotKey & ~(ST_VF_SHIFT | ST_VF_CONTROL | ST_VF_MENU | ST_VF_COMMAND | ST_VF_FUNCTION);
    if(aKey == 0
    || aKey >= ST_VK_NB) {
        return ST_VK_NULL;
    }
    return (StVirtKey )aKey;
}

/**
 * Encode single Virtual Key.
 * @param theKey Virtual Key code within 0-255 range
 * @return string representation for specified Virtual Key
 */
ST_CPPEXPORT const char* encodeVirtKey(const StVirtKey theKey);

/**
 * Encode keys combination.
 * @param theKey StVirtKey within StVirtFlags combination
 * @return string representation for specified keys combination
 */
ST_CPPEXPORT StString encodeHotKey(const unsigned int theKey);

/**
 * Decode keys combination from string representation.
 * @param string representation of keys combination
 * @return decoded keys combination
 */
ST_CPPEXPORT unsigned int decodeHotKey(const StString& theString);

#endif // StVirtualKey
