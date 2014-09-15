/**
 * Copyright © 2010-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StTranslations.h>

#include <StFile/StFolder.h>
#include <StFile/StRawFile.h>
#include <StThreads/StProcess.h>

namespace {
    static const char ST_GLOBAL_SETTINGS_GROUP[] = "sview";
    static const char ST_SETTING_LANGUAGE[]      = "language";
}

const StString StTranslations::DEFAULT_EXTENSION =  "lng";
const StString StTranslations::DEFAULT_SUFFIX    = ".lng";

StTranslations::StTranslations(const StHandle<StResourceManager>& theResMgr,
                               const StString&                    theModuleName)
: myResMgr(theResMgr),
  myModuleName(theModuleName),
  myWasReloaded(false) {
    params.language = new StInt32Param(0);

    // detect available translations
    StArrayList<StString> aFolders;
    myResMgr->listSubFolders("lang", aFolders);
    for(size_t aNodeId = 0; aNodeId < aFolders.size(); ++aNodeId) {
        myLangFolderList.add(aFolders[aNodeId]);

        const StString aNameFile = StString("lang" ST_FILE_SPLITTER) + aFolders[aNodeId] + ST_FILE_SPLITTER "language.lng";
        StHandle<StResource> aRes = myResMgr->getResource(aNameFile);
        StString aName;
        if(!aRes.isNull()
        &&  aRes->read()) {
            const char*  aSrc = (const char* )aRes->getData();
            const size_t aLen = (size_t      )aRes->getSize();
            aName = StString(aSrc, aLen);
        }
        myLangList.add(aName.isEmpty() ? aFolders[aNodeId] : aName);
    }
#if defined(__ANDROID__)
    if(myLangList.isEmpty()) {
        // no way to list sub-folder on Android - check known translations
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "English" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            myLangList.add("English");
            myLangFolderList.add("English");
        }
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "Russian" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            myLangList.add("русский");
            myLangFolderList.add("Russian");
        }
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "French" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            myLangList.add("français");
            myLangFolderList.add("French");
        }
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "German" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            myLangList.add("Deutsch");
            myLangFolderList.add("German");
        }
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "Korean" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            myLangList.add("한국어");
            myLangFolderList.add("Korean");
        }
    }
#endif

    if(myLangList.isEmpty()) {
        // add built-in language
        myLangList.add("English");
        myLangFolderList.add("English");
    }

    size_t     anIdInList = 0;
    StString   aLangParam("English");
    StSettings aGlobalSettings(ST_GLOBAL_SETTINGS_GROUP);
    bool isLangSet = false;
    if(!aGlobalSettings.loadString(ST_SETTING_LANGUAGE, aLangParam)) {
        // try to use system-wide language settings
        const StString& aLang = myResMgr->getSystemLanguage();
        if(aLang.isEqualsIgnoreCase(stCString("ru"))) {
            if(myLangFolderList.contains(stCString("Russian"),  anIdInList)
            || myLangFolderList.contains(stCString("русский"),  anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if(aLang.isEqualsIgnoreCase(stCString("de"))) {
            if(myLangFolderList.contains(stCString("German"),   anIdInList)
            || myLangFolderList.contains(stCString("Deutsch"),  anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if(aLang.isEqualsIgnoreCase(stCString("fr"))) {
            if(myLangFolderList.contains(stCString("French"),   anIdInList)
            || myLangFolderList.contains(stCString("français"), anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if(aLang.isEqualsIgnoreCase(stCString("ko"))) {
            if(myLangFolderList.contains(stCString("Korean"),   anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        }
    }
    if(!isLangSet) {
        if(myLangFolderList.contains(aLangParam,           anIdInList)
        || myLangFolderList.contains(stCString("English"), anIdInList)) {
            params.language->setValue(int32_t(anIdInList));
        }
    }

    const StString& aFolderName = myLangFolderList[anIdInList];
    const StString  aResName    = StString()
                                + "lang"       + SYS_FS_SPLITTER
                                + aFolderName  + SYS_FS_SPLITTER
                                + myModuleName + StTranslations::DEFAULT_SUFFIX;
    StHandle<StResource> aRes = myResMgr->getResource(aResName);
    if(!aRes.isNull()
    &&  aRes->read()) {
        const char* aSrc = (const char* )aRes->getData();
        const int   aLen = aRes->getSize();
        read(aSrc, aLen);
    }

    // connect signal
    params.language->signals.onChanged.connect(this, &StTranslations::setLanguage);
}

StTranslations::~StTranslations() {
    //
}

const StString& StTranslations::getLanguage() const {
    return myLangList[params.language->getValue()];
}

void StTranslations::setLanguage(const int32_t theNewLang) {
    // save global setting
    if(size_t(theNewLang) >= myLangList.size()) {
        return;
    }
    const StString& aFolderName = myLangFolderList[theNewLang];
    StSettings aGlobalSettings(ST_GLOBAL_SETTINGS_GROUP);
    aGlobalSettings.saveString(ST_SETTING_LANGUAGE, aFolderName);

    // reload translation file
    StLangMap::clear();

    const StString aResName = StString()
                            + "lang" ST_FILE_SPLITTER
                            + aFolderName  + SYS_FS_SPLITTER
                            + myModuleName + StTranslations::DEFAULT_SUFFIX;
    StHandle<StResource> aRes = myResMgr->getResource(aResName);
    if(!aRes.isNull()
    &&  aRes->read()) {
        const char* aSrc = (const char* )aRes->getData();
        const int   aLen = aRes->getSize();
        read(aSrc, aLen);
    }
    myWasReloaded = true;
}
