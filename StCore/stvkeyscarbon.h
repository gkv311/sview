/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
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

#if (defined(__APPLE__))

#ifndef __stVKEYS_CARBON_H_
#define __stVKEYS_CARBON_H_

#include <StCore/StVirtualKeys.h>

/**
 * This is lookup array to convert Carbon key codes to ST_VKEY codes.
 * Notice, this array useless for text input!
 */
#define ST_CARBON2ST_VK_SIZE 128
static const size_t ST_CARBON2ST_VK[ST_CARBON2ST_VK_SIZE] = {
    ST_VK_A,
    ST_VK_S,
    ST_VK_D,
    ST_VK_F,
    ST_VK_H,
    ST_VK_G,
    ST_VK_Z,
    ST_VK_X,
    ST_VK_C,
    ST_VK_V,
    0, // 10
    ST_VK_B,
    ST_VK_Q,
    ST_VK_W,
    ST_VK_E,
    ST_VK_R,
    ST_VK_Y,
    ST_VK_T,
    ST_VK_1,
    ST_VK_2,
    ST_VK_3,
    ST_VK_4,
    ST_VK_6,
    ST_VK_5,
    ST_VK_OEM_PLUS,
    ST_VK_9,
    ST_VK_7,
    ST_VK_OEM_MINUS,
    ST_VK_8,
    ST_VK_0,
    ST_VK_BRACKETRIGHT,
    ST_VK_O,
    ST_VK_U,
    ST_VK_BRACKETLEFT,
    ST_VK_I,
    ST_VK_P,
    ST_VK_RETURN, // return
    ST_VK_L,
    ST_VK_J,
    ST_VK_APOSTROPHE,
    ST_VK_K,
    ST_VK_SEMICOLON,
    ST_VK_BACKSLASH,
    0, //ST_VK_OEM_COMMA, // 43, ',<'
    0, //ST_VK_OEM_2, // 44, '?/'
    ST_VK_N,
    ST_VK_M,
    0, //ST_VK_OEM_PERIOD, // 47, '.>'
    ST_VK_TAB,
    ST_VK_SPACE,
    0, //ST_VK_OEM_3, 50, '~`'
    ST_VK_BACK,
    0, // 52
    ST_VK_ESCAPE,
    0, // 54
    0, // 55
    0, // 56
    0, // 57
    0, // 58
    0, // 59
    0, // 60
    0, // 61
    0, // 62
    0, // 63
    0, // 64
    0, // 65
    0, // 66
    0, // 67
    0, // 68
    0, // 69
    0, // 70
    0, // 71
    0, // 72
    0, // 73
    0, // 74
    0, // 75
    ST_VK_RETURN, // 76, fn + return
    0, // 77
    0, // 78
    0, // 79
    0, // 80
    0, // 81
    0, // 82
    0, // 83
    0, // 84
    0, // 85
    0, // 86
    0, // 87
    0, // 88
    0, // 89
    0, // 90
    0, // 91
    0, // 92
    0, // 93
    0, // 94
    0, // 95
    ST_VK_F5,
    ST_VK_F6,
    ST_VK_F7,
    ST_VK_F3,
    ST_VK_F8,
    ST_VK_F9,
    0, // 102
    ST_VK_F11,
    0, // 104
    0, // 105
    0, // 106
    0, // 107
    0, // 108
    ST_VK_F10,
    0, // 110
    ST_VK_F12,
    0, // 112
    0, // 113
    0, // 114
    ST_VK_HOME,
    ST_VK_PAGE_UP,
    ST_VK_DELETE,
    ST_VK_F4,
    ST_VK_END,
    ST_VK_F2,
    ST_VK_PAGE_DOWN,
    ST_VK_F1,
    ST_VK_LEFT,
    ST_VK_RIGHT,
    ST_VK_DOWN,
    ST_VK_UP,
    0, // 127
};

#endif // __stVKEYS_CARBON_H_
#endif // __APPLE__
