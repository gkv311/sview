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

        private:

    ST_CPPEXPORT void addItem(const StArrayList<StString>& theExtensions,
                              int theDeep,
                              const StString& theSearchFolderPath,
                              const StString& theCurrentItemName);

        public:

    ST_CPPEXPORT virtual bool isFolder() const;

    ST_CPPEXPORT static bool isFolder(const StString& thePath);

    ST_CPPEXPORT StFolder(const StString& theFolderPath = StString(), StNode* theParentNode = NULL);
    ST_CPPEXPORT virtual ~StFolder();
    ST_CPPEXPORT void init(const StArrayList<StString>& theExtensions, int theDeep = 1);

};

#endif //__StFolder_h__
