/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StActiveX plugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StActiveX plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StActiveXModule_H__
#define __StActiveXModule_H__

#ifdef _MSC_VER

#include <afxctl.h> // MFC support for ActiveX Controls

#include <StTemplates/StHandle.h>
#include <StTemplates/StArrayList.h>

/**
 * Module entry class.
 */
class StActiveXModule : public COleControlModule {

        public:

    static const GUID TYPELIB_GUID;
    static const WORD VER_MAJOR;
    static const WORD VER_MINOR;

        public:

    StActiveXModule();
    virtual ~StActiveXModule();

    BOOL InitInstance();
    int  ExitInstance();

};

#endif // _MSC_VER
#endif // __StActiveXModule_H__
