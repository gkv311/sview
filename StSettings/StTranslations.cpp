/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StTranslations.h>

namespace {
    static const char ST_GLOBAL_SETTINGS_GROUP[] = "sview";
    static const char ST_SETTING_LANGUAGE[]      = "language";
};

const StString StTranslations::DEFAULT_EXTENSION =  "lng";
const StString StTranslations::DEFAULT_SUFFIX    = ".lng";

StTranslations::StTranslations(const StString& theModuleName)
: StLangMap(),
  myModuleName(theModuleName),
  myLangList(),
  myWasReloaded(false) {
    params.language = new StInt32Param(0);

    // detect available translations
    StFolder stFolder(StProcess::getStShareFolder() + "lang" + SYS_FS_SPLITTER);
    StArrayList<StString> anExtensions(1);
    anExtensions.add(StTranslations::DEFAULT_EXTENSION);
    stFolder.init(anExtensions, 2);
    for(size_t nodeId = 0; nodeId < stFolder.size(); ++nodeId) {
        StFileNode* subFileNode = stFolder.changeValue(nodeId);
        if(subFileNode->isFolder()) {
            myLangList.add(subFileNode->getSubPath());
        }
    }

    if(myLangList.isEmpty()) {
        // add built-in language
        myLangList.add("English");
    }

    StString aLang("English");
    StSettings aGlobalSettings(ST_GLOBAL_SETTINGS_GROUP);
    aGlobalSettings.loadString(ST_SETTING_LANGUAGE, aLang);

    size_t anIdInList = 0;
    if(myLangList.contains(aLang, anIdInList)) {
        params.language->setValue(int32_t(anIdInList));
    }

    StLangMap::open(StProcess::getStShareFolder()
                  + "lang"       + SYS_FS_SPLITTER
                  + aLang        + SYS_FS_SPLITTER
                  + myModuleName + StTranslations::DEFAULT_SUFFIX);

    // connect signal
    params.language->signals.onChanged.connect(this, &StTranslations::setLanguage);
}

StTranslations::~StTranslations() {
    //
}

StString StTranslations::getLanguage() const {
    return myLangList[params.language->getValue()];
}

void StTranslations::setLanguage(const int32_t theNewLang) {
    // save global setting
    if(size_t(theNewLang) >= myLangList.size()) {
        return;
    }
    StString aLang = myLangList[theNewLang];
    StSettings aGlobalSettings(ST_GLOBAL_SETTINGS_GROUP);
    aGlobalSettings.saveString(ST_SETTING_LANGUAGE, aLang);

    // reload translation file
    StLangMap::clear();
    StLangMap::open(StProcess::getStShareFolder()
                  + "lang"       + SYS_FS_SPLITTER
                  + aLang        + SYS_FS_SPLITTER
                  + myModuleName + StTranslations::DEFAULT_SUFFIX);
    myWasReloaded = true;
}
