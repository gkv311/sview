/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StMIMEList_h__
#define __StMIMEList_h__

#include "StMIME.h"

class StMIMEList : public StArrayList<StMIME> {

        public:

    // inherited constructors
    StMIMEList(size_t initialSize = 8) : StArrayList<StMIME>(initialSize) {}
    StMIMEList(const StMIMEList& toCopy) : StArrayList<StMIME>(toCopy) {}
    ~StMIMEList() {}

    /**
     * Create MIME list from formatted string (like this: 'image/jpg:jpg:info;image/png:png:info').
     * @param theString (const StString& ) - formatted string.
     */
    StMIMEList(const StString& theString) : StArrayList<StMIME>() {
        StHandle< StArrayList<StString> > aSplitList = theString.split(';');
        for(size_t index = 0; index < aSplitList->size(); ++index) {
            StArrayList<StMIME>::add(StMIME(aSplitList->getValue(index)));
        }
    }

    /**
     * Creates list of extensions from MIME description list.
     * @return extensions list (StArrayList<StString> ).
     */
    StArrayList<StString> getExtensionsList() const {
        StArrayList<StString> anExtensionsList;
        size_t anExtId = 0;
        for(size_t aMimeId = 0; aMimeId < StArrayList<StMIME>::size(); ++aMimeId) {
            StString anExt = StArrayList<StMIME>::getValue(aMimeId).getExtension();
            for(anExtId = 0; anExtId < anExtensionsList.size(); ++anExtId) {
                if(anExt.isEqualsIgnoreCase(anExtensionsList[anExtId])) {
                    // prevent duplicates
                    break;
                }
            }
            if(anExtId == anExtensionsList.size()) {
                anExtensionsList.add(anExt);
            }
        }
        return anExtensionsList;
    }

    /**
     * Verify the filename extension is in supported list.
     */
    bool checkExtension(const StString& theExt) const {
        for(size_t aMimeId = 0; aMimeId < StArrayList<StMIME>::size(); ++aMimeId) {
            const StString& anExt = StArrayList<StMIME>::getValue(aMimeId).getExtension();
            if(anExt.isEqualsIgnoreCase(theExt)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns toString, splitter argument should not be changed!
     * @param theSplitter (const StString& ) - splitter char should not be changed!
     * @return string (StString ).
     */
    virtual StString toString(const StString& theSplitter = StString(';')) const {
        return StArrayList<StMIME>::toString(theSplitter);
    }

};

#endif //__StMIMEList_h__
