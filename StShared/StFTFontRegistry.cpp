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

#if !defined(_WIN32) && !defined(__ANDROID__) && !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
  // use fontconfig library on Linux
  #include <fontconfig/fontconfig.h>
#endif

namespace {
    static const StFTFontFamily THE_NO_FAMILY;
}

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
    myFilesMinor.add(stCString("Times.dfont")); // old macOS
    myFilesMajor.add(stCString("Times.ttc"));
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
    myFilesMinor.add(stCString("华文仿宋.ttf")); // old macOS
    myFilesMajor.add(stCString("Songti.ttc"));
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
    myFilesMinor.add(stCString("NotoSansCJK-Regular.ttc"));

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

    // Arabic
    myFilesMinor.add(stCString("NotoNaskhArabic-Regular.ttf"));
    //myFilesMinor.add(stCString("NotoNaskhArabic-Bold.ttf"));
#else

#if !defined(__EMSCRIPTEN__)
    if(FcConfig* aFcCfg = FcInitLoadConfig()) {
        if(FcStrList* aFcFontDir = FcConfigGetFontDirs(aFcCfg)) {
            for(;;) {
                FcChar8* aFcFolder = FcStrListNext(aFcFontDir);
                if(aFcFolder == NULL) {
                    break;
                }

                myFolders.add(StString((const char* )aFcFolder));
            }
            FcStrListDone(aFcFontDir);
        }
        FcConfigDestroy(aFcCfg);
    }
#endif
    if(myFolders.isEmpty()) {
        myFolders.add(stCString("/usr/share/fonts"));
        myFolders.add(stCString("/usr/local/share/fonts"));
    }

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
    myFilesMinor.add(stCString("NotoSerifCJK-Regular.ttc"));

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

