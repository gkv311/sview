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

    if(myLangList.isEmpty()) {
        // add built-in language
        myLangList.add("English");
        myLangFolderList.add("English");
    }

    StString aLangParam("English");
    StSettings aGlobalSettings(ST_GLOBAL_SETTINGS_GROUP);
    aGlobalSettings.loadString(ST_SETTING_LANGUAGE, aLangParam);

    size_t anIdInList = 0;
    if(myLangFolderList.contains(aLangParam,           anIdInList)
    || myLangFolderList.contains(stCString("English"), anIdInList)) {
        params.language->setValue(int32_t(anIdInList));
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
