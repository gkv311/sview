/**
 * Copyright © 2014-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StThreads/StResourceManager.h>

#include <StThreads/StProcess.h>
#include <StFile/StFolder.h>

#include <limits>
#ifdef max
    #undef max
#endif

#if defined(_WIN32)
  #include <Shlobj.h>
#elif defined(__ANDROID__)

#include <android/asset_manager.h>
#include <android/configuration.h>

/**
 * File resource.
 */
class StAssetResource : public StResource {

        public:

    /**
     * Default constructor.
     */
    ST_LOCAL StAssetResource(const StString& theName,
                             const StString& thePath,
                             AAsset*         theAsset)
    : StResource(theName, thePath),
      myAsset(theAsset) {
        //
    }

    /**
     * Destructor.
     */
    ST_LOCAL virtual ~StAssetResource() {
        if(myAsset != NULL) {
            AAsset_close(myAsset);
        }
    }

    /**
     * This is NOT a file.
     */
    ST_LOCAL virtual bool isFile() const { return false; }

    /**
     * Read file content.
     */
    ST_LOCAL virtual bool read() {
        if(myData != NULL) {
            return true;
        } else if(myAsset == NULL) {
            return false;
        }

        myData = (uint8_t* )AAsset_getBuffer(myAsset);
        mySize = AAsset_getLength(myAsset);
        return myData != NULL;
    }

        protected:

    AAsset* myAsset; //!< handle to asset

};

#endif

