/**
 * Copyright © 2013-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StFT/StFTFontRegistry.h>

#include <StFile/StFolder.h>
#include <StStrings/StLogger.h>
#include <StThreads/StProcess.h>
#include <stAssert.h>

namespace {
    const StFTFontFamily THE_NO_FAMILY;
};

StFTFontRegistry::StFTFontRegistry() {
    myFTLib = new StFTLibrary();
    myExtensions.add("ttf");
    myExtensions.add("ttc");
    myExtensions.add("otf");

#ifdef _WIN32
    myFolders.add(StProcess::getWindowsFolder() + "fonts");

    // western
    myFilesMajor.add(stCString("times.ttf"));
    myFilesMajor.add(stCString("timesbd.ttf"));
    myFilesMajor.add(stCString("timesi.ttf"));
    myFilesMajor.add(stCString("timesbi.ttf"));
    myFilesMajor.add(stCString("trebuc.ttf"));
    myFilesMajor.add(stCString("trebucbd.ttf"));
    myFilesMajor.add(stCString("trebucit.ttf"));
    myFilesMajor.add(stCString("trebucbi.ttf"));
    myFilesMajor.add(stCString("tahoma.ttf"));
    myFilesMajor.add(stCString("tahomabd.ttf"));
    myFilesMajor.add(stCString("micross.ttf"));
    // korean
    myFilesMajor.add(stCString("malgun.ttf"));
    myFilesMajor.add(stCString("malgunbd.ttf"));
    myFilesMinor.add(stCString("gulim.ttc")); // Win XP
    myFilesMinor.add(stCString("gulim.ttf"));
    // chinese
    myFilesMajor.add(stCString("simsun.ttc"));
#elif defined(__APPLE__)
    myExtensions.add("dfont");
    myFolders.add(stCString("/System/Library/Fonts"));
    myFolders.add(stCString("/Library/Fonts"));

    // western
    myFilesMajor.add(stCString("Times.dfont"));
    myFilesMajor.add(stCString("Times New Roman.ttf"));
    myFilesMajor.add(stCString("Times New Roman Bold.ttf"));
    myFilesMajor.add(stCString("Times New Roman Italic.ttf"));
    myFilesMajor.add(stCString("Times New Roman Bold Italic.ttf"));
    myFilesMajor.add(stCString("Trebuchet MS.ttf"));
    myFilesMajor.add(stCString("Trebuchet MS Bold.ttf"));
    myFilesMajor.add(stCString("Trebuchet MS Italic.ttf"));
    myFilesMajor.add(stCString("Trebuchet MS Bold Italic.ttf"));
    myFilesMajor.add(stCString("Tahoma.ttf"));
    myFilesMajor.add(stCString("Tahoma Bold.ttf"));
    myFilesMajor.add(stCString("Monaco.dfont"));
    // korean
    //myFilesMajor.add(stCString("AppleMyungjo.ttf"));
    myFilesMajor.add(stCString("AppleGothic.ttf"));
    // chinese
    myFilesMajor.add(stCString("华文仿宋.ttf"));
#elif defined(__ANDROID__)
    myFolders.add(stCString("/system/fonts"));

    // western

    // Android 6
    myFilesMajor.add(stCString("NotoSerif-Regular.ttf"));
    myFilesMajor.add(stCString("NotoSerif-Bold.ttf"));
    myFilesMajor.add(stCString("NotoSerif-Italic.ttf"));
    myFilesMajor.add(stCString("NotoSerif-BoldItalic.ttf"));

    // Android 4
    myFilesMinor.add(stCString("DroidSerif-Regular.ttf"));
    myFilesMinor.add(stCString("DroidSerif-Bold.ttf"));
    myFilesMinor.add(stCString("DroidSerif-Italic.ttf"));
    myFilesMinor.add(stCString("DroidSerif-BoldItalic.ttf"));

    myFilesMajor.add(stCString("DroidSans.ttf"));
    myFilesMajor.add(stCString("DroidSans-Bold.ttf"));
    myFilesMajor.add(stCString("DroidSansMono.ttf"));

    // not all phones have the following fonts

    // Korean
    myFilesMinor.add(stCString("NanumGothic.ttf"));
    myFilesMinor.add(stCString("NotoSansKR-Regular.otf"));

    // Simplified Chinese
    myFilesMinor.add(stCString("DroidSansFallback.ttf"));
    myFilesMinor.add(stCString("NotoSansSC-Regular.otf"));

    // Traditional Chinese
    //myFilesMinor.add(stCString("NotoSansTC-Regular.otf"));

    // Japanese
    //myFilesMinor.add(stCString("NotoSansJP-Regular.otf"));
#else
    myFolders.add(stCString("/usr/share/fonts"));
    myFolders.add(stCString("/usr/local/share/fonts"));

    // western
    myFilesMajor.add(stCString("DejaVuSerif.ttf"));
    myFilesMajor.add(stCString("DejaVuSerif-Bold.ttf"));
    myFilesMajor.add(stCString("DejaVuSans.ttf"));
    myFilesMajor.add(stCString("DejaVuSans-Bold.ttf"));
    myFilesMajor.add(stCString("DejaVuSansMono.ttf"));
    myFilesMajor.add(stCString("DejaVuSansMono-Bold.ttf"));

    myFilesMajor.add(stCString("FreeSerif.ttf"));
    myFilesMajor.add(stCString("FreeSerifBold.ttf"));
    myFilesMajor.add(stCString("FreeSerifItalic.ttf"));
    myFilesMajor.add(stCString("FreeSerifBoldItalic.ttf"));
    myFilesMajor.add(stCString("FreeSans.ttf"));
    myFilesMajor.add(stCString("FreeSansBold.ttf"));
    myFilesMajor.add(stCString("FreeSansOblique.ttf"));
    myFilesMajor.add(stCString("FreeSansBoldOblique.ttf"));

    // korean
    myFilesMajor.add(stCString("NanumMyeongjo.ttf"));
    myFilesMajor.add(stCString("NanumMyeongjoBold.ttf"));
    myFilesMajor.add(stCString("NanumGothic.ttf"));
    myFilesMajor.add(stCString("NanumGothicBold.ttf"));

    // chinese
    //myFilesMajor.add(stCString("DroidSansJapanese.ttf"));
    myFilesMajor.add(stCString("DroidSansFallbackFull.ttf"));
#endif
}

StFTFontRegistry::~StFTFontRegistry() {
    //
}

void StFTFontRegistry::appendSearchPath(const StString& theFolder) {
    myFolders.add(theFolder);
}

void StFTFontRegistry::searchFiles(const StArrayList<StString>& theNames,
                                   const bool                   theIsMajor) {
    for(size_t aNameIter = 0; aNameIter < theNames.size(); ++aNameIter) {
        const StString& aName = theNames.getValue(aNameIter);
        StString aPath;
        if(StFileNode::isAbsolutePath(aName)) {
            aPath = aName;
        } else {
            const StFileNode* aNode = myFoldersRoot.findValue(aName);
            if(aNode != NULL) {
                aPath = aNode->getPath();
            }
        }
        if(!StFileNode::isFileExists(aPath)) {
            if(theIsMajor) {
                ST_ERROR_LOG("StFTFontRegistry, major font file '" + aName + "' does not exist!");
            }
            continue;
        }

        FT_Face aFace = NULL;
        if(FT_New_Face(myFTLib->getInstance(), aPath.toCString(), 0, &aFace) != 0) {
            if(theIsMajor) {
                ST_ERROR_LOG("StFTFontRegistry, major font file '" + aName + "' fail to load"
                            + " from path '" + aPath + "'!");
            }
            if(aFace != NULL) {
                FT_Done_Face(aFace);
            }
            continue;
        }

        StFTFontFamily& aFamily = myFonts[aFace->family_name];
        aFamily.FamilyName = aFace->family_name;
        if(aFace->style_flags == (FT_STYLE_FLAG_ITALIC | FT_STYLE_FLAG_BOLD)) {
            aFamily.BoldItalic = aPath;
        } else if(aFace->style_flags == FT_STYLE_FLAG_BOLD) {
            aFamily.Bold = aPath;
        } else if(aFace->style_flags == FT_STYLE_FLAG_ITALIC) {
            aFamily.Italic = aPath;
        } else {
            aFamily.Regular = aPath;
        }
        //ST_DEBUG_LOG("StFTFontRegistry, font file '" + aName + "', family '" + aFamily.FamilyName + "', contains " + aFace->num_glyphs + " glyphs!");

        FT_Done_Face(aFace);
    }
}

void StFTFontRegistry::init(const bool theToSearchAll) {
    myFoldersRoot.clear();
    myFonts.clear();
    for(size_t aFolderIter = 0; aFolderIter < myFolders.size(); ++aFolderIter) {
        StFolder* aSubFolder = new StFolder(myFolders.getValue(aFolderIter), &myFoldersRoot);
        aSubFolder->init(myExtensions, 4);
        myFoldersRoot.add(aSubFolder);
    }

    searchFiles(myFilesMajor, true);
    searchFiles(myFilesMinor, false);

    if(theToSearchAll) {
        //
    }

    StFTFontPack& aSerif = myTypefaces[StFTFont::Typeface_Serif];
    StFTFontPack& aSans  = myTypefaces[StFTFont::Typeface_SansSerif];
    StFTFontPack& aMono  = myTypefaces[StFTFont::Typeface_Monospace];
#ifdef _WIN32
    aSerif.Western = findFont(stCString("Times New Roman"));
    aSans .Western = findFont(stCString("Trebuchet MS"));
    aMono .Western = findFont(stCString("Tahoma"));
    const StFTFontFamily& aMalgun = findFont(stCString("Malgun Gothic"));
    const StFTFontFamily& aGulim  = findFont(stCString("Gulim"));
    const StFTFontFamily& aKor = aMalgun.FamilyName.isEmpty() && !aGulim.FamilyName.isEmpty()
                               ? aGulim
                               : aMalgun;
    const StFTFontFamily& anArabic = findFont(stCString("Times New Roman"));
    aSerif.Arabic  = anArabic;
    aSans .Arabic  = anArabic;
    aMono .Arabic  = anArabic;
    aSerif.Korean  = aKor;
    aSans .Korean  = aKor;
    aMono .Korean  = aKor;
    const StFTFontFamily& aSimSun  = findFont(stCString("SimSun"));
    aSerif.CJK     = aSimSun;
    aSans .CJK     = aSimSun;
    aMono .CJK     = aSimSun;
#elif defined(__APPLE__)
    aSerif.Western = findFont(stCString("Times New Roman"));
    aSans .Western = findFont(stCString("Trebuchet MS"));
    aMono .Western = findFont(stCString("Monaco"));
    aSerif.Korean  = findFont(stCString("AppleGothic")); // AppleMyungjo can not be loaded
    aSans .Korean  = findFont(stCString("AppleGothic"));
    aMono .Korean  = findFont(stCString("AppleGothic"));
    aSerif.CJK     = findFont(stCString("STFangsong"));
    aSans .CJK     = findFont(stCString("STFangsong"));
    aMono .CJK     = findFont(stCString("STFangsong"));
    aSerif.Arabic  = findFont(stCString("DecoType Naskh"));
    aSans .Arabic  = findFont(stCString("DecoType Naskh"));
    aMono .Arabic  = findFont(stCString("DecoType Naskh"));
#elif defined(__ANDROID__)
    aSerif.Western = findFont(stCString("Noto Serif"));
    if(aSerif.Western.FamilyName.isEmpty()) {
        aSerif.Western = findFont(stCString("Droid Serif"));
    }
    aSans .Western = findFont(stCString("Roboto")); // actually DroidSans.ttf
    aMono .Western = findFont(stCString("Droid Sans Mono"));
    aSerif.Korean  = findFont(stCString("NanumGothic")); // no serif
    aSans .Korean  = findFont(stCString("NanumGothic"));
    aMono .Korean  = findFont(stCString("NanumGothic"));
    if(aSerif.Korean.FamilyName.isEmpty()) {
        aSerif.Korean = findFont(stCString("Noto Sans KR"));
        aSans .Korean = findFont(stCString("Noto Sans KR"));
        aMono .Korean = findFont(stCString("Noto Sans KR"));
    }
    aSerif.CJK     = findFont(stCString("Droid Sans Fallback"));
    aSans .CJK     = findFont(stCString("Droid Sans Fallback"));
    aMono .CJK     = findFont(stCString("Droid Sans Fallback"));
    if(aSerif.CJK.FamilyName.isEmpty()) {
        aSerif.CJK = findFont(stCString("Noto Sans SC"));
        aSans .CJK = findFont(stCString("Noto Sans SC"));
        aMono .CJK = findFont(stCString("Noto Sans SC"));
    }

    aSerif.Arabic = findFont(stCString("Droid Arabic Naskh"));
    aSans .Arabic = findFont(stCString("Droid Arabic Naskh"));
    aMono .Arabic = findFont(stCString("Droid Arabic Naskh"));
    if(aSerif.Arabic.FamilyName.isEmpty()) {
        aSerif.Arabic = findFont(stCString("Noto Naskh Arabic"));
        aSans .Arabic = findFont(stCString("Noto Naskh Arabic"));
        aMono .Arabic = findFont(stCString("Noto Naskh Arabic"));
    }
#else
    aSerif.Western = findFont(stCString("FreeSerif"));
    aSans .Western = findFont(stCString("FreeSans"));
    if(aSerif.Western.FamilyName.isEmpty()) {
        aSerif.Western = findFont(stCString("DejaVu Serif"));
        aSans .Western = findFont(stCString("DejaVu Sans"));
    }
    aMono .Western = findFont(stCString("DejaVu Sans Mono"));
    aSerif.Korean  = findFont(stCString("NanumMyeongjo"));
    aSans .Korean  = findFont(stCString("NanumGothic"));
    aMono .Korean  = findFont(stCString("NanumGothic"));
    aSerif.CJK     = findFont(stCString("Droid Sans Fallback"));
    aSans .CJK     = findFont(stCString("Droid Sans Fallback"));
    aMono .CJK     = findFont(stCString("Droid Sans Fallback"));
#endif
}

const StFTFontFamily& StFTFontRegistry::findFont(const StCString& theFamily) {
    std::map<StString, StFTFontFamily>::iterator anIter = myFonts.find(theFamily.toCString());
    return anIter != myFonts.end()
         ? anIter->second
         : THE_NO_FAMILY;
}
