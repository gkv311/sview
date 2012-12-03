/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StFolder_h__
#define __StFolder_h__

#include <StFile/StFileNode.h>

class ST_LOCAL StFolder : public StFileNode {

        private:

    void addItem(const StArrayList<StString>& theExtensions,
                 int theDeep,
                 const StString& theSearchFolderPath,
                 const StString& theCurrentItemName);

        public:

    virtual bool isFolder() const {
        return true;
    }

    static bool isFolder(const StString& thePath);

    StFolder(const StString& theFolderPath = StString(), StNode* theParentNode = NULL);
    virtual ~StFolder();
    void init(const StArrayList<StString>& theExtensions, int theDeep = 1);

};

#endif //__StFolder_h__
