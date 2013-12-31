/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
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

#ifdef _WIN32
    myFolders.add(StProcess::getWindowsFolder() + "fonts");

    // western
    myFilesMajor.add(stCString("times.ttf"));
    myFilesMajor.add(stCString("timesbd.ttf"));
    myFilesMajor.add(stCString("timesi.ttf"));
    myFilesMajor.add(stCString("timesbi.ttf"));
    myFilesMajor.add(stCString("micross.ttf"));
    myFilesMajor.add(stCString("tahoma.ttf"));
    myFilesMajor.add(stCString("tahomabd.ttf"));
    myFilesMajor.add(stCString("trebuc.ttf"));
    myFilesMajor.add(stCString("trebucbd.ttf"));
    myFilesMajor.add(stCString("trebucit.ttf"));
    myFilesMajor.add(stCString("trebucbi.ttf"));
    // korean
    myFilesMajor.add(stCString("malgun.ttf"));
    myFilesMajor.add(stCString("malgunbd.ttf"));
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
    myFilesMajor.add(stCString("Tahoma.ttf"));
    myFilesMajor.add(stCString("Tahoma Bold.ttf"));
    myFilesMajor.add(stCString("Trebuchet MS.ttf"));
    myFilesMajor.add(stCString("Trebuchet MS Bold.ttf"));
    myFilesMajor.add(stCString("Trebuchet MS Italic.ttf"));
    myFilesMajor.add(stCString("Trebuchet MS Bold Italic.ttf"));
    myFilesMajor.add(stCString("Monaco.dfont"));
    // korean
    myFilesMajor.add(stCString("AppleMyungjo.ttf"));
    myFilesMajor.add(stCString("AppleGothic.ttf"));
    // chinese
    myFilesMajor.add(stCString("Hei.ttf"));
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
                ST_ERROR_LOG("StFTFontRegistry, major font file '" + aName + "' fail to load!");
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
    aSans .Western = findFont(stCString("Microsoft Sans Serif"));
    aMono .Western = findFont(stCString("Tahoma"));
    aSerif.Korean  = findFont(stCString("Malgun Gothic"));
    aSans .Korean  = findFont(stCString("Malgun Gothic"));
    aMono .Korean  = findFont(stCString("Malgun Gothic"));
    aSerif.CJK     = findFont(stCString("SimSun"));
    aSans .CJK     = findFont(stCString("SimSun"));
    aMono .CJK     = findFont(stCString("SimSun"));
#elif defined(__APPLE__)
    aSerif.Western = findFont(stCString("Times New Roman"));
    aSans .Western = findFont(stCString("Trebuchet MS"));
    aMono .Western = findFont(stCString("Monaco"));
    aSerif.Korean  = findFont(stCString("AppleMyungjo"));
    aSans .Korean  = findFont(stCString("AppleGothic"));
    aMono .Korean  = findFont(stCString("AppleGothic"));
    aSerif.CJK     = findFont(stCString("Hei"));
    aSans .CJK     = findFont(stCString("Hei"));
    aMono .CJK     = findFont(stCString("Hei"));
#else
    aSerif.Western = findFont(stCString("DejaVu Serif"));
    aSans .Western = findFont(stCString("DejaVu Sans"));
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
    std::unordered_map<std::string, StFTFontFamily>::iterator anIter = myFonts.find(theFamily.toCString());
    return anIter != myFonts.end()
         ? anIter->second
         : THE_NO_FAMILY;
}
