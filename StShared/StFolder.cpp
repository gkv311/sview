/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StFile/StFolder.h>
#include <StStrings/StLogger.h>

#if (defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <dirent.h>
#endif

namespace {
    static const StString IGNORE_DIR_CURR_NAME('.');
    static const StString IGNORE_DIR_UP_NAME("..");
};

StFolder::StFolder(const StString& theFolderPath, StNode* theParentNode)
: StFileNode(theFolderPath, theParentNode, NODE_TYPE_FOLDER) {
    //
}

StFolder::~StFolder() {
    //
}

bool StFolder::isFolder(const StString& thePath) {
#if (defined(_WIN32) || defined(__WIN32__))
    DWORD aFileAttributes = GetFileAttributesW(thePath.toUtfWide().toCString());
    if(aFileAttributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    if((aFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return true;
    }
    return false;
#else
    DIR* aDir = opendir(thePath.toCString());
    if(aDir == NULL) {
        return false;
    }
    closedir(aDir);
    return true;
#endif
}

void StFolder::addItem(const StArrayList<StString>& theExtensions,
                       int theDeep,
                       const StString& theSearchFolderPath,
                       const StString& theCurrentItemName) {
    if(theCurrentItemName == IGNORE_DIR_CURR_NAME || theCurrentItemName == IGNORE_DIR_UP_NAME) {
        return;
    }

    StString aCurrItemFullName = theSearchFolderPath + SYS_FS_SPLITTER + theCurrentItemName;
    if(isFolder(aCurrItemFullName)) {
        if(theDeep > 1) {
            StFolder* aSubFolder = new StFolder(theCurrentItemName, this);
            aSubFolder->init(theExtensions, theDeep - 1);
            if(aSubFolder->size() > 0) {
                add(aSubFolder);
            } else {
                // ignore empty folders
                delete aSubFolder;
            }
        }
    } else {
        StString anItemExtension = StFileNode::getExtension(theCurrentItemName);
        for(size_t anExt = 0; anExt < theExtensions.size(); ++anExt) {
            if(anItemExtension.isEqualsIgnoreCase(theExtensions[anExt])) {
                add(new StFileNode(theCurrentItemName, this));
                break;
            }
        }
    }
}

void StFolder::init(const StArrayList<StString>& theExtensions, int theDeep) {
    // clean up old list...
    clear();
    StString aSearchFolderPath = getPath();
#if (defined(_WIN32) || defined(__WIN32__))
    WIN32_FIND_DATAW aFindFile;
    StString aStrSearchMask = getPath() + StString(SYS_FS_SPLITTER) + '*';

    HANDLE hFind = FindFirstFileW(aStrSearchMask.toUtfWide().toCString(), &aFindFile);
    for(BOOL hasFile = (hFind != INVALID_HANDLE_VALUE); hasFile == TRUE;
        hasFile = FindNextFileW(hFind, &aFindFile)) {
        //
        StString aCurrItemName(aFindFile.cFileName);
        addItem(theExtensions, theDeep, aSearchFolderPath, aCurrItemName);
    }
    FindClose(hFind);
#else
    DIR* aSearchedFolder = opendir(aSearchFolderPath.toCString());
    if(aSearchedFolder == NULL) {
        return;
    }
    for(dirent* aDirItem = readdir(aSearchedFolder); aDirItem != NULL;
        aDirItem = readdir(aSearchedFolder)) {
        //
    #if (defined(__APPLE__))
        // automatically convert filenames from decomposed form used by Mac OS X file systems
        StString aCurrItemName = stFromUtf8Mac(aDirItem->d_name);
    #else
        StString aCurrItemName(aDirItem->d_name);
    #endif
        addItem(theExtensions, theDeep, aSearchFolderPath, aCurrItemName);
    }
    closedir(aSearchedFolder);
#endif
    // perform sorting...
    sort();
}