StResourceManager::StResourceManager(const StString& theAppName)
: myAppName(theAppName),
  myUserHomeFolder(StProcess::getEnv(StString("HOME")) + SYS_FS_SPLITTER),
  myResFolder(StProcess::getStShareFolder()),
  myLang("en") {

#if defined(_WIN32)
    typedef HRESULT (WINAPI *SHGetKnownFolderPath_t)(const GUID* , DWORD , HANDLE , PWSTR* );
    HMODULE aShell32Module = GetModuleHandleW(L"Shell32"); // should be already loaded
    SHGetKnownFolderPath_t aGetFolder = aShell32Module != NULL
                                      ? (SHGetKnownFolderPath_t )GetProcAddress(aShell32Module, "SHGetKnownFolderPath")
                                      : (SHGetKnownFolderPath_t )NULL;
    StString anAppDataLocal, anAppDataLocalLow, anAppDataRoam;
    if(aGetFolder != NULL) {
        // Vista+
        wchar_t* aPath = NULL;

        const GUID THE_FOLDER_APPLOC = { 0xF1B32785, 0x6FBA, 0x4FCF, {0x9D, 0x55, 0x7B, 0x8E, 0x7F, 0x15, 0x70, 0x91} }; // FOLDERID_LocalAppData
        const GUID THE_FOLDER_APPLOW = { 0xA520A1A4, 0x1780, 0x4FF6, {0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16} }; // FOLDERID_LocalAppDataLow
        const GUID THE_FOLDER_APPROA = { 0x3EB685DB, 0x65F9, 0x4CF6, {0xA0, 0x3A, 0xE3, 0xEF, 0x65, 0x72, 0x9F, 0x3D} }; // FOLDERID_RoamingAppData
        const GUID THE_FOLDER_DOCS   = { 0xFDD39AD0, 0x238F, 0x46AF, {0xAD, 0xB4, 0x6C, 0x85, 0x48, 0x03, 0x69, 0xC7} }; // FOLDERID_Documents

        const GUID THE_FOLDER_DOWN   = { 0x374DE290, 0x123F, 0x4565, {0x91, 0x64, 0x39, 0xC4, 0x92, 0x5E, 0x46, 0x7B} }; // FOLDERID_Downloads
        const GUID THE_FOLDER_PICS   = { 0x33E28130, 0x4E1E, 0x4676, {0x83, 0x5A, 0x98, 0x39, 0x5C, 0x3B, 0xC3, 0xBB} }; // FOLDERID_Pictures
        const GUID THE_FOLDER_MUSIC  = { 0x4BD8D571, 0x6D19, 0x48D3, {0xBE, 0x97, 0x42, 0x22, 0x20, 0x08, 0x0E, 0x43} }; // FOLDERID_Music
        const GUID THE_FOLDER_VIDS   = { 0x18989B1D, 0x99B5, 0x455B, {0x84, 0x1C, 0xAB, 0x7C, 0x74, 0xE4, 0xDD, 0xFC} }; // FOLDERID_Videos

        if(aGetFolder(&THE_FOLDER_APPLOC, 0, NULL, &aPath) == S_OK) {
            anAppDataLocal.fromUnicode(aPath);
            ::CoTaskMemFree(aPath);
        }
        if(aGetFolder(&THE_FOLDER_APPLOW, 0, NULL, &aPath) == S_OK) {
            anAppDataLocalLow.fromUnicode(aPath);
            ::CoTaskMemFree(aPath);
        }
        if(aGetFolder(&THE_FOLDER_APPROA, 0, NULL, &aPath) == S_OK) {
            anAppDataRoam.fromUnicode(aPath);
            ::CoTaskMemFree(aPath);
        }
        if(aGetFolder(&THE_FOLDER_DOCS, 0, NULL, &aPath) == S_OK) {
            myUserHomeFolder.fromUnicode(aPath);
            myUserHomeFolder += "\\";
            ::CoTaskMemFree(aPath);
        }

        if(aGetFolder(&THE_FOLDER_DOWN, 0, NULL, &aPath) == S_OK) {
            myFolders[FolderId_Downloads].fromUnicode(aPath);
            ::CoTaskMemFree(aPath);
        }
        if(aGetFolder(&THE_FOLDER_PICS, 0, NULL, &aPath) == S_OK) {
            myFolders[FolderId_Pictures].fromUnicode(aPath);
            ::CoTaskMemFree(aPath);
        }
        if(aGetFolder(&THE_FOLDER_MUSIC, 0, NULL, &aPath) == S_OK) {
            myFolders[FolderId_Music].fromUnicode(aPath);
            ::CoTaskMemFree(aPath);
        }
        if(aGetFolder(&THE_FOLDER_VIDS, 0, NULL, &aPath) == S_OK) {
            myFolders[FolderId_Videos].fromUnicode(aPath);
            ::CoTaskMemFree(aPath);
        }
    } else {
        wchar_t aPath[MAX_PATH];
        if(::SHGetFolderPathW(NULL, CSIDL_FLAG_CREATE | CSIDL_LOCAL_APPDATA, NULL, 0, aPath) == S_OK) {
            anAppDataLocal.fromUnicode(aPath);
        }
        if(::SHGetFolderPathW(NULL, CSIDL_FLAG_CREATE | CSIDL_APPDATA, NULL, 0, aPath) == S_OK) {
            anAppDataRoam.fromUnicode(aPath);
        }
        if(::SHGetFolderPathW(NULL, CSIDL_FLAG_CREATE | CSIDL_PERSONAL, NULL, 0, aPath) == S_OK) {
            myUserHomeFolder.fromUnicode(aPath);
            myUserHomeFolder += "\\";
        }
    }

    StString anAppData;
    if(!anAppDataLocal.isEmpty()) {
        anAppData = anAppDataLocal + "/"  + myAppName + "/";
        if(!StFolder::createFolder(anAppData)) {
            anAppData.clear();
        }
    }
    if(anAppData.isEmpty() && !anAppDataLocalLow.isEmpty()) {
        anAppData = anAppDataLocalLow + "/"  + myAppName + "/";
        if(!StFolder::createFolder(anAppData)) {
            //anAppData.clear();
        }
    }
    if(!anAppData.isEmpty()) {
        myUserDataFolder = anAppData + "share/";
        mySettingsFolder = anAppData + "config/";
        myCacheFolder    = anAppData + "cache/";
    }
#elif defined(__APPLE__)
    // OS X
    myUserDataFolder = myUserHomeFolder + "Library/Application Support/" + myAppName + "/";
    mySettingsFolder = myUserHomeFolder + "Library/Preferences/"         + myAppName + "/";
    myCacheFolder    = myUserHomeFolder + "Library/Caches/"              + myAppName + "/";

    myFolders[FolderId_Downloads] = myUserHomeFolder + "Downloads";
    myFolders[FolderId_Pictures]  = myUserHomeFolder + "Pictures";
    myFolders[FolderId_Music]     = myUserHomeFolder + "Music";
    myFolders[FolderId_Videos]    = myUserHomeFolder + "Movies";

    // make sure parent paths are also exist (on broken home)
    StFolder::createFolder(myUserHomeFolder + "Library");
    StFolder::createFolder(myUserHomeFolder + "Library/Application Support");
    StFolder::createFolder(myUserHomeFolder + "Library/Preferences");
    StFolder::createFolder(myUserHomeFolder + "Library/Caches");
#else
    // Linux world
    myUserDataFolder = myUserHomeFolder + ".local/share/" + myAppName + "/";
    mySettingsFolder = myUserHomeFolder + ".config/"      + myAppName + "/";
    myCacheFolder    = myUserHomeFolder + ".cache/"       + myAppName + "/";

    myFolders[FolderId_Downloads] = myUserHomeFolder + "Downloads";
    myFolders[FolderId_Pictures]  = myUserHomeFolder + "Pictures";
    myFolders[FolderId_Music]     = myUserHomeFolder + "Music";
    myFolders[FolderId_Videos]    = myUserHomeFolder + "Videos";

    // make sure parent paths are also exist (on broken home)
    StFolder::createFolder(myUserHomeFolder + ".local");
    StFolder::createFolder(myUserHomeFolder + ".local/share");
    StFolder::createFolder(myUserHomeFolder + ".config");
    StFolder::createFolder(myUserHomeFolder + ".cache");
#endif

    StFolder::createFolder(myUserDataFolder);
    StFolder::createFolder(mySettingsFolder);
    StFolder::createFolder(myCacheFolder);

#if !defined(__ANDROID__)
    const char* aLocaleStr = ::setlocale(LC_CTYPE, NULL);
    StString    aSysLoc(aLocaleStr != NULL ? aLocaleStr : "");
    aSysLoc.toLowerCase();
    if(aSysLoc.getLength() >= 3
    && aSysLoc.getChar(2) == '-') {
        myLang = aSysLoc.subString(0, 2);
    } else if(aSysLoc.isStartsWith(stCString("russian"))) {
        myLang = "ru";
    } else if(aSysLoc.isStartsWith(stCString("french"))) {
        myLang = "fr";
    } else if(aSysLoc.isStartsWith(stCString("german"))) {
        myLang = "de";
    } else if(aSysLoc.isStartsWith(stCString("korean"))) {
        myLang = "ko";
    } else if(aSysLoc.isStartsWith(stCString("chinese"))) {
        myLang = "zh";
    } else if(aSysLoc.isStartsWith(stCString("czech"))) {
        myLang = "cs";
    }
#endif
}