bool StFTFontRegistry::registerFamily(const StString& theFontPath, int theFaceId) {
    const FT_Long aFaceId = theFaceId != -1 ? theFaceId : 0;
    FT_Face aFace = NULL;
    if(FT_New_Face(myFTLib->getInstance(), theFontPath.toCString(), aFaceId, &aFace) != 0) {
        if(aFace != NULL) {
            FT_Done_Face(aFace);
        }
        return false;
    }
    if(aFace->family_name == NULL // skip broken fonts (error in FreeType?)
    || FT_Select_Charmap(aFace, ft_encoding_unicode) != 0) { // handle only UNICODE fonts
        FT_Done_Face(aFace);
        return false;
    }

    // generate font family name
    StString aStyle(aFace->style_name != NULL ? aFace->style_name : "");
    {
        // remove standard style combinations reflected by style_flags
        // and keep only extra styles like Light, Condensed and similar
        static const StString THE_EMPTY;
        static const StString THE_SPACE1(" ");
        static const StString THE_SPACE2("  ");
        static const StString THE_ITALIC1("Italic");
        static const StString THE_ITALIC2("Oblique");
        static const StString THE_BOLD("Bold");
        static const StString THE_REGULAR1("Regular");
        static const StString THE_REGULAR2("Book");
        if(aFace->style_flags == (FT_STYLE_FLAG_ITALIC | FT_STYLE_FLAG_BOLD)) {
            aStyle = aStyle.replace(THE_BOLD, THE_EMPTY);
            const size_t aLen = aStyle.Length;
            aStyle = aStyle.replace(THE_ITALIC1, THE_EMPTY);
            if(aLen == aStyle.Length) {
                aStyle = aStyle.replace(THE_ITALIC2, THE_EMPTY);
            }
        } else if(aFace->style_flags == FT_STYLE_FLAG_BOLD) {
            aStyle = aStyle.replace(THE_BOLD, THE_EMPTY);
        } else if(aFace->style_flags == FT_STYLE_FLAG_ITALIC) {
            const size_t aLen = aStyle.Length;
            aStyle = aStyle.replace(THE_ITALIC1, THE_EMPTY);
            if(aLen == aStyle.Length) {
                aStyle = aStyle.replace(THE_ITALIC2, THE_EMPTY);
            }
        }
        const size_t aLen2 = aStyle.Length;
        aStyle = aStyle.replace(THE_REGULAR1, THE_EMPTY);
        if(aLen2 == aStyle.Length) {
            aStyle = aStyle.replace(THE_REGULAR2, THE_EMPTY);
        }
        aStyle.leftAdjust();
        aStyle.rightAdjust();
        aStyle.replace(THE_SPACE2, THE_SPACE1);
    }

    StString aFamilyName = aFace->family_name;
    if(!aStyle.isEmpty()) {
        aFamilyName = aFamilyName + " " + aStyle;
    }

    StFTFontFamily& aFamily = myFonts[aFamilyName];
    aFamily.FamilyName = aFamilyName;
    if(aFace->style_flags == (FT_STYLE_FLAG_ITALIC | FT_STYLE_FLAG_BOLD)) {
        aFamily.BoldItalic = theFontPath;
        aFamily.BoldItalicFace = (int )aFaceId;
    } else if(aFace->style_flags == FT_STYLE_FLAG_BOLD) {
        aFamily.Bold = theFontPath;
        aFamily.BoldFace = (int )aFaceId;
    } else if(aFace->style_flags == FT_STYLE_FLAG_ITALIC) {
        aFamily.Italic = theFontPath;
        aFamily.ItalicFace = (int )aFaceId;
    } else {
        aFamily.Regular = theFontPath;
        aFamily.RegularFace = (int )aFaceId;
    }
    //ST_DEBUG_LOG("StFTFontRegistry, font file '" + theFontPath + " [" + aFaceId + "]" + "', family '" + aFamily.FamilyName + "', contains " + aFace->num_glyphs + " glyphs!");

    if(theFaceId < aFace->num_faces) {
        const FT_Long aNbInstances = aFace->style_flags >> 16;
        for(FT_Long anInstIter = 1; anInstIter < aNbInstances; ++anInstIter) {
            const FT_Long aSubFaceId = aFaceId + (anInstIter << 16);
            registerFamily(theFontPath, aSubFaceId);
        }
    }
    if(theFaceId == -1) {
        for(FT_Long aFaceIter = 1; aFaceIter < aFace->num_faces; ++aFaceIter) {
            registerFamily(theFontPath, aFaceIter);
        }
    }
    FT_Done_Face(aFace);
    return true;
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

        if(!registerFamily(aPath, -1)) {
            if(theIsMajor) {
                ST_ERROR_LOG("StFTFontRegistry, major font file '" + aName + "' fail to load"
                            + " from path '" + aPath + "'!");
            }
            continue;
        }
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
    aSans.MiscSymbols = aSerif.Western; // Trebuchet font does not include this sub-set
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
    const StFTFontFamily& aSongtiSC = findFont(stCString("Songti SC Light"));
    const StFTFontFamily& aFang = findFont(stCString("STFangsong"));
    const StFTFontFamily& aCjk = aSongtiSC.FamilyName.isEmpty() && !aFang.FamilyName.isEmpty()
                               ? aFang
                               : aSongtiSC;
    aSerif.CJK     = aCjk;
    aSans .CJK     = aCjk;
    aMono .CJK     = aCjk;
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
    aSans .Korean  = aSerif.Korean;
    aMono .Korean  = aSerif.Korean;
    if(aSerif.Korean.FamilyName.isEmpty()) {
        aSerif.Korean = findFont(stCString("Noto Sans KR"));
        aSans .Korean = aSerif.Korean;
        aMono .Korean = aSerif.Korean;
    }
    aSerif.CJK     = findFont(stCString("Droid Sans Fallback"));
    aSans .CJK     = aSerif.CJK;
    aMono .CJK     = aSerif.CJK;
    if(aSerif.CJK.FamilyName.isEmpty()) {
        aSerif.CJK = findFont(stCString("Noto Sans SC"));
        aSans .CJK = aSerif.CJK;
        aMono .CJK = aSerif.CJK;
    }
    if(aSerif.CJK.FamilyName.isEmpty()) {
        aSerif.CJK = findFont(stCString("Noto Sans CJK JP"));
        aSans .CJK = aSerif.CJK;
        aMono .CJK = aSerif.CJK;
    }
    if(aSerif.Korean.FamilyName.isEmpty()) {
        aSerif.Korean = aSerif.CJK;
        aSans .Korean = aSerif.CJK;
        aMono .Korean = aSerif.CJK;
    }

    aSerif.Arabic = findFont(stCString("Droid Arabic Naskh"));
    aSans .Arabic = aSerif.Arabic;
    aMono .Arabic = aSerif.Arabic;
    if(aSerif.Arabic.FamilyName.isEmpty()) {
        aSerif.Arabic = findFont(stCString("Noto Naskh Arabic"));
        aSans .Arabic = aSerif.Arabic;
        aMono .Arabic = aSerif.Arabic;
    }
#else
    aSerif.Western = findFont(stCString("FreeSerif"));
    aSans .Western = findFont(stCString("FreeSans"));
    if(aSerif.Western.FamilyName.isEmpty()) {
        aSerif.Western = findFont(stCString("DejaVu Serif"));
        aSans .Western = findFont(stCString("DejaVu Sans"));
    }
    aMono .Western = findFont(stCString("DejaVu Sans Mono"));

    const StFTFontFamily& aNanumMyeon     = findFont(stCString("NanumMyeongjo"));
    const StFTFontFamily& aNotoSerifCjkJp = findFont(stCString("Noto Serif CJK JP"));
    const StFTFontFamily& aNanumGoth      = findFont(stCString("NanumGothic"));
    aSerif.Korean  = aNanumMyeon.FamilyName.isEmpty() && !aNotoSerifCjkJp.FamilyName.isEmpty()
                   ? aNotoSerifCjkJp
                   : aNanumMyeon;;
    aSans .Korean  = aNanumGoth.FamilyName.isEmpty() && !aNotoSerifCjkJp.FamilyName.isEmpty()
                   ? aNotoSerifCjkJp
                   : aNanumGoth;
    aMono .Korean  = aSans.Korean;
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
