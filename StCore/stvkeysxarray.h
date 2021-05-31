/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __stVKEYS_XARRAY_H_
#define __stVKEYS_XARRAY_H_

#include <StCore/StVirtualKeys.h>

#if defined(__linux__) && !defined(__ANDROID__)
#include <X11/keysym.h>
/**
 * This is FAT lookup array to convert X key codes to ST_VKEY codes.
 * Notice, this array useless for text input!
 */
#define ST_XK2ST_VK_SIZE 0x10000
static const unsigned int ST_XK2ST_VK[ST_XK2ST_VK_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x0000-0x000F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x0010-0x001F
    ST_VK_SPACE,  // 0x20 = XK_space
    0, // 0x21
    0, // 0x22
    0, // 0x23
    0, // 0x24
    0, // 0x25
    0, // 0x26
    ST_VK_APOSTROPHE, // 0x27 = XK_apostrophe
    0, // 0x28
    0, // 0x29
    0, // 0x2A
    0, // 0x2B
    ST_VK_COMMA,     // 0x2C = XK_comma
    ST_VK_OEM_MINUS, // 0x2D = XK_minus
    ST_VK_PERIOD,    // 0x2E = XK_period
    ST_VK_SLASH,  // 0x2F = XK_slash
    ST_VK_0,      // 0x30 = XK_0
    ST_VK_1,      // 0x31 = XK_1
    ST_VK_2,      // 0x32 = XK_2
    ST_VK_3,      // 0x33 = XK_3
    ST_VK_4,      // 0x34 = XK_4
    ST_VK_5,      // 0x35 = XK_5
    ST_VK_6,      // 0x36 = XK_6
    ST_VK_7,      // 0x37 = XK_7
    ST_VK_8,      // 0x38 = XK_8
    ST_VK_9,      // 0x39 = XK_9
    0, // 0x3A
    ST_VK_SEMICOLON, // 0x3B = XK_semicolon
    0, // 0x3C
    ST_VK_OEM_PLUS,  // 0x3D = XK_equal
    0, // 0x3E
    0, // 0x3F
    0, // 0x40
    ST_VK_A,      // 0x41 = XK_A
    ST_VK_B,      // 0x42 = XK_B
    ST_VK_C,      // 0x43 = XK_C
    ST_VK_D,      // 0x44 = XK_D
    ST_VK_E,      // 0x45 = XK_E
    ST_VK_F,      // 0x46 = XK_F
    ST_VK_G,      // 0x47 = XK_G
    ST_VK_H,      // 0x48 = XK_H
    ST_VK_I,      // 0x49 = XK_I
    ST_VK_J,      // 0x4A = XK_J
    ST_VK_K,      // 0x4B = XK_K
    ST_VK_L,      // 0x4C = XK_L
    ST_VK_M,      // 0x4D = XK_M
    ST_VK_N,      // 0x4E = XK_N
    ST_VK_O,      // 0x4F = XK_O
    ST_VK_P,      // 0x50 = XK_P
    ST_VK_Q,      // 0x51 = XK_Q
    ST_VK_R,      // 0x52 = XK_R
    ST_VK_S,      // 0x53 = XK_S
    ST_VK_T,      // 0x54 = XK_T
    ST_VK_U,      // 0x55 = XK_U
    ST_VK_V,      // 0x56 = XK_V
    ST_VK_W,      // 0x57 = XK_W
    ST_VK_X,      // 0x58 = XK_X
    ST_VK_Y,      // 0x59 = XK_Y
    ST_VK_Z,      // 0x5A = XK_Z
    ST_VK_BRACKETLEFT,  // 0x5B = XK_bracketleft
    ST_VK_BACKSLASH,    // 0x5C = XK_backslash
    ST_VK_BRACKETRIGHT, // 0x5D = XK_bracketright
    ST_VK_NULL,   // 0x5E
    ST_VK_NULL,   // 0x5F
    ST_VK_TILDE,  // 0x60 = XK_grave
    ST_VK_A,      // 0x61 = XK_a
    ST_VK_B,      // 0x62 = XK_b
    ST_VK_C,      // 0x63 = XK_c
    ST_VK_D,      // 0x64 = XK_d
    ST_VK_E,      // 0x65 = XK_e
    ST_VK_F,      // 0x66 = XK_f
    ST_VK_G,      // 0x67 = XK_g
    ST_VK_H,      // 0x68 = XK_h
    ST_VK_I,      // 0x69 = XK_i
    ST_VK_J,      // 0x6A = XK_j
    ST_VK_K,      // 0x6B = XK_k
    ST_VK_L,      // 0x6C = XK_l
    ST_VK_M,      // 0x6D = XK_m
    ST_VK_N,      // 0x6E = XK_n
    ST_VK_O,      // 0x6F = XK_o
    ST_VK_P,      // 0x70 = XK_p
    ST_VK_Q,      // 0x71 = XK_q
    ST_VK_R,      // 0x72 = XK_r
    ST_VK_S,      // 0x73 = XK_s
    ST_VK_T,      // 0x74 = XK_t
    ST_VK_U,      // 0x75 = XK_u
    ST_VK_V,      // 0x76 = XK_v
    ST_VK_W,      // 0x77 = XK_w
    ST_VK_X,      // 0x78 = XK_x
    ST_VK_Y,      // 0x79 = XK_y
    ST_VK_Z,      // 0x7A = XK_z
    0, // 0x7B
    0, // 0x7C
    0, // 0x7D
    0, // 0x7E
    0, // 0x7F

    // ZEROS part
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x0080-0x008F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x0090-0x009F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00A0-0x00AF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00B0-0x00BF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00C0-0x00CF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00D0-0x00DF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00E0-0x00EF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00F0-0x00FF
    #include "zerosarray256.h"  // 0x0100-0x01FF
    #include "zerosarray256.h"  // 0x0200-0x02FF
    #include "zerosarray256.h"  // 0x0300-0x03FF
    #include "zerosarray256.h"  // 0x0400-0x04FF
    #include "zerosarray256.h"  // 0x0500-0x05FF
    #include "zerosarray256.h"  // 0x0600-0x06FF
    #include "zerosarray256.h"  // 0x0700-0x07FF
    #include "zerosarray256.h"  // 0x0800-0x08FF
    #include "zerosarray256.h"  // 0x0900-0x09FF
    #include "zerosarray256.h"  // 0x0A00-0x0AFF
    #include "zerosarray256.h"  // 0x0B00-0x0BFF
    #include "zerosarray256.h"  // 0x0C00-0x0CFF
    #include "zerosarray256.h"  // 0x0D00-0x0DFF
    #include "zerosarray256.h"  // 0x0E00-0x0EFF
    #include "zerosarray256.h"  // 0x0F00-0x0FFF
    #include "zerosarray4096.h" // 0x1000-0x1FFF
    #include "zerosarray4096.h" // 0x2000-0x2FFF
    #include "zerosarray4096.h" // 0x3000-0x3FFF
    #include "zerosarray4096.h" // 0x4000-0x4FFF
    #include "zerosarray4096.h" // 0x5000-0x5FFF
    #include "zerosarray4096.h" // 0x6000-0x6FFF
    #include "zerosarray4096.h" // 0x7000-0x7FFF
    #include "zerosarray4096.h" // 0x8000-0x8FFF
    #include "zerosarray4096.h" // 0x9000-0x9FFF
    #include "zerosarray4096.h" // 0xA000-0xAFFF
    #include "zerosarray4096.h" // 0xB000-0xBFFF
    #include "zerosarray4096.h" // 0xC000-0xCFFF
    #include "zerosarray4096.h" // 0xD000-0xDFFF
    #include "zerosarray4096.h" // 0xE000-0xEFFF
    #include "zerosarray256.h"  // 0xF000-0xF0FF
    #include "zerosarray256.h"  // 0xF100-0xF1FF
    #include "zerosarray256.h"  // 0xF200-0xF2FF
    #include "zerosarray256.h"  // 0xF300-0xF3FF
    #include "zerosarray256.h"  // 0xF400-0xF4FF
    #include "zerosarray256.h"  // 0xF500-0xF5FF
    #include "zerosarray256.h"  // 0xF600-0xF6FF
    #include "zerosarray256.h"  // 0xF700-0xF7FF
    #include "zerosarray256.h"  // 0xF800-0xF8FF
    #include "zerosarray256.h"  // 0xF900-0xF9FF
    #include "zerosarray256.h"  // 0xFA00-0xFAFF
    #include "zerosarray256.h"  // 0xFB00-0xFBFF
    #include "zerosarray256.h"  // 0xFC00-0xFCFF
    #include "zerosarray256.h"  // 0xFD00-0xFDFF
    #include "zerosarray256.h"  // 0xFE00-0xFEFF
    0, // 0xFF00
    0, // 0xFF01
    0, // 0xFF02
    0, // 0xFF03
    0, // 0xFF04
    0, // 0xFF05
    0, // 0xFF06
    0, // 0xFF07
    ST_VK_BACK,   // 0xFF08 = XK_BackSpace
    ST_VK_TAB,    // 0xFF09 = XK_Tab
    0, // 0xFF0A = XK_Linefeed ('LF')
    0, // 0xFF0B = XK_Clear
    0, // 0xFF0C
    ST_VK_RETURN, // 0xFF0D = XK_Return ('Enter')
    0, // 0xFF0E
    0, // 0xFF0F
    0, // 0xFF10
    0, // 0xFF11
    0, // 0xFF12
    ST_VK_PAUSE,  // 0xFF13 = XK_Pause
    0, // 0xFF14
    0, // 0xFF15
    0, // 0xFF16
    0, // 0xFF17
    0, // 0xFF18
    0, // 0xFF19
    0, // 0xFF1A
    ST_VK_ESCAPE, // 0xFF1B = XK_Escape
    0, // 0xFF1C
    0, // 0xFF1D
    0, // 0xFF1E
    0, // 0xFF1F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xFF20-0xFF2F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xFF30-0xFF3F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xFF40-0xFF4F
    ST_VK_HOME,   // 0xFF50 = XK_Home
    ST_VK_LEFT,   // 0xFF51 = XK_Left
    ST_VK_UP,     // 0xFF52 = XK_Up
    ST_VK_RIGHT,  // 0xFF53 = XK_Right
    ST_VK_DOWN,   // 0xFF54 = XK_Down
    ST_VK_PRIOR,  // 0xFF55 = XK_Prior
    ST_VK_NEXT,   // 0xFF56 = XK_Next
    ST_VK_END,    // 0xFF57 = XK_End
    0, // 0xFF58 = XK_Begin
    0, // 0xFF59
    0, // 0xFF5A
    0, // 0xFF5B
    0, // 0xFF5C
    0, // 0xFF5D
    0, // 0xFF5E
    0, // 0xFF5F
    0, // 0xFF60
    0, // 0xFF61
    0, // 0xFF62
    ST_VK_INSERT, // 0xFF63 = XK_Insert
    0, // 0xFF64
    0, // 0xFF65
    0, // 0xFF66
    ST_VK_MENU,   // 0xFF67 = XK_Menu
    0, // 0xFF68
    0, // 0xFF69
    0, // 0xFF6A
    0, // 0xFF6B
    0, // 0xFF6C
    0, // 0xFF6D
    0, // 0xFF6E
    0, // 0xFF6F
    0, // 0xFF70
    0, // 0xFF71
    0, // 0xFF72
    0, // 0xFF73
    0, // 0xFF74
    0, // 0xFF75
    0, // 0xFF76
    0, // 0xFF77
    0, // 0xFF78
    0, // 0xFF79
    0, // 0xFF7A
    0, // 0xFF7B
    0, // 0xFF7C
    0, // 0xFF7D
    0, // 0xFF7E
    ST_VK_NUMLOCK,// 0xFF7F = XK_Num_Lock
    0, // 0xFF80
    0, // 0xFF81
    0, // 0xFF82
    0, // 0xFF83
    0, // 0xFF84
    0, // 0xFF85
    0, // 0xFF86
    0, // 0xFF87
    0, // 0xFF88
    0, // 0xFF89
    0, // 0xFF8A
    0, // 0xFF8B
    0, // 0xFF8C
    ST_VK_RETURN, /// 0xFF8D = XK_KP_Enter // TODO
    0, // 0xFF8E
    0, // 0xFF8F
    0, // 0xFF90
    0, // 0xFF91
    0, // 0xFF92
    0, // 0xFF93
    0, // 0xFF94
    0, // 0xFF95
    0, // 0xFF96
    0, // 0xFF97
    0, // 0xFF98
    0, // 0xFF99
    0, // 0xFF9A
    0, // 0xFF9B
    0, // 0xFF9C
    0, // 0xFF9D
    0, // 0xFF9E
    0, /// 0xFF9F = XK_KP_Delete (numpad)
    0, // 0xFFA0
    0, // 0xFFA1
    0, // 0xFFA2
    0, // 0xFFA3
    0, // 0xFFA4
    0, // 0xFFA5
    0, // 0xFFA6
    0, // 0xFFA7
    0, // 0xFFA8
    0, // 0xFFA9
    ST_VK_MULTIPLY,  // 0xFFAA = XK_KP_Multiply
    ST_VK_ADD,       // 0xFFAB = XK_KP_Add
    ST_VK_SEPARATOR, // 0xFFAC = XK_KP_Separator (separator, often comma)
    ST_VK_SUBTRACT,  // 0xFFAD = XK_KP_Subtract
    ST_VK_DECIMAL,   // 0xFFAE = XK_KP_Decimal
    ST_VK_DIVIDE,    // 0xFFAF = XK_KP_Divide
    0, // 0xFFB0
    0, // 0xFFB1
    0, // 0xFFB2
    0, // 0xFFB3
    0, // 0xFFB4
    0, // 0xFFB5
    0, // 0xFFB6
    0, // 0xFFB7
    0, // 0xFFB8
    0, // 0xFFB9
    0, // 0xFFBA
    0, // 0xFFBB
    0, // 0xFFBC
    0, // 0xFFBD
    ST_VK_F1,     // 0xFFBE = XK_F1
    ST_VK_F2,     // 0xFFBF = XK_F2
    ST_VK_F3,     // 0xFFC0 = XK_F3
    ST_VK_F4,     // 0xFFC1 = XK_F4
    ST_VK_F5,     // 0xFFC2 = XK_F5
    ST_VK_F6,     // 0xFFC3 = XK_F6
    ST_VK_F7,     // 0xFFC4 = XK_F7
    ST_VK_F8,     // 0xFFC5 = XK_F8
    ST_VK_F9,     // 0xFFC6 = XK_F9
    ST_VK_F10,    // 0xFFC7 = XK_F10
    ST_VK_F11,    // 0xFFC8 = XK_F11
    ST_VK_F12,    // 0xFFC9 = XK_F12
    ST_VK_F13,    // 0xFFCA = XK_F13
    ST_VK_F14,    // 0xFFCB = XK_F14
    ST_VK_F15,    // 0xFFCC = XK_F15
    ST_VK_F16,    // 0xFFCD = XK_F16
    ST_VK_F17,    // 0xFFCE = XK_F17
    ST_VK_F18,    // 0xFFCF = XK_F18
    ST_VK_F19,    // 0xFFD0 = XK_F19
    ST_VK_F20,    // 0xFFD1 = XK_F20
    ST_VK_F21,    // 0xFFD2 = XK_F21
    ST_VK_F22,    // 0xFFD3 = XK_F22
    ST_VK_F23,    // 0xFFD4 = XK_F23
    ST_VK_F24,    // 0xFFD5 = XK_F24
    0, // 0xFFD6
    0, // 0xFFD7
    0, // 0xFFD8
    0, // 0xFFD9
    0, // 0xFFDA
    0, // 0xFFDB
    0, // 0xFFDC
    0, // 0xFFDD
    0, // 0xFFDE
    0, // 0xFFDF
    0, // 0xFFE0
    ST_VK_SHIFT,  // 0xFFE1 = XK_Shift_L // TODO (Kirill Gavrilov #9#) we break left/right distinctness here!
    ST_VK_SHIFT,  // 0xFFE2 = XK_Shift_R
    ST_VK_CONTROL,// 0xFFE3 = XK_Control_L
    ST_VK_CONTROL,// 0xFFE4 = XK_Control_R
    ST_VK_CAPITAL,// 0xFFE5 = XK_Caps_Lock
    0, // 0xFFE6
    0, // 0xFFE7
    0, // 0xFFE8
    ST_VK_MENU,   // 0xFFE9 = XK_Alt_L
    ST_VK_MENU,   // 0xFFEA = XK_Alt_R
    0,            // 0xFFEB = XK_Super_L
    0,            // 0xFFEC = XK_Super_R
    0, // 0xFFED
    0, // 0xFFEE
    0, // 0xFFEF
    0, // 0xFFF0
    0, // 0xFFF1
    0, // 0xFFF2
    0, // 0xFFF3
    0, // 0xFFF4
    0, // 0xFFF5
    0, // 0xFFF6
    0, // 0xFFF7
    0, // 0xFFF8
    0, // 0xFFF9
    0, // 0xFFFA
    0, // 0xFFFB
    0, // 0xFFFC
    0, // 0xFFFD
    0, // 0xFFFE
    ST_VK_DELETE  // 0xFFFF = XK_Delete
};

