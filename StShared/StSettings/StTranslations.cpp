/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StSettings/StTranslations.h>

#include <StSettings/StSettings.h>

#include <StFile/StFolder.h>
#include <StFile/StRawFile.h>
#include <StThreads/StProcess.h>

#include <algorithm>

namespace {
    static constexpr char ST_GLOBAL_SETTINGS_GROUP[] = "sview";
    static constexpr char ST_SETTING_LANGUAGE[]      = "language";
    static constexpr StCString ST_LNG_FILE_SUFFIX    = stCString(".lng");
}

StTranslations::StTranslations(const std::shared_ptr<StResourceManager>& theResMgr,
                               const StString& theModuleName)
: myResMgr(theResMgr),
  myModuleName(theModuleName),
  myWasReloaded(false) {
    params.language = new StEnumParam(0, stCString("language"), stCString("Language"));
    reload();
}

void StTranslations::reload() {
    params.language->changeValues().clear();

    // detect available translations
    std::vector<StString> aFolders;
    myResMgr->listSubFolders("lang", aFolders);
    for(size_t aNodeId = 0; aNodeId < aFolders.size(); ++aNodeId) {
        myLangFolderList.push_back(aFolders[aNodeId]);

        const StString aNameFile = StString("lang" ST_FILE_SPLITTER) + aFolders[aNodeId] + ST_FILE_SPLITTER "language.lng";
        std::shared_ptr<StResource> aRes = myResMgr->getResource(aNameFile);
        StString aName;
        if (aRes.get() != nullptr && aRes->read()) {
            const char*  aSrc = (const char* )aRes->getData();
            const size_t aLen = (size_t      )aRes->getSize();
            aName = StString(aSrc, aLen);
        }
        params.language->changeValues().push_back(aName.isEmpty() ? aFolders[aNodeId] : aName);
    }

#if defined(__ANDROID__)
    if (params.language->getValues().empty()) {
        // no way to list sub-folder on Android - check known translations
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "English" ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("English");
            myLangFolderList.push_back("English");
        }
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "Spanish" ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("Español");
            myLangFolderList.push_back("Spanish");
        }
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "Russian" ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("русский");
            myLangFolderList.push_back("Russian");
        }
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "French" ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("français");
            myLangFolderList.push_back("French");
        }
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "German" ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("Deutsch");
            myLangFolderList.push_back("German");
        }
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "Korean" ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("한국어");
            myLangFolderList.push_back("Korean");
        }
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "ChineseS" ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("简体中文");
            myLangFolderList.push_back("ChineseS");
        }
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "ChineseT" ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("正體中文 (臺灣)");
            myLangFolderList.push_back("ChineseT");
        }
        if (myResMgr->isResourceExist(StString("lang" ST_FILE_SPLITTER "Czech"  ST_FILE_SPLITTER) + myModuleName + ST_LNG_FILE_SUFFIX)) {
            params.language->changeValues().push_back("Čeština");
            myLangFolderList.push_back("Czech");
        }
    }
#endif

    if (params.language->getValues().empty()) {
        // add built-in language
        params.language->changeValues().push_back("English");
        myLangFolderList.push_back("English");
    }

    const auto isInLangFolderList = [this](const StString& theLang, size_t& theIndex) -> bool {
        const auto aPosInList = std::find(myLangFolderList.begin(), myLangFolderList.end(), theLang);
        if (aPosInList == myLangFolderList.end()) {
            return false;
        }
        theIndex = aPosInList - myLangFolderList.begin();
        return true;
    };

    size_t     anIdInList = 0;
    StString   aLangParam("English");
    StSettings aGlobalSettings(myResMgr, ST_GLOBAL_SETTINGS_GROUP);
    bool isLangSet = false;
    if (!aGlobalSettings.loadString(ST_SETTING_LANGUAGE, aLangParam)) {
        // try to use system-wide language settings
        const StString& aLang = myResMgr->getSystemLanguage();
        if (aLang.isEqualsIgnoreCase(stCString("ru"))) {
            if (isInLangFolderList(stCString("Russian"),  anIdInList)
             || isInLangFolderList(stCString("русский"),  anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if (aLang.isEqualsIgnoreCase(stCString("de"))) {
            if (isInLangFolderList(stCString("German"),   anIdInList)
             || isInLangFolderList(stCString("Deutsch"),  anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if (aLang.isEqualsIgnoreCase(stCString("es"))) {
            if (isInLangFolderList(stCString("Spanish"), anIdInList)
             || isInLangFolderList(stCString("Español"), anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if (aLang.isEqualsIgnoreCase(stCString("fr"))) {
            if (isInLangFolderList(stCString("French"),   anIdInList)
             || isInLangFolderList(stCString("français"), anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if (aLang.isEqualsIgnoreCase(stCString("ko"))) {
            if (isInLangFolderList(stCString("Korean"),   anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if (aLang.isEqualsIgnoreCase(stCString("zh"))) {
            if (isInLangFolderList(stCString("ChineseS"), anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if (aLang.isEqualsIgnoreCase(stCString("zh-tw"))) {
            if (isInLangFolderList(stCString("ChineseT"), anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        } else if (aLang.isEqualsIgnoreCase(stCString("cs"))) {
            if (isInLangFolderList(stCString("Čeština"), anIdInList)) {
                params.language->setValue(int32_t(anIdInList));
                isLangSet = true;
            }
        }
    }
    if (!isLangSet) {
        if (isInLangFolderList(aLangParam,           anIdInList)
         || isInLangFolderList(stCString("English"), anIdInList)) {
            params.language->setValue(int32_t(anIdInList));
        }
    }
    updateLangCode(int32_t(anIdInList));

    const StString& aFolderName = myLangFolderList[anIdInList];
    const StString  aResName    = StString()
                                + "lang"       + SYS_FS_SPLITTER
                                + aFolderName  + SYS_FS_SPLITTER
                                + myModuleName + ST_LNG_FILE_SUFFIX;
    std::shared_ptr<StResource> aRes = myResMgr->getResource(aResName);
    if (aRes.get() != nullptr && aRes->read()) {
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
    } else if(aLang == stCString("Español")) {
        myLangCode = "spa";
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
                            + myModuleName + ST_LNG_FILE_SUFFIX;
    std::shared_ptr<StResource> aRes = myResMgr->getResource(aResName);
    if (aRes.get() != nullptr && aRes->read()) {
        const char* aSrc = (const char* )aRes->getData();
        const int   aLen = aRes->getSize();
        read(aSrc, aLen);
    }
    myWasReloaded = true;
}
