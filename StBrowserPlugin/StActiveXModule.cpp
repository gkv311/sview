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

#include "StActiveXModule.h"

#if(defined(_WIN32) || defined(__WIN32__))

#include <StCore/StCore.h>

StActiveXModule NEAR theApp;

// f6ae5619-dcdc-4d32-9b3d-d1518e98e0eb
const GUID StActiveXModule::TYPELIB_GUID = { 0xf6ae5619, 0xdcdc, 0x4d32, {0x9b, 0x3d, 0xd1, 0x51, 0x8e, 0x98, 0xe0, 0xeb} };
const WORD StActiveXModule::VER_MAJOR = 1;
const WORD StActiveXModule::VER_MINOR = 0;

static const wchar_t ST_OCX_REG_KEY[] = L"CLSID\\{027792d0-5136-4ea3-9bec-34276dfe4362}\\InprocServer32";

STDAPI DllRegisterServer() {
    AFX_MANAGE_STATE(_afxModuleAddrThis);
    if(!AfxOleRegisterTypeLib(AfxGetInstanceHandle(),
                              StActiveXModule::TYPELIB_GUID)) {
        return ResultFromScode(SELFREG_E_TYPELIB);
    }
    if(!COleObjectFactoryEx::UpdateRegistryAll(TRUE)) {
        return ResultFromScode(SELFREG_E_CLASS);
    }
    return NOERROR;
}

STDAPI DllUnregisterServer() {
    AFX_MANAGE_STATE(_afxModuleAddrThis);
    if(!AfxOleUnregisterTypeLib(StActiveXModule::TYPELIB_GUID,
                                StActiveXModule::VER_MAJOR,
                                StActiveXModule::VER_MINOR)) {
        return ResultFromScode(SELFREG_E_TYPELIB);
    }
    if(!COleObjectFactoryEx::UpdateRegistryAll(FALSE)) {
        return ResultFromScode(SELFREG_E_CLASS);
    }
    return NOERROR;
}

StActiveXModule::StActiveXModule() {
    //
}

StActiveXModule::~StActiveXModule() {
    //
}

BOOL StActiveXModule::InitInstance() {
    if(!COleControlModule::InitInstance()) {
        return FALSE;
    }

    // check module is registered in system
    HKEY aKey = NULL;
    if(RegOpenKeyExW(HKEY_CLASSES_ROOT, ST_OCX_REG_KEY, 0, KEY_EXECUTE, &aKey) != ERROR_SUCCESS) {
        // If this key does not exist then this method is called during registration.
        // We must return true in this case.
        return TRUE;
    }
    RegCloseKey(aKey);

    // Initialize core library
    if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError("StCore library not available!\nMake sure you install sView correctly.");
        return FALSE;
    }

    return TRUE;
}

int StActiveXModule::ExitInstance() {
    //StCore::FREE();
    return COleControlModule::ExitInstance();
}

#endif // __WIN32__
