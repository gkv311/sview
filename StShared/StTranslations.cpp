/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StTranslations.h>

#include <StSettings/StSettings.h>

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
    params.language = new StEnumParam(0, "Language");

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
        params.language->changeValues().add(aName.isEmpty() ? aFolders[aNodeId] : aName);
    }
#if defined(__ANDROID__)
    if(params.language->getValues().isEmpty()) {
        // no way to list sub-folder on Android - check known translations
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "English" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            params.language->changeValues().add("English");
            myLangFolderList.add("English");
        }
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "Russian" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            params.language->changeValues().add("русский");
            myLangFolderList.add("Russian");
        }
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "French" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            params.language->changeValues().add("français");
            myLangFolderList.add("French");
        }
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "German" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            params.language->changeValues().add("Deutsch");
            myLangFolderList.add("German");
        }
        if(myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "Korean" ST_FILE_SPLITTER) + myModuleName + StTranslations::DEFAULT_SUFFIX)) {
            params.language->changeValues().add("한국어");
            myLangFolderList.add("Korean");
        }
    }
#endif

    if(params.language->getValues().isEmpty()) {
        // add built-in language
        params.language->changeValues().add("English");
        myLangFolderList.add("English");
    }

    size_t     anIdInList = 0;
    StString   aLangParam("English");
    StSettings aGlobalSettings(myResMgr, ST_GLOBAL_SETTINGS_GROUP);
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
        } else if(aLang.isEqualsIgnoreCase(stCString("zh"))) {
            if(myLangFolderList.contains(stCString("ChineseS"), anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if(aLang.isEqualsIgnoreCase(stCString("cs"))) {
            if(myLangFolderList.contains(stCString("Čeština"), anIdInList)) {
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
    updateLangCode(int32_t(anIdInList));

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
    return params.language->getValues()[params.language->getValue()];
}

void StTranslations::updateLangCode(const int32_t theNewLang) {
    const StString& aLang = params.language->getValues()[theNewLang];
    if(aLang == stCString("русский")) {
        myLangCode = "rus";
    } else if(aLang == stCString("français")) {
        myLangCode = "fre";
        //myLangCode = "fra";
    } else if(aLang == stCString("Deutsch")) {
        myLangCode = "ger";
        //myLangCode = "deu";
    } else if(aLang == stCString("한국어")) {
        myLangCode = "kor";
    } else if(aLang == stCString("简体中文")) {
        myLangCode = "chi";
    } else if(aLang == stCString("Čeština")) {
        myLangCode = "cze";
    } else if(aLang == stCString("English")) {
        myLangCode = "eng";
    } else {
        myLangCode.clear();
    }
}

void StTranslations::setLanguage(const int32_t theNewLang) {
    if(size_t(theNewLang) >= params.language->getValues().size()) {
        return;
    }

    // save global setting
    const StString& aFolderName = myLangFolderList[theNewLang];
    StSettings aGlobalSettings(myResMgr, ST_GLOBAL_SETTINGS_GROUP);
    aGlobalSettings.saveString(ST_SETTING_LANGUAGE, aFolderName);

    updateLangCode(theNewLang);

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
