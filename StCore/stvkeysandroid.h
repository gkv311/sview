/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__ANDROID__))

#ifndef __stVKEYS_ANDROID_H_
#define __stVKEYS_ANDROID_H_

#include <StCore/StVirtualKeys.h>

/**
 * This is lookup array to convert Android NDK key codes to ST_VKEY codes.
 * Notice, this array useless for text input!
 */
#define ST_ANDROID2ST_VK_SIZE 256
static const unsigned int ST_ANDROID2ST_VK[ST_ANDROID2ST_VK_SIZE] = {
    0,            // AKEYCODE_UNKNOWN         = 0,
    0,            // AKEYCODE_SOFT_LEFT       = 1,
    0,            // AKEYCODE_SOFT_RIGHT      = 2,
    0,            // AKEYCODE_HOME            = 3,
    0,            // AKEYCODE_BACK            = 4,
    0,            // AKEYCODE_CALL            = 5,
    0,            // AKEYCODE_ENDCALL         = 6,
    ST_VK_0,      // AKEYCODE_0               = 7,
    ST_VK_1,      // AKEYCODE_1               = 8,
    ST_VK_2,      // AKEYCODE_2               = 9,
    ST_VK_3,      // AKEYCODE_3               = 10,
    ST_VK_4,      // AKEYCODE_4               = 11,
    ST_VK_5,      // AKEYCODE_5               = 12,
    ST_VK_6,      // AKEYCODE_6               = 13,
    ST_VK_7,      // AKEYCODE_7               = 14,
    ST_VK_8,      // AKEYCODE_8               = 15,
    ST_VK_9,      // AKEYCODE_9               = 16,
    0,            // AKEYCODE_STAR            = 17,
    0,            // AKEYCODE_POUND           = 18,
    ST_VK_UP,     // AKEYCODE_DPAD_UP         = 19,
    ST_VK_DOWN,   // AKEYCODE_DPAD_DOWN       = 20,
    ST_VK_LEFT,   // AKEYCODE_DPAD_LEFT       = 21,
    ST_VK_RIGHT,  // AKEYCODE_DPAD_RIGHT      = 22,
    0,            // AKEYCODE_DPAD_CENTER     = 23,
    ST_VK_VOLUME_UP,   // AKEYCODE_VOLUME_UP       = 24,
    ST_VK_VOLUME_DOWN, // AKEYCODE_VOLUME_DOWN     = 25,
    0,            // AKEYCODE_POWER           = 26,
    0,            // AKEYCODE_CAMERA          = 27,
    0,            // AKEYCODE_CLEAR           = 28,
    ST_VK_A,      // AKEYCODE_A               = 29,
    ST_VK_B,      // AKEYCODE_B               = 30,
    ST_VK_C,      // AKEYCODE_C               = 31,
    ST_VK_D,      // AKEYCODE_D               = 32,
    ST_VK_E,      // AKEYCODE_E               = 33,
    ST_VK_F,      // AKEYCODE_F               = 34,
    ST_VK_G,      // AKEYCODE_G               = 35,
    ST_VK_H,      // AKEYCODE_H               = 36,
    ST_VK_I,      // AKEYCODE_I               = 37,
    ST_VK_J,      // AKEYCODE_J               = 38,
    ST_VK_K,      // AKEYCODE_K               = 39,
    ST_VK_L,      // AKEYCODE_L               = 40,
    ST_VK_M,      // AKEYCODE_M               = 41,
    ST_VK_N,      // AKEYCODE_N               = 42,
    ST_VK_O,      // AKEYCODE_O               = 43,
    ST_VK_P,      // AKEYCODE_P               = 44,
    ST_VK_Q,      // AKEYCODE_Q               = 45,
    ST_VK_R,      // AKEYCODE_R               = 46,
    ST_VK_S,      // AKEYCODE_S               = 47,
    ST_VK_T,      // AKEYCODE_T               = 48,
    ST_VK_U,      // AKEYCODE_U               = 49,
    ST_VK_V,      // AKEYCODE_V               = 50,
    ST_VK_W,      // AKEYCODE_W               = 51,
    ST_VK_X,      // AKEYCODE_X               = 52,
    ST_VK_Y,      // AKEYCODE_Y               = 53,
    ST_VK_Z,      // AKEYCODE_Z               = 54,
    ST_VK_COMMA,  // AKEYCODE_COMMA           = 55,
    ST_VK_PERIOD, // AKEYCODE_PERIOD          = 56,
    ST_VK_MENU,   // AKEYCODE_ALT_LEFT        = 57,
    ST_VK_MENU,   // AKEYCODE_ALT_RIGHT       = 58,
    ST_VK_SHIFT,  // AKEYCODE_SHIFT_LEFT      = 59,
    ST_VK_SHIFT,  // AKEYCODE_SHIFT_RIGHT     = 60,
    ST_VK_TAB,    // AKEYCODE_TAB             = 61,
    ST_VK_SPACE,  // AKEYCODE_SPACE           = 62,
    0,            // AKEYCODE_SYM             = 63,
    0,            // AKEYCODE_EXPLORER        = 64,
    0,            // AKEYCODE_ENVELOPE        = 65,
    ST_VK_RETURN, // AKEYCODE_ENTER           = 66,
    ST_VK_BACK,   // AKEYCODE_DEL             = 67,
    0,            // AKEYCODE_GRAVE           = 68,
    ST_VK_OEM_MINUS,    // AKEYCODE_MINUS           = 69,
    ST_VK_OEM_PLUS,     // AKEYCODE_EQUALS          = 70,
    ST_VK_BRACKETLEFT,  // AKEYCODE_LEFT_BRACKET    = 71,
    ST_VK_BRACKETRIGHT, // AKEYCODE_RIGHT_BRACKET   = 72,
    ST_VK_BACKSLASH,    // AKEYCODE_BACKSLASH       = 73,
    ST_VK_SEMICOLON,    // AKEYCODE_SEMICOLON       = 74,
    ST_VK_APOSTROPHE,   // AKEYCODE_APOSTROPHE      = 75,
    ST_VK_SLASH,        // AKEYCODE_SLASH           = 76,
    0, // AKEYCODE_AT              = 77,
    0, // AKEYCODE_NUM             = 78,
    0, // AKEYCODE_HEADSETHOOK     = 79,
    0, // AKEYCODE_FOCUS           = 80,   // *Camera* focus
    0, // AKEYCODE_PLUS            = 81,
    0, // AKEYCODE_MENU            = 82,
    0, // AKEYCODE_NOTIFICATION    = 83,
    0, // AKEYCODE_SEARCH          = 84,
    ST_VK_MEDIA_PLAY_PAUSE, // AKEYCODE_MEDIA_PLAY_PAUSE= 85,
    ST_VK_MEDIA_STOP,       // AKEYCODE_MEDIA_STOP      = 86,
    ST_VK_MEDIA_NEXT_TRACK, // AKEYCODE_MEDIA_NEXT      = 87,
    ST_VK_MEDIA_PREV_TRACK, // AKEYCODE_MEDIA_PREVIOUS  = 88,
    0,                      // AKEYCODE_MEDIA_REWIND    = 89,
    0,                      // AKEYCODE_MEDIA_FAST_FORWARD = 90,
    ST_VK_VOLUME_MUTE,      // AKEYCODE_MUTE            = 91,
    ST_VK_PAGE_UP,          // AKEYCODE_PAGE_UP         = 92,
    ST_VK_PAGE_DOWN,        // AKEYCODE_PAGE_DOWN       = 93,
    0, // AKEYCODE_PICTSYMBOLS     = 94,
    0, // AKEYCODE_SWITCH_CHARSET  = 95,
    0, // AKEYCODE_BUTTON_A        = 96,
    0, // AKEYCODE_BUTTON_B        = 97,
    0, // AKEYCODE_BUTTON_C        = 98,
    0, // AKEYCODE_BUTTON_X        = 99,
    0, // AKEYCODE_BUTTON_Y        = 100,
    0, // AKEYCODE_BUTTON_Z        = 101,
    0, // AKEYCODE_BUTTON_L1       = 102,
    0, // AKEYCODE_BUTTON_R1       = 103,
    0, // AKEYCODE_BUTTON_L2       = 104,
    0, // AKEYCODE_BUTTON_R2       = 105,
    0, // AKEYCODE_BUTTON_THUMBL   = 106,
    0, // AKEYCODE_BUTTON_THUMBR   = 107,
    0, // AKEYCODE_BUTTON_START    = 108,
    0, // AKEYCODE_BUTTON_SELECT   = 109,
    0, // AKEYCODE_BUTTON_MODE     = 110,
    ST_VK_ESCAPE,    // AKEYCODE_ESCAPE          = 111,
    ST_VK_DELETE,    // AKEYCODE_FORWARD_DEL     = 112,
    ST_VK_CONTROL,   // AKEYCODE_CTRL_LEFT       = 113,
    ST_VK_CONTROL,   // AKEYCODE_CTRL_RIGHT      = 114,
    ST_VK_CAPITAL,   // AKEYCODE_CAPS_LOCK       = 115,
    ST_VK_SCROLL,    // AKEYCODE_SCROLL_LOCK     = 116,
    0,               // AKEYCODE_META_LEFT       = 117,
    0,               // AKEYCODE_META_RIGHT      = 118,
    0,               // AKEYCODE_FUNCTION        = 119,
    0,               // AKEYCODE_SYSRQ           = 120,
    0,               // AKEYCODE_BREAK           = 121,
    ST_VK_HOME,      // AKEYCODE_MOVE_HOME       = 122,
    ST_VK_END,       // AKEYCODE_MOVE_END        = 123,
    ST_VK_INSERT,    // AKEYCODE_INSERT          = 124,
    0,               // AKEYCODE_FORWARD         = 125,
    ST_VK_MEDIA_PLAY_PAUSE, // AKEYCODE_MEDIA_PLAY      = 126,
    ST_VK_MEDIA_PLAY_PAUSE, // AKEYCODE_MEDIA_PAUSE     = 127,
    0,               // AKEYCODE_MEDIA_CLOSE     = 128,
    0,               // AKEYCODE_MEDIA_EJECT     = 129,
    0,               // AKEYCODE_MEDIA_RECORD    = 130,
    ST_VK_F1,        // AKEYCODE_F1              = 131,
    ST_VK_F2,        // AKEYCODE_F2              = 132,
    ST_VK_F3,        // AKEYCODE_F3              = 133,
    ST_VK_F4,        // AKEYCODE_F4              = 134,
    ST_VK_F5,        // AKEYCODE_F5              = 135,
    ST_VK_F6,        // AKEYCODE_F6              = 136,
    ST_VK_F7,        // AKEYCODE_F7              = 137,
    ST_VK_F8,        // AKEYCODE_F8              = 138,
    ST_VK_F9,        // AKEYCODE_F9              = 139,
    ST_VK_F10,       // AKEYCODE_F10             = 140,
    ST_VK_F11,       // AKEYCODE_F11             = 141,
    ST_VK_F12,       // AKEYCODE_F12             = 142,
    0,               // AKEYCODE_NUM_LOCK        = 143,
    ST_VK_NUMPAD0,   // AKEYCODE_NUMPAD_0        = 144,
    ST_VK_NUMPAD1,   // AKEYCODE_NUMPAD_1        = 145,
    ST_VK_NUMPAD2,   // AKEYCODE_NUMPAD_2        = 146,
    ST_VK_NUMPAD3,   // AKEYCODE_NUMPAD_3        = 147,
    ST_VK_NUMPAD4,   // AKEYCODE_NUMPAD_4        = 148,
    ST_VK_NUMPAD5,   // AKEYCODE_NUMPAD_5        = 149,
    ST_VK_NUMPAD6,   // AKEYCODE_NUMPAD_6        = 150,
    ST_VK_NUMPAD7,   // AKEYCODE_NUMPAD_7        = 151,
    ST_VK_NUMPAD8,   // AKEYCODE_NUMPAD_8        = 152,
    ST_VK_NUMPAD9,   // AKEYCODE_NUMPAD_9        = 153,
    ST_VK_DIVIDE,    // AKEYCODE_NUMPAD_DIVIDE   = 154,
    ST_VK_MULTIPLY,  // AKEYCODE_NUMPAD_MULTIPLY = 155,
    ST_VK_SUBTRACT,  // AKEYCODE_NUMPAD_SUBTRACT = 156,
    ST_VK_ADD,       // AKEYCODE_NUMPAD_ADD      = 157,
    ST_VK_DECIMAL,   // AKEYCODE_NUMPAD_DOT      = 158,
    ST_VK_SEPARATOR, // AKEYCODE_NUMPAD_COMMA    = 159,
    ST_VK_RETURN,    // AKEYCODE_NUMPAD_ENTER    = 160,
    0, // AKEYCODE_NUMPAD_EQUALS   = 161,
    0, // AKEYCODE_NUMPAD_LEFT_PAREN = 162,
    0, // AKEYCODE_NUMPAD_RIGHT_PAREN = 163,
    ST_VK_VOLUME_MUTE, // AKEYCODE_VOLUME_MUTE     = 164,
    0, // AKEYCODE_INFO            = 165,
    0, // AKEYCODE_CHANNEL_UP      = 166,
    0, // AKEYCODE_CHANNEL_DOWN    = 167,
    0, // AKEYCODE_ZOOM_IN         = 168,
    0, // AKEYCODE_ZOOM_OUT        = 169,
    0, // AKEYCODE_TV              = 170,
    0, // AKEYCODE_WINDOW          = 171,
    0, // AKEYCODE_GUIDE           = 172,
    0, // AKEYCODE_DVR             = 173,
    0, // AKEYCODE_BOOKMARK        = 174,
    0, // AKEYCODE_CAPTIONS        = 175,
    0, // AKEYCODE_SETTINGS        = 176,
    0, // AKEYCODE_TV_POWER        = 177,
    0, // AKEYCODE_TV_INPUT        = 178,
    0, // AKEYCODE_STB_POWER       = 179,
    0, // AKEYCODE_STB_INPUT       = 180,
    0, // AKEYCODE_AVR_POWER       = 181,
    0, // AKEYCODE_AVR_INPUT       = 182,
    0, // AKEYCODE_PROG_RED        = 183,
    0, // AKEYCODE_PROG_GREEN      = 184,
    0, // AKEYCODE_PROG_YELLOW     = 185,
    0, // AKEYCODE_PROG_BLUE       = 186,
    0, // AKEYCODE_APP_SWITCH      = 187,
    0, // AKEYCODE_BUTTON_1        = 188,
    0, // AKEYCODE_BUTTON_2        = 189,
    0, // AKEYCODE_BUTTON_3        = 190,
    0, // AKEYCODE_BUTTON_4        = 191,
    0, // AKEYCODE_BUTTON_5        = 192,
    0, // AKEYCODE_BUTTON_6        = 193,
    0, // AKEYCODE_BUTTON_7        = 194,
    0, // AKEYCODE_BUTTON_8        = 195,
    0, // AKEYCODE_BUTTON_9        = 196,
    0, // AKEYCODE_BUTTON_10       = 197,
    0, // AKEYCODE_BUTTON_11       = 198,
    0, // AKEYCODE_BUTTON_12       = 199,
    0, // AKEYCODE_BUTTON_13       = 200,
    0, // AKEYCODE_BUTTON_14       = 201,
    0, // AKEYCODE_BUTTON_15       = 202,
    0, // AKEYCODE_BUTTON_16       = 203,
    0, // AKEYCODE_LANGUAGE_SWITCH = 204,
    0, // AKEYCODE_MANNER_MODE     = 205,
    0, /// AKEYCODE_3D_MODE         = 206,
    0, // AKEYCODE_CONTACTS        = 207,
    0, // AKEYCODE_CALENDAR        = 208,
    0, // AKEYCODE_MUSIC           = 209,
    0, // AKEYCODE_CALCULATOR      = 210,
    0, // 211
    0, // 212
    0, // 213
    0, // 214
    0, // 215
    0, // 216
    0, // 217
    0, // 218
    0, // 219
    0, // 220
    0, // 221
    0, // 222
    0, // 223
    0, // 224
    0, // 225
    0, // 226
    0, // 227
    0, // 228
    0, // 229
    0, // 230
    0, // 231
    0, // 232
    0, // 233
    0, // 234
    0, // 235
    0, // 236
    0, // 237
    0, // 238
    0, // 239
    0, // 240
    0, // 241
    0, // 242
    0, // 243
    0, // 244
    0, // 245
    0, // 246
    0, // 247
    0, // 248
    0, // 249
    0, // 250
    0, // 251
    0, // 252
    0, // 253
    0, // 254
    0  // 255
};

#endif // __stVKEYS_ANDROID_H_
#endif // __ANDROID__