#if defined(__ANDROID__)
StResourceManager::StResourceManager(AAssetManager*  theAssetMgr,
                                     const StString& theAppName)
: myAppName(theAppName),
  myResFolder(StProcess::getStShareFolder()),
  myLang("en"),
  myAssetMgr(theAssetMgr) {
    const StString aRootInt = "/data/data/";
    const StString aRootExt = "/sdcard/Android/data/";
    const StString aRoot    = StFolder::isFolder(aRootExt) ? aRootExt : aRootInt;

    myUserDataFolder = aRoot + theAppName + "/databases/";
    mySettingsFolder = aRoot + theAppName + "/shared_prefs/";
    myCacheFolder    = aRoot + theAppName + "/cache/";

    // create folders when needed
    StFolder::createFolder(aRoot + theAppName);
    StFolder::createFolder(myUserDataFolder);
    StFolder::createFolder(mySettingsFolder);
    StFolder::createFolder(myCacheFolder);

    if(myAssetMgr == NULL) {
        return;
    }

    AConfiguration* aConfig = AConfiguration_new();
    AConfiguration_fromAssetManager(aConfig, myAssetMgr);
    char aLang[3], aCountry[3];
    AConfiguration_getLanguage(aConfig, aLang);
    AConfiguration_getCountry (aConfig, aCountry);
    aLang[2]    = '\0';
    aCountry[2] = '\0';
    myLang = aLang;
    AConfiguration_delete(aConfig);
}
#endif

StResourceManager::~StResourceManager() {
    //
}

bool StResourceManager::isResourceExist(const StString& theName) const {
    const StString aPath = myResFolder + theName;
    if(StFileNode::isFileExists(aPath)) {
        return true;
    }
#if defined(__ANDROID__)
    if(myAssetMgr != NULL) {
        AAsset* anAsset = AAssetManager_open(myAssetMgr, aPath.toCString(), AASSET_MODE_UNKNOWN);
        if(anAsset != NULL) {
            AAsset_close(anAsset);
            return true;
        }
    }
#endif
    return false;
}

StHandle<StResource> StResourceManager::getResource(const StString& theName) const {
    const StString aPath = myResFolder + theName;
    if(StFileNode::isFileExists(aPath)) {
        return new StFileResource(theName, aPath);
    }
#if defined(__ANDROID__)
    if(myAssetMgr != NULL) {
        AAsset* anAsset = AAssetManager_open(myAssetMgr, aPath.toCString(), AASSET_MODE_UNKNOWN);
        if(anAsset != NULL) {
            return new StAssetResource(theName, aPath, anAsset);
        }
    }
#endif
    return StHandle<StResource>();
}

void StResourceManager::listSubFolders(const StString&        theFolder,
                                       StArrayList<StString>& theSubFolders) const {
    const StString aPath = myResFolder + theFolder;
    StFolder aFileDir(aPath);
    StArrayList<StString> anExtensions(1);
    aFileDir.init(anExtensions, 1, true);
    for(size_t aNodeId = 0; aNodeId < aFileDir.size(); ++aNodeId) {
        const StFileNode* aFileNode = aFileDir.getValue(aNodeId);
        if(aFileNode->isFolder()) {
            theSubFolders.add(aFileNode->getSubPath());
        }
    }
    if(aFileDir.size() != 0) {
        return;
    }
}

int StResourceManager::openFileDescriptor(const StString& thePath) const {
    (void )thePath;
    return -1;
}

StResource::StResource(const StString& theName,
                       const StString& thePath)
: myName(theName),
  myPath(thePath),
  myData(NULL),
  mySize(0) {
    //
}

StResource::~StResource() {
    //
}

StFileResource::StFileResource(const StString& theName,
                               const StString& thePath)
: StResource(theName, thePath) {
    //
}

StFileResource::~StFileResource() {
    //
}

bool StFileResource::read() {
    if(myData != NULL) {
        return true;
    } else if(!myFile.readFile(myPath)) {
        return false;
    } else if(myFile.getSize() > size_t(std::numeric_limits<int>::max())) {
        return false;
    }

    myData = myFile.getBuffer();
    mySize = (int )myFile.getSize();
    return true;
}
