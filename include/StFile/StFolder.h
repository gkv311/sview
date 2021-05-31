/**
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StFolder_h__
#define __StFolder_h__

#include <StFile/StFileNode.h>

class StFolder : public StFileNode {

        public:

    ST_CPPEXPORT static bool isFolder(const StCString& thePath);

    /**
     * Tries to create folder.
     * @return true on success.
     */
    ST_CPPEXPORT static bool createFolder(const StCString& thePath);

        public:

    /**
     * Override implementation to return TRUE.
     */
    ST_CPPEXPORT virtual bool isFolder() const ST_ATTR_OVERRIDE;

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StFolder();

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StFolder(const StCString& theFolderPath,
                          StNode*          theParentNode = NULL);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StFolder();

    /**
     * Read files list in this folder.
     * @param theExtensions Extensions filter
     * @param theDeep       Recursion level to read subfolders
     */
    ST_CPPEXPORT void init(const StArrayList<StString>& theExtensions,
                           const int                    theDeep = 1,
                           const bool                   theToAddEmptyFolders = false);

        private:

    ST_LOCAL void addItem(const StArrayList<StString>& theExtensions,
                          int             theDeep,
                          const StString& theSearchFolderPath,
                          const StString& theCurrentItemName,
                          const bool      theToAddEmptyFolders);

};

#endif //__StFolder_h__
