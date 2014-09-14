/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StFolder_h__
#define __StFolder_h__

#include <StFile/StFileNode.h>

class StFolder : public StFileNode {

        public:

    /**
     * Override implementation to return TRUE.
     */
    ST_CPPEXPORT virtual bool isFolder() const;

    ST_CPPEXPORT static bool isFolder(const StCString& thePath);

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