//#include <X11/XF86keysym.h>
#define ST_XKMEDIA_FIRST 0x1008FF00
#define ST_XKMEDIA_LAST  0x1008FF36
static const unsigned int ST_XKMEDIA2ST_VK[ST_XKMEDIA_LAST - ST_XKMEDIA_FIRST + 1] = {
    0,                      // 0x1008FF00
    0,                      // 0x1008FF01 = XF86ModeLock
    0,                      // 0x1008FF02
    0,                      // 0x1008FF03
    0,                      // 0x1008FF04
    0,                      // 0x1008FF05
    0,                      // 0x1008FF06
    0,                      // 0x1008FF07
    0,                      // 0x1008FF08
    0,                      // 0x1008FF09
    0,                      // 0x1008FF0A
    0,                      // 0x1008FF0B
    0,                      // 0x1008FF0C
    0,                      // 0x1008FF0D
    0,                      // 0x1008FF0E
    0,                      // 0x1008FF0F
    0,                      // 0x1008FF10 = XF86Standby
    ST_VK_VOLUME_DOWN,      // 0x1008FF11 = XF86AudioLowerVolume
    ST_VK_VOLUME_MUTE,      // 0x1008FF12 = XF86AudioMute
    ST_VK_VOLUME_UP,        // 0x1008FF13 = XF86AudioRaiseVolume
    ST_VK_MEDIA_PLAY_PAUSE, // 0x1008FF14 = XF86AudioPlay
    ST_VK_MEDIA_STOP,       // 0x1008FF15 = XF86AudioStop
    ST_VK_MEDIA_PREV_TRACK, // 0x1008FF16 = XF86AudioPrev
    ST_VK_MEDIA_NEXT_TRACK, // 0x1008FF17 = XF86AudioNext
    ST_VK_BROWSER_HOME,     // 0x1008FF18 = XF86HomePage
    ST_VK_LAUNCH_MAIL,      // 0x1008FF19 = XF86Mail
    0,                      // 0x1008FF1A = XF86Start
    0,                      // 0x1008FF1B = XF86Search
    0,                      // 0x1008FF1C = XF86AudioRecord
    0,                      // 0x1008FF1D = XF86Calculator
    0,                      // 0x1008FF1E = XF86Memo
    0,                      // 0x1008FF1F = XF86ToDoList
    0,                      // 0x1008FF20 = XF86Calendar
    0,                      // 0x1008FF21 = XF86PowerDown
    0,                      // 0x1008FF22 = XF86ContrastAdjust
    0,                      // 0x1008FF23 = XF86RockerUp
    0,                      // 0x1008FF24 = XF86RockerDown
    0,                      // 0x1008FF25 = XF86RockerEnter
    ST_VK_BROWSER_BACK,     // 0x1008FF26 = XF86Back
    ST_VK_BROWSER_FORWARD,  // 0x1008FF27 = XF86Forward
    ST_VK_BROWSER_STOP,     // 0x1008FF28 = XF86Stop
    ST_VK_BROWSER_REFRESH,  // 0x1008FF29 = XF86Refresh
    0,                      // 0x1008FF2A
    0,                      // 0x1008FF2B
    0,                      // 0x1008FF2C
    0,                      // 0x1008FF2D
    0,                      // 0x1008FF2E
    0,                      // 0x1008FF2F
    0,                      // 0x1008FF30 = XF86Favorites
    0,                      // 0x1008FF31 = XF86AudioPause
    0,                      // 0x1008FF32 = XF86AudioMedia
    0,                      // 0x1008FF33 = XF86MyComputer
    0,                      // 0x1008FF34 = XF86VendorHome
    0,                      // 0x1008FF35 = XF86LightBulb
    0                       // 0x1008FF36 = XF86Shop
};
#endif

#endif // __stVKEYS_XARRAY_H_
