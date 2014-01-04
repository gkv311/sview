/**
 * Copyright © 2010-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StTranslations.h>

#include <StFile/StRawFile.h>

namespace {
    static const char ST_GLOBAL_SETTINGS_GROUP[] = "sview";
    static const char ST_SETTING_LANGUAGE[]      = "language";
};

const StString StTranslations::DEFAULT_EXTENSION =  "lng";
const StString StTranslations::DEFAULT_SUFFIX    = ".lng";

StTranslations::StTranslations(const StString& theModuleName)
: myModuleName(theModuleName),
  myWasReloaded(false) {
    params.language = new StInt32Param(0);

    // detect available translations
    StFolder aLangRoot(StProcess::getStShareFolder() + "lang" + SYS_FS_SPLITTER);
    StArrayList<StString> anExtensions(1);
    anExtensions.add(StTranslations::DEFAULT_EXTENSION);
    aLangRoot.init(anExtensions, 2);
    for(size_t aNodeId = 0; aNodeId < aLangRoot.size(); ++aNodeId) {
        const StFileNode* aFileNode = aLangRoot.getValue(aNodeId);
        if(!aFileNode->isFolder()) {
            continue;
        }

        myLangFolderList.add(aFileNode->getSubPath());
        const StString aName = StRawFile::readTextFile(aFileNode->getPath() + SYS_FS_SPLITTER + "language.lng");
        myLangList.add(aName.isEmpty() ? aFileNode->getSubPath() : aName);
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
    StLangMap::open(StProcess::getStShareFolder()
                  + "lang"       + SYS_FS_SPLITTER
                  + aFolderName  + SYS_FS_SPLITTER
                  + myModuleName + StTranslations::DEFAULT_SUFFIX);

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
    StLangMap::open(StProcess::getStShareFolder()
                  + "lang"       + SYS_FS_SPLITTER
                  + aFolderName  + SYS_FS_SPLITTER
                  + myModuleName + StTranslations::DEFAULT_SUFFIX);
    myWasReloaded = true;
}
