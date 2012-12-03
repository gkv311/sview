/**
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
 *
 * StBrowserPlugin NPAPI plugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StBrowserPlugin NPAPI plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef npplat_h_
#define npplat_h_

#if(defined(_WIN32) || defined(__WIN32__))
    #include "windows.h"
#endif

#include "npapi.h"
#include "npfunctions.h"

#ifdef XP_UNIX
    #include <stdio.h>
#endif

#ifdef XP_MAC
    #include <Carbon/Carbon.h>
#endif

#ifndef HIBYTE
    #define HIBYTE(i) (i >> 8)
#endif

#ifndef LOBYTE
    #define LOBYTE(i) (i & 0xff)
#endif

#endif // npplat_h_
