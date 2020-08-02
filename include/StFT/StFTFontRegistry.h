/**
 * Copyright © 2013-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StFTFontRegistry_h_
#define __StFTFontRegistry_h_

#include <StFT/StFTFont.h>
#include <StFile/StFolder.h>

#include <map>

/**
 * Class to manage the list of available fonts in the system.
 * Unlike font management classes this one does not share access to font instances,
 * but only the list to the font files.
 */
class StFTFontRegistry {

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StFTFontRegistry();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StFTFontRegistry();

    /**
     * Initialize the fonts list.
     * @param theToSearchAll flag to register ALL font files within specified search folders (slower)
     */
    ST_CPPEXPORT void init(const bool theToSearchAll = false);

    /**
     * Append folder to the list of search paths.
     */
    ST_CPPEXPORT void appendSearchPath(const StString& theFolder);

    /**
     * Search the font.
     */
    ST_CPPEXPORT const StFTFontFamily& findFont(const StCString& theFamily);

    /**
     * @return fonts pack for specified typeface
     */
    ST_LOCAL const StFTFontPack& getTypeface(const StFTFont::Typeface theTypeface) const {
        return myTypefaces[theTypeface];
    }

        private:

    /**
     * Search the specified font files.
     */
    void searchFiles(const StArrayList<StString>& theNames,
                     const bool                   theIsMajor);

    /**
     * Process font file.
     */
    bool registerFamily(const StString& theFontPath, int theFaceId);

        private:

    StArrayList<StString> myExtensions;  //!< list of supported font file extensions
    StArrayList<StString> myFolders;     //!< font search paths
    StArrayList<StString> myFilesMajor;  //!< major font file names which should present in the system
    StArrayList<StString> myFilesMinor;  //!< minor font file names

    StFolder              myFoldersRoot; //!< files tree
    StHandle<StFTLibrary> myFTLib;       //!< handle to the FT library object

    std::map<StString, StFTFontFamily> myFonts; //!< map family name -> font files

    StFTFontPack myTypefaces[StFTFont::TypefacesNB]; //!< predefined typefaces

};

#endif // __StFTFontRegistry_h_
